
#ifndef LIBUSEFUL_ANSI_H
#define LIBUSEFUL_ANSI_H

#include "includes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {ANSI_NONE, ANSI_BLACK, ANSI_RED, ANSI_GREEN, ANSI_YELLOW, ANSI_BLUE, ANSI_MAGENTA, ANSI_CYAN, ANSI_WHITE, ANSI_RESET, ANSI_RESET2, ANSI_DARKGREY, ANSI_LIGHTRED, ANSI_LIGHTGREEN, ANSI_LIGHTYELLOW, ANSI_LIGHTBLUE, ANSI_LIGHTMAGENTA, ANSI_LIGHTCYAN, ANSI_LIGHTWHITE} T_ANSI_COLORS;


#define ANSI_HIDE			65536
#define ANSI_BOLD			131072
#define ANSI_FAINT		262144
#define ANSI_UNDER		524288
#define ANSI_BLINK		1048576
#define ANSI_INVERSE  2097152
#define ANSI_NORM "\x1b[0m"
#define ANSI_BACKSPACE 0x08

#define TERM_SHOWTEXT  1
#define TERM_SHOWSTARS 2
#define TERM_SHOWTEXTSTARS 4

#define TERM_NORM 1
#define TERM_TEXT 2
#define TERM_COLOR 4
#define TERM_CLEAR_SCREEN 8
#define TERM_CLEAR_ENDLINE 16
#define TERM_CLEAR_STARTLINE 32
#define TERM_CLEAR_LINE 64
#define TERM_CURSOR_HOME 128
#define TERM_CURSOR_MOVE 256
#define TERM_CURSOR_SAVE 512
#define TERM_CURSOR_UNSAVE 1024
#define TERM_SCROLL 2048
#define TERM_UNICODE 4096


char *ANSICode(int Color, int BgColor, int Flags);
int ANSIParseColor(const char *Str);
char *TerminalReadText(char *RetStr, int Flags, STREAM *S);
int TerminalCommand(int Cmd, int Arg1, int Arg2, STREAM *S);
char *TerminalFormatStr(char *RetStr, const char *Str);
void TerminalPutStr(const char *Str, STREAM *S);

#ifdef __cplusplus
}
#endif



#endif
