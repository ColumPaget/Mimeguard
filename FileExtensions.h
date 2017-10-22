#ifndef FILEGUARD_EXTN_H
#define FILEGUARD_EXTN_H

#include "common.h"

void FileExtensionsAdd(const char *Extn, const char *MimeType);
void FileExtensionsLoad(const char *Path);
void FileExtensionsLoadDefaults();
const char *FileExtensionLookup(const char *Extn);

#endif
