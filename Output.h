#ifndef FILEFILTER_DS_H
#define FILEFILTER_DS_H

#include "common.h"
#include "Mime.h"

void OutputHeaders(TMimeItem *Item);
void OutputItem(TMimeItem *Top, TMimeItem *Item, int Level, int PrevVal);


#endif
