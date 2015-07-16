/*
** afx.h
** The ubiquitous header used for precompilation.
**
**---------------------------------------------------------------------------
** Copyright 2001 Randy Heit
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**---------------------------------------------------------------------------
*/

typedef unsigned char UBYTE;
typedef signed char BYTE;
typedef unsigned short UWORD;
typedef signed short WORD;
typedef unsigned int ULONG;
typedef signed int LONG;

#ifndef __cplusplus
typedef enum { false, true } bool;
#endif

#include "packer.h"
#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

#ifndef __BIG_ENDIAN__
#define MAKE_ID(a,b,c,d)	((a)|((b)<<8)|((c)<<16)|((d)<<24))
#define BigShort(x)		((((x)>>8)&0xff)|((((x)<<8))&0xff00))
#define BigLong(x)		((((x)>>24)&0xff)|(((x)>>8)&0xff00)|(((x)<<8)&0xff0000)|((x)<<24))
#define LittleShort(x)	(x)
#define LittleLong(x)	(x)
#else
#define MAKE_ID(a,b,c,d)	((d)|((c)<<8)|((b)<<16)|((a)<<24))
#define BigShort(x)		(x)
#define BigLong(x)		(x)
#define LittleShort(x)	((((x)>>8)&0xff)|(((x)<<8))&0xff00)
#define LittleLong(x)	((((x)>>24)&0xff)|(((x)>>8)&0xff00)|(((x)<<8)&0xff0000)|((x)<<24))
#endif

void LoadPic (char *filename, UBYTE **data, int *width, int *height,
	int *srcwidth, int *cx, int *cy, UBYTE *palette);

int WriteConFont (const char *name, UBYTE *data, int width, int height, int srcwidth);

int WriteImage (const char *name, UBYTE *data, int width, int height,
	int srcwidth, int cx, int cy, UBYTE *palette);

int WritePCXfile (const char *name, UBYTE *data, int width, int height, int srcwidth, UBYTE *palette);
int WriteBMPfile (const char *name, UBYTE *data, int width, int height, int srcwidth, UBYTE *palette);
int WriteILBMfile (const char *name, UBYTE *data, int width, int height, int srcwidth,
	int cx, int cy, UBYTE *palette);

extern UBYTE RetransImage;

UBYTE *ImageData;
int ImageWidth, ImageHeight, ImageSrcWidth;
int ImageCX, ImageCY;
UBYTE ImagePalette[768];

typedef enum
{
	SHADING_Normal,		// black->color
	SHADING_Console,	// black->color->white
	SHADING_None
} ShadeType;

void StartFont (const char *fontname, UBYTE *data, int width, int height, int srcwidth, UBYTE palette[768]);
void GrabFont (int x, int y, int firstchar);
bool EndFont (void);
void SetFontShading (ShadeType shade);


typedef struct
{
	UBYTE Magic[4];
	UWORD Width;
	UWORD Height;
	WORD LeftOffset;
	WORD TopOffset;
	UBYTE Compression;
	UBYTE Reserved[11];
} RawImageHeader;

typedef struct
{
	UBYTE Magic[4];		// 'FON1'
	UWORD CharWidth;
	UWORD CharHeight;
} ConsoleFontHeader;

typedef struct
{
	UBYTE Magic[4];		// 'FON2'
	UWORD FontHeight;
	UBYTE FirstChar;
	UBYTE LastChar;
	UBYTE bConstantWidth;
	UBYTE ShadingType;
	UBYTE PaletteSize;		// size of palette in entries (not bytes!)
	UBYTE Pad;
	//UWORD CharWidths[LastChar-FirstChar+1];
	//UBYTE Palette[PaletteSize+1][3];	// last entry is the delimiter color
	//UBYTE CharacterData[...];
} FontHeader;
