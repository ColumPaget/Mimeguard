#ifndef FILEFILTER_RTF_H
#define FILEFILTER_RTF_H

#include "common.h"
#include "Mime.h"

#define RTF_MAGIC "{\\rtf1"

int RTFFileProcess(const char *Path, TMimeItem *Parent);

#endif

