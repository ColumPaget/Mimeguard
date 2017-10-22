FLAGS=
LIBS=-lm -lz 
prefix=/usr/local
sysconfdir=/etc
bindir=${exec_prefix}/bin

OBJ=common.o DocumentTypes.o Mime.o FileMagics.o FileExtensions.o FileTypeRules.o Export.o DocumentStrings.o ConfigFile.o Output.o Zip.o PDF.o RTF.o OLE.o HTML.o URL.o IPRegion.o libUseful-2.8/libUseful-2.8.a


mimeguard: $(OBJ) main.c
	gcc $(FLAGS) -omimeguard main.c $(OBJ) $(LIBS) -DPREFIX=\"$(prefix)\" -DSYSCONFDIR=\"$(sysconfdir)\"

Mime.o: Mime.h Mime.c
	gcc $(FLAGS) -c Mime.c

FileMagics.o: FileMagics.h FileMagics.c
	gcc $(FLAGS) -c FileMagics.c

FileExtensions.o: FileExtensions.h FileExtensions.c
	gcc $(FLAGS) -c FileExtensions.c

FileTypeRules.o: FileTypeRules.h FileTypeRules.c
	gcc $(FLAGS) -c FileTypeRules.c

DocumentStrings.o: DocumentStrings.h DocumentStrings.c
	gcc $(FLAGS) -c DocumentStrings.c

DocumentTypes.o: DocumentTypes.h DocumentTypes.c
	gcc $(FLAGS) -c DocumentTypes.c

Zip.o: Zip.h Zip.c
	gcc $(FLAGS) -c Zip.c

PDF.o: PDF.h PDF.c
	gcc $(FLAGS) -c PDF.c

RTF.o: RTF.h RTF.c
	gcc $(FLAGS) -c RTF.c

OLE.o: OLE.h OLE.c
	gcc $(FLAGS) -c OLE.c

HTML.o: HTML.h HTML.c
	gcc $(FLAGS) -c HTML.c

URL.o: URL.h URL.c
	gcc $(FLAGS) -c URL.c

IPRegion.o: IPRegion.h IPRegion.c
	gcc $(FLAGS) -c IPRegion.c

Export.o: Export.h Export.c
	gcc $(FLAGS) -c Export.c

ConfigFile.o: ConfigFile.h ConfigFile.c
	gcc $(FLAGS) -c ConfigFile.c

Output.o: Output.h Output.c
	gcc $(FLAGS) -c Output.c

common.o: common.h common.c
	gcc $(FLAGS) -c common.c

libUseful-2.8/libUseful-2.8.a: 
	$(MAKE) -C libUseful-2.8

clean:
	@rm -f *.o */*.o */*.a */*.so mimeguard.exe

install: mimeguard
	install-sh mimeguard $(bindir)
