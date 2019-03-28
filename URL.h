#ifndef MIMEGUARD_URL_H
#define MIMEGUARD_URL_H

#include "common.h"
#include "Mime.h"


int URLRuleCheck(TMimeItem *Item, const char *URL);
void URLParseRule(const char *Rule);

#endif
