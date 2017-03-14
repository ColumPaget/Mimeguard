#include "ParseURL.h"

char *ParsePort(char *Str, char **Port)
{
char *ptr;

//IP6 addresses, because it's a badly designed protocol, use the same separator as used to identify the port
//hence we have to enclose them in square braces, which we must handle here
ptr=Str;
if (*ptr=='[')
{
  while ((*ptr !=']') && (*ptr !='\0')) ptr++;
}

//if there's a port specified then copy it to 'Port' and then clip it off the host specification
ptr=strchr(ptr,':');
if (ptr)
{
if (Port) *Port=CopyStr(*Port,ptr+1);
*ptr='\0';
}

return(ptr);
}


const char *ParseHostDetails(const char *Data,char **Host,char **Port,char **User, char **Password)
{
char *Token=NULL, *tptr;
const char *ptr; 

if (Port) *Port=CopyStr(*Port, "");
if (Host) *Host=CopyStr(*Host, "");
if (User) *User=CopyStr(*User, "");
if (Password) *Password=CopyStr(*Password, "");

ptr=strrchr(Data,'@');
if (ptr)
{
  //in some cases there will be an '@' in the username, so GETTOKEN_QUOTES
  //should handle any @ which is prefixed with a \ to quote it out
  ptr=GetToken(Data,"@",&Token, GETTOKEN_QUOTES);
  if (User)
  {
	 tptr=GetToken(Token,":",User,0);
 	 if (StrLen(tptr)) *Password=CopyStr(*Password,tptr);
  }
}
else ptr=Data;

ptr=GetToken(ptr,"/",&Token,0);
ptr=ParsePort(Token, Port);
if (Host) *Host=CopyStr(*Host, Token);

DestroyString(Token);

return(ptr);
}

void ParseURL(const char *URL, char **Proto, char **Host, char **Port, char **User, char **Password, char **Path, char **Args)
{
const char *ptr;
char *Token=NULL, *tProto=NULL, *aptr;

//we might not return a protocol!
if (Proto) *Proto=CopyStr(*Proto,"");

//Even if they pass NULL for protocol, we need to take a copy for use in
//the 'guess the port' section below
ptr=strchr(URL,':');
if (ptr)
{
	tProto=CopyStrLen(tProto,URL,ptr-URL);
	strlwr(tProto);
	aptr=strchr(tProto, '.');
	if (aptr)
	{
		//protocol name is not allowed to contain '.', so this must be a hostname or
		//ip address. 
		ptr=URL;
	}
	else
	{
		if (Proto) *Proto=CopyStr(*Proto,tProto);
		ptr++;
		//some number of '//' follow protocol
		while (*ptr=='/') ptr++;
	}

	ptr=GetToken(ptr,"/",&Token,0);
	ParseHostDetails(Token,Host,Port,User,Password);
}
else ptr=URL;

while (*ptr=='/') ptr++;

if (ptr)
{
if (Path) 
{
	*Path=MCopyStr(*Path,"/",ptr,NULL);

	//Only split the HTTP CGI arguments from the document path if we were 
	//asked to return the args seperately
	if (Args)
	{
		aptr=strrchr(*Path,'?');
		if (aptr) 
		{
		*aptr='\0';
		aptr++;
		*Args=CopyStr(*Args,aptr);
		}
	}
}
}

//the 'GetToken' call will have thrown away the '/' at the start of the path
//add it back in

if (Port && (! StrLen(*Port)) && StrLen(tProto))
{
	if (strcmp(tProto,"http")==0) *Port=CopyStr(*Port,"80");
	else if (strcmp(tProto,"https")==0) *Port=CopyStr(*Port,"443");
	else if (strcmp(tProto,"ssh")==0) *Port=CopyStr(*Port,"22");
	else if (strcmp(tProto,"ftp")==0) *Port=CopyStr(*Port,"21");
	else if (strcmp(tProto,"telnet")==0) *Port=CopyStr(*Port,"23");
	else if (strcmp(tProto,"smtp")==0) *Port=CopyStr(*Port,"25");
	else if (strcmp(tProto,"mailto")==0) *Port=CopyStr(*Port,"25");

}


DestroyString(Token);
DestroyString(tProto);
}






void ParseConnectDetails(const char *Str, char **Type, char **Host, char **Port, char **User, char **Pass, char **Path)
{
char *ptr, *Token=NULL, *Args=NULL;

ptr=GetToken(Str," ",&Token,0);
ParseURL(Token, Type, Host, Port, User, Pass, Path, &Args);

if (Path && StrLen(Args)) *Path=MCatStr(*Path,"?",Args,NULL);

while (ptr)
{
	if (strcmp(Token,"-password")==0) ptr=GetToken(ptr," ",Pass,0);
	else if (strcmp(Token,"-keyfile")==0)
	{
		ptr=GetToken(ptr," ",&Token,0);
		*Pass=MCopyStr(*Pass,"keyfile:",Token,NULL);
	}
ptr=GetToken(ptr," ",&Token,0);
}

DestroyString(Token);
DestroyString(Args);
}


