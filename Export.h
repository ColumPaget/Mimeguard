#ifndef FILEGUARD_EXPORT_H
#define FILEGUARD_EXPORT_H

#include "common.h"
#include "Mime.h"

int ExportOpen(TMimeItem *Item, char **SavePath);
void RewriteCopyDocument(TMimeItem *Outer, TMimeItem *Item, const char *Path);

#endif
