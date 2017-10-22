#include "../libUseful.h"

#define POCKET_ID "64045-7bc88d0cc96a4215df7a41c5"

OAUTH *Ctx=NULL;

void PocketOAuthGet(OAUTH *Ctx)
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
OAuthStage1(Ctx, "https://getpocket.com/v3/oauth/request");
printf("GOTO: %s and type in code %s\n",Ctx->VerifyURL, Ctx->VerifyCode);

read(0,&result, 1);
OAuthFinalize(Ctx, "https://getpocket.com/v3/oauth/authorize");
}

OAuthSave(Ctx, "/tmp/pocket.oauth");

DestroyString(Tempstr);
}


void OAuthSet(HTTPInfoStruct *Info, OAUTH *Ctx)
{
char *Tempstr=NULL;

Tempstr=MCopyStr(Tempstr,"Bearer ",Ctx->AccessToken,NULL);
SetVar(Info->CustomSendHeaders,"Authorization", Tempstr);

DestroyString(Tempstr);
}



int PocketTransact(const char *URL)
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

HTTPSetFlags(HTTP_DEBUG);
//LibUsefulSetValue("HTTP:NoCompression","y");

Ctx=OAuthCreate("getpocket.com","pocket", POCKET_ID, "","");
if (! OAuthLoad(Ctx, "/tmp/pocket.oauth", "pocket")) 
{
printf("OAUT LOAD FALSE\n");
PocketOAuthGet(Ctx);
}

Tempstr=MCopyStr(Tempstr, "https://getpocket.com/v3/get?consumer_key=", POCKET_ID,"&access_token=", Ctx->AccessToken, "&detailType=complete&sort=oldest",NULL);
if (! PocketTransact(Tempstr))
{
	PocketOAuthGet(Ctx);
	PocketTransact(Tempstr);
}

DestroyString(Tempstr);
}
