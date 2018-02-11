# MIMEGUARD

This is a utility that attempts to provide a 'firewall' for mail files. It allows the user to create policies governing which files are allowed within mails. It can also analyze OLE documents for macros, PDF documents for javascript or embedded files, and RTF documents for embedded files. I created it after seeing too many virus-checkers allow files ending in extensions like .exe, .js, .bat, and so on, or word documents containing macros, or whatever.

Mimeguard is intended to process email files, returning 'true' (exit code 0) if the file passes all checks, and 'false' (some non-zero exit code) if it fails a check. It's designed to be run from a script that takes actions on the basis of these exit codes. It also prints out a report of which files it passed/failed as it runs.

I have been using mimeguard and getting good results with it for some time now, but it still has some missing features. Please read the section 'WEAKNESSES AND TODO' before using this software.

Mimeguard and libUseful are (C) 2017 Colum Paget. They are released under the GPL so you may do anything with them that the GPL allows. 

Mimeguard is free software and comes with no warranties or guarentees of any kind.


# INSTALL

The usual proceedure:

```
./configure --enable-zlib
make
make install
```

Should work. The 'make install' stage will have to be done as root. The default install paths will put the mimeguard exectuable in /usr/local/bin and the default config file /usr/local/etc. This can be changed by passing `--prefix=<path>` to configure. If, like most people, you want to put mimeguard in /usr/bin but the config file in /etc then use `--prefix=/usr --sysconfdir=/etc`

Please supply '--enable-zlib' if your system has this library, as this will enable mimeguard to look deeper into PDF documents.



# INVOCATION

```
mimeguard <options> <paths>
```

Options are:
```
  --help          Print this help
  -help           Print this help
  -?              Print this help
  --version       Print program version
  -version        Print program version
  -c <path>       Path to config file
  -d              Print debugging
  -x              Dump/export/unpack file contents
  -safe           Only show safe files
  -evil           Only show unsafe/evil files
  -show <email header> Show Email Header
	-smtp           Run in SMTP proxy mode
```

When run on a console mimeguard will print out a color-coded breakdown of files/mime-types, highlighting any that it considers safe/not safe. This allows easy testing of changes made to your configuration file. When run with the `-smtp` flag mimeguard will run as an SMTP server/proxy, allowing mails to be sent to it for analysis and sorting (see SMTP MODE below).


# CONFIGURATION

Mimeguard is configured using a single config file 'mimeguard.conf'. An example config file is provided at the end of this document. The following entries can occur in the config file

**MagicsFile <path>**   
**MagicsFiles <path>**   
Path to the 'magics' file that's used to identify filetypes. (This file is part of the apache http server distribution).  `<path>` can be a comma-separated list of paths, see '#CONFIG PATHS' for more details.

**MimeTypesFile <path>**   
**MimeTypesFiles <path>**   
Path to the 'mime.types' file that's used to identify filetypes from file extensions. (This file is part of the apache http server distribution).  `<path>` can be a comma-separated list of paths, see '#CONFIG PATHS' for more details.

**RegionFile <path>**   
**RegionFiles <path>**   
Path to IP registrar files. These are files containing lists of IP ranges and the countries they are assigned to. They are used by the 'region' URL rule to look up the region for a given IP address.  `<path>` can be a comma-separated list of paths, see '#CONFIG PATHS' for more details.

When a URL rule is called all the matching files will be checked for an entry for the IP address. Unfortunately this currently only works for IPv4, IPv6 lookup is planned in a future release.

Currently region files can be found at:

http://ftp.lacnic.net/pub/stats/lacnic/delegated-lacnic-latest","delegated-lacnic-latest
http://ftp.ripe.net/ripe/stats/delegated-ripencc-latest","delegated-ripencc-latest
http://ftp.arin.net/pub/stats/arin/delegated-arin-extended-latest","delegated-arin-extended-latest
http://ftp.afrinic.net/pub/stats/afrinic/delegated-afrinic-latest","delegated-afrinic-latest
http://ftp.apnic.net/stats/apnic/delegated-apnic-latest","delegated-apnic-latest

**URLRule <result> <type> <arg>**
A rule defining behavior to take on a URL being discovered in an HTML file. `<type>` defines the type of checking to take place. `<result>` can have the values 'safe' and 'evil'. The `<arg>` value is dependant on the type of checking.

available `<type>` values are: 

   * ip       `<arg>` is a comma separated list of ip addresses (fnmatch style wildcards allowed)
   * host     `<arg>` is a comma seperated list of hostnames (fnmatch style wildcards allowed)
   * iplist   `<arg>` is a path to a file containing a list of IP addresses
   * hostlist `<arg>` is a path to a file containing a list of hostnames
   * region   `<arg>` is either a registrar name (arin, ripencc, afrinic, apnic, lacnic) or a country code (GB, DE, US, RU, CN, etc).

URLRules are processed in order. If no matching rule is found then the url is taken to be 'safe'. If you want a default 'evil' rule then start your rules with:

```
urlrule evil host *
```

An example set of urlrules:

```
urlrule iplist /etc/netspork/badips.blocklist evil
urlrule region ripencc evil
urlrule region apnic evil
urlrule region DE safe
```

This would check the IP Address associated with the URL against a list of IP addresses in the file `/etc/netspork/badips.blocklist`. If found in the file the url would be considered 'evil'. Then region rules are checked. If the IP associated with the URL is found to be in the regions covered by the apnic or ripencc registrars (asia pacific and europe) then it will be considered evil UNLESS it's Germany (country code DE) which is considered safe. Note that in this config german IPs are considered safe even if in the blocklist file. This is because later rules override earlier ones. If you want all ips in the blocklist file to be considered evil, even if they're in Germany, then you want this config:

```
urlrule region ripencc evil
urlrule region apnic evil
urlrule region DE safe
urlrule iplist /etc/netspork/badips.blocklist evil
```

**Extn <mime type> <extn> ...**
Binds a list of file extensions to a particular mime-type, overriding information from the apache mime.types file.

**FileType <mime type> <commands>**
This config does most of the work. The 'mime type' argument is an fnmatch style pattern that matches against a mime type. For a given pattern one can supply the arguments 'safe', 'evil', 'container', 'contains', and 'equiv'. 'safe' and 'evil' are equivalent to 'ACCEPT' and 'REJECT' in an iptables firewall. mimeguard will return '0' (which is 'true' in bash scripts) if a file and all its contained files are declared 'safe'. If will return various non-zero values (all of which count as 'false') if the file, or any of it's contents, match against an 'evil' rule. For example:

```
FileType * evil
FileType text/* safe
```

With this config mimeguard will only return true for files whose mime-type matches 'text/*', such as text/plain, text/html and text/csv. Anything that doesn't match this will be marked as 'evil' by the previous rule, and mimeguard will return false.

One gotcha to look out for here is that modern emails usually contain many subdocuments wrapped up in a 'multipart/mixed' mime type. Thus we need a config that looks like: 

```
FileType * evil
FileType text/* safe
FileType multipart/mixed container safe
```

This not only declares the multipart/mixed mimetype to be safe, but also to be a container. Mimeguard will thus investigate files within this mimetype. If any of these subfiles match 'evil' then mimeguard will return false.

However, in addition to 'multipart/mixed' there is also 'multipart/related' which is pretty much the same thing. So now we have a rule:

```
FileType * evil
FileType text/* safe
FileType multipart/* container safe
```

But often emails also use a 'multipart/alternative' mime-type to provide alternative text for mail clients that do/don't support HTML. This isn't a container in the sense of having files within it, it's just a block of text. However, if all 'multipart' document types are declared as containers mimeguard will fail 'multipart/alternative' as an empty container. To handle this we have:

```
FileType * evil
FileType text/* safe
FileType multipart/* container safe
FileType multipart/alternative safe
```

Later rules override previous ones, so all multipart documents will be treated as containers except multipart/alternative.

Finally there are a couple of other benign mime-types that can appear in emails:

```
FileType * evil
FileType text/* safe
FileType multipart/* container safe
FileType multipart/alternative safe
FileType message/rfc822 safe
FileType application/ms-tnef safe
```

With this set of rules mimeguard will only return 'true' if an email file contains documents with 'text/*' mimetypes.

The 'container' argument allows specifying that a mime-type is a container for other documents. If you want to constrain which mime-types can appear within a container you can use the 'contains' argument, like this:

```
FileType application/vnd.openxmlformats-officedocument.wordprocessingml.document safe contains=application/xml,image/*,application/x-msmetafile
```

This specifies that this mimetype (which is the mimetype for ms-office docx files) is a container that can contain image files, xml, and ms metafiles. Unfortunately mimeguard has one problem with this container type: it doesn't know how to unpack it. Mimeguard only natively knows how to deal with multipart/mixed and zip containers. Fortunately the docx file is actually a type of pk-zip file, and we tell mimeguard this by using the 'equiv' keyword:

```
FileType application/vnd.openxmlformats-officedocument.wordprocessingml.document safe contains=application/xml,image/*,application/x-msmetafile equiv=application/zip

FileType application/zip safe container
```

The 'equiv' keyword forwards processing to the 'FileType application/zip' configuration. Notice that we've also had to add a line saying that 'application/zip' is a safe format and a container. The 'container' keyword triggers mimeguard's container evaluation, which knows how to look inside zip documents. 

Finally, we can replace 'container' with 'contains' if we want to constrain what can appear in a zip file, like this:

```
FileType application/zip safe contains=*,!application/zip,!application/msword
```

The '!' before the mime-type in the 'contains' statement says that these mimetypes may not occur in the file. Because we are specifying that these may not occur, we have to include the '*' mimetype to say that anything other than these can occur in the container. Thus this line specifies that .zip files may not contain other zip files.

**String <mime type> <string> ...**
The 'String' config allows us to specify a list of strings that may *not* occur in files of a particular mime-type. Currently this option only works for .rtf and .pdf files. With this option one can outlaw certain commands in those files. For example:

```
String application/pdf /JS /JavaScript /Launch /EmbeddedFile /ASCII85Decode /OpenAction
String application/rtf \\objocx pFragments
```

**Header <header type> <header argument> <config>**
The 'Header' config can be used to match against an email header, and trigger a config option that overrides previously set options. The `<header type>` argument is an email header type like "To", "From" or "Subject". The `<header argument>` option is an fnmatch style pattern that matches against the variable part of the header. The config is a 'FileType' or 'String' command. For example:

```
Header From *@microsoft.com FileType applicationa/x-ms* safe
```

This allows different configs for different email senders/recipients.

# CONFIG PATHS

The MagicsFile, MimeTypesFile and RegionFile configuration entries all take a 'path' argument. This 'path' can actually be a comma-separated list of paths, and it can include fnmatch-style wildcards. For example:

```
MagicsFile /etc/magic,/usr/local/etc/magic
RegionFiles /etc/ip-regions/*,/usr/local/etc/ip-regions/*
```

Each entry in a path list can also have a 'protocol' prefix. Currently the protocol can only have the value 'mmap:'. If a path has an 'mmap' prefix then the file will be mapped into shared memory. This allows many applications using this file to share a copy of it in memory, granting them faster access as they don't need to load it from disk. However, for this to work some application has to keep the file permanently mapped into memory.


# SMTP MODE

Mimeguard can act as an SMTP server to receive mails for processing. This is activated by the '-smtp' command-line option. In this mode a number of extra config-file options become activated. 

```
SmtpPassDir - directory to put 'safe' mails into
SmtpFailDir - directory to put 'evil' mails into
SmtpPassServer - mailserver to send 'safe' mails to
SmtpFailServer - mailserver to send 'evil' mails to
SmtpFailRedirect - email address to send 'evil' mails to
```

SmtpPassDir and SmtpFailDir specify two directories that mail is sorted into, depending on whether it is considered 'safe' (pass) or 'evil' (fail). If no SmtpPassServer or SmtpFailServer is specified then these mails just sit in these directories, perhaps to be processed by some other program. However, if SmtpPassServer, and/or SmtpFailServer are specified, then the mails are sent to those servers (and deleted from the directories as they are sent). 

Finally SmtpFailRedirect allows an email address to be specified that 'evil' mails will be sent to. An 'SmtpFailServer' still needs to be specified for this to work, but instead of mail being sent to the original recipient, it will be sent to the email address specified in this option. This allows using the same server for both pass and fail, but sending all failed (evil) mails to a specific email address.

Smtp servers to send fail and pass mail to can be specified with any of the following forms:

```
<host>
<host>:port
<user>:<password>@host
<user>:<password>@host:port
```

The `<user>` and `<password>` values here are credentials to be used on SMTP servers that require authentication.


# MACROS

By default mimeguard will interrogate items of mime-type 'application/x-ole-storage' for macros. If the object contains an internal directory entry called 'Macros' or 'VBA' then it will be considered 'evil'. If you want to allow macros (say you're using 'Header From' to allow a specific user to send files containing macros) then you can use the 'allow-macros' argument, like this:

```
Header From *@microsoft.com FileType applicationa/x-ms* safe allow-macros
```

# ENCRYPTED FILES

MS Office documents can be encrypted. By default mimeguard treats these as 'evil' as it's a commonly used method to slip by anti-virus. If you wish to allow these files use 'allow-encrypted' like so:

```
Header From *@microsoft.com FileType applicationa/x-ms* safe allow-encrypted
```

# HTML FILES

Html files are a bit of a special case. They can be containers, because links within the file can point to a remote document, effectively 'containing' that document. However, mimeguard normally throws an error if a container is empty, but HTML can be used purely for formatting, without any links. This means that for HTML documents you need a config line like:

```
FileType text/html safe container allow-empty
```


# 'STRINGS' for HTML
Mimeguard can examine tags used in HTML documents. Normally you would use th following command to rule out strings that shouldn't be in an email. HTML in an email should just be formatting, it shouldn't contain any scripting or embedded objects. Both 'link' and 'iframe' can be used to drag more HTML into an HTML document from a remote source, so these should really be ruled out to as otherwise a malicious link could be put in such a remote document to get it past mimeguard.

```
String text/html script object embed applet link iframe
```

# 'STRINGS' for PDF

Mimeguard can examine commands used in PDF documents. Please supply the --enable-zlib configure option during the build if your system has zlib, as this will allow much better processing of PDF documents. 

Almost all malicious PDF documents use javascript. Thus denying the strings /JS and /JavaScript will catch a lot of malicious docs. Generally macros in PDF are launched using the /OpenAction event that's run when the document is opened. So the config:

```
string application/pdf /JS /JavaScript /OpenAction
```

Will already catch the great majority of malicious PDFs.

Other PDF commands of interest are:

  * /Action         allows binding code to various document events
  * /AA             'Additional Action', allows binding code to various document events
	* /Launch         Launches a program from within the document!
  * /URI            Accesses a resource at a URL (meaning you can pull code from the internet)
  * /RichMedia      Embed shockwave flash
  * /AcroForm       Can call javacript 
	* /EmbeddedFile   Can embed other file types in a PDF
	* /ASCII85Decode  Can conceal code in ASCII85 encoding (mimeguard doesn't unpack this yet)

However, forbidding all of thse will likely result in jailing mails that you want to pass. Some trial and error is required to establish the best config for you.


# 'STRINGS' for RTF

I have not been able to obtain many malicious .rtf files for testing. The main strings I know of that indicate a malicious rtf are:

	* \objocx     embed an OLE/ActiveX object in the document
	* pFragments  reassemble commands/data from fragments, used for obfuscation of malicious code

This gives a 'String' line of:

```
String application/rtf \\objocx pFragments
```

Notice the use of a double backslash before objocx. This is needed because backslash on it's own is used as an escaping character. With a single backslash '\objocx' would reduce to simply 'objocx', which would likely still work, but might result in some false positives as we're not matching the full string. pFragments has no leading '\' because it's an argument to another command.


# WEAKNESSES AND TODO

This is the initial release of mimeguard, and it has some failings. These are things I plan to fix at some future date:

   * Mimeguard doesn't fully unpack .zip files, it just looks one level into them. Thus you should disallow zips within zips
   * Mimeguard doesn't yet check for macros within Office 97 documents within a zip
   * Mimeguard doesn't have support for .rar or .ace container files
   * Mimeguard doesn't unpack ASCII85 encoded data within .pdf files.


# EXAMPLE CONFIG FILE

```
# Include standard 'magic' and 'mime.types' files for identifying files
MagicsFile /etc/magic
MimeTypesFile /etc/mime.types

#you will need to uncomment this and provide these files to use 'region' url rules
#RegionFiles /etc/ip-regions/*

urlrule iplist /etc/myblocklist.lst evil

#in this example these are places we'd never expect to receive mail from. You will need to uncomment 'RegionFiles' and supply
#IP-Registrar files
urlrule region afrinic evil
urlrule region apnic evil
urlrule region ru evil

# some file extension to mime-type mappings that might not be in /etc/mime.types
Extn application/script scr ws wsf wsc wsh cmd
Extn application/powershell ps1 ps1xml ps2 ps2xml psc1 psc2 msh msh1 msh2 mshxml msh1xml msh2xml
Extn windows/shortcut scf lnk inf reg

# by default declare all mime-types as evil
FileType * evil

# now start listing mime-types that are safe
FileType message/rfc822 safe
FileType application/ms-tnef safe
FileType multipart/* safe container
FileType multipart/alternative safe
FileType */xml safe
FileType */csv safe
FileType text/* safe
FileType audio/* safe
FileType video/* safe

### must define 'html' as a container so that links within an html doc get examined as files
FileType text/html container safe allow-empty

## xls doc ppt, these are all of type 'application/x-ole-storage'. Mimeguard recognizes that mimetype internally
## and does further checks on it
FileType office/macros safe  equiv=application/x-ole-storage
FileType application/msword safe  equiv=application/x-ole-storage
FileType application/vnd.ms-excel safe  equiv=application/x-ole-storage
FileType application/vnd.ms-project safe  equiv=application/x-ole-storage


## xlsx docx pptx. these map to 'application/zip' which is also internally handled by Mimeguard 
FileType application/vnd.openxmlformats-officedocument.wordprocessingml.document safe  contains=application/xml,image/*,application/x-msmetafile,application/binary,application/vml equiv=application/zip
FileType application/vnd.openxmlformats-officedocument.spreadsheetml.sheet safe  contains=application/xml,image/*,application/x-msmetafile,application/binary,application/vml equiv=application/zip,application/x-zip
FileType application/vnd.openxmlformats-officedocument.presentationml.presentation safe  contains=application/xml,image/*,application/x-msmetafile,application/binary,application/vml equiv=application/zip

## zip files. mustn't contain a zip file or an msword file (as that's a standard malware trick)
FileType application/zip safe contains=*,!application/zip,!application/msword

## Naughty strings in pdf, rtf and html documents
String application/pdf /JS /JavaScript /OpenAction /AA /Launch /EmbeddedFile /ASCII85Decode
String application/rtf \\objocx pFragments
String text/html script object embed link applet iframe

```


