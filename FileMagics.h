#ifndef FILEGUARD_MAGIC_H
#define FILEGUARD_MAGIC_H

#include "common.h"

void FileMagicsAdd(const char *FileMagics, char *ContentType);
const char *FileMagicsLookupContentType(char *Data);
void FileMagicsLoadFile(const char *Path);
void FileMagicsLoadDefaults();
int FileMagicsIsText(const char *Data);
char *FileMagicsExamine(char *RetStr, STREAM *S, int *IsText);

#endif
