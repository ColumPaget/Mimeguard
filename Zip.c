#include "Zip.h"
#include "FileTypeRules.h"
#include <wait.h>


/*

	General purpose bit flag:
Bit 00: encrypted file
Bit 01: compression option 
Bit 02: compression option 
Bit 03: data descriptor
Bit 04: enhanced deflation
Bit 05: compressed patched data
Bit 06: strong encryption
Bit 07-10: unused
Bit 11: language encoding
Bit 12: reserved
Bit 13: mask header values
Bit 14-15: reserved

	Compression method:
00: no compression
01: shrunk
02: reduced with compression factor 1
03: reduced with compression factor 2 
04: reduced with compression factor 3 
05: reduced with compression factor 4 
06: imploded
07: reserved
08: deflated
09: enhanced deflated
10: PKWare DCL imploded
11: reserved
12: compressed using BZIP2
13: reserved
14: LZMA
15-17: reserved
18: compressed using IBM TERSE
19: IBM LZ77 z
98: PPMd version I, Rev 1 
*/

#define ZIP_ENCRYPTED 1
#define ZIP_ENCRYPTED_STRONG 64

#define ECOD_SIZE 22

typedef struct
{
uint32_t sig;
uint16_t this_disk;
uint16_t start_disk;
uint16_t recs;
uint16_t total_recs;
uint32_t size;
uint32_t offset;
uint16_t comment_len;
} ZipEOCD;

#define ZCDH_SIZE 38

typedef struct
{
uint32_t sig;
uint16_t creator_version;
uint16_t min_version;
uint16_t flags;
uint16_t compression;
uint16_t mtime;
uint16_t mdate;
uint32_t crc;
uint32_t compressed_size;
uint32_t uncompressed_size;
uint16_t namelen;
uint16_t extralen;
uint16_t commentlen;
uint16_t start_disk;
uint16_t internal_attribs;
uint32_t offset; //we don't include this in the read from disk, due to structure packing issues
								 //so this isn't included in ZCDH_SIZE
} ZipCentDirHeader;



/*
0	4	Local file header signature = 0x04034b50 (read as a little-endian number)
4	2	Version needed to extract (minimum)
6	2	General purpose bit flag
8	2	Compression method
10	2	File last modification time
12	2	File last modification date
14	4	CRC-32
18	4	Compressed size
22	4	Uncompressed size
26	2	File name length (n)
28	2	Extra field length (m)
30	n	File name
30+n	m	Extra field
*/

#define ZLH_SIZE 30

typedef struct
{
uint32_t sig;
uint16_t min_version;
uint16_t flags;
uint16_t compression;
uint16_t mtime;
uint16_t mdate;
uint32_t crc;
uint32_t compressed_size;
uint32_t uncompressed_size;
uint16_t namelen;
uint16_t extralen;
} ZipLocalFileHeader;



void ZipReadCentralFileHeader(STREAM *S, ZipCentDirHeader *Head, char **FileName)
{
char *Tempstr=NULL;

STREAMReadBytes(S, Head, ZCDH_SIZE);
if (Head->sig == 0x02014b50)
{
	STREAMReadBytes(S, &(Head->offset), sizeof(uint32_t));
	STREAMReadBytes(S, &(Head->offset), sizeof(uint32_t));
	*FileName=SetStrLen(*FileName, Head->namelen * 2);
	memset(*FileName, 0, Head->namelen * 2);
	STREAMReadBytes(S, *FileName, Head->namelen);
	
	if (Head->extralen)
	{
	Tempstr=SetStrLen(Tempstr, Head->extralen * 2);
	memset(Tempstr, 0, Head->extralen * 2);
	STREAMReadBytes(S, Tempstr, Head->extralen);
	}
	
	if (Head->commentlen)
	{
	Tempstr=SetStrLen(Tempstr, Head->commentlen * 2);
	memset(Tempstr, 0, Head->commentlen * 2);
	STREAMReadBytes(S, Tempstr, Head->commentlen);
	}
}

DestroyString(Tempstr);
}


ZipEOCD *ZipReadCentralDirectory(STREAM *S)
{
ZipEOCD *head=NULL;
ZipCentDirHeader FileHead;
char *Tempstr=NULL;
char *iptr, *end;

Tempstr=SetStrLen(Tempstr, BUFSIZ);
STREAMSeek(S, 0-BUFSIZ, SEEK_END);
STREAMReadBytes(S, Tempstr, BUFSIZ);
iptr=Tempstr;
end=iptr+BUFSIZ;
for ( ;iptr < end; iptr++)
{
if (*(uint32_t *) iptr==0x06054b50) break;
} 

if (*(uint32_t *) iptr==0x06054b50)
{
head=(ZipEOCD *) calloc(1, sizeof(ZipEOCD));
memcpy(head, iptr, ECOD_SIZE);
//goto first entry in directory
STREAMSeek(S, head->offset, SEEK_SET);
}

DestroyString(Tempstr);

return(head);
}


void ZipFileExtract(STREAM *S, int Offset, const char *FileName, TMimeItem *Item)
{
TProcessingModule *Mod=NULL;
char *Tempstr=NULL, *Decomp=NULL;
STREAM *tmpFile;
int result, len;
long val, compressed_size, name_len, extra_len;

STREAMSeek(S, Offset, SEEK_SET);


STREAMReadUint32(S, &val); //sig
STREAMReadUint16(S, &val); //min_version
STREAMReadUint16(S, &val); //flags
STREAMReadUint16(S, &val); //compression
STREAMReadUint16(S, &val); //mtime
STREAMReadUint16(S, &val); //mdate
STREAMReadUint32(S, &val); //crc
STREAMReadUint32(S, &compressed_size); //compressed_size
STREAMReadUint32(S, &val); //uncompressed_size
STREAMReadUint16(S, &name_len); //namelen
STREAMReadUint16(S, &extra_len); //extralen

Tempstr=SetStrLen(Tempstr, name_len * 2);
STREAMReadBytes(S, Tempstr,name_len);

if (strcmp(Tempstr, FileName) !=0)
{
Item->RulesResult |= RULE_MALFORMED;
SetVar(Item->Errors,"malformed: local file name doesn't match directory header","");
}

if (extra_len > 0)
{
Tempstr=SetStrLen(Tempstr, extra_len * 2);
STREAMReadBytes(S, Tempstr, extra_len);
}

Mod=StandardDataProcessorCreate("decompress","zlib","");
printf("MOD: %d\n",Mod);
if (Mod)
{
tmpFile=STREAMOpen("tmp-XXXXXX","wt");
if (tmpFile)
{
	Tempstr=SetStrLen(Tempstr, BUFSIZ);
	result=STREAMReadBytes(S, Tempstr, BUFSIZ);
	while (result > 0)
	{
		len=result * 2;
		Decomp=SetStrLen(Decomp, len);
		result=Mod->Read(Mod,Tempstr,result,Decomp,&len,TRUE);
printf("wr: %d %d\n",result, len);
		STREAMWriteBytes(tmpFile, Decomp, len);
		result=STREAMReadBytes(S, Tempstr, BUFSIZ);
	}
STREAMFlush(tmpFile);
STREAMSeek(tmpFile, 0, SEEK_SET);
DocTypeProcess(tmpFile, Item, FileName);
}
}

DataProcessorDestroy(Mod);
DestroyString(Tempstr);
DestroyString(Decomp);
}



void ZipFileProcess(const char *Path, TMimeItem *Parent)
{
    char *Tempstr=NULL;
    const char *ptr;
    TMimeItem *Item;
    int result, i;
    STREAM *S;
		ZipEOCD *Head;
		ZipCentDirHeader FileHead;

S=STREAMOpen(Path, "r");
if (S)
{
		Head=ZipReadCentralDirectory(S);
		if (! Head)
		{
			Parent->RulesResult=RULE_MALFORMED;
			SetVar(Parent->Errors,"malformed: cannot read zip directory","");
		}
		else
		{
		for (i=0; i < Head->recs; i++) 
		{
			ZipReadCentralFileHeader(S, &FileHead, &Tempstr);
      if ((g_Flags & FLAG_DEBUG)) printf("         : [%s]\n",Tempstr);
      Item=MimeItemCreate(Tempstr,"","");
      ListAddItem(Parent->SubItems, Item);
			if (FileHead.flags & (ZIP_ENCRYPTED | ZIP_ENCRYPTED_STRONG)) Item->RulesResult=RULE_ENCRYPTED;
//			ZipFileExtract(S, FileHead.offset, Tempstr, Item);
			
		}
		Destroy(Head);
		}
	STREAMClose(S);
}


Destroy(Tempstr);
}
