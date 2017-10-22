#ifndef FILEGUARD_MAGIC_H
#define FILEGUARD_MAGIC_H

#include "common.h"

void FileMagicsAdd(const char *FileMagics, char *ContentType);
const char *FileMagicsLookupContentType(char *Data);
void FileMagicsLoadFile(const char *Path);
void FileMagicsLoadDefaults();

#endif
