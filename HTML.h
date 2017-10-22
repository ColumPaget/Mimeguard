#ifndef MIMEGUARD_HTML_H
#define MIMEGUARD_HTML_H

#include "common.h"
#include "Mime.h"

#define HTML_MAGIC "<html"

int HTMLFileProcess(const char *Path, TMimeItem *Parent);

#endif

