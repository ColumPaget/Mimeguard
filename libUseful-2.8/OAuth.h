#ifndef LIBUSEFUL_OAUTH_H
#define LIBUSEFUL_OAUTH_H

/*
OAuth is not really a standard, most sites seem to implement it differently. These functions help with
implementing OAuth 2.0 authentication
*/


#include "includes.h"

#define OAUTH_IMPLICIT 1
#define OAUTH_STDIN 2

typedef struct
{
int Flags;
char *Name;
char *Stage1;
char *Stage2;
char *VerifyTemplate;
char *AccessToken;
char *RefreshToken;
char *RefreshURL;
char *VerifyURL;
char *VerifyCode;
char *Creds;
char *SavePath;
ListNode *Vars;
} OAUTH;


#ifdef __cplusplus
extern "C" {
#endif


OAUTH *OAuthCreate(const char *Type, const char *Name, const char *ClientID, const char *ClientSecret, const char *Scopes, const char *RefreshURL);
void OAuthDestroy(void *p_OAUTH);

int OAuthParseReply(OAUTH *Ctx, const char *ContentType, const char *Reply);

int OAuthRegister(OAUTH *Ctx, const char *URL);
int OAuthSave(OAUTH *Ctx, const char *Path);
int OAuthLoad(OAUTH *Ctx, const char *Name, const char *Path);

int OAuthRefresh(OAUTH *Ctx, const char *URL);

int OAuthStage1(OAUTH *Ctx, const char *URL);
int OAuthFinalize(OAUTH *Ctx, const char *URL);
void OAuthSetUserCreds(OAUTH *Ctx, const char *UserName, const char *Password);

int OAuthConnectBack(OAUTH *Ctx, int sock);
int OAuthListen(OAUTH *Ctx, int Port, const char *URL, int Flags);

#ifdef __cplusplus
}
#endif

#endif

