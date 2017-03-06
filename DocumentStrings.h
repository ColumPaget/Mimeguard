#ifndef FILEFILTER_DS_H
#define FILEFILTER_DS_H

#include "common.h"

void DocumentStringsAdd(const char *DocType, const char *String, int Flags);
ListNode *DocumentStringsGetList(const char *DocType);
int DocumentStringsCheck(ListNode *Items, const char *String);
void DocumentStringsClearOverrides();

#endif
