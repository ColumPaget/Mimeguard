#include "Export.h"
#include "FileTypeRules.h"

int ExportOpen(TMimeItem *Item, char **SavePath)
{
    int fd=-1;

    if (SavePath)
    {
        if (Config->Flags & FLAG_EXPORT)
        {
            if (Config->Flags & FLAG_DEBUG) printf("EXPORT: %s\n",Item->FileName);
            if (StrValid(Item->FileName))
            {
                *SavePath=MCopyStr(*SavePath, Config->ExportPath, Item->FileName, NULL);
                MakeDirPath(*SavePath, 0700);
                fd=open(*SavePath, O_CREAT | O_TRUNC | O_RDWR, 0600);
            }
            else
            {
                *SavePath=MCopyStr(*SavePath, Config->ExportPath, "tmpXXXXXX", NULL);
                MakeDirPath(*SavePath, 0700);
                fd=mkstemp(*SavePath);
            }
        }
        else
        {
            *SavePath=CopyStr(*SavePath,".tmpfileXXXXXX");
            if (Config->Flags & FLAG_DEBUG) printf("TMPFILE: %s %s\n",*SavePath, Item->ContentType);
            MakeDirPath(*SavePath, 0700);
            fd=mkstemp(*SavePath);
        }
    }

    if (fd > -1) fchmod(fd,0600);

    return(fd);
}



void RewriteCopyDocument(TMimeItem *Outer, TMimeItem *Item, const char *Path)
{
char *Tempstr=NULL, *Encoded=NULL;
STREAM *In;
int result, RulesResult, encode=FALSE;

if (g_Rewrite)
{
	
	RulesResult=IsItSafe(Item);
	if (
			(strncmp(Item->ContentType, "multipart/", 10) != 0) &&
			(strncmp(Item->ContentType, "text/", 5) != 0) 
		) encode=TRUE;


	if (! (RulesResult & RULE_EVIL))
	{
		Tempstr=MCopyStr(Tempstr, "\r\n", Outer->Boundary, "\r\n", NULL);
		if (StrValid(Item->ContentType))
		{
			if (strncmp(Item->ContentType,"multipart/",10)==0) Tempstr=MCatStr(Tempstr, "Content-Type: ", Item->ContentType, ";\n	boundary\"", Item->Boundary, "\"\r\n", NULL);
			else if (StrValid(Item->FileName)) Tempstr=MCatStr(Tempstr, "Content-Type: ", Item->ContentType, ";\n	name=\"", Item->FileName, "\"\r\n", NULL);
			else Tempstr=MCatStr(Tempstr, "Content-Type: ", Item->ContentType, "\r\n", NULL);
		}
	
	
		if (StrValid(Item->Disposition)) 
		{
			if (StrValid(Item->FileName)) Tempstr=MCatStr(Tempstr, "Content-Disposition: ", Item->Disposition, ";\n	filename=\"", Item->FileName, "\"\r\n", NULL);
			else Tempstr=MCatStr(Tempstr, "Content-Disposition: ", Item->Disposition, "\r\n", NULL);
		}
	
		if (encode) Tempstr=CatStr(Tempstr, "Content-Transfer-Encoding: base64\r\n");
	
		Tempstr=CatStr(Tempstr, "\r\n");
		STREAMWriteLine(Tempstr, g_Rewrite);
	
		In=STREAMOpen(Path, "r");
		if (In)
		{
		Tempstr=SetStrLen(Tempstr, 255);
		result=STREAMReadBytes(In, Tempstr, 57);
		while (result > 0)
		{
			Tempstr[result]='\0';
			if (encode)
			{
				Encoded=EncodeBytes(Encoded, Tempstr,  result, ENCODE_BASE64);
				Encoded=CatStr(Encoded,"\r\n");
				STREAMWriteLine(Encoded, g_Rewrite);
			}
			else STREAMWriteBytes(g_Rewrite, Tempstr, result);
			result=STREAMReadBytes(In, Tempstr, 57);
		}
		STREAMClose(In);
		}
	}
}

Destroy(Encoded);
Destroy(Tempstr);
}

