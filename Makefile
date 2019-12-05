FLAGS=-g -O2 -DPACKAGE_NAME=\"\" -DPACKAGE_TARNAME=\"\" -DPACKAGE_VERSION=\"\" -DPACKAGE_STRING=\"\" -DPACKAGE_BUGREPORT=\"\" -DPACKAGE_URL=\"\" -DSTDC_HEADERS=1 -D_FILE_OFFSET_BITS=64 -DHAVE_LIBZ=1
LIBS=-lm -lz 
prefix=/usr/local
sysconfdir=${prefix}/etc
bindir=${exec_prefix}/bin

OBJ=common.o DocumentTypes.o Mime.o FileMagics.o FileExtensions.o EmailHeaders.o FileTypeRules.o Export.o DocumentStrings.o Settings.o Output.o Zip.o PDF.o RTF.o OLE.o XML.o HTML.o URL.o IPRegion.o Smtp.o libUseful-4/libUseful.a


mimeguard: $(OBJ) main.c
	$(CC) $(FLAGS) -omimeguard main.c $(OBJ) $(LIBS) -DPREFIX=\"$(prefix)\" -DSYSCONFDIR=\"$(sysconfdir)\"

Mime.o: Mime.h Mime.c
	$(CC) $(FLAGS) -c Mime.c

FileMagics.o: FileMagics.h FileMagics.c
	$(CC) $(FLAGS) -c FileMagics.c

FileExtensions.o: FileExtensions.h FileExtensions.c
	$(CC) $(FLAGS) -c FileExtensions.c

FileTypeRules.o: FileTypeRules.h FileTypeRules.c
	$(CC) $(FLAGS) -c FileTypeRules.c

EmailHeaders.o: EmailHeaders.h EmailHeaders.c
	$(CC) $(FLAGS) -c EmailHeaders.c

DocumentStrings.o: DocumentStrings.h DocumentStrings.c
	$(CC) $(FLAGS) -c DocumentStrings.c

DocumentTypes.o: DocumentTypes.h DocumentTypes.c
	$(CC) $(FLAGS) -c DocumentTypes.c

Zip.o: Zip.h Zip.c
	$(CC) $(FLAGS) -c Zip.c

PDF.o: PDF.h PDF.c
	$(CC) $(FLAGS) -c PDF.c

RTF.o: RTF.h RTF.c
	$(CC) $(FLAGS) -c RTF.c

OLE.o: OLE.h OLE.c
	$(CC) $(FLAGS) -c OLE.c

HTML.o: HTML.h HTML.c
	$(CC) $(FLAGS) -c HTML.c

XML.o: XML.h XML.c
	$(CC) $(FLAGS) -c XML.c

URL.o: URL.h URL.c
	$(CC) $(FLAGS) -c URL.c

IPRegion.o: IPRegion.h IPRegion.c
	$(CC) $(FLAGS) -c IPRegion.c

Export.o: Export.h Export.c
	$(CC) $(FLAGS) -c Export.c

Smtp.o: Smtp.h Smtp.c
	$(CC) $(FLAGS) -c Smtp.c

Settings.o: Settings.h Settings.c
	$(CC) $(FLAGS) -c Settings.c

Output.o: Output.h Output.c
	$(CC) $(FLAGS) -c Output.c

common.o: common.h common.c
	$(CC) $(FLAGS) -c common.c

libUseful-4/libUseful.a: 
	$(MAKE) -C libUseful-4

clean:
	@rm -f *.o */*.o */*.a */*.so mimeguard

install: mimeguard
	./install-sh mimeguard $(bindir)

test:
	-echo "No tests written yet"
