#include "../libUseful.h"

#define MS_CLIENT_ID "a499c82f-64e2-4499-aaaa-3cfe2d156ef6"

OAUTH *Ctx=NULL;

void MSOAuthGet(OAUTH *Ctx)
{
char *Tempstr=NULL;
int result;

if (StrValid(Ctx->RefreshToken))
{
printf("REFERSH!\n");
OAuthRefresh(Ctx, "https://www.googleapis.com/oauth2/v4/token");
}
else
{
//Ctx->RedirectURI=CopyStr(Ctx->RedirectURI, "https://login.live.com/oauth20_desktop.srf");
Ctx->RedirectURI=CopyStr(Ctx->RedirectURI, "https://127.0.0.1/");
OAuthStage1(Ctx, "https://login.microsoftonline.com/common/oauth2/v2.0/authorize");
printf("GOTO: %s and type in code %s\n",Ctx->VerifyURL, Ctx->VerifyCode);

read(0,&result, 1);
OAuthFinalize(Ctx, "https://www.googleapis.com/oauth2/v4/token");
}

OAuthSave(Ctx, "/tmp/ms.oauth");

DestroyString(Tempstr);
}


void OAuthSet(HTTPInfoStruct *Info, OAUTH *Ctx)
{
char *Tempstr=NULL;

Tempstr=MCopyStr(Tempstr,"Bearer ",Ctx->AccessToken,NULL);
SetVar(Info->CustomSendHeaders,"Authorization", Tempstr);

DestroyString(Tempstr);
}



int GoogleTransact(const char *URL)
{
HTTPInfoStruct *Info;
char *Tempstr=NULL;
int result=FALSE;
STREAM *S;

Info=HTTPInfoFromURL("GET", "https://www.googleapis.com/oauth2/v2/userinfo");
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

HTTPSetFlags(HTTP_DEBUG);
//LibUsefulSetValue("HTTP:NoCompression","y");

Ctx=OAuthCreate(OAUTH_AUTHCODE,"mslive-account", MS_CLIENT_ID, "","files.readwrite.all");
if (! OAuthLoad(Ctx, "/tmp/ms.oauth", "mslive-account")) 
{
printf("OAUT LOAD FALSE\n");
MSOAuthGet(Ctx);
exit(1);
}

if (! GoogleTransact("https://www.googleapis.com/oauth2/v2/userinfo"))
{
	MSOAuthGet(Ctx);
	GoogleTransact("https://www.googleapis.com/oauth2/v2/userinfo");
}

}
