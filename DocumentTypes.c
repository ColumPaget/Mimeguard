#include "Mime.h"
#include "Zip.h"
#include "PDF.h"
#include "RTF.h"
#include "OLE.h"
#include "HTML.h"
#include "FileTypeRules.h"

int DocTypeMatch(TMimeItem *Item, const char *MatchType)
{
    const char *ptr;

    if (strcasecmp(MatchType, Item->ContentType)==0) return(TRUE);
    if (strcasecmp(MatchType, Item->FileMagicsType)==0) return(TRUE);
    if (strcasecmp(MatchType, Item->ExtnType)==0) return(TRUE);

    ptr=TranslateMimeTypeEquivalent(Item->ContentType);
    if (StrValid(ptr) && (strcasecmp(MatchType, ptr)==0)) return(TRUE);

    ptr=TranslateMimeTypeEquivalent(Item->FileMagicsType);
    if (StrValid(ptr) && (strcasecmp(MatchType, ptr)==0)) return(TRUE);

    ptr=TranslateMimeTypeEquivalent(Item->ExtnType);
    if (StrValid(ptr) && (strcasecmp(MatchType, ptr)==0)) return(TRUE);

    return(FALSE);
}



int DocTypeProcess(STREAM *Doc, TMimeItem *Item, const char *Path)
{
    int RetVal=RULE_NONE;
    char *TmpPath=NULL;


    if (! Item) return(RetVal);

    TmpPath=CopyStr(TmpPath, Path);

    //if the root document is not a multipart mail then we'll have to export it first
    if (Item->Flags & MIMEFLAG_ROOT)
    {
        if (StrEnd(Item->ContentType)) Doc=MimeReadDocument(Doc, Item, "", &TmpPath);
    }


    if (
        (DocTypeMatch(Item, "application/zip")) ||
        (DocTypeMatch(Item,"application/x-zip-compressed"))
    )
    {
        ZipFileProcess(TmpPath, Item);
    }
    else if (DocTypeMatch(Item,"application/pdf"))
    {
        PDFFileProcess(TmpPath, Item);
    }
    else if (DocTypeMatch(Item,"application/rtf"))
    {
        RTFFileProcess(TmpPath, Item);
    }
    else if (DocTypeMatch(Item,"application/x-ole-storage"))
    {
        OLEFileProcess(TmpPath, Item);
    }
    else if (DocTypeMatch(Item,"text/html"))
    {
        HTMLFileProcess(TmpPath, Item);
    }
    else if (
        (DocTypeMatch(Item,"multipart/mixed")) ||
        (DocTypeMatch(Item,"multipart/alternative")) ||
        (DocTypeMatch(Item,"multipart/related"))
    )
    {
        MimeReadMultipart(Doc, Item);
    }

    FileRulesConsider(Item);

    Destroy(TmpPath);

    return(RetVal);
}

