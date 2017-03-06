#ifndef FILEGUARD_COMMON_H
#define FILEGUARD_COMMON_H

#include "libUseful-2.8/libUseful.h"

#define FLAG_EXPORT 1
#define FLAG_SHOW_SAFE 2
#define FLAG_SHOW_EVIL 4
#define FLAG_SHOW_CURR 8
#define FLAG_DEBUG 8192

#define MIMEENC_BASE64 1
#define MIMEENC_QUOTEDPRINTABLE 2

extern int Flags;
extern char *ExportPath;

int DecodeDocumentLine(const char *Line, int Encoding, char **Data);


#endif
