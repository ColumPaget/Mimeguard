
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
#define TERM_HIDECURSOR 32
#define TERM_RAWKEYS 64

#define TERMBAR_UPPER 1
#define TERMBAR_LOWER 2
#define TERMBAR_CENTER 4

typedef enum {TERM_NORM, TERM_TEXT, TERM_COLOR, TERM_CLEAR_SCREEN, TERM_CLEAR_ENDLINE, TERM_CLEAR_STARTLINE, TERM_CLEAR_LINE, TERM_CURSOR_HOME, TERM_CURSOR_MOVE, TERM_CURSOR_SAVE, TERM_CURSOR_UNSAVE, TERM_CURSOR_HIDE, TERM_CURSOR_SHOW, TERM_SCROLL, TERM_SCROLL_REGION, TERM_UNICODE} ETerminalCommands; 

#define ESCAPE 0x1b

#define KEY_F1 0x111
#define KEY_F2 0x112
#define KEY_F3 0x113
#define KEY_F4 0x114
#define KEY_F5 0x115
#define KEY_F6 0x116
#define KEY_F7 0x117
#define KEY_F8 0x118
#define KEY_F9 0x119
#define KEY_F10 0x11A
#define KEY_F11 0x11B
#define KEY_F12 0x11C
#define KEY_F13 0x11D
#define KEY_F14 0x11E

#define KEY_SHIFT_F1 0x121
#define KEY_SHIFT_F2 0x122
#define KEY_SHIFT_F3 0x123
#define KEY_SHIFT_F4 0x124
#define KEY_SHIFT_F5 0x125
#define KEY_SHIFT_F6 0x126
#define KEY_SHIFT_F7 0x127
#define KEY_SHIFT_F8 0x128
#define KEY_SHIFT_F9 0x129
#define KEY_SHIFT_F10 0x12A
#define KEY_SHIFT_F11 0x12B
#define KEY_SHIFT_F12 0x12C
#define KEY_SHIFT_F13 0x12D
#define KEY_SHIFT_F14 0x12E

#define KEY_CTRL_F1 0x131
#define KEY_CTRL_F2 0x132
#define KEY_CTRL_F3 0x133
#define KEY_CTRL_F4 0x134
#define KEY_CTRL_F5 0x135
#define KEY_CTRL_F6 0x136
#define KEY_CTRL_F7 0x137
#define KEY_CTRL_F8 0x138
#define KEY_CTRL_F9 0x139
#define KEY_CTRL_F10 0x13A
#define KEY_CTRL_F11 0x13B
#define KEY_CTRL_F12 0x13C
#define KEY_CTRL_F13 0x13D
#define KEY_CTRL_F14 0x13E

#define KEY_UP 0x141
#define KEY_DOWN 0x142
#define KEY_LEFT 0x143
#define KEY_RIGHT 0x144
#define KEY_HOME 0x145
#define KEY_END 0x146
#define KEY_PAUSE 0x147
#define KEY_FOCUS_IN 0x148
#define KEY_FOCUS_OUT 0x149
#define KEY_INSERT 0x149
#define KEY_DELETE 0x14A
#define KEY_PGUP   0x14B
#define KEY_PGDN   0x14C
#define KEY_WIN    0x14D
#define KEY_MENU   0x14E

#define KEY_SHIFT_UP 0x151
#define KEY_SHIFT_DOWN 0x152
#define KEY_SHIFT_LEFT 0x153
#define KEY_SHIFT_RIGHT 0x154
#define KEY_SHIFT_HOME 0x155
#define KEY_SHIFT_END 0x156
#define KEY_SHIFT_PAUSE 0x157
#define KEY_SHIFT_FOCUS_IN 0x158
#define KEY_SHIFT_FOCUS_OUT 0x159
#define KEY_SHIFT_INSERT 0x149
#define KEY_SHIFT_DELETE 0x14A
#define KEY_SHIFT_PGUP   0x14B
#define KEY_SHIFT_PGDN   0x14C
#define KEY_SHIFT_WIN    0x14D
#define KEY_SHIFT_MENU   0x14E

#define KEY_CTRL_UP 0x161
#define KEY_CTRL_DOWN 0x162
#define KEY_CTRL_LEFT 0x163
#define KEY_CTRL_RIGHT 0x164
#define KEY_CTRL_HOME 0x165
#define KEY_CTRL_END 0x166
#define KEY_CTRL_PAUSE 0x167
#define KEY_CTRL_FOCUS_IN 0x168
#define KEY_CTRL_FOCUS_OUT 0x169
#define KEY_CTRL_INSERT 0x149
#define KEY_CTRL_DELETE 0x14A
#define KEY_CTRL_PGUP   0x14B
#define KEY_CTRL_PGDN   0x14C
#define KEY_CTRL_WIN    0x14D
#define KEY_CTRL_MENU   0x14E



#define TerminalClear(S) ( TerminalCommand(TERM_CLEAR_SCREEN, 0, 0, S))
#define TerminalEraseLine(S) ( TerminalCommand(TERM_CLEAR_ENDLINE, 0, 0, S))
#define TerminalCursorHide(S) ( TerminalCommand(TERM_CURSOR_HIDE, 0, 0, S))
#define TerminalCursorShow(S) ( TerminalCommand(TERM_CURSOR_SHOW, 0, 0, S))
#define TerminalCursorSave(S) ( TerminalCommand(TERM_CURSOR_SAVE, 0, 0, S))
#define TerminalCursorRestore(S) ( TerminalCommand(TERM_CURSOR_UNSAVE, 0, 0, S))
#define TerminalCursorMove(S, x, y) ( TerminalCommand(TERM_CURSOR_MOVE, x, y, S))
#define TerminalScroll(S, x, y) ( TerminalCommand(TERM_SCROLL, y, x, S))
#define TerminalScrollUp(S) ( TerminalCommand(TERM_SCROLL, 1, 0, S))
#define TerminalScrollDown(S) ( TerminalCommand(TERM_SCROLL, -1, 0, S))

typedef struct
{
int Flags;
char *ForeColor;
char *BackColor;
int TextLen;
char *Text;
STREAM *Term;
} TERMBAR;

int TerminalStrLen(const char *Str);
char *ANSICode(int Color, int BgColor, int Flags);
int ANSIParseColor(const char *Str);
int TermStrLen(const char *Str);
int TerminalCommand(int Cmd, int Arg1, int Arg2, STREAM *S);
char *TerminalFormatStr(char *RetStr, const char *Str);
void TerminalPutStr(const char *Str, STREAM *S);
void TerminalPutChar(int Char, STREAM *S);
char *TerminalReadText(char *RetStr, int Flags, STREAM *S);
char *TerminalReadPrompt(char *RetStr, const char *Prompt, int Flags, STREAM *S);
void TerminalGeometry(STREAM *S, int *wid, int *len);
TERMBAR *TerminalBarCreate(STREAM *Term, const char *Config, const char *Text);
void TerminalBarUpdate(TERMBAR *TB, const char *Text);
char *TerminalBarReadText(char *RetStr, TERMBAR *TB, const char *Prompt);
int TerminalInit(STREAM *S, int Flags);

#ifdef __cplusplus
}
#endif



#endif
