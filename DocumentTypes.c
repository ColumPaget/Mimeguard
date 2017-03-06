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

	if (! Item) return(RetVal);

  if (
			(DocTypeMatch(Item, "application/zip")) ||
			(DocTypeMatch(Item,"application/x-zip-compressed"))
		)
  {
    ZipFileProcess(Path, Item);
  }
  else if (DocTypeMatch(Item,"application/pdf"))
  {
    PDFFileProcess(Path, Item);
  }
  else if (DocTypeMatch(Item,"application/rtf"))
  {
    RTFFileProcess(Path, Item);
  }
  else if (DocTypeMatch(Item,"application/x-ole-storage"))
	{
    OLEFileProcess(Path, Item);
	}
  else if (DocTypeMatch(Item,"text/html"))
  {
    //HTMLFileProcess(Doc, Path, Item);
  }
  else if (DocTypeMatch(Item,"multipart/mixed"))
  {
    MimeReadMultipart(Doc, Item);
  }

FileRulesConsider(Item);

return(RetVal);
}

