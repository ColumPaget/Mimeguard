#MIMEGUARD

This is a utility that attempts to provide a 'firewall' for mail files. It allows the user to create policies governing which files are allowed within mails. It can also analyze OLE documents for macros, PDF documents for javascript or embedded files, and RTF documents for embedded files. I created it after seeing too many virus-checkers allow files ending in extensions like .exe, .js, .bat, and so on, or word documents containing macros, or whatever.

Mimeguard is intended to process email files, returning 'true' (exit code 0) if the file passes all checks, and 'false' (some non-zero exit code) if it fails a check. It's designed to be run from a script that takes actions on the basis of these exit codes. It also prints out a report of which files it passed/failed as it runs.

Mimeguard and libUseful are (C) 2017 Colum Paget. They are released under the GPL so you may do anything with them that the GPL allows.

#INSTALL

The usual proceedure:

```
./configure
make
make install
```

Should work. The 'make install' stage will have to be done as root. The default install paths will put the mimeguard exectuable in /usr/local/bin and the default config file /usr/local/etc. This can be changed by passing --prefix=<path> to configure.



#CONFIGURATION

Mimeguard is configured using a single config file 'mimeguard.conf'. An example config file is provided at the end of this document. The following entries can occur in the config file

**MagicsFile <path>**
Path to the 'magics' file that's used to identify filetypes. (This file is part of the apache http server distribution).

**MimeTypesFile <path>**
Path to the 'mime.types' file that's used to identify filetypes from file extensions. (This file is part of the apache http server distribution).

**Extn <mime type> <extn> ...**
Binds a list of file extensions to a particular mime-type, overriding information from the apache mime.types file.

**FileType <mime type> <commands>**
This config does most of the work. The 'mime type' argument is an fnmatch style pattern that matches against a mime type. For a given pattern one can supply the arguments 'safe', 'evil', 'container', 'contains', and 'equiv'. 'safe' and 'evil' are equivalent to 'ACCEPT' and 'REJECT' in an iptables firewall. mimeguard will return '0' (which is 'true' in bash scripts) if a file and all it's contained files are declared 'safe'. If will return various non-zero values if the file, or any of it's contents, match against an 'evil' rule. For example:

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

Often emails also use a 'multipart/alternative' mime-type to provide alternative text for mail clients that do/don't support HTML. Thus our config expands to:

```
FileType * evil
FileType text/* safe
FileType multipart/* safe
FileType multipart/mixed container safe
```

Finally there are a couple of other benign mime-types that can appear in emails:

```
FileType * evil
FileType text/* safe
FileType multipart/* safe
FileType multipart/mixed container safe
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
The 'Header' config can be used to match against an email header, and trigger a config option that overrides previously set options. The <header type> argument is an email header type like "To", "From" or "Subject". The <header argument> option is an fnmatch style pattern that matches against the variable part of the header. The config is a 'FileType' or 'String' command. For example:

```
Header From *@microsoft.com FileType applicationa/x-ms* safe
```

This allows different configs for different email senders/recipients.

# MACROS

By default mimeguard will interrogate items of mime-type 'application/x-ole-storage' for macros. If the object contains an internal directory entry called 'Macros' or 'VBA' then it will be considered 'evil'. 

# INVOCATION

**mimeguard <options> <paths>**

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
```


# EXAMPLE CONFIG FILE

```
# Include standard 'magic' and 'mime.types' files for identifying files
MagicsFile /etc/magic
MimeTypesFile /etc/mime.types

# some file extension to mime-type mappings that might not be in /etc/mime.types
Extn application/script scr ws wsf wsc wsh cmd
Extn application/powershell ps1 ps1xml ps2 ps2xml psc1 psc2 msh msh1 msh2 mshxml msh1xml msh2xml
Extn windows/shortcut scf lnk inf reg

# by default declare all mime-types as evil
FileType * evil

# now start listing mime-types that are safe
FileType message/rfc822 safe
FileType application/ms-tnef safe
FileType multipart/* safe
FileType multipart/mixed container safe
FileType */xml safe allow-blank-ctype
FileType */csv safe allow-blank-ctype
FileType text/* safe allow-blank-ctype
FileType audio/* safe allow-blank-ctype
FileType video/* safe allow-blank-ctype


## xls doc ppt, these are all of type 'application/x-ole-storage'. Mimeguard recognizes that mimetype internally
## and does further checks on it
FileType office/macros safe allow-blank-ctype equiv=application/x-ole-storage
FileType application/msword safe allow-blank-ctype equiv=application/x-ole-storage
FileType application/vnd.ms-excel safe allow-blank-ctype equiv=application/x-ole-storage
FileType application/vnd.ms-project safe allow-blank-ctype equiv=application/x-ole-storage


## xlsx docx pptx. these map to 'application/zip' which is also internally handled by Mimeguard 
FileType application/vnd.openxmlformats-officedocument.wordprocessingml.document safe allow-blank-ctype contains=application/xml,image/*,application/x-msmetafile,application/binary,application/vml equiv=application/zip
FileType application/vnd.openxmlformats-officedocument.spreadsheetml.sheet safe allow-blank-ctype contains=application/xml,image/*,application/x-msmetafile,application/binary,application/vml equiv=application/zip,application/x-zip
FileType application/vnd.openxmlformats-officedocument.presentationml.presentation safe allow-blank-ctype contains=application/xml,image/*,application/x-msmetafile,application/binary,application/vml equiv=application/zip

## zip files. mustn't contain a zip file or an msword file (as that's a standard malware trick)
FileType application/zip  safe container allow-blank-ctype contains=*,!application/zip,!application/msword

## Naughty strings in pdf and rtf documents
String application/pdf /JS /JavaScript /OpenAction /AA /Launch /EmbeddedFile /ASCII85Decode
String application/rtf \\objocx pFragments

```


