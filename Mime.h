#ifndef FILEGUARD_H
#define FILEGUARD_H

#include "common.h"

typedef struct
{
int Flags;
char *FileName;
char *ContentType;
char *MagicType;
char *ExtnType;
char *Disposition;
char *Boundary;
int RulesResult;
char *ResultInfo;
ListNode *Headers;
ListNode *SubItems;
} TMimeItem;

#define MIMEFLAG_ROOT 1
#define MIMEFLAG_QUOTEDPRINTABLE 32
#define MIMEFLAG_BASE64 64

#define MIMEFLAG_ENCODING (MIMEFLAG_QUOTEDPRINTABLE | MIMEFLAG_BASE64)

TMimeItem *MimeItemCreate(const char *FileName, const char *ContentType, const char *MagicType);
void MimeItemDestroy(void *pItem);
TMimeItem *MimeReadHeaders(STREAM *S);
void MimeReadMultipart(STREAM *S, TMimeItem *Outer);


#endif
