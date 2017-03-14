#include "Export.h"

int ExportOpen(TMimeItem *Item, char **SavePath)
{
int fd=-1;

if (SavePath)
{
if (Flags & FLAG_EXPORT)
{
	if (Flags & FLAG_DEBUG) printf("EXPORT: %s\n",Item->FileName);
  if (StrValid(Item->FileName))
  {
    *SavePath=MCopyStr(*SavePath, ExportPath, Item->FileName, NULL);
    MakeDirPath(*SavePath, 0700);
    fd=open(*SavePath, O_CREAT | O_TRUNC | O_RDWR);
  }
  else
  {
			*SavePath=MCopyStr(*SavePath, ExportPath, "tmpXXXXXX", NULL);
    	MakeDirPath(*SavePath, 0700);
    	fd=mkstemp(*SavePath);
  }
}
else
{
  *SavePath=CopyStr(*SavePath,".tmpfileXXXXXX");
	if (Flags & FLAG_DEBUG) printf("TMPFILE: %s\n",*SavePath);
  MakeDirPath(*SavePath, 0700);
  fd=mkstemp(*SavePath);
}
}

if (fd > -1) fchmod(fd,0600);

return(fd);
}
