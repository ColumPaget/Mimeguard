#include "../libUseful.h"

#define DEVIANTART_CLIENTID "6827"
#define DEVIANTART_CLIENTSECRET "e031f3c2461ad733a6504c6a54ad86b3"

OAUTH *Ctx=NULL;
STREAM *StdIO;

void DeviantArtOAuthGet(OAUTH *Ctx)
{
char *Tempstr=NULL;
const char *ptr;
int fd, listenfd;
int result;

if (StrValid(Ctx->RefreshToken))
{
printf("REFERSH!\n");
OAuthRefresh(Ctx, "");
}
else
{
listenfd=IPServerInit(SOCK_STREAM, "", 8989);

SetVar(Ctx->Vars,"scope","browse");
SetVar(Ctx->Vars,"redirect_uri","http://localhost:8989/deviantart.callback");
OAuthStage1(Ctx, "https://www.deviantart.com/oauth2/authorize");
printf("GOTO: %s and then copy URL back here\n",Ctx->VerifyURL);

//STREAMSetTimeout(StdIO, 0);
//Tempstr=STREAMReadLine(Tempstr, StdIO);

listen(listenfd, 10);
OAuthConnectBack(Ctx, listenfd);
OAuthFinalize(Ctx, "https://www.deviantart.com/oauth2/token");
}

OAuthSave(Ctx, "/tmp/deviantart.oauth");
DestroyString(Tempstr);
}


void OAuthSet(HTTPInfoStruct *Info, OAUTH *Ctx)
{
char *Tempstr=NULL;

Tempstr=MCopyStr(Tempstr,"Bearer ",Ctx->AccessToken,NULL);
SetVar(Info->CustomSendHeaders,"Authorization", Tempstr);

DestroyString(Tempstr);
}



int DeviantArtTransact(const char *URL)
{
HTTPInfoStruct *Info;
char *Tempstr=NULL;
int result=FALSE;
STREAM *S;

Info=HTTPInfoFromURL("GET", URL);
OAuthSet(Info, Ctx);
S=HTTPTransact(Info);
if (S)
{
	Tempstr=STREAMReadDocument(Tempstr, S);
	printf("%s\n",Tempstr);
	STREAMClose(S);
	result=TRUE;
}

DestroyString(Tempstr);

return(result);
}


main(int argc, char *argv[])
{
char *Tempstr=NULL;

StdIO=STREAMFromDualFD(0,1);
HTTPSetFlags(HTTP_DEBUG);
//LibUsefulSetValue("HTTP:NoCompression","y");
LibUsefulSetValue("SSL:Level","ssl");


Ctx=OAuthCreate("auth","deviantart", DEVIANTART_CLIENTID, DEVIANTART_CLIENTSECRET, "basic");
if (! OAuthLoad(Ctx, "/tmp/deviantart.oauth", "deviantart")) 
{
printf("OAUTH LOAD FALSE\n");
DeviantArtOAuthGet(Ctx);
}


if (! DeviantArtTransact("https://www.deviantart.com/api/v1/oauth2/browse/dailydeviations"))
{
//	PocketOAuthGet(Ctx);
//	PocketTransact(Tempstr);
}

DestroyString(Tempstr);
}
