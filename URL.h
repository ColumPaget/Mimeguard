#ifndef MIMEGUARD_URL_H
#define MIMEGUARD_URL_H

#include "common.h"
#include "Mime.h"


extern ListNode *GlobalURLRules;
extern ListNode *DocumentURLRules;


int URLRuleCheck(TMimeItem *Item, const char *URL);
void URLParseRule(ListNode *URLRules, const char *Rule);
void URLRulesClear(ListNode *URLRules);

#endif
