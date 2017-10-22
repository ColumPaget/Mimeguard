#ifndef MIMEGUARD_URL_H
#define MIMEGUARD_URL_H

#include "common.h"
#include "Mime.h"


void URLRuleAdd(int Type, const char *Arg);
int URLRuleCheck(TMimeItem *Item, const char *URL);
void URLParseRule(const char *Rule);

#endif
