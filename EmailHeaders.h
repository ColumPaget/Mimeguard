
#ifndef MIMEGUARD_EMAIL_HEADERS_H
#define MIMEGUARD_EMAIL_HEADERS_H

#include "common.h"
#include "Mime.h"

void EmailHeaderRulesAdd(const char *Header, const char *Value, const char *Action);
void EmailHeaderRulesConsider(TMimeItem *Item, const char *Header, const char *Value);

#endif
