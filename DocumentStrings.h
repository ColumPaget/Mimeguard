#ifndef MIMEGUARD_DS_H
#define MIMEGUARD_DS_H

#include "common.h"

void DocumentStringsAdd(const char *DocType, const char *String, int Flags);
const char *DocumentStringsGetList(const char *DocType);
int DocumentStringsCheck(const char *List, const char *String);

#endif
