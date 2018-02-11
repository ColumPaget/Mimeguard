#ifndef FILEGUARD_H
#define FILEGUARD_H

#include "common.h"

typedef struct
{
int Flags;
char *FileName;
char *ContentType;
char *FileMagicsType;
char *ExtnType;
char *Disposition;
char *Boundary;
int RulesResult;
ListNode *Headers;
ListNode *SubItems;
ListNode *Errors;
} TMimeItem;

#define MIMEFLAG_ROOT 1
#define MIMEFLAG_QUOTEDPRINTABLE 32
#define MIMEFLAG_BASE64 64

#define ERROR_BASIC 1
#define ERROR_STRING 2
#define ERROR_URL 3

#define MIMEFLAG_ENCODING (MIMEFLAG_QUOTEDPRINTABLE | MIMEFLAG_BASE64)

TMimeItem *MimeItemCreate(const char *FileName, const char *ContentType, const char *FileMagicsType);
void MimeItemDestroy(void *pItem);
char *MimeItemGetContentType(TMimeItem *Item);
TMimeItem *MimeReadHeaders(STREAM *S);
void MimeReadMultipart(STREAM *S, TMimeItem *Outer);
STREAM *MimeReadDocument(STREAM *S, TMimeItem *Item, const char *Boundary, char **SavePath);

#endif
