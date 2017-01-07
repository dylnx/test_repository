#ifndef __MakeString__
#define __MakeString__

#include "system.h"

int adjusttime(BYTE* obuffer);
int getpower(BYTE* obuffer);
int setpower(BYTE* obuffer, int power);
int setbright(BYTE* obuffer, int bright);
int getboardparam(BYTE* obuffer);
int setboardparam(BYTE* obuffer, PBoardParam param);
int setcurchapter(BYTE* obuffer, int index);
int setcurleaf(BYTE* obuffer, int index);

void MakeRoot(BYTE* buffer);
void MakeRegion(BYTE* buffer, long chapterindex, long regionindex, long left, long top, long width, long height);
void MakeObject(BYTE* buffer, long chapterindex, long regionindex, long leafindex, long objectindex);
void AddChapter(DWORD time, WORD wait);
void AddRegion(long left, long top, long width, long height);
void AddLeaf(DWORD time, WORD wait);
void AddStrings(long left, long top, long width, long height, long transparent);
void AddChildString(char* string, long fontset, long color, long inmethod, long inspeed, long outmethod, long outspeed, long stopmethod, long stopspeed, long stoptime);
void AddWindows(long left, long top, long width, long height, long transparent);
void AddChildWindow(BYTE* dibbuffer, long cx, long cy, long inmethod, long inspeed, long outmethod, long outspeed, long stopmethod, long stopspeed, long stoptime);
void AddChildText(char* str, char* fontname, DWORD fontsize, DWORD fontcolor, long inmethod, long inspeed, long outmethod, long outspeed, long stopmethod, long stopspeed, long stoptime);
void AddChildFontSetText(char* str, char* fontfile, unsigned long color, long inmethod, long inspeed, long outmethod, long outspeed, long stopmethod, long stopspeed, long stoptime);
void AddDateTime(long left, long top, long width, long height, long transparent);
void AddCounter(long left, long top, long width, long height, long transparent, long counter_type, long format, long year, long month, long day, long hour, long minute, long second);


DWORD DoBeginPacket(BYTE* obuf, BYTE dstAddr);

DWORD DoBeginPacketEx(BYTE* obuf, BYTE dstAddr, DWORD key);

DWORD DoAllDataPacket(BYTE* ibuf, BYTE* obuf, BYTE dstAddr);

DWORD GetDataPacketCount(BYTE* ibuf);

DWORD DoDataPacket(BYTE* ibuf, DWORD index, BYTE* obuf, BYTE dstAddr);

DWORD DoEndPacket(BYTE* obuf, DWORD index, BYTE dstAddr);

extern PRoot root;

#endif
