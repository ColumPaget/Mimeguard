#include "common.h"
#include "Mime.h"
#include "FileTypeRules.h"
#include "FileExtensions.h"
#include "PDF.h"
#include "RTF.h"
#include "URL.h"
#include "FileMagics.h"
#include "Output.h"
#include "Settings.h"
#include "DocumentTypes.h"
#include "Smtp.h"

#define VERSION "4.5"
char *ConfigPath=NULL;


void PrintVersion()
{
    printf("mimeguard version %s\n",VERSION);
    printf("default config file: %s/mimeguard.conf\n",SYSCONFDIR);
    exit(0);
}



void PrintUsage()
{
    printf("Mimeguard: version %s\n",VERSION);
    printf("Author: Colum Paget\n");
    printf("Email: colums.projects@gmail.com\n");

    printf("\n");
    printf("Usage:\n    mimeguard [options] [path to file]...\n");
    printf("Options:\n");
    printf("  %-25s %s\n","--help", "Print this help");
    printf("  %-25s %s\n","-help", "Print this help");
    printf("  %-25s %s\n","-?", "Print this help");
    printf("  %-25s %s\n","--version", "Print program version");
    printf("  %-25s %s\n","-version", "Print program version");
    printf("  %-25s %s\n","-c <path>", "Path to config file");
    printf("  %-25s %s\n","-d", "Print debugging");
    printf("  %-25s %s\n","-x", "Dump/export/unpack file contents");
    printf("  %-25s %s\n","-safe", "Only show safe files");
    printf("  %-25s %s\n","-evil", "Only show unsafe/evil files");
    printf("  %-25s %s\n","-strip", "Rewrite email files with harmful items removed");
    printf("  %-25s %s\n","-safe-dir <path>", "Move safe files to directory <path>");
    printf("  %-25s %s\n","-evil-dir <path>", "Move unsafe files to directory <path>");
    printf("  %-25s %s\n","-show <email header>", "Show specified email header");
		printf("  %-25s %s\n","-no-url", "Do not perform checks on urls in document (following URLs can slow checks up a lot");
		printf("  %-25s %s\n","-color", "Output result info with color");
		printf("  %-25s %s\n","-no-color", "Output result info without color");
    printf("  %-25s %s\n","-smtp <address>", "Run in SMTP mode. <address> is an optional argument of an address/port to bind to");
    printf("  %-25s %s\n","-smtp-banner <string>", "Initial server banner when running in SMTP mode");
    printf("  %-25s %s\n","-smtp-safe   <address>", "Server to send 'safe' mails to");
    printf("  %-25s %s\n","-smtp-evil   <address>", "Server to send 'evil' mails to");
    printf("  %-25s %s\n","-smtp-dest   <address>", "Server to send all mails to");
		printf("\n\n");
		printf("The <address> argument to -smtp is optional. The default address is 127.0.0.1:25, meaning 'bind to port 25 on the local interface'. If <address> lacks a port then port 25 is the default.\n");
    exit(0);
}


ListNode *ParseCommandLine(int argc, char *argv[])
{
    int i;
    char *Tempstr=NULL;
		const char *ptr;
		CMDLINE *Args;
		ListNode *Docs;

		Docs=ListCreate();
		Args=CommandLineParserCreate(argc, argv);
		ptr=CommandLineNext(Args);
		while (ptr)
    {
        if (strcmp(ptr,"-c")==0)
        {
            ConfigPath=CopyStr(ConfigPath, CommandLineNext(Args));
        }
        else if (strcmp(ptr,"-x")==0) Config->Flags |= FLAG_EXPORT;
        else if (strcmp(ptr,"-d")==0) Config->Flags |= FLAG_DEBUG;
        else if (strcmp(ptr,"-safe")==0) Config->Flags &= ~(FLAG_SHOW_EVIL | FLAG_SHOW_CURR);
        else if (strcmp(ptr,"-evil")==0) Config->Flags &= ~(FLAG_SHOW_SAFE | FLAG_SHOW_CURR);
        else if (strcmp(ptr,"-strip")==0) Config->Flags |= FLAG_STRIP;
        else if (strcmp(ptr,"-no-url")==0) Config->Flags |= FLAG_NO_URL_CHECKS;
        else if (strcmp(ptr,"-color")==0) Config->Flags |= FLAG_COLOR;
        else if (strcmp(ptr,"-no-color")==0) Config->Flags |= FLAG_NO_COLOR;
        else if (strcmp(ptr,"-show")==0)
        {
            Tempstr=CopyStr(Tempstr,CommandLineNext(Args));
						if (strcasecmp(Tempstr, "url")==0) Config->Flags |= FLAG_SHOWURL;
						else
						{
            strrep(Tempstr,',', '|');
            strrep(Tempstr,' ', '|');
						EmailHeaderRulesAdd(Tempstr, "*", "show");
						}
        }
        else if (strcmp(ptr,"-smtp")==0)
        {
            Config->Flags |= FLAG_SMTP;
						ptr=CommandLinePeek(Args);
						if (StrValid(ptr) && (*ptr != '-')) 
						{
							ptr=CommandLineNext(Args);
							Config->SmtpAddress=CopyStr(Config->SmtpAddress, ptr);
						}
        }
        else if (strcmp(ptr,"-safe-dir")==0) Config->SafeDir=CopyStr(Config->SafeDir, CommandLineNext(Args));
        else if (strcmp(ptr,"-evil-dir")==0) Config->EvilDir=CopyStr(Config->EvilDir, CommandLineNext(Args));
        else if (strcmp(ptr,"-smtp-banner")==0) Config->SmtpBanner=CopyStr(Config->SmtpBanner, CommandLineNext(Args));
        else if (strcmp(ptr,"-smtp-safe")==0) Config->SmtpPassServer=CopyStr(Config->SmtpPassServer, CommandLineNext(Args));
        else if (strcmp(ptr,"-smtp-evil")==0) Config->SmtpFailServer=CopyStr(Config->SmtpFailServer, CommandLineNext(Args));
        else if (strcmp(ptr,"-smtp-dest")==0) 
				{
					Config->SmtpPassServer=CopyStr(Config->SmtpPassServer, CommandLineNext(Args));
					Config->SmtpFailServer=CopyStr(Config->SmtpFailServer, CommandLineNext(Args));
				}
        else if (
            (strcmp(ptr,"--help")==0) ||
            (strcmp(ptr,"-help")==0) ||
            (strcmp(ptr,"-?")==0)
        ) PrintUsage();
        else if (
            (strcmp(ptr,"--version")==0) ||
            (strcmp(ptr,"-version")==0) ||
            (strcmp(ptr,"-v")==0)
        )	PrintVersion();
				else ListAddNamedItem(Docs, ptr, NULL);
		ptr=CommandLineNext(Args);
    }

    Destroy(Tempstr);
		return(Docs);
}




int ProcessFile(const char *Path)
{
    char *ExtnType=NULL, *FileMagicsType=NULL, *Tempstr=NULL, *ptr;
    TMimeItem *MimeOuter=NULL;
    STREAM *S;
    int ExitVal=RULE_NONE;


    S=STREAMOpen(Path, "r");
    if (S)
    {
				Tempstr=MCopyStr(Tempstr, GetBasename(Path), ".rewrite", NULL);
    		if (Config->Flags & FLAG_STRIP) g_Rewrite=STREAMOpen(Tempstr, "w");
				else g_Rewrite=NULL;

        ptr=strrchr(Path, '.');
        if (ptr) ExtnType=CopyStr(ExtnType, FileExtensionLookup(ptr));
        else ExtnType=CopyStr(ExtnType,"");
        FileMagicsType=FileMagicsExamine(FileMagicsType, S, NULL);

        Config->ExportPath=MCopyStr(Config->ExportPath, GetBasename((char *) Path),".dump/",NULL);

        if ((! StrValid(FileMagicsType)) && (! StrValid(ExtnType))) MimeOuter=MimeReadHeaders(S, TRUE);
        else MimeOuter=MimeItemCreate(Path,FileMagicsType,ExtnType);

        if (MimeOuter)
        {
            MimeOuter->Flags |= MIMEFLAG_ROOT;
            MimeOuter->FileName=CopyStr(MimeOuter->FileName, Path);
            DocTypeProcess(S, MimeOuter, Path);
            ExitVal=IsItSafe(MimeOuter);
            OutputItem(MimeOuter, MimeOuter,0, ExitVal==RULE_SAFE);
						if (g_Rewrite && StrValid(MimeOuter->Boundary)) 
						{
							Tempstr=MCopyStr(Tempstr, MimeOuter->Boundary, "--\r\n", NULL);
							STREAMWriteLine(Tempstr, g_Rewrite);
						}
        }
			
				STREAMClose(g_Rewrite);
        STREAMClose(S);

    }

    //if any were loaded for this document then clear them
    URLRulesClear(DocumentURLRules);

    if (MimeOuter) MimeItemDestroy(MimeOuter);
    Destroy(ExtnType);
    Destroy(FileMagicsType);
    Destroy(Tempstr);

    return(ExitVal);
}



int ProcessDocuments(ListNode *Docs)
{
    ListNode *Curr, *Node;
    struct stat Stat;
    int DocCount=0, Safe=0, Evil=0, Malformed=0;
    int ExitVal=RULE_NONE, i;

		Curr=ListGetNext(Docs);
		while (Curr)
    {
        if (StrValid(Curr->Tag))
        {
            if (stat(Curr->Tag, &Stat)==0)
            {
                if (S_ISREG(Stat.st_mode))
                {
                    ExitVal=ProcessFile(Curr->Tag);

                    if (ExitVal & RULE_EVIL) 
										{
											if (StrValid(Config->EvilDir)) FileMoveToDir(Curr->Tag, Config->EvilDir);
											Evil++;
										}
                    else if (ExitVal & RULE_SAFE) 
										{
											if (StrValid(Config->SafeDir)) FileMoveToDir(Curr->Tag, Config->SafeDir);
											Safe++;
										}

                    if (ExitVal & RULE_MALFORMED) Malformed++;
                    DocCount++;
                }
                else
                {
                    ExitVal=-2;
                    printf("ERROR: Not a regular file %s\n", Curr->Tag);
                }
            }
            else
            {
                ExitVal=-1;
                printf("ERROR: No such file %s\n", Curr->Tag);
            }

//		printf("exit: %d e: %d s: %d m:%d \n",ExitVal, ExitVal & RULE_EVIL, ExitVal & RULE_SAFE, ExitVal & RULE_MALFORMED);
        }
			Curr=ListGetNext(Curr);
    }

    printf("%d Documents considered. %d safe %d evil %d malformed\n",DocCount, Safe, Evil, Malformed);
    if (Evil > 0) ExitVal=RULE_EVIL;

    return(ExitVal);
}

#define KV_SIZE 100


int main(int argc, char *argv[])
{
    int ExitVal=RULE_NONE;
		ListNode *Docs;

		ConfigInit();
    g_KeyValueStore=MapCreate(KV_SIZE,LIST_FLAG_CACHE);
    FileMagicsLoadDefaults();
    FileExtensionsLoadDefaults();

#ifdef SYSCONFDIR
    ConfigPath=MCopyStr(ConfigPath,SYSCONFDIR,"/mimeguard.conf",NULL);
#endif

    Docs=ParseCommandLine(argc, argv);
    ConfigFileLoad(ConfigPath);

    if (Config->Flags & FLAG_SMTP) ExitVal=SmtpServer(Config->SmtpAddress);
    else ExitVal=ProcessDocuments(Docs);

		//the order of these is important, in case both RULE_EVIL and RULE_SAFE are set
    if (ExitVal & RULE_EVIL) exit(EvilExitStatus);
    if (ExitVal & RULE_SAFE) exit(0);
    if (ExitVal==RULE_NONE) exit(2);
    exit(3);
}
