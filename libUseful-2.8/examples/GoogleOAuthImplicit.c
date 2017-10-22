#include "../libUseful.h"

#define GOOGLE_CLIENT_ID "596195748702.apps.googleusercontent.com" 
#define GOOGLE_CLIENT_SECRET "YDYv4JZMI3umD80S7Xh_WNJV"


OAUTH *Ctx=NULL;

void GoogleOAuthGet(OAUTH *Ctx)
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
OAuthStage1(Ctx, "https://accounts.google.com/o/oauth2/device/code");
printf("GOTO: %s and type in code %s\n",Ctx->VerifyURL, Ctx->VerifyCode);

read(0,&result, 1);
OAuthFinalize(Ctx, "https://www.googleapis.com/oauth2/v4/token");
}

OAuthSave(Ctx, "/tmp/google.oauth");

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
if (strcmp(Info->ResponseCode,"200") ==0)
{
	printf("%s\n",Tempstr);
	STREAMClose(S);
	result=TRUE;
}
}

DestroyString(Tempstr);

return(result);
}


main(int argc, char *argv[])
{

HTTPSetFlags(HTTP_DEBUG);
//LibUsefulSetValue("HTTP:NoCompression","y");

Ctx=OAuthCreate("device","google-account", GOOGLE_CLIENT_ID, GOOGLE_CLIENT_SECRET,"email profile");
if (! OAuthLoad(Ctx, "/tmp/google.oauth", "google-account")) 
{
printf("OAUT LOAD FALSE\n");
//GoogleOAuthGet(Ctx);
}

if (! GoogleTransact("https://www.googleapis.com/oauth2/v2/userinfo"))
{
	GoogleOAuthGet(Ctx);
	GoogleTransact("https://www.googleapis.com/oauth2/v2/userinfo");
}

}
