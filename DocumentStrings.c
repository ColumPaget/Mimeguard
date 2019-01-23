#include "DocumentStrings.h"
#include "FileTypeRules.h"

void DocumentStringsAdd(const char *DocType, const char *String, int Flags)
{
int Type;

	if (Flags & RULE_OVERRIDE) Type=KV_DOCSTRINGS_OVERRIDE;
	else Type=KV_DOCSTRINGS;

  SetTypedVar(g_KeyValueStore, DocType, String, Type);
}


const char *DocumentStringsGetList(const char *DocType)
{
const char *ptr;

  ptr=GetTypedVar(g_KeyValueStore, DocType, KV_DOCSTRINGS_OVERRIDE);
	if (StrValid(ptr)) return(ptr);

  ptr=GetTypedVar(g_KeyValueStore, DocType, KV_DOCSTRINGS);
	if (StrValid(ptr)) return(ptr);

  return(NULL);
}


int DocumentStringsCheck(const char *List, const char *String)
{
char *Match=NULL;
const char *ptr;

		ptr=GetToken(List, "\\S", &Match, GETTOKEN_QUOTES);
    while (ptr)
    {
			if (pmatch(Match, String, StrLen(String), NULL, 0)) 
			{
				Destroy(Match);
				return(RULE_EVIL);
			}
			ptr=GetToken(ptr, "\\S", &Match, GETTOKEN_QUOTES);
    }

Destroy(Match);
return(RULE_NONE);
}

