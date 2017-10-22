#include "../libUseful.h"

main(int argc, char *argv[])
{
OAUTH *Ctx;

HTTPSetFlags(HTTP_DEBUG);
Ctx=OAuthCreate("password","Mastodon","libUsefulTest","","read,write");
OAuthSetUserCreds(Ctx,"colum.paget@gmail.com","HFS3d6Hr5vN70JTsoKQHl+TcVQY=");
OAuthStage1(Ctx, "https://hostux.social/api/v1/apps");
OAuthFinalize(Ctx, "https://hostux.social/oauth/token");
}
