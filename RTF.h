#ifndef MIMEGUARD_RTF_H
#define MIMEGUARD_RTF_H

#include "common.h"
#include "Mime.h"

#define RTF_MAGIC "{\\\\rtf1"

int RTFFileProcess(const char *Path, TMimeItem *Parent);

#endif

