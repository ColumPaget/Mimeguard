#include "includes.h"
#include "Tokenizer.h"
#include "string.h"

#define TOK_SPACE 1
#define TOK_CODE  2


//Does the current position match against Pattern
int GetTokenSepMatch(const char *Pattern,const char **start, const char **end, int Flags)
{
const char *pptr, *eptr;
int MatchType=0;

//if start and end pointers are same, then we've no string
if (*start==*end) return(FALSE);

pptr=Pattern;
eptr=*start;


while (1)
{
//check the current 'pattern' char
switch (*pptr)
{
	//if we run out of pattern, then we got a match
	case '\0':
	*end=eptr;
	return(TRUE);
	break;


	//Quoted char
	case '\\':
  pptr++;
  if (*pptr=='S') MatchType=TOK_SPACE;
  if (*pptr=='X') MatchType=TOK_CODE;
	break;

}

switch (*eptr)
{
	//if we run out of string, then we got a part match, but its not
	//a full match, so we return fail
	case '\0': 
		*start=eptr;
		*end=eptr;
		return(FALSE); 
	break;

	case '\\':
	//if we got a quoted character we can't have found
	//the separator, so return false
	if (Flags & GETTOKEN_BACKSLASH)
	{
	if (*eptr != *pptr) return(FALSE);
	}
	else
	{
	eptr++;
	*start=eptr;
	return(FALSE);
	}
	break;

	case '"':
	case '\'':
	if (Flags & GETTOKEN_HONOR_QUOTES)
	{
		eptr=traverse_quoted(eptr);
		if (*eptr == '\0') eptr--;	//because there's a ++ going to happen when we return
		*start=eptr;
		return(FALSE);
	}
	else if (*eptr != *pptr) return(FALSE);
	break;

	case ' ':
	case '	':
	case '\n':
	case '\r':
		if ((MatchType==TOK_SPACE) || (MatchType==TOK_CODE))
		{
			while (isspace(*eptr)) eptr++;
			eptr--;
			MatchType=0;
		}
		else if (*eptr != *pptr) return(FALSE);
	break;

	case '(':
	case ')':
	case '=':
	case '!':
	case '<':
	case '>':
		if (MatchType==TOK_CODE) MatchType=0; 
		else if (*eptr != *pptr) return(FALSE);
	break;

	default:
		if (MatchType != 0) return(FALSE);
		if (*eptr != *pptr) return(FALSE);
	break;
}

pptr++;
eptr++;
}

return(FALSE);
}





//Searches through 'String' for a match of a Pattern
int GetTokenFindSeparator(const char *Pattern, const char *String, const char **SepStart, const char **SepEnd, int Flags)
{
const char *start_ptr=NULL, *end_ptr=NULL, *ptr;

start_ptr=String;
while (*start_ptr != '\0')
{
	if ((*start_ptr=='\\') && (! (Flags & GETTOKEN_BACKSLASH)))
	{
		start_ptr++;
		start_ptr++;
		continue;
	}
	
	
	if (GetTokenSepMatch(Pattern,&start_ptr, &end_ptr, Flags))
	{
		*SepStart=start_ptr;
		*SepEnd=end_ptr;
		return(TRUE);
	}
	if ((*start_ptr) !='\0') start_ptr++;
}

//We found nothing, set sep start to equal end of string
*SepStart=start_ptr;
*SepEnd=start_ptr;

return(FALSE);
}

char **BuildMultiSeparators(const char *Pattern)
{
const char *ptr, *next;
int count=0;
char **separators;

ptr=strchr(Pattern, '|');
while (ptr)
{
	count++;
	ptr++;
	ptr=strchr(ptr,'|');
}

//count + 2 because last item will lack a '|' and  we want a NULL at the end
//of the separator array
separators=(char **) calloc(count+2,sizeof(char *));
ptr=Pattern;
count=0;
while (ptr && (*ptr !='\0'))
{
	while (*ptr=='|') ptr++;
	if (*ptr!='\0')
	{
	next=strchr(ptr,'|');
	if (next) separators[count]=CopyStrLen(NULL, ptr, next-ptr);
	else separators[count]=CopyStr(NULL, ptr);
	count++;
	ptr=next;
	}
}

return(separators);
}


char *BuildMultiSepStarts(char **separators)
{
char **ptr;
char *Starts=NULL;
int len=0;

ptr=separators;
while (*ptr != NULL)
{
Starts=AddCharToBuffer(Starts,len++,**ptr);
ptr++;
}

return(Starts);
}

void DestroyMultiSepStarts(char **separators, char *starts)
{
char **ptr;

ptr=separators;
while (*ptr !=NULL)
{
free(*ptr);
ptr++;
}
free(separators);
if (starts) free(starts);
}


int GetTokenMultiSepMatch(char **Separators, const char **start_ptr, const char **end_ptr, int Flags)
{
char **ptr;
const char *sptr, *eptr;

ptr=Separators;
while (*ptr !=NULL)
{
	//must do this as GetTokenSepMatch moves these pointers on, and that'll cause problems
	//if one of our separators fails to match part way through
	sptr=*start_ptr;
	eptr=*end_ptr;

	if (GetTokenSepMatch(*ptr, &sptr, &eptr, Flags)) 
	{
		*start_ptr=sptr;
		*end_ptr=eptr;
		return(TRUE);
	}
	ptr++;
}

*start_ptr=sptr;
*end_ptr=eptr;

return(FALSE);
}


//Searches through 'String' for a match of a Pattern
int GetTokenFindMultiSeparator(const char *Pattern, const char *String, const char **SepStart, const char **SepEnd, int Flags)
{
const char *start_ptr=NULL, *end_ptr=NULL;
char **separators=NULL, *starts=NULL; 

separators=BuildMultiSeparators(Pattern);
starts=BuildMultiSepStarts(separators);
start_ptr=String;

while (*start_ptr != '\0')
{
	if ((*start_ptr=='\\') && (! (Flags & GETTOKEN_BACKSLASH)))
	{
		start_ptr++;
		start_ptr++;
		continue;
	}
	
	//must handle quotes here as we don't always go into 'GetTokenMultiSepMatch' because we want to
	//maximize speed in a simple loop
	if ((*start_ptr=='"') || (*start_ptr=='\'')) start_ptr=traverse_quoted(start_ptr);
	//for speed we check of this character matches any of the start characters of our multiple separators	
	else if (strchr(starts, *start_ptr) && GetTokenMultiSepMatch(separators,&start_ptr, &end_ptr, Flags))
	{
		*SepStart=start_ptr;
		*SepEnd=end_ptr;
		DestroyMultiSepStarts(separators, starts);
		return(TRUE);
	}

	//this char didn't match, move onto the next
	if ((*start_ptr) !='\0') start_ptr++;
}

//We found nothing, set sep start to equal end of string
*SepStart=start_ptr;
*SepEnd=start_ptr;

DestroyMultiSepStarts(separators, starts);
return(FALSE);
}


char *GetToken(const char *SearchStr, const char *Separator, char **Token, int Flags)
{
const char *SepStart=NULL, *SepEnd=NULL;
const char *sptr, *eptr;


/* this is a safety measure so that there is always something in Token*/
if (Token) *Token=CopyStr(*Token,"");

if ((! Token) || StrEnd(SearchStr)) return(NULL);

sptr=SearchStr;
if (Flags & GETTOKEN_MULTI_SEPARATORS) 
{
	GetTokenFindMultiSeparator(Separator, SearchStr, &SepStart, &SepEnd, Flags);
}
else GetTokenFindSeparator(Separator, SearchStr, &SepStart, &SepEnd, Flags);

if (Flags & GETTOKEN_INCLUDE_SEP)
{
	if (SepStart==SearchStr) eptr=SepEnd;
	else 
	{
		eptr=SepStart;
		SepEnd=SepStart;
	}
}
else if (Flags & GETTOKEN_APPEND_SEP) eptr=SepEnd;
else if (SepStart) eptr=SepStart;
else eptr=SepEnd;

if (Flags & GETTOKEN_STRIP_QUOTES)
{
	if ((*sptr=='"') || (*sptr=='\'')) 
	{
		//is character before the sep a quote? If so, we copy one less char, and also start one character later
		//else we copy the characters as well
		eptr--;
		if (*sptr==*eptr) sptr++;
		else eptr++;
	}
}

if (eptr > sptr) *Token=CopyStrLen(*Token, sptr, eptr-sptr);
else *Token=CopyStr(*Token, sptr);

if (Flags & GETTOKEN_STRIP_SPACE)
{
	StripTrailingWhitespace(*Token);
	StripLeadingWhitespace(*Token);
}

//return empty string, but not null
if ((! SepEnd) || (*SepEnd=='\0')) 
{
  SepEnd=SearchStr+StrLen((char *) SearchStr);
}

return((char *) SepEnd);
}


int GetTokenParseConfig(const char *Config)
{
const char *ptr;
int Flags=0;

for (ptr=Config; *ptr != '\0'; ptr++)
{
switch (*ptr)
{
case 'm': Flags |= GETTOKEN_MULTI_SEPARATORS; break;
case 'q': Flags |= GETTOKEN_HONOR_QUOTES; break;
case 'Q': Flags |= GETTOKEN_QUOTES; break;
case 's': Flags |= GETTOKEN_INCLUDE_SEPARATORS; break;
case '+': Flags |= GETTOKEN_APPEND_SEPARATORS; break;
}
}

return(Flags);
}


char *GetNameValuePair(const char *Input, const char *PairDelim, const char *NameValueDelim, char **Name, char **Value)
{
char *ptr, *ptr2;
char *Token=NULL;

*Name=CopyStr(*Name,"");
*Value=CopyStr(*Value,"");
ptr=GetToken(Input,PairDelim,&Token,GETTOKEN_HONOR_QUOTES);
if (StrValid(Token))
{
if ((Token[0]=='"') || (Token[0]=='\''))
{
	StripQuotes(Token);
}
ptr2=GetToken(Token,NameValueDelim,Name,GETTOKEN_HONOR_QUOTES);
//ptr2=GetToken(ptr2,PairDelim,Value,GETTOKEN_HONOR_QUOTES);
*Value=CopyStr(*Value,ptr2);
StripQuotes(*Name);
StripQuotes(*Value);
}

DestroyString(Token);
return(ptr);
}

