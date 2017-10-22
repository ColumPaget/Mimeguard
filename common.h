#ifndef MIMEGUARD_COMMON_H
#define MIMEGUARD_COMMON_H

#include "libUseful-2.8/libUseful.h"

#define FLAG_EXPORT 1
#define FLAG_SHOW_SAFE 2
#define FLAG_SHOW_EVIL 4
#define FLAG_SHOW_CURR 8
#define FLAG_DEBUG 8192

typedef enum {KV_FILE_EXTN, KV_DOCSTRINGS, KV_DOCSTRINGS_OVERRIDE, KV_EQUIV_MIMETYPE, KV_IPREGION} EKeyValueTypes;

extern int g_Flags;
extern STREAM *g_Rewrite;
extern ListNode *g_KeyValueStore;

extern char *ExportPath;

char *DecodeMailText(char *RetStr, const char *Text);
int DecodeDocumentLine(const char *Line, int Encoding, char **Data);
char *FileListExpand(char *RetStr, const char *FilesList);

#endif
