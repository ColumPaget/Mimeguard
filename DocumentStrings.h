#ifndef MIMEGUARD_DS_H
#define MIMEGUARD_DS_H

#include "common.h"

void DocumentStringsAdd(const char *DocType, const char *String, int Flags);
ListNode *DocumentStringsGetList(const char *DocType);
int DocumentStringsCheck(ListNode *Items, const char *String);

#endif
