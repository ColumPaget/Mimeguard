#ifndef FILEGUARD_TYPERULES_H
#define FILEGUARD_TYPERULES_H

#include "common.h"
#include "Mime.h"

#define RULE_NONE 0
#define RULE_SAFE 1
#define RULE_EVIL 2
#define RULE_STRIP 4
#define RULE_MISMATCH  8
#define RULE_BLANK_CONTYPE 16
#define RULE_BLANK_MAGIC 32
#define RULE_EXTN_MATCHES_MAGIC 64
#define RULE_CONTYPE_MATCHES_MAGIC 128
#define RULE_HEADER 256
#define RULE_OVERRIDE 512
#define RULE_ECHO 1024
#define RULE_FILENAME 2048
#define RULE_MACROS 4096
#define RULE_MALFORMED 8192
#define RULE_EMPTY 16384
#define RULE_ENCRYPTED 32768
#define RULE_ALLOW_ENCRYPTED 65536
#define RULE_ALLOW_MACROS 131072
#define RULE_ALLOW_EMPTY  262144
#define RULE_CONTAINER 524288

#define RULE_MASK (RULE_SAFE|RULE_EVIL|RULE_MISMATCH)

void FileRulesAdd(const char *Type, int Flags, const char *Contains, const char *Equivalent);
void FileTypeRuleParse(const char *Data, int Flags);
void FileExtnRuleParse(const char *Data);
int FileRulesConsider(TMimeItem *Item);
void FileRulesLoadPostProcess();
void HeaderRulesConsider(TMimeItem *Item, const char *Header, const char *Value);
const char *TranslateMimeTypeEquivalent(const char *MimeType);

int IsItSafe(TMimeItem *Item);

#endif
