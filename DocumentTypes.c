#include "Mime.h"
//#include "Html.h"
#include "Zip.h"
#include "PDF.h"
#include "RTF.h"
#include "OLE.h"
#include "FileTypeRules.h"

int DocTypeMatch(TMimeItem *Item, const char *MatchType)
{
const char *ptr;

if (strcasecmp(MatchType, Item->ContentType)==0) return(TRUE);
if (strcasecmp(MatchType, Item->MagicType)==0) return(TRUE);
if (strcasecmp(MatchType, Item->ExtnType)==0) return(TRUE);

ptr=TranslateMimeTypeEquivalent(Item->ContentType);
if (StrValid(ptr) && (strcasecmp(MatchType, ptr)==0)) return(TRUE);

ptr=TranslateMimeTypeEquivalent(Item->MagicType);
if (StrValid(ptr) && (strcasecmp(MatchType, ptr)==0)) return(TRUE);

ptr=TranslateMimeTypeEquivalent(Item->ExtnType);
if (StrValid(ptr) && (strcasecmp(MatchType, ptr)==0)) return(TRUE);

return(FALSE);
}

int DocTypeProcess(STREAM *Doc, TMimeItem *Item, const char *Path)
{
int RetVal=RULE_NONE;
const char *TmpPath=NULL;

	if (! Item) return(RetVal);

	TmpPath=CopyStr(TmpPath, Path);
	//if the root document is not a multipart mail then we'll have to export it first
	if ((Item->Flags & MIMEFLAG_ROOT) && (strncmp(Item->ContentType, "multipart/",10) != 0))
	{
		    Doc=MimeReadDocument(Doc, Item, "", &TmpPath);
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
    //HTMLFileProcess(Doc, TmpPath, Item);
  }
  else if (DocTypeMatch(Item,"multipart/mixed"))
  {
    MimeReadMultipart(Doc, Item);
  }

FileRulesConsider(Item);

DestroyString(TmpPath);

return(RetVal);
}

