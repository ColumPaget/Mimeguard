#include "IPRegion.h"
#include <fnmatch.h>

char *RegionFileList=NULL;

void RegionSetFiles(const char *FilesList)
{
RegionFileList=FileListExpand(RegionFileList, FilesList);
}


char *RegionFileLookup(char *RetStr, const char *Path, const char *IPStr)
{
    char *Tempstr=NULL, *Registrar=NULL, *Country=NULL, *Type=NULL, *Subnet=NULL, *Token=NULL;
    const char *ptr;
    int result=FALSE;
    uint32_t IP, Mask, val;
    STREAM *S;
		
    IP=StrtoIP(IPStr);
    S=STREAMOpen(Path, "r");
    if (S)
    {
        Tempstr=STREAMReadLine(Tempstr, S);
        while (Tempstr)
        {
            ptr=GetToken(Tempstr,"|",&Registrar,0);
            ptr=GetToken(ptr,"|",&Country,0);
            ptr=GetToken(ptr,"|",&Type,0);
            ptr=GetToken(ptr,"|",&Subnet,0);

            if (*Subnet != '*')
            {
                if (strcmp(Type,"ipv4")==0)
                {
                    ptr=GetToken(ptr,"|",&Token,0);
                    val=atoi(Token);
                    //'val' is number of assigned IPs. Netmask is this -1
                    Mask=htonl(~(val-1));

                    if ((IP & Mask) == StrtoIP(Subnet))
                    {
                        RetStr=MCopyStr(RetStr,Registrar,":",Country,NULL);
                        break;
                    }
                }
                else if (strcmp(Type,"ipv6")==0)
                {
                    ptr=GetToken(ptr,"|",&Token,0);
                    /*
                    if (IP6Compare(IPStr, Subnet, atoi(Token)))
                    {
                    	RetStr=MCopyStr(RetStr,Registrar,":",Country,NULL);
                    	break;
                    }
                    */
                }
            }
            Tempstr=STREAMReadLine(Tempstr, S);
        }

        STREAMClose(S);
    }

    DestroyString(Registrar);
    DestroyString(Tempstr);
    DestroyString(Country);
    DestroyString(Subnet);
    DestroyString(Token);
    DestroyString(Type);

    return(RetStr);
}


char *RegionLookup(char *RetStr, const char *IP)
{
    char *Path=NULL;
    const char *ptr;

		if (! StrValid(IP)) return(CopyStr(RetStr, ""));
    if (strncmp(IP,"127.",4)==0) return(CopyStr(RetStr,"local"));
    if (strncmp(IP,"192.168.",8)==0) return(CopyStr(RetStr,"local"));
    if (strncmp(IP,"10.",3)==0) return(CopyStr(RetStr,"local"));
    if (fnmatch(IP,"172.1[6-9].*",0)==0) return(CopyStr(RetStr,"local"));
    if (fnmatch(IP,"172.2?.*",0)==0) return(CopyStr(RetStr,"local"));
    if (strncmp(IP,"172.30.",7)==0) return(CopyStr(RetStr,"local"));
    if (strncmp(IP,"172.31.",7)==0) return(CopyStr(RetStr,"local"));


    ptr=GetTypedVar(g_KeyValueStore, IP, KV_IPREGION);
    if (StrValid(ptr)) return(CopyStr(RetStr, ptr));

//after here we start allocating memory, so we can't just return, we have to
//go to the end of function

    ptr=GetToken(RegionFileList,",",&Path,0);
    while (ptr)
    {
        StripLeadingWhitespace(Path);
        StripTrailingWhitespace(Path);
        RetStr=RegionFileLookup(RetStr, Path, IP);
        SetTypedVar(g_KeyValueStore, IP, RetStr, KV_IPREGION);
        if (StrLen(RetStr)) break;

        ptr=GetToken(ptr,",",&Path,0);
    }

    DestroyString(Path);

    return(RetStr);
}


