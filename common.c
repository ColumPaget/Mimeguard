#include "common.h"
#include "Mime.h"

int Flags=0;
char *ExportPath=NULL;

char *DecodeQuotedPrintable(char *Data, const char *Line)
{
const char *ptr;
char *Token=NULL;
int len=0;

		for (ptr=Line; *ptr !='\0'; ptr++)
		{
			switch (*ptr)
			{
				case '=':
					ptr++;
					if (*ptr=='\n')
					{
						//do nothing. This is a line continuation
					}
					{
						Token=CopyStrLen(Token,ptr,2);
						Data=AddCharToBuffer(Data,len,strtol(Token,NULL,16));
						len++;
						ptr++;
					}
				break;

				default:
						Data=AddCharToBuffer(Data,len,*ptr);
						len++;
				break;
			}
		}

DestroyString(Token);

return(Data);
}


int DecodeDocumentLine(const char *Line, int Encoding, char **Data)
{
int result;

	switch (Encoding)
	{
	case MIMEFLAG_BASE64:
  *Data=SetStrLen(*Data, StrLen(Line));
  result=from64tobits(*Data, Line);
  (*Data)[result]='\0';
	break;

	case MIMEFLAG_QUOTEDPRINTABLE:
		*Data=DecodeQuotedPrintable(*Data, Line);
		result=StrLen(*Data);
	break;

	default:
    result=StrLen(Line);
    *Data=CopyStrLen(*Data, Line, result);
	break;
  }

return(result);
}

