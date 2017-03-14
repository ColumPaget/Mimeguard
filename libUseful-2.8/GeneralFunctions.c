#include "includes.h"
#include "base64.h"
#include "Hash.h"
#include "Time.h"
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/file.h>
#include "base64.h"
#include <math.h>
#include <pwd.h>
#include <grp.h>

//xmemset uses a 'volatile' pointer so that it won't be optimized out
void xmemset(char *Str, char fill, off_t size)
{
volatile char *p;

for (p=Str; p < (Str+size); p++) *p=fill;
}

int xsetenv(const char *Name, const char *Value)
{
if (setenv(Name, Value,TRUE)==0) return(TRUE);
return(FALSE);
}

int ptr_incr(const char **ptr, int count)
{
const char *end;

end=(*ptr) + count;
for (; (*ptr) < end; (*ptr)++)
{
if ((**ptr) == '\0') return(FALSE);
}
return(TRUE);
}


const char *traverse_quoted(const char *ptr)
{
char Quote;

    Quote=*ptr;
    ptr++;
    while ((*ptr != Quote) && (*ptr != '\0'))
    {
      //handle quoted chars
      if ((*ptr=='\\') && (*(ptr+1) != '\0')) ptr++;
      ptr++;
    }
	return(ptr);
}


#define FNV_INIT_VAL 2166136261
unsigned int fnv_hash(unsigned const char *key, int NoOfItems)
{
  unsigned const char *p, *end;
  unsigned int h = FNV_INIT_VAL;
  int i;

  for (p=key; *p !='\0' ; p++ ) h = ( h * 16777619 ) ^ *p;

  return(h % NoOfItems);
}



char *CommaList(char *RetStr, const char *AddStr)
{
if (StrValid(RetStr)) RetStr=MCatStr(RetStr,", ",AddStr,NULL);
else RetStr=CopyStr(RetStr,AddStr);
return(RetStr);
}



char *BytesToHexStr(char *Buffer, char *Bytes, int len)
{
int i;
char *Str=NULL, *ptr;


Str=SetStrLen(Buffer,(len *2) +1);
ptr=Str;
for (i=0; i < len; i++)
{
	snprintf(ptr,2,"%02x",Bytes[i]);
	ptr+=2;
}
*ptr='\0';

return(Str);
}


int HexStrToBytes(char **Buffer, char *HexStr)
{
int i, len;
char *Str=NULL, *ptr;

len=StrLen(HexStr);
*Buffer=SetStrLen(*Buffer,len / 2);
ptr=*Buffer;
for (i=0; i < len; i+=2)
{
   Str=CopyStrLen(Str,HexStr+i,2);
   *ptr=strtol(Str,NULL,16);
   ptr++;
}

DestroyString(Str);
return(len / 2);
}


char *Ascii85(char *RetStr, const char *Bytes, int ilen, const char *CharMap)
{
const char *ptr, *block, *end;
uint32_t val, mod;
int olen=0, i;
char Buff[6];

end=Bytes+ilen;
for (ptr=Bytes; ptr < end; )
{
	block=ptr;
	val = ((*ptr & 0xFF) << 24); ptr++;
	if (ptr < end)
	{
	val |= ((*ptr & 0xFF) << 16); ptr++;
	}

	if (ptr < end)
	{
	val |= ((*ptr & 0xFF) << 8); ptr++;
	}

	if (ptr < end)
	{
	val |= (*ptr & 0xFF); ptr++;
	}

	if (val==0) strcpy(Buff,"z");
	else for (i=4; i >-1; i--)
	{
		mod=val % 85;
		val /= 85;
		Buff[i]=CharMap[mod & 0xFF];
	}

	//we only add as many characters as we encoded
	//so for the last chracter
	RetStr=CatStrLen(RetStr,Buff,ptr-block);
} 

return(RetStr);
}


char *EncodeBytes(char *Buffer, const char *Bytes, int len, int Encoding)
{
char *Tempstr=NULL, *RetStr=NULL;
int i;

RetStr=CopyStr(Buffer,"");
switch (Encoding)
{
	case ENCODE_BASE64: 
	RetStr=SetStrLen(RetStr,len * 4);
	to64frombits((unsigned char *) RetStr,(unsigned char *) Bytes,len); break;
	break;

	case ENCODE_IBASE64: 
	RetStr=SetStrLen(RetStr,len * 4);
	Radix64frombits((unsigned char *) RetStr,(unsigned char *) Bytes,len,IBASE64_CHARS,'\0'); break;
	break;

	case ENCODE_PBASE64: 
	RetStr=SetStrLen(RetStr,len * 4);
	Radix64frombits((unsigned char *) RetStr,(unsigned char *) Bytes,len,SBASE64_CHARS,'\0'); break;
	break;


	case ENCODE_CRYPT: 
	RetStr=SetStrLen(RetStr,len * 4);
	Radix64frombits((unsigned char *) RetStr,(unsigned char *) Bytes,len,CRYPT_CHARS,'\0'); break;
	break;

	case ENCODE_XXENC: 
	RetStr=SetStrLen(RetStr,len * 4);
	Radix64frombits((unsigned char *) RetStr,(unsigned char *) Bytes,len,XXENC_CHARS,'+'); break;
	break;

	case ENCODE_UUENC: 
	RetStr=SetStrLen(RetStr,len * 4);
	Radix64frombits((unsigned char *) RetStr,(unsigned char *) Bytes,len,UUENC_CHARS,'\''); break;
	break;

	case ENCODE_ASCII85: 
	RetStr=Ascii85(RetStr,Bytes,len,ASCII85_CHARS); break;
	break;

	case ENCODE_Z85: 
	RetStr=Ascii85(RetStr,Bytes,len,Z85_CHARS); break;
	break;

	case ENCODE_OCTAL:
  for (i=0; i < len; i++)
  {
  Tempstr=FormatStr(Tempstr,"%03o",Bytes[i] & 255);
  RetStr=CatStr(RetStr,Tempstr);
  }
	break;

	case ENCODE_DECIMAL:
  for (i=0; i < len; i++)
  {
  Tempstr=FormatStr(Tempstr,"%03d",Bytes[i] & 255);
  RetStr=CatStr(RetStr,Tempstr);
  }
	break;

	case ENCODE_HEX:
  for (i=0; i < len; i++)
  {
  Tempstr=FormatStr(Tempstr,"%02x",Bytes[i] & 255);
  RetStr=CatStr(RetStr,Tempstr);
  }
	break;

	case ENCODE_HEXUPPER:
  for (i=0; i < len; i++)
  {
  Tempstr=FormatStr(Tempstr,"%02X",Bytes[i] & 255);
  RetStr=CatStr(RetStr,Tempstr);
  }
	break;
	

	default:
	RetStr=SetStrLen(RetStr,len );
	memcpy(RetStr,Bytes,len);
	RetStr[len]='\0';
	break;
}

DestroyString(Tempstr);
return(RetStr);
}




int GenerateRandomBytes(char **RetBuff, int ReqLen, int Encoding)
{
struct utsname uts;
int i, len;
clock_t ClocksStart, ClocksEnd;
char *Tempstr=NULL, *RandomBytes=NULL;
int fd;


fd=open("/dev/urandom",O_RDONLY);
if (fd > -1)
{
	RandomBytes=SetStrLen(RandomBytes,ReqLen);
  len=read(fd,RandomBytes,ReqLen);
  close(fd);
}
else
{
	ClocksStart=clock();
	//how many clock cycles used here will depend on overall
	//machine activity/performance/number of running processes
	for (i=0; i < 100; i++) sleep(0);
	uname(&uts);
	ClocksEnd=clock();


	Tempstr=FormatStr(Tempstr,"%lu:%lu:%lu:%lu:%llu\n",getpid(),getuid(),ClocksStart,ClocksEnd,GetTime(TIME_MILLISECS));
	//This stuff should be unique to a machine
	Tempstr=CatStr(Tempstr, uts.sysname);
	Tempstr=CatStr(Tempstr, uts.nodename);
	Tempstr=CatStr(Tempstr, uts.machine);
	Tempstr=CatStr(Tempstr, uts.release);
	Tempstr=CatStr(Tempstr, uts.version);


	len=HashBytes(&RandomBytes, "sha256", Tempstr, StrLen(Tempstr), 0);
	if (len > ReqLen) len=ReqLen;
}


*RetBuff=EncodeBytes(*RetBuff, RandomBytes, len, Encoding);

DestroyString(Tempstr);
DestroyString(RandomBytes);

return(len);
}




char *GetRandomData(char *RetBuff, int len, char *AllowedChars)
{
int fd;
char *Tempstr=NULL, *RetStr=NULL;
int i;
uint8_t val, max_val;

srand(time(NULL));
max_val=StrLen(AllowedChars);

RetStr=CopyStr(RetBuff,"");
fd=open("/dev/urandom",O_RDONLY);
for (i=0; i < len ; i++)
{
	if (fd > -1) read(fd,&val,1);
	else val=rand();

	RetStr=AddCharToStr(RetStr,AllowedChars[val % max_val]);
}

if (fd) close(fd);

DestroyString(Tempstr);
return(RetStr);
}


char *GetRandomHexStr(char *RetBuff, int len)
{
return(GetRandomData(RetBuff,len,HEX_CHARS));
}


char *GetRandomAlphabetStr(char *RetBuff, int len)
{
return(GetRandomData(RetBuff,len,ALPHA_CHARS));
}



double FromMetric(const char *Data, int Type)
{
double val;
char *ptr=NULL;

val=strtod(Data,&ptr);
while (isspace(*ptr)) ptr++;
switch (*ptr)
{
case 'k': val=val*1000; break;
case 'M': val=val*pow(1000,2); break;
case 'G': val=val*pow(1000,3); break;
case 'T': val=val*pow(1000,4); break;
case 'P': val=val*pow(1000,5); break;
case 'E': val=val*pow(1000,6); break;
case 'Z': val=val*pow(1000,7); break;
case 'Y': val=val*pow(1000,8); break;
}

return(val);
}



const char *ToMetric(double Size, int Type)
{
static char *Str=NULL;
double val=0, next;
//Set to 0 to keep valgrind happy
int i=0;
char suffix=' ', *sufflist=" kMGTPEZY";

val=Size;

for (i=0; sufflist[i] !='\0'; i++)
{
	next=pow(1000,i+1);
	if (next > val) break;
}

if ((sufflist[i] > 0) && (sufflist[i] !='\0'))
{
	val=val / pow(1000,i);
  suffix=sufflist[i];
}


Str=FormatStr(Str,"%0.1f%c",(float) val,suffix);
return(Str);
}



char *DecodeBase64(char *Return, int *len, char *Text)
{
char *RetStr;

RetStr=SetStrLen(Return,StrLen(Text) *2);
*len=from64tobits(RetStr,Text);

return(RetStr);
}




int LookupUID(const char *User)
{
struct passwd *pwent;
char *ptr;

pwent=getpwnam(User);
if (! pwent)
{
	syslog(LOG_ERR,"ERROR: Cannot lookup '%s'. No such user",User);
	ptr=LibUsefulGetValue("SwitchUserAllowFail");
	if (ptr && (strcasecmp(ptr,"yes")==0)) return(-1);
	exit(1);
}
return(pwent->pw_uid);
}


int LookupGID(const char *Group)
{
struct group *grent;
char *ptr;

grent=getgrnam(Group);
if (! grent)
{
	syslog(LOG_ERR,"ERROR: Cannot switch to group '%s'. No such group",Group);
	ptr=LibUsefulGetValue("SwitchGroupAllowFail");
	if (ptr && (strcasecmp(ptr,"yes")==0)) return(-1);
	exit(1);
}
return(grent->gr_gid);
}



//This is a convienice function for use by modern languages like
//lua that have an 'os' object that returns information 
const char *OSSysInfoString(int Info)
{
static struct utsname UtsInfo;
static struct sysinfo SysInfo;
struct passwd *pw;
const char *ptr;

uname(&UtsInfo);
sysinfo(&SysInfo);

switch (Info)
{
case OSINFO_TYPE: return(UtsInfo.sysname); break;
case OSINFO_ARCH: return(UtsInfo.machine); break;
case OSINFO_RELEASE: return(UtsInfo.release); break;
case OSINFO_HOSTNAME: return(UtsInfo.nodename); break;
case OSINFO_HOMEDIR:
  pw=getpwuid(getuid());
  if (pw) return(pw->pw_dir);
break;

case OSINFO_TMPDIR:
  ptr=getenv("TMPDIR");
  if (! ptr) ptr=getenv("TEMP");
  if (! ptr) ptr="/tmp";
  if (ptr) return(ptr);
break;

/*
case OSINFO_UPTIME: MuJSNumber((double) SysInfo.uptime); break;
case OSINFO_TOTALMEM: MuJSNumber((double) SysInfo.totalram); break;
case OSINFO_FREEMEM: MuJSNumber((double) SysInfo.freeram); break;
case OSINFO_PROCS: MuJSNumber((double) SysInfo.procs); break;
case OSINFO_LOAD: MuJSArray(TYPE_ULONG, 3, (void *) SysInfo.loads); break;
*/


/*
case OSINFO_USERINFO:
  pw=getpwuid(getuid());
  if (pw)
  {
    MuJSNewObject(TYPE_OBJECT);
    MuJSNumberProperty("uid",pw->pw_uid);
    MuJSNumberProperty("gid",pw->pw_gid);
    MuJSStringProperty("username",pw->pw_name);
    MuJSStringProperty("shell",pw->pw_shell);
    MuJSStringProperty("homedir",pw->pw_dir);
  }
break;
}
*/

}


return("");
}


//This is a convienice function for use by modern languages like
//lua that have an 'os' object that returns information 
unsigned long OSSysInfoLong(int Info)
{
struct utsname UtsInfo;
struct sysinfo SysInfo;
struct passwd *pw;
const char *ptr;

uname(&UtsInfo);
sysinfo(&SysInfo);

switch (Info)
{
case OSINFO_UPTIME: return((unsigned long) SysInfo.uptime); break;
case OSINFO_TOTALMEM: return((unsigned long) SysInfo.totalram); break;
case OSINFO_FREEMEM: return((unsigned long) SysInfo.freeram); break;
case OSINFO_PROCS: return((unsigned long) SysInfo.procs); break;
//case OSINFO_LOAD: MuJSArray(TYPE_ULONG, 3, (void *) SysInfo.loads); break;
return(0);
}
}
