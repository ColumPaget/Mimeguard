#ifndef MIMEGUARD_COMMON_H
#define MIMEGUARD_COMMON_H

#include "libUseful-3/libUseful.h"

#define FLAG_EXPORT 1
#define FLAG_SHOW_SAFE 2
#define FLAG_SHOW_EVIL 4
#define FLAG_SHOW_CURR 8
#define FLAG_SMTP 16
#define FLAG_DEBUG 8192
#define FLAG_STRIP 16384

typedef enum {KV_FILE_EXTN, KV_DOCSTRINGS, KV_DOCSTRINGS_OVERRIDE, KV_EQUIV_MIMETYPE, KV_IPREGION, KV_IP} EKeyValueTypes;


typedef struct
{
int Flags;
char *SmtpAddress;
char *ConfigPath;
char *ExportPath;
char *SafeDir;
char *EvilDir;
char *SmtpPassServer;
char *SmtpFailServer;
char *SmtpBanner;
char *SmtpFailRedirect;
int SmtpFlags;
} TConfig;

extern TConfig *Config;

extern STREAM *g_Rewrite;
extern ListNode *g_KeyValueStore;

void ConfigInit();
char *DecodeMailText(char *RetStr, const char *Text);
int DecodeDocumentLine(const char *Line, int Encoding, char **Data);
char *FileListExpand(char *RetStr, const char *FilesList);

#endif
