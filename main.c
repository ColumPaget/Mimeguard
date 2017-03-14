#include "common.h"
#include "Mime.h"
#include "FileTypeRules.h"
#include "FileExtensions.h"
#include "PDF.h"
#include "RTF.h"
#include "Magic.h"
#include "Output.h"
#include "ConfigFile.h"
#include "DocumentTypes.h"

#define VERSION "1.0"
char *ConfigPath=NULL;

void PrintUsage()
{
printf("Mimeguard: version %s\n",VERSION);
printf("Author: Colum Paget\n");
printf("Email: colums.projects@gmail.com\n");

printf("\n");
printf("Usage:\n    mimeguard [options] [path to file]...\n");
printf("Options:\n");
printf("  %-15s %s\n","--help", "Print this help");
printf("  %-15s %s\n","-help", "Print this help");
printf("  %-15s %s\n","-?", "Print this help");
printf("  %-15s %s\n","--version", "Print program version");
printf("  %-15s %s\n","-version", "Print program version");
printf("  %-15s %s\n","-c <path>", "Path to config file");
printf("  %-15s %s\n","-d", "Print debugging");
printf("  %-15s %s\n","-x", "Dump/export/unpack file contents");
printf("  %-15s %s\n","-safe", "Only show safe files");
printf("  %-15s %s\n","-evil", "Only show unsafe/evil files");
printf("  %-15s %s\n","-show <email header>", "Show Email Header");
exit(0);
}


void PrintVersion()
{
	printf("mimeguard version %s\n",VERSION);
	printf("default config file: %s/mimeguard.conf\n",SYSCONFDIR);
	exit(0);
}

void ParseCommandLine(int argc, char *argv[])
{
int i;
char *Tempstr=NULL;

for (i=1; i < argc; i++)
{
	if (strcmp(argv[i],"-c")==0)
	{
		strcpy(argv[i], "");
		ConfigPath=CopyStr(ConfigPath, argv[++i]);
		strcpy(argv[i], "");
	}
	else if (strcmp(argv[i],"-x")==0)
	{
		Flags |= FLAG_EXPORT;
		strcpy(argv[i], "");
	}
	else if (strcmp(argv[i],"-d")==0)
	{
		Flags |= FLAG_DEBUG;
		strcpy(argv[i], "");
	}
	else if (strcmp(argv[i],"-safe")==0)
	{
		Flags &= ~(FLAG_SHOW_EVIL | FLAG_SHOW_CURR);
		strcpy(argv[i], "");
	}
	else if (strcmp(argv[i],"-evil")==0)
	{
		Flags &= ~(FLAG_SHOW_SAFE | FLAG_SHOW_CURR);
		strcpy(argv[i], "");
	}
	else if (strcmp(argv[i],"-show")==0)
	{
		strcpy(argv[i], "");
		i++;
		Tempstr=CopyStr(Tempstr,argv[i]);	
		strrep(Tempstr,',', '|');
		strrep(Tempstr,' ', '|');
		FileRulesAdd(Tempstr, RULE_HEADER, "*", "show");
		strcpy(argv[i], "");
	}
	else if (
		(strcmp(argv[i],"--help")==0) ||
		(strcmp(argv[i],"-help")==0) ||
		(strcmp(argv[i],"-?")==0)
	) PrintUsage();
	else if (
		(strcmp(argv[i],"--version")==0) ||
		(strcmp(argv[i],"-version")==0) ||
		(strcmp(argv[i],"-v")==0)
	)	PrintVersion();


}

DestroyString(Tempstr);
}



char *FileType(char *RetStr, STREAM *S)
{
char *Tempstr=NULL;

  Tempstr=SetStrLen(Tempstr, 255);
  STREAMPeekBytes(S,Tempstr,255);

  RetStr=CopyStr(RetStr, MagicLookupContentType(Tempstr));

	DestroyString(Tempstr);

	return(RetStr);
}



int ProcessFile(const char *Path)
{
char *ExtnType=NULL, *MagicType=NULL, *ptr;
TMimeItem *MimeOuter=NULL;
STREAM *S;
int ExitVal=RULE_NONE;


	S=STREAMFileOpen(Path, SF_RDONLY | SF_NOCACHE);
	if (S)
	{
		ptr=strrchr(Path, '.');
		if (ptr) ExtnType=CopyStr(ExtnType, FileExtensionLookup(ptr));
		else ExtnType=CopyStr(ExtnType,"");
		MagicType=FileType(MagicType, S);

		ExportPath=MCopyStr(ExportPath,GetBasename((char *) Path),".dump/",NULL);

		if ((! StrValid(MagicType)) && (! StrValid(ExtnType))) 
		{
		MimeOuter=MimeReadHeaders(S);
		if (MimeOuter) MimeOuter->FileName=CopyStr(MimeOuter->FileName, Path);
		}
		else MimeOuter=MimeItemCreate(Path,MagicType,ExtnType);

		if (MimeOuter)
		{
		MimeOuter->Flags |= MIMEFLAG_ROOT;
		DocTypeProcess(S, MimeOuter, Path);
		ExitVal=IsItSafe(MimeOuter);
		OutputItem(MimeOuter, MimeOuter,0, ExitVal==RULE_SAFE);
		MimeItemDestroy(MimeOuter);
		}

		STREAMClose(S);
	}

DestroyString(ExtnType);
DestroyString(MagicType);


return(ExitVal);
}




int main(int argc, char *argv[])
{
int ExitVal=RULE_NONE, i;
int DocCount=0, Safe=0, Evil=0, Malformed=0;
struct stat Stat;

Flags = FLAG_SHOW_SAFE | FLAG_SHOW_EVIL | FLAG_SHOW_CURR;
MagicLoadDefaults();
FileExtensionsLoadDefaults();

#ifdef SYSCONFDIR
ConfigPath=MCopyStr(ConfigPath,SYSCONFDIR,"/mimeguard.conf",NULL);
#endif

ParseCommandLine(argc, argv);
ConfigFileLoad(ConfigPath);


for (i=1; i < argc; i++)
{
	if (StrValid(argv[i]))
	{
		if (stat(argv[i], &Stat)==0)
		{
			if (S_ISREG(Stat.st_mode))
			{
			ExitVal=ProcessFile(argv[i]);

			if (ExitVal & RULE_EVIL) Evil++;
			else if (ExitVal & RULE_SAFE) Safe++;
			if (ExitVal & RULE_MALFORMED) Malformed++;
			DocCount++;
			}
			else 
			{
				ExitVal=-2;
				printf("ERROR: Not a regular file %s\n",argv[i]);
			}
		}
		else 
		{
			ExitVal=-1;
			printf("ERROR: No such file %s\n",argv[i]);
		}

//		printf("exit: %d e: %d s: %d m:%d \n",ExitVal, ExitVal & RULE_EVIL, ExitVal & RULE_SAFE, ExitVal & RULE_MALFORMED);
	}
}

printf("%d Documents considered. %d safe %d evil %d malformed\n",DocCount, Safe, Evil, Malformed);

if (Evil > 0) exit(1);
if (ExitVal & RULE_SAFE) exit(0);
if (ExitVal==RULE_NONE) exit(2);
exit(3);
}
