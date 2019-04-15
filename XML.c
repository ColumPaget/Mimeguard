#include "XML.h"
#include "FileTypeRules.h"
#include "DocumentStrings.h"

const char *XML_DOCX_Field(const char *Doc, TMimeItem *Item)
{
    char *Tempstr=NULL, *Name=NULL, *Value=NULL;
		const char *ptr;

		ptr=XMLGetTag(Doc, NULL, &Name, &Value);
		while (ptr)
		{
//				printf("FIELD [%s] %s\n", Name, Value);
				if (strcasecmp(Name, "w:fldChar")==0) break;
				if (! StrValid(Name)) Tempstr=CatStr(Tempstr, Value);
				ptr=XMLGetTag(ptr, NULL, &Name, &Value);
		}

		if (StrValid(Tempstr))
		{
		StripLeadingWhitespace(Tempstr);
		StripTrailingWhitespace(Tempstr);
		if (strncmp(Tempstr, "DDEAUTO ",8)==0)
		{
        Item->RulesResult=RULE_MACROS;
        SetTypedVar(Item->Errors, "DDE", "", ERROR_BASIC);
		}
		}

    Destroy(Tempstr);
    Destroy(Name);
    Destroy(Value);

return(ptr);
}


int XMLFileProcess(const char *Path, TMimeItem *Item)
{
    int RetVal=RULE_SAFE;
    char *Tempstr=NULL, *Name=NULL, *Value=NULL;
		const char *ptr;
    STREAM *S;

    if ((Config->Flags & FLAG_DEBUG)) printf("Check XML: [%s]\n",Path);
    S=STREAMOpen(Path, Item);
    if (S)
    {
				Tempstr=STREAMReadDocument(Tempstr, S);
				ptr=XMLGetTag(Tempstr, NULL, &Name, &Value);
				while (ptr)
				{
		//		printf("XML [%s] %s\n", Name, Value);
				if (strcasecmp(Name, "w:fldChar")==0) ptr=XML_DOCX_Field(ptr, Item);
				ptr=XMLGetTag(ptr, NULL, &Name, &Value);
				}
        STREAMClose(S);
    }

    Destroy(Tempstr);
    Destroy(Name);
    Destroy(Value);
    return(RetVal);
}

