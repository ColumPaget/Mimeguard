#include "Smtp.h"
#include "Mime.h"
#include "DocumentTypes.h"
#include "FileTypeRules.h"
#include <wait.h>
#include <glob.h>

const char *SmtpCommands[]= {"HELO ", "EHLO", "MAIL FROM:", "RCPT TO:", "DATA", "QUIT", NULL};
typedef enum {SMTP_HELO, SMTP_EHLO, SMTP_MAILFROM, SMTP_RCPTTO, SMTP_DATA, SMTP_QUIT} ESmtpCommands;

#define SMTP_REJECT 1

char *SmtpPassDir=NULL;
char *SmtpFailDir=NULL;
char *SmtpPassServer=NULL;
char *SmtpFailServer=NULL;
char *SmtpBanner=NULL;
char *SmtpFailRedirect=NULL;
int SmtpFlags=0;


void SmtpConfig(const char *Setting, const char *Value)
{
    if (strcmp(Setting, "SmtpPassDir")==0) SmtpPassDir=CopyStr(SmtpPassDir, Value);
    if (strcmp(Setting, "SmtpFailDir")==0) SmtpFailDir=CopyStr(SmtpFailDir, Value);
    if (strcmp(Setting, "SmtpPassServer")==0) SmtpPassServer=CopyStr(SmtpPassServer, Value);
    if (strcmp(Setting, "SmtpFailServer")==0) SmtpFailServer=CopyStr(SmtpFailServer, Value);
    if (strcmp(Setting, "SmtpFailRedirect")==0) SmtpFailRedirect=CopyStr(SmtpFailRedirect, Value);
    if (strcmp(Setting, "SmtpBanner")==0) SmtpBanner=CopyStr(SmtpBanner, Value);
    if ((strcmp(Setting, "SmtpRejectFails")==0) && (strtobool(Value))) SmtpFlags |= SMTP_REJECT;
}


void SmtpFileReadHeaders(const char *Path, char **Sender, char **Recipients)
{
    STREAM *S;
    char *Tempstr=NULL, *Token=NULL;
    const char *ptr;

    *Sender=CopyStr(*Sender, "");
    *Recipients=CopyStr(*Recipients, "");
    S=STREAMOpen(Path, "r");
    if (S)
    {
        Tempstr=STREAMReadLine(Tempstr, S);
        while (Tempstr)
        {
            StripTrailingWhitespace(Tempstr);

            if (! StrValid(Tempstr)) break;
            ptr=GetToken(Tempstr,":", &Token, 0);
            while (ptr && isspace(*ptr)) ptr++;

            if (strcmp(Token,"X-Envelope-From")==0) *Sender=CopyStr(*Sender, ptr);
            if (strcmp(Token,"X-Envelope-To")==0) *Recipients=MCatStr(*Recipients, ptr, ", ", NULL);
            Tempstr=STREAMReadLine(Tempstr, S);
        }
        STREAMClose(S);
    }

    DestroyString(Tempstr);
    DestroyString(Token);
}


void SmtpForwardFiles(const char *Server, const char *Dir, char *To)
{
    glob_t Glob;
    char *Tempstr=NULL, *Sender=NULL, *Recipients=NULL;
    int i;

    LibUsefulSetValue("SMTP:Server", Server);

    Tempstr=MCopyStr(Tempstr, Dir, "/*",NULL);
    glob(Tempstr, 0, 0, &Glob);
    for (i=0; i < Glob.gl_pathc; i++)
    {
        SmtpFileReadHeaders(Glob.gl_pathv[i], &Sender, &Recipients);
        if (StrValid(To)) Recipients=CopyStr(Recipients, To);
        if (SMTPSendMailFile(Sender, Recipients, Glob.gl_pathv[i], 0)) unlink(Glob.gl_pathv[i]);
    }
    globfree(&Glob);

    DestroyString(Sender);
    DestroyString(Tempstr);
    DestroyString(Recipients);
}


void SmtpPostProcessFile(const char *Path, int result)
{
    char *Tempstr=NULL;

    if (result & RULE_EVIL)
    {
        if (StrValid(SmtpFailDir))
        {
            Tempstr=MCopyStr(Tempstr, SmtpFailDir, "/", GetBasename(Path), NULL);
            MakeDirPath(Tempstr, 0700);
            rename(Path, Tempstr);
        }
    }
    else
    {
        if (StrValid(SmtpPassDir))
        {
            Tempstr=MCopyStr(Tempstr, SmtpPassDir, "/", GetBasename(Path), NULL);
            MakeDirPath(Tempstr, 0700);
            rename(Path, Tempstr);
        }
    }



    DestroyString(Tempstr);
}


void SmtpProcessQueue()
{
    if (StrValid(SmtpPassServer)) SmtpForwardFiles(SmtpPassServer, SmtpPassDir, "");
    if (StrValid(SmtpFailServer)) SmtpForwardFiles(SmtpFailServer, SmtpFailDir, SmtpFailRedirect);
}


int SmtpProcessDataCommand(STREAM *S, const char *Sender, const char *Recipients)
{
    char *Tempstr=NULL, *Path=NULL;
    STREAM *tmpS;
    TMimeItem *MimeOuter=NULL;
    int Result=FALSE;

    Path=CopyStr(Path, getenv("TMPDIR"));
    if (! StrValid(Path)) Path=CopyStr(Path, "/tmp/");

    Tempstr=FormatStr(Tempstr, "/%x%x.mail", time(NULL), getpid());
    Path=CatStr(Path, Tempstr);
    tmpS=STREAMOpen(Path, "w");
    Tempstr=MCopyStr(Tempstr,"X-Envelope-From: ",Sender, "\r\n", NULL);
    STREAMWriteLine(Tempstr, tmpS);
    Tempstr=MCopyStr(Tempstr,"X-Envelope-To: ",Recipients, "\r\n", NULL);
    STREAMWriteLine(Tempstr, tmpS);

//Read Headers
    Tempstr=STREAMReadLine(Tempstr, S);
    while (Tempstr)
    {
//don't copy these values if they're set in the document we're reading from
        if (strncasecmp(Tempstr,"X-Envelope-From:",16)==0) /*do nothing */ ;
        else if (strncasecmp(Tempstr,"X-Envelope-To:",14)==0) /*do nothing */ ;
        else STREAMWriteLine(Tempstr, tmpS);

//blank line ends headers, but we must check AFTER having written the line
        StripTrailingWhitespace(Tempstr);
        if (! StrValid(Tempstr)) break;

        Tempstr=STREAMReadLine(Tempstr, S);
    }

//Read Message Body
    Tempstr=STREAMReadLine(Tempstr, S);
    while (Tempstr)
    {
        if ( (strcmp(Tempstr,".\r\n")==0) || (strcmp(Tempstr, ".\n")==0)) break;
        STREAMWriteLine(Tempstr, tmpS);
        Tempstr=STREAMReadLine(Tempstr, S);
    }
    STREAMClose(tmpS);

    tmpS=STREAMOpen(Path, "r");
    MimeOuter=MimeReadHeaders(tmpS);
    if (MimeOuter)
    {
        MimeOuter->Flags |= MIMEFLAG_ROOT;
        MimeOuter->FileName=CopyStr(MimeOuter->FileName, Path);
        DocTypeProcess(tmpS, MimeOuter, Path);
        Result=IsItSafe(MimeOuter);
        MimeItemDestroy(MimeOuter);
    }
    STREAMClose(tmpS);

    SmtpPostProcessFile(Path, Result);
//if it wasn't moved by SmtpPostProcessFile then it's going to be deleted
    unlink(Path);

    DestroyString(Tempstr);
    DestroyString(Path);

    return(Result);
}



pid_t SmtpHandleConnection(STREAM *S)
{
    char *Tempstr=NULL, *Token=NULL;
    char *Sender=NULL, *Recipients=NULL;
    const char *ptr;
    int val, More=TRUE;

    if (StrValid(SmtpBanner))
    {
        Tempstr=MCopyStr(Tempstr, "220 ", SmtpBanner, "\r\n",NULL);
        STREAMWriteLine("220 OKAY\r\n", S);
    }
    else STREAMWriteLine("220 OKAY\r\n", S);

    Tempstr=STREAMReadLine(Tempstr, S);
    while (More && Tempstr)
    {
        if (StrValid(Tempstr))
        {
            StripTrailingWhitespace(Tempstr);
            val=MatchTokenFromList(Tempstr, SmtpCommands,  MATCH_TOKEN_PART);
            switch (val)
            {
            case SMTP_HELO:
            case SMTP_EHLO:
                STREAMWriteLine("220 OKAY\r\n", S);
                break;

            case SMTP_MAILFROM:
                Sender=CopyStr(Sender, Tempstr+StrLen(SmtpCommands[val]));
                StripLeadingWhitespace(Sender);
                StripTrailingWhitespace(Sender);
                STREAMWriteLine("250 OKAY\r\n", S);
                break;

            case SMTP_RCPTTO:
                Recipients=CopyStr(Recipients, Tempstr+StrLen(SmtpCommands[val]));
                StripLeadingWhitespace(Recipients);
                StripTrailingWhitespace(Recipients);
                STREAMWriteLine("250 OKAY\r\n", S);
                break;

            case SMTP_DATA:
                STREAMWriteLine("354 CONTINUE\r\n", S);
                if (SmtpProcessDataCommand(S, Sender, Recipients) == RULE_SAFE) STREAMWriteLine("250 OKAY\r\n", S);
                else if (SmtpFlags & SMTP_REJECT) STREAMWriteLine("550 FAIL\r\n", S);
                else STREAMWriteLine("250 OKAY\r\n", S);
                break;

            case SMTP_QUIT:
                STREAMWriteLine("250 OKAY\r\n", S);
                STREAMFlush(S);
                More=FALSE;
                break;

            default:
                STREAMWriteLine("502 Unrecognized command\r\n", S);
                break;
            }
        }

        if (More) Tempstr=STREAMReadLine(Tempstr, S);
    }

    DestroyString(Recipients);
    DestroyString(Tempstr);
    DestroyString(Sender);
    DestroyString(Token);
    STREAMClose(S);

    _exit(0);
}


void SigHandler(int sig)
{
}

void SetupSigChild()
{
    struct sigaction sa;

    sa.sa_handler=SigHandler;
    sa.sa_flags=SA_NOCLDSTOP;
    sigemptyset (&sa.sa_mask);

    sigaction(SIGCHLD, &sa, NULL);
}


int SmtpServer(const char *URL)
{
    STREAM *S, *Server;
    pid_t pid;
    int RunQueue, result;

    Server=STREAMServerInit(URL);
    if (! Server)
    {
        RaiseError(ERRFLAG_ERRNO, "SmtpServer", "Failed to bind %s",URL);
        exit(1);
    }

    SetupSigChild();

    while (1)
    {
        S=STREAMServerAccept(Server);
        if (S)
        {
            pid=xfork("");
            if (pid==0) SmtpHandleConnection(S);
        }

        RunQueue=FALSE;
        result=waitpid(-1,NULL,WNOHANG);
        while (result > 0)
        {
            RunQueue=TRUE;
            result=waitpid(-1,NULL,WNOHANG);
        }

        if (RunQueue) SmtpProcessQueue();
        STREAMClose(S);
    }
}
