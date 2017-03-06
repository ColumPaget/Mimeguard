#ifndef FILEGUARD_H
#define FILEGUARD_H

#include "common.h"

typedef struct
{
char *FileName;
char *ContentType;
char *MagicType;
char *ExtnType;
char *Disposition;
char *Boundary;
int Encoding;
int RulesResult;
char *ResultInfo;
ListNode *Headers;
ListNode *SubItems;
} TMimeItem;


TMimeItem *MimeItemCreate(const char *FileName, const char *ContentType, const char *MagicType);
void MimeItemDestroy(void *pItem);
TMimeItem *MimeReadHeaders(STREAM *S);
void MimeReadMultipart(STREAM *S, TMimeItem *Outer);


#endif
