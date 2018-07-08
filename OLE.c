#include "OLE.h"
#include "FileTypeRules.h"
#include <math.h>

// From Wikipedia https://en.wikipedia.org/wiki/Compound_File_Binary_Format

#define FREESECT 0xFFFFFFFF	// denotes an unused sector
#define ENDOFCHAIN 0xFFFFFFFE	// marks the last sector in a FAT chain
#define FATSECT 0xFFFFFFFD	// marks a sector used to store part of the FAT
#define DIFSECT 0xFFFFFFFC	// marks a sector used to store part of the DIFAT

typedef struct
{
    char FileSig[8];            // [00H,08] {0xd0, 0xcf, 0x11, 0xe0, 0xa1, 0xb1, 0x1a, 0xe1} for current version
    char CLSID[16];             // [08H,16] reserved must be zero (WriteClassStg GetClassFile uses root directory class id)
    uint16_t MinorVersion;      // [18H,02] minor version of the format: 33 is
    uint16_t DllVersion;        // [1AH,02] major version of the dll/format: 3 for 512-byte sectors, 4 for 4 KB sectors
    uint16_t ByteOrder;         // [1CH,02] 0xFFFE: indicates Intel byte-ordering
    uint16_t SectorShift;       // [1EH,02] size of sectors in power-of-two typically 9 indicating 512-byte sectors
    uint16_t MiniSectorShift;   // [20H,02] size of mini-sectors in power-of-two typically 6 indicating 64-byte mini-sectors
    uint16_t Reserved1;         // [22H,02] reserved, must be zero
    uint32_t Reserved2;         // [24H,04] reserved, must be zero
    uint32_t DirSects;          // [28H,04] must be zero for 512-byte sectors number of SECTs in directory chain for 4 KB sectors
    uint32_t FatSects;          // [2CH,04] number of SECTs in the FAT chain
    uint32_t sectDirStart;      // [30H,04] first SECT in the directory chain
    uint32_t signature;         // [34H,04] signature used for transactions; must be zero.
    uint32_t lMiniSectorCutoff; // [38H,04] maximum size for a mini stream typically 4096 bytes
    uint32_t sectMiniFatStart;  // [3CH,04] first SECT in the MiniFAT chain
    uint32_t sectMiniFat;       // [40H,04] number of SECTs in the MiniFAT chain
    uint32_t sectDifStart;      // [44H,04] first SECT in the DIFAT chain
    uint32_t sectDif;           // [48H,04] number of SECTs in the DIFAT chain
    uint32_t sectFat[109];      // [4CH,436] the SECTs of first 109 FAT sectors
} TOLEHeader;


typedef struct
{
    char Name[64];
    uint16_t NameLen;
    uint8_t Type;
    uint8_t Color;
    uint32_t LeftDirID;
    uint32_t RightDirID;
    uint32_t RootDirID;
    char UniqueID[16];
    uint32_t Flags;
    uint64_t CreationTime;
    uint64_t ModificationTime;
    uint32_t SecID;
    uint32_t StreamSize;
    uint32_t NotUsed;
} TOLEDirectory;

uint32_t *SectList=NULL, *MiniSectList=NULL, MiniStreamStart=0;
uint32_t SectSize, MiniSize;
size_t DocLen;
int DirCount=0, ParseFlags=0;

#define OLEPARSE_ROOTENTRY 1
#define OLEPARSE_MACROS    2
#define OLEPARSE_BADSECTOR 4
#define OLEPARSE_ENCRYPTED 8


long GetNextSector(long Curr)
{
    long Next;
    unsigned long long pos;

    if (Curr < 0) return(-1);
    if (Curr > ((DocLen - 512) / SectSize))
    {
        if (Config->Flags & FLAG_DEBUG) printf("Next Sector '%lu' is beyond end of file\n",Next);
        return(-1);
    }

    pos=512 + (Curr * SectSize);
    if (pos > DocLen)
    {
        if (Config->Flags & FLAG_DEBUG) printf("Position of next sector '%llu' is beyond end of file '%llu'\n",pos,DocLen);
        return(-1);
    }

    Next=SectList[Curr];

    if (Next==FREESECT) return(0);
    if (Next==ENDOFCHAIN) return(0);

    if (Next < 0) return(-1);
    if (Next > ((DocLen - 512) / SectSize))
    {
        if (Config->Flags & FLAG_DEBUG) printf("Next Sector '%lu' is beyond end of file\n",Next);
        return(-1);
    }

    return(Next);
}

int ReadSector(STREAM *S, int Sector, char *Data)
{
    size_t pos;

    pos=512 + (Sector * SectSize);
    STREAMSeek(S,pos,SEEK_SET);
    STREAMReadBytes(S,Data,SectSize);
}

void ProcessSectList(STREAM *S, uint32_t *MSAT, int size)
{
    int i, count=0;
    uint32_t *ptr;

    for (i=0; i < size; i++)
    {
        if ((MSAT[i] == FREESECT)  || (MSAT[i] == ENDOFCHAIN)) break;
        count++;
//printf("%lx %d\n",MSAT[i],MSAT[i]);
    }

    SectList=(uint32_t *) calloc(1, SectSize * (count +1));
    for (i=0; i < count; i++)
    {
        ReadSector(S, MSAT[i], ((char *) SectList) +(SectSize * i));
    }

    ptr=SectList;
    i=0;
    while (*ptr > 0)
    {
//printf("%d %lx %lu\n",i, *ptr, *ptr);
        i++;
        ptr++;
    }

}


/*
void ProcessMiniSectList(STREAM *S, uint32_t start, int size)
{
int i, count=0, cur;
uint32_t *ptr;

MiniSectList=(uint32_t *) calloc(1, 512 * (size+1));
cur=start;
for (i=0; i <size; i++)
{
ReadSector(S, cur, ((char *) MiniSectList) +(512 * i));
cur=SectList[cur];
}

ptr=MiniSectList;
i=0;
while (*ptr > 0)
{
//printf("mini: %d %lx %lu\n",i, *ptr, *ptr);
i++;
ptr++;
}

}
*/



char *Translate16String(char *RetStr, const char *Bytes, int size)
{
    const char *ptr, *end;
    int len=0;

    end=Bytes+size;
    for (ptr=Bytes; (ptr < end) && (*ptr !='\0'); ptr+=2)
    {
        RetStr=AddCharToBuffer(RetStr,len++,*ptr);
    }

    return(RetStr);
}


/* One day may be able to extract from OLE files, but not yet
void SaveShortStream(STREAM *InS, const char *Name, unsigned long Size, long Start)
{
STREAM *OutS;
char *Bytes=NULL, *ptr;
long MiniSec=Start, pos;

if (MiniSec < 1) return;
OutS=STREAMFileOpen(Name, SF_CREAT | SF_WRONLY| SF_TRUNC);
if (OutS)
{
Bytes=SetStrLen(Bytes,512);
while(MiniSec > 0)
{
pos=(MiniSec * MiniSize) / SectSize;
ReadSector(InS, pos, Bytes);
ptr=Bytes+((MiniSec * MiniSize) % SectSize);
STREAMWriteBytes(OutS, ptr, MiniSize);
MiniSec=MiniSectList[MiniSec];
}
}

Destroy(Bytes);
STREAMClose(OutS);
}

void SaveStream(STREAM *InS, const char *Name, unsigned long Size, long Start)
{
STREAM *OutS;
char *Bytes=NULL, *ptr;
long Sec=Start, pos;

if (Sec < 0) return;
OutS=STREAMFileOpen(Name, SF_CREAT | SF_WRONLY| SF_TRUNC);
if (OutS)
{
Bytes=SetStrLen(Bytes, SectSize);
while(Sec > -1)
{
	printf("%d %d\n",Sec,SectSize);
	ReadSector(InS, Sec, Bytes);
	STREAMWriteBytes(OutS, Bytes, SectSize);
	Sec=SectList[Sec];
}
}

Destroy(Bytes);
STREAMClose(OutS);
}
*/


void OLEProcessDirBlock(STREAM *S, int Sector, int Extract)
{
    size_t pos, i;
    char *Bytes=NULL, *Tempstr=NULL, *ptr;
    TOLEDirectory *Dir;
    int result=0;

    Bytes=SetStrLen(Bytes, SectSize);
    ReadSector(S, Sector, Bytes);

    ptr=Bytes;
    for (i=0; i < 4; i++)
    {
        Dir=(TOLEDirectory *) ptr;

        if (Dir->Type > 0)
        {
            DirCount++;
            //Translate Directory name from UTF-16
            Tempstr=Translate16String(Tempstr, Dir->Name, SectSize);

            if (StrValid(Tempstr))
            {
                if (Config->Flags & FLAG_DEBUG) printf("STREAM: %s\n",Tempstr);
                if (strcmp(Tempstr,"Root Entry")==0) ParseFlags |=OLEPARSE_ROOTENTRY;
                else if ((strcmp(Tempstr,"Macros")==0) || (strcmp(Tempstr,"VBA")==0) || (strcmp(Tempstr,"_VBA_PROJECT_CUR")==0)) ParseFlags |=OLEPARSE_MACROS;
                else if (strcmp(Tempstr,"EncryptedPackage")==0) ParseFlags |=OLEPARSE_ENCRYPTED;
            }
        }

        /* One day file extraction, maybe. But not yet
        if (Extract && (Dir->Type==2))
        {
        	if (Dir->StreamSize < 4096) SaveShortStream(S, Tempstr, Dir->StreamSize, Dir->SecID);
        	else SaveStream(S, Tempstr, Dir->StreamSize, Dir->SecID);
        }
        */

        ptr+=sizeof(TOLEDirectory);
    }

    Destroy(Tempstr);
    Destroy(Bytes);
}


STREAM *OLEFileOpen(const char *Path, TOLEHeader *Header, TMimeItem *Item)
{
    struct stat Stat;
    STREAM *S;


    stat(Path, &Stat);
    DocLen=Stat.st_size;

    if (DocLen < sizeof(TOLEHeader))
    {
        Item->RulesResult=RULE_MALFORMED | RULE_EVIL;
        SetTypedVar(Item->Errors, "malformed: too short", "", ERROR_BASIC);
        return(NULL);
    }


    S=STREAMFileOpen(Path, SF_RDONLY);
    if (! S) return(NULL);

    STREAMReadBytes(S, (char *) Header, sizeof(TOLEHeader));

    if (memcmp(Header->FileSig, OLE_MAGIC, 8) !=0)
    {
        Item->RulesResult=RULE_MALFORMED | RULE_EVIL;
        SetTypedVar(Item->Errors, "malformed: bad header signature", "", ERROR_BASIC);
        STREAMClose(S);
        return(NULL);
    }

    SectSize=pow(2, Header->SectorShift);
    MiniSize=pow(2, Header->MiniSectorShift);

    if (SectSize < 512)
    {
        Item->RulesResult=RULE_MALFORMED | RULE_EVIL;
        SetTypedVar(Item->Errors, "malformed: sector size too small", "", ERROR_BASIC);
        STREAMClose(S);
        return(NULL);
    }


//printf("min: %d Dll: %d BO: %d SS: %d mSS: %d fat: %d first: %d\n",Header->MinorVersion,Header->DllVersion, Header->ByteOrder, Header->SectorShift, Header->MiniSectorShift, Header->FatSects, Header->sectDirStart);


    ProcessSectList(S, Header->sectFat, 109);
//ProcessMiniSectList(S, Header.sectMiniFatStart, Header.sectMiniFat);

    return(S);
}



int OLEFileProcess(const char *Path, TMimeItem *Item)
{
    STREAM *S;
    TOLEHeader Header;
    long Sect, DirBlockCount=0;

    ParseFlags=0;
    Item->RulesResult=RULE_SAFE;
    if ((Config->Flags & FLAG_DEBUG)) printf("Check OLE: [%s]\n",Path);
    S=OLEFileOpen(Path, &Header, Item);

    if (S)
    {
        Sect=Header.sectDirStart;
        while (Sect)
        {
            OLEProcessDirBlock(S, Sect, FALSE);
            Sect=GetNextSector(Sect);
            if (Sect < 0)
            {
                ParseFlags |= OLEPARSE_BADSECTOR;
                break;
            }
        }
        STREAMClose(S);
    }

//if we found no streams/directories, then declare this is 'malformed'
    if (ParseFlags & OLEPARSE_MACROS)
    {
        Item->RulesResult=RULE_MACROS;
        SetTypedVar(Item->Errors, "macros", "", ERROR_BASIC);
    }

    if (ParseFlags & OLEPARSE_ENCRYPTED) Item->RulesResult=RULE_ENCRYPTED;

    if (ParseFlags & OLEPARSE_BADSECTOR)
    {
        Item->RulesResult=RULE_MALFORMED | RULE_EVIL;
        SetTypedVar(Item->Errors, "malformed: bad sector", "", ERROR_BASIC);
    }

    if (DirCount==0)
    {
        //if we found no streams/directories, then declare this is 'malformed'
        Item->RulesResult=RULE_MALFORMED | RULE_EVIL;
        SetTypedVar(Item->Errors, "malformed: no content streams", "", ERROR_BASIC);
    }
    else if (! (ParseFlags & OLEPARSE_ROOTENTRY) )
    {
        Item->RulesResult=RULE_MALFORMED | RULE_EVIL;
        SetTypedVar(Item->Errors, "malformed: no root entry", "", ERROR_BASIC);
    }



    return(Item->RulesResult);
}


