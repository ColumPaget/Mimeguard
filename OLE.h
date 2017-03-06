#ifndef FILEFILTER_OLE_H
#define FILEFILTER_OLE_H

#include "common.h"
#include "Mime.h"

#define OLE_MAGIC "\xd0\xcf\x11\xe0\xa1\xb1\x1a\xe1"

int OLEFileProcess(const char *Path, TMimeItem *Parent);

#endif

