#ifndef FILEGUARD_MAGIC_H
#define FILEGUARD_MAGIC_H

#include "common.h"

void MagicAdd(const char *Magic, char *ContentType);
const char *MagicLookupContentType(char *Data);
void MagicLoadFile(const char *Path);
void MagicLoadDefaults();

#endif
