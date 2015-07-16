/*
** imageread.c
** Routines for reading ILBM, BMP, PCX, IMGZ, and Doom patches.
** Routines for converting FON1 and FON2 to source images.
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

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <math.h>

#include "afx.h"
#include "ilbm.h"
#include "pcx.h"
#include "bmp.h"
#include "patch.h"

#define MAXPLANEWIDTH		(1600/8)

#define ID_FON1		MAKE_ID('F','O','N','1')
#define ID_FON2		MAKE_ID('F','O','N','2')
#define ID_IMGZ		MAKE_ID('I','M','G','Z')

static const UBYTE DoomPalette[768] =
{
	  0,  0,  0, 31, 23, 11, 23, 15,  7, 75, 75, 75,255,255,255, 27, 27, 27,
	 19, 19, 19, 11, 11, 11,  7,  7,  7, 47, 55, 31, 35, 43, 15, 23, 31,  7,
	 15, 23,  0, 79, 59, 43, 71, 51, 35, 63, 43, 27,255,183,183,247,171,171,
	243,163,163,235,151,151,231,143,143,223,135,135,219,123,123,211,115,115,
	203,107,107,199, 99, 99,191, 91, 91,187, 87, 87,179, 79, 79,175, 71, 71,
	167, 63, 63,163, 59, 59,155, 51, 51,151, 47, 47,143, 43, 43,139, 35, 35,
	131, 31, 31,127, 27, 27,119, 23, 23,115, 19, 19,107, 15, 15,103, 11, 11,
	 95,  7,  7, 91,  7,  7, 83,  7,  7, 79,  0,  0, 71,  0,  0, 67,  0,  0,
	255,235,223,255,227,211,255,219,199,255,211,187,255,207,179,255,199,167,
	255,191,155,255,187,147,255,179,131,247,171,123,239,163,115,231,155,107,
	223,147, 99,215,139, 91,207,131, 83,203,127, 79,191,123, 75,179,115, 71,
	171,111, 67,163,107, 63,155, 99, 59,143, 95, 55,135, 87, 51,127, 83, 47,
	119, 79, 43,107, 71, 39, 95, 67, 35, 83, 63, 31, 75, 55, 27, 63, 47, 23,
	 51, 43, 19, 43, 35, 15,239,239,239,231,231,231,223,223,223,219,219,219,
	211,211,211,203,203,203,199,199,199,191,191,191,183,183,183,179,179,179,
	171,171,171,167,167,167,159,159,159,151,151,151,147,147,147,139,139,139,
	131,131,131,127,127,127,119,119,119,111,111,111,107,107,107, 99, 99, 99,
	 91, 91, 91, 87, 87, 87, 79, 79, 79, 71, 71, 71, 67, 67, 67, 59, 59, 59,
	 55, 55, 55, 47, 47, 47, 39, 39, 39, 35, 35, 35,119,255,111,111,239,103,
	103,223, 95, 95,207, 87, 91,191, 79, 83,175, 71, 75,159, 63, 67,147, 55,
	 63,131, 47, 55,115, 43, 47, 99, 35, 39, 83, 27, 31, 67, 23, 23, 51, 15,
	 19, 35, 11, 11, 23,  7,191,167,143,183,159,135,175,151,127,167,143,119,
	159,135,111,155,127,107,147,123, 99,139,115, 91,131,107, 87,123, 99, 79,
	119, 95, 75,111, 87, 67,103, 83, 63, 95, 75, 55, 87, 67, 51, 83, 63, 47,
	159,131, 99,143,119, 83,131,107, 75,119, 95, 63,103, 83, 51, 91, 71, 43,
	 79, 59, 35, 67, 51, 27,123,127, 99,111,115, 87,103,107, 79, 91, 99, 71,
	 83, 87, 59, 71, 79, 51, 63, 71, 43, 55, 63, 39,255,255,115,235,219, 87,
	215,187, 67,195,155, 47,175,123, 31,155, 91, 19,135, 67,  7,115, 43,  0,
	255,255,255,255,219,219,255,187,187,255,155,155,255,123,123,255, 95, 95,
	255, 63, 63,255, 31, 31,255,  0,  0,239,  0,  0,227,  0,  0,215,  0,  0,
	203,  0,  0,191,  0,  0,179,  0,  0,167,  0,  0,155,  0,  0,139,  0,  0,
	127,  0,  0,115,  0,  0,103,  0,  0, 91,  0,  0, 79,  0,  0, 67,  0,  0,
	231,231,255,199,199,255,171,171,255,143,143,255,115,115,255, 83, 83,255,
	 55, 55,255, 27, 27,255,  0,  0,255,  0,  0,227,  0,  0,203,  0,  0,179,
	  0,  0,155,  0,  0,131,  0,  0,107,  0,  0, 83,255,255,255,255,235,219,
	255,215,187,255,199,155,255,179,123,255,163, 91,255,143, 59,255,127, 27,
	243,115, 23,235,111, 15,223,103, 15,215, 95, 11,203, 87,  7,195, 79,  0,
	183, 71,  0,175, 67,  0,255,255,255,255,255,215,255,255,179,255,255,143,
	255,255,107,255,255, 71,255,255, 35,255,255,  0,167, 63,  0,159, 55,  0,
	147, 47,  0,135, 35,  0, 79, 59, 39, 67, 47, 27, 55, 35, 19, 47, 27, 11,
	  0,  0, 83,  0,  0, 71,  0,  0, 59,  0,  0, 47,  0,  0, 35,  0,  0, 23,
	  0,  0, 11,  0,  0,  0,255,159, 67,255,231, 75,255,123,255,255,  0,255,
	207,  0,207,159,  0,155,111,  0,107,167,107,107
};

static void LoadPCX (char *filename, UBYTE **data, int *width, int *height,
	int *srcwidth, int *cx, int *cy, UBYTE *palette);
static void LoadBMP (char *filename, UBYTE **data, int *width, int *height,
	int *srcwidth, int *cx, int *cy, UBYTE *palette);

static void LoadID (char *filename, UBYTE **data, int *width, int *height,
	int *srcwidth, int *cx, int *cy, UBYTE *palette);
static void LoadILBM (FILE *file, char *filename, UBYTE **data, int *width, int *height,
	int *srcwidth, int *cx, int *cy, UBYTE *palette);
static void LoadPatch (FILE *file, char *filename, UBYTE **data, int *width, int *height,
	int *srcwidth, int *cx, int *cy, UBYTE *palette);
static void LoadIMGZ (FILE *file, char *filename, UBYTE **data, int *width, int *height,
	int *srcwidth, int *cx, int *cy, UBYTE *palette);
static void LoadFON1 (FILE *file, char *filename, UBYTE **data, int *width, int *height,
	int *srcwidth, int *cx, int *cy, UBYTE *palette);
static void LoadFON2 (FILE *file, char *filename, UBYTE **data, int *width, int *height,
	int *srcwidth, int *cx, int *cy, UBYTE *palette);
static int Unpack (FILE *file, const char *filename, UBYTE *dest, int destSize);

static void SwapTrans (UBYTE *data, int width, int height);
static void BoxRow (UBYTE *dest, int j, int k, int y, int w);

void LoadPic (char *filename, UBYTE **data, int *width, int *height,
	int *srcwidth, int *cx, int *cy, UBYTE *palette)
{
	int namelen = strlen (filename);

	if (namelen > 4)
	{
		if (stricmp (filename + namelen - 4, ".pcx") == 0)
		{
			LoadPCX (filename, data, width, height, srcwidth, cx, cy, palette);
			SwapTrans (*data, *width, *height);
			return;
		}
		else if (stricmp (filename + namelen - 4, ".bmp") == 0)
		{
			LoadBMP (filename, data, width, height, srcwidth, cx, cy, palette);
			SwapTrans (*data, *width, *height);
			return;
		}
	}
	LoadID (filename, data, width, height, srcwidth, cx, cy, palette);
	SwapTrans (*data, *width, *height);
}

static void SwapTrans (UBYTE *data, int width, int height)
{
	int i;
	UBYTE tcolor;

	if (RetransImage == 0 || data == NULL)
		return;

	tcolor = RetransImage;

	for (i = width*height; i != 0; --i)
	{
		if (*data == 0)
			*data = tcolor;
		else if (*data == tcolor)
			*data = 0;
		data++;
	}
}

static void LoadID (char *filename, UBYTE **data, int *width, int *height,
	int *srcwidth, int *cx, int *cy, UBYTE *palette)
{
	ULONG id;
	FILE *file;

	*cx = 0x8000;
	*data = NULL;

	file = fopen (filename, "rb");
	if (file == NULL)
	{
		fprintf (stderr, "Could not open %s for reading\n", filename);
		return;
	}
	fread (&id, 4, 1, file);
	switch (id)
	{
	case ID_FORM:
		LoadILBM (file, filename, data, width, height, srcwidth, cx, cy, palette);
		break;
	case ID_IMGZ:
		LoadIMGZ (file, filename, data, width, height, srcwidth, cx, cy, palette);
		break;
	case ID_FON1:
		LoadFON1 (file, filename, data, width, height, srcwidth, cx, cy, palette);
		break;
	case ID_FON2:
		LoadFON2 (file, filename, data, width, height, srcwidth, cx, cy, palette);
		break;
	default:
		LoadPatch (file, filename, data, width, height, srcwidth, cx, cy, palette);
		break;
	}
	fclose (file);
}

static void LoadILBM (FILE *file, char *filename, UBYTE **data, int *width, int *height,
	int *srcwidth, int *cx, int *cy, UBYTE *palette)
{
	BitmapHeader header;
	ULONG temp1, temp2, filelen, curpos;
	int padwidth, planewidth;
	UBYTE planes[9][MAXPLANEWIDTH];
	int i, j;

	header.pad1 = 1;

	fread (&filelen, 4, 1, file);
	filelen = BigLong (filelen) + 8;

	fread (&temp1, 4, 1, file);

	if (temp1 != ID_ILBM)
	{
		fprintf (stderr, "%s is not an ILBM\n", filename);
		return;
	}

	curpos = 12;

	do
	{
		fread (&temp1, 4, 1, file);
		fread (&temp2, 4, 1, file);

		curpos += 8;
		temp2 = BigLong (temp2);

		printf ("%c%c%c%c (%d bytes)\n",
			temp1 & 255, (temp1 >> 8) & 255,
			(temp1 >> 16) & 255, temp1 >> 24, temp2);

		if (curpos + temp2 > filelen)
		{
			fprintf (stderr, "%s is incomplete (filelen: %d, pos: %d)\n", filename, filelen, curpos);
			return;
		}

		if (temp1 == ID_BMHD)
		{
			fread (&header, sizeof(BitmapHeader), 1, file);
			header.w = BigShort (header.w);
			header.h = BigShort (header.h);
			header.x = BigShort (header.x);
			header.y = BigShort (header.y);
			header.transparentColor = BigShort (header.transparentColor);
			header.pageWidth = BigShort (header.pageWidth);
			header.pageHeight = BigShort (header.pageHeight);
			header.pad1 = 0;
			if (sizeof(BitmapHeader) < temp2)
				fseek (file, temp2 - sizeof(BitmapHeader) + ((temp2 - sizeof(BitmapHeader)) & 1), SEEK_CUR);
		}
		else if (temp1 == ID_CMAP)
		{
			memset (palette, 0, 768);
			fread (palette, temp2, 1, file);
			if (temp2 & 1)
				fseek (file, 1, SEEK_CUR);
		}
		else if (temp1 == ID_ANNO)
		{
			ULONG left = temp2;
			planes[1][0] = 0;
			while (left >= MAXPLANEWIDTH)
			{
				fread (planes[0], MAXPLANEWIDTH, 1, file);
				printf ("%s", planes[0]);
				left -= MAXPLANEWIDTH;
			}
			if (left > 0)
			{
				planes[0][left] = 0;
				fread (planes[0], left, 1, file);
				printf ("%s", planes[0]);
			}
			printf ("\n");
			if (temp2 & 1)
				fseek (file, 1, SEEK_CUR);
		}
		else if (temp1 == ID_GRAB)
		{
			WORD val;
			fread (&val, 2, 1, file);
			*cx = BigShort (val);
			fread (&val, 2, 1, file);
			*cy = BigShort (val);
		}
		else if (temp1 != ID_BODY)
		{
			fseek (file, temp2 + (temp2 & 1), SEEK_CUR);
		}
		curpos += temp2 + (temp2 & 1);
	} while (temp1 != ID_BODY);

	if (header.pad1 != 0)
	{
		fprintf (stderr, "BODY encountered before BMHD\n");
		return;
	}

	if (header.compression > 1)
	{
		fprintf (stderr, "%s has unknown compression\n", filename);
		return;
	}

	*srcwidth = header.w;
	padwidth = (header.w + 15) & ~15;
	planewidth = ((header.w + 15) / 16) * 2;
	fprintf (stderr, "Dimensions: %d x %d\n", header.w, header.h);

	*data = malloc (padwidth * header.h);
	memset (*data, header.transparentColor, padwidth * header.h);

	if (*data == NULL)
	{
		fprintf (stderr, "out of memory\n");
		return;
	}

	for (i = 0; i < 9; i++)
		memset (planes[i], 0, MAXPLANEWIDTH);

	curpos = 0;
	for (j = 0; j < header.h && curpos < temp2; j++)
	{
		UBYTE v;

		for (i = 0; i < (header.nPlanes + (header.masking == mskHasMask ? 1 : 0)); i++)
		{
			if (header.compression == cmpNone)
			{
				fread (planes[i], planewidth, 1, file);
			}
			else
			{
				BYTE c;
				int ofs;

				for (ofs = 0; ofs < planewidth; )
				{
					fread (&c, 1, 1, file);

					if (c >= 0)
					{
						fread (&planes[i][ofs], c + 1, 1, file);
						ofs += c + 1;
					}
					else if (c != -128)
					{
						UBYTE color;
						fread (&color, 1, 1, file);
						memset (&planes[i][ofs], color, -c+1);
						ofs += -c + 1;
					}
				}
			}
		}

		v = 0;
		for (i = 0; i < padwidth;)
		{
			int sel;
			for (sel = 7; sel >= 0; sel--)
			{
				int plane;
				UBYTE color = 0;

				for (plane = 0; plane < header.nPlanes; plane++)
					color |= ((planes[plane][v]>>sel)&1)<<plane;

				(*data)[j * padwidth + i] = color;
				i++;
			}
			v++;
		}
	}

	*width = padwidth;
	*height = header.h;

	if (header.transparentColor)
	{
		UBYTE oldtrans = RetransImage;
		UBYTE pal[3];

		RetransImage = (UBYTE)header.transparentColor;
		RetransImage = oldtrans;
		pal[0] = palette[0];
		pal[1] = palette[1];
		pal[2] = palette[2];
		palette[0] = palette[header.transparentColor*3];
		palette[1] = palette[header.transparentColor*3+1];
		palette[2] = palette[header.transparentColor*3+2];
		palette[header.transparentColor*3] = pal[0];
		palette[header.transparentColor*3+1] = pal[1];
		palette[header.transparentColor*3+2] = pal[2];
	}
}

static void LoadPCX (char *filename, UBYTE **data, int *width, int *height,
	int *srcwidth, int *cx, int *cy, UBYTE *palette)
{
	FILE *file;
	pcxHeader header;
	int padwidth;
	int x;
	UBYTE *decodepos;
	UBYTE c;
	int run;

	*cx = 0x8000;
	*data = NULL;

	file = fopen (filename, "rb");
	if (file == NULL)
	{
		fprintf (stderr, "Could not open %s for reading\n", filename);
		return;
	}
	if (fread (&header, sizeof(header), 1, file) != 1 ||
		header.manufacturer != 10 ||
		header.encoding != 1)
	{
		fprintf (stderr, "%s is not a pcx file\n", filename);
		fclose (file);
		return;
	}
	if (header.version != 5 || header.bits_per_pixel != 8 || header.color_planes != 1)
	{
		fprintf (stderr, "%s is not 256-color\n", filename);
		fclose (file);
		return;
	}

	*srcwidth = LittleShort(header.xmax) - LittleShort(header.xmin) + 1;
	*width = LittleShort(header.bytes_per_line);
	*height = LittleShort(header.ymax) - LittleShort(header.ymin) + 1;
	padwidth = *width;
	fprintf (stderr, "Dimensions: %d x %d\n", *srcwidth, *height);

	*data = malloc (padwidth * (*height));

	if (*data == NULL)
	{
		fprintf (stderr, "out of memory\n");
		fclose (file);
		return;
	}
	memset (*data, 0, padwidth * (*height));

	fseek (file, 0, SEEK_END);
	x = ftell (file);
	fseek (file, 128, SEEK_SET);

	// Is image uncompressed? (Shouldn't the header tell us this?)
	if (x == padwidth * (*height) + 128 + 769)
	{
		if (fread (*data, padwidth * (*height), 1, file) != 1)
		{
			fprintf (stderr, "%s is corrupt\n", filename);
			fclose (file);
			free (*data);
			*data = NULL;
			return;
		}
	}
	else
	{
		decodepos = *data;
		for (x = 0; x < padwidth * (*height); )
		{
			if (fread (&c, 1, 1, file) != 1)
			{
				fprintf (stderr, "%s is corrupt\n", filename);
				fclose (file);
				free (*data);
				*data = NULL;
				return;
			}
			if ((c & 0xc0) == 0xc0)
			{
				run = c & 0x3f;
				if (fread (&c, 1, 1, file) != 1)
				{
					fprintf (stderr, "%s is corrupt\n", filename);
					fclose (file);
					free (*data);
					*data = NULL;
					return;
				}
				memset (&decodepos[x], c, run);
				x += run;
			}
			else
			{
				decodepos[x++] = c;
			}
		}
		if (x > padwidth * *height)
		{
			fprintf (stderr, "eek! %d > %d\n", x, padwidth * (*height));
			fclose (file);
			free (*data);
		}
	}

	if (fread (&c, 1, 1, file) == 1 && c == 12)
	{
		memset (palette, 0, 768);
		fread (palette, 3, 256, file);
	}
	else
	{
		decodepos = palette;
		for (x = 0; x < 256; x++)
		{
			decodepos[0] = x;
			decodepos[1] = x;
			decodepos[2] = x;
			decodepos += 3;
		}
	}

	fclose (file);
}

static void LoadBMP (char *filename, UBYTE **data, int *width, int *height,
	int *srcwidth, int *cx, int *cy, UBYTE *palette)
{
	FILE *file;
	BitmapFileHeader fheader;
	BitmapInfoHeader iheader;
	ULONG isize;
	int padwidth, step;
	int y;
	UBYTE *decodepos;
	UBYTE c;

	*cx = 0x8000;
	*data = NULL;

	file = fopen (filename, "rb");
	if (file == NULL)
	{
		fprintf (stderr, "Could not open %s for reading\n", filename);
		return;
	}
	if (fread (&fheader, sizeof(fheader), 1, file) != 1 ||
		fheader.id[0] != 'B' ||
		fheader.id[1] != 'M')
	{
		fprintf (stderr, "%s is not a bmp file\n", filename);
		fclose (file);
		return;
	}
	if (fread (&isize, 4, 1, file) != 1)
	{
		fprintf (stderr, "%s is missing BITMAPINFOHEADER\n", filename);
		fclose (file);
		return;
	}
	isize = LittleLong(isize) - 4;
	if (fread (&iheader.w,
		sizeof(iheader)-4 > isize ? isize : sizeof(iheader)-4,
		1, file) != 1)
	{
		fprintf (stderr, "%s is missing BITMAPINFOHEADER\n", filename);
		fclose (file);
		return;
	}

	iheader.w = LittleLong (iheader.w);
	iheader.h = LittleLong (iheader.h);
	iheader.nPlanes = LittleShort (iheader.nPlanes);
	iheader.bitCount = LittleShort (iheader.bitCount);
	iheader.compression = LittleLong (iheader.compression);
	iheader.sizeImage = LittleLong (iheader.sizeImage);
	iheader.xPelsPerMeter = LittleLong (iheader.xPelsPerMeter);
	iheader.yPelsPerMeter = LittleLong (iheader.yPelsPerMeter);
	iheader.clrUsed = LittleLong (iheader.clrUsed);
	iheader.clrImportant = LittleLong (iheader.clrImportant);

	if (iheader.nPlanes != 1)
	{
		fprintf (stderr, "%s has %d planes (should be 1).\n", filename, iheader.nPlanes);
		fclose (file);
		return;
	}
	if (iheader.bitCount != 8)
	{
		fprintf (stderr, "%s is not 8 bit.\n", filename);
		fclose (file);
		return;
	}
	if (iheader.compression != 0)
	{
		fprintf (stderr, "%s must be uncompressed.\n", filename);
		fclose (file);
		return;
	}

	fseek (file, sizeof(fheader) + isize + 4, SEEK_SET);
	for (y = 0; y < 256; y++)
	{
		if (fread (palette + y*3, 1, 3, file) != 3)
		{
			fprintf (stderr, "%s has an incomplete palette.\n", filename);
			break;
		}
		if (fread (&c, 1, 1, file) != 1)
		{
			fprintf (stderr, "%s has an incomplete palette.\n", filename);
			break;
		}
	}

	*srcwidth = iheader.w;
	*height = abs (iheader.h);
	*width = padwidth = (iheader.w+3) & (~3);
	fprintf (stderr, "Dimensions: %d x %d\n", *srcwidth, *height);

	*data = malloc (padwidth * (*height));
	memset (*data, 0, padwidth * (*height));

	if (*data == NULL)
	{
		fprintf (stderr, "out of memory\n");
		fclose (file);
		return;
	}

	if (iheader.h > 0)
	{ // bottom-up DIB
		decodepos = *data + (iheader.h - 1) * padwidth;
		step = -padwidth;
	}
	else
	{ // top-down DIB
		decodepos = *data;
		step = padwidth;
	}

	for (y = abs(iheader.h); y > 0; y--)
	{
		if (fread (decodepos, padwidth, 1, file) != 1)
		{
			fprintf (stderr, "%s is corrupt\n", filename);
			fclose (file);
			free (*data);
			*data = NULL;
			return;
		}
		decodepos += step;
	}

	fclose (file);
}

static void LoadFON1 (FILE *file, char *filename, UBYTE **data, int *width, int *height,
	int *srcwidth, int *cx, int *cy, UBYTE *palette)
{
	UWORD w, h;
	UBYTE *buff;
	int i;

	w = h = 0;
	fread (&w, 2, 1, file);
	fread (&h, 2, 1, file);

	if (w == 0 || h == 0)
	{
		fprintf (stderr, "%s is not a valid console font\n", filename);
		return;
	}

	w = LittleShort (w);
	h = LittleShort (h);

	buff = malloc (w * h);
	if (buff == NULL)
	{
		fprintf (stderr, "Out of memory\n");
		return;
	}

	*data = malloc (w * h * 256);
	if (*data == NULL)
	{
		fprintf (stderr, "Out of memory\n");
		free (buff);
		return;
	}

	for (i = 0; i < 256; ++i)
	{
		UBYTE *dest, *src;
		int y;

		if (Unpack (file, filename, buff, w*h))
		{
			free (buff);
			free (*data);
			*data = NULL;
			return;
		}

		src = buff;
		dest = *data + w*(i&15) + h*w*(i&0xf0);

		for (y = h; y > 0; --y)
		{
			memcpy (dest, src, w);
			dest += w*16;
			src += w;
		}
	}

	free (buff);

	*width = w * 16;
	*height = h * 16;
	*srcwidth = w * 16;

	palette[0] = 0;
	palette[1] = 0;
	palette[2] = 255;

	for (i = 1; i < 256; ++i)
	{
		palette[3*i  ] = i;
		palette[3*i+1] = i;
		palette[3*i+2] = i;
	}
}

static void LoadFON2 (FILE *file, char *filename, UBYTE **data, int *width, int *height,
	int *srcwidth, int *cx, int *cy, UBYTE *palette)
{
	FontHeader header;
	UWORD widths[256];
	int i, j, k, w, h, pixels, totalwidth, maxwidth;
	UBYTE *buff;

	if (fread (&header.FontHeight, sizeof (header)-4, 1, file) != 1)
	{
		goto tooshort;
	}

	header.FontHeight = LittleShort (header.FontHeight);

	// Read character widths
	memset (widths, 0, sizeof(widths));
	if (header.bConstantWidth)
	{
		UWORD width;

		if (fread (&width, 2, 1, file) != 1)
		{
			goto tooshort;
		}
		width = LittleShort (width);
		for (i = header.FirstChar; i <= header.LastChar; ++i)
		{
			widths[i] = width;
		}
		totalwidth = (width + 1) * (header.LastChar - header.FirstChar + 1) + 1;
		maxwidth = width;
	}
	else
	{
		size_t count = header.LastChar - header.FirstChar + 1;
		if (fread (&widths[header.FirstChar], 2, count, file) != count)
		{
			goto tooshort;
		}
		totalwidth = 1;
		maxwidth = 0;
		for (i = header.FirstChar; i <= header.LastChar; ++i)
		{
			widths[i] = LittleShort (widths[i]);
			totalwidth += widths[i] + 1;
			if (widths[i] > maxwidth)
			{
				maxwidth = widths[i];
			}
		}
	}

	// Read palette
	memset (palette, 0, 768);
	if (fread (palette, 3, header.PaletteSize, file) != header.PaletteSize)
	{
		goto tooshort;
	}
	if (fread (palette+255*3, 3, 1, file) != 1)
	{
		goto tooshort;
	}

	// Determine a suitable image size
	pixels = totalwidth * (header.FontHeight + 2);
	w = (int)sqrt ((double)pixels);
	if (w < maxwidth + 2) w = maxwidth + 2;
	h = 1;
	j = 1;

	for (i = header.FirstChar; i <= header.LastChar; ++i)
	{
		if (j + widths[i] + 1 > w)
		{
			j = 1;
			h++;
		}
		j += widths[i] + 1;
	}
	h = h * (header.FontHeight + 1) + 1;
	fprintf (stderr, "Dimensions: %d x %d\n", w, h);

	buff = malloc (maxwidth * header.FontHeight);
	if (buff == NULL)
	{
		fprintf (stderr, "Out of memory\n");
		return;
	}

	*data = malloc (w * h);
	if (*data == NULL)
	{
		free (buff);
		fprintf (stderr, "Out of memory\n");
		return;
	}
	memset (*data, 0, w * h);

	j = 1;
	k = 1;
	for (i = header.FirstChar; i <= header.LastChar; ++i)
	{
		UBYTE *dest, *src;
		int y;

		if (widths[i])
		{
			if (Unpack (file, filename, buff, widths[i]*header.FontHeight))
			{
				free (buff);
				free (*data);
				*data = NULL;
				return;
			}
		}

		if (j + widths[i] + 1 > w)
		{
			BoxRow (*data, j, k, header.FontHeight, w);
			j = 1;
			k += header.FontHeight + 1;
		}

		src = buff;
		dest = *data + k*w + j;
		for (y = header.FontHeight; y > 0; --y)
		{
			*(dest - 1) = 255;
			memcpy (dest, src, widths[i]);
			dest += w;
			src += widths[i];
		}
		j += widths[i] + 1;
	}

	BoxRow (*data, j, k, header.FontHeight, w);
	free (buff);

	*width = *srcwidth = w;
	*height = h;

	return;

tooshort:
	fprintf (stderr, "%s is too short\n", filename);
	return;
}

static void BoxRow (UBYTE *dest, int j, int k, int y, int w)
{
	dest += (k-1)*w;
	memset (dest, 255, j);
	dest += w;
	for (; y > 0; --y)
	{
		dest[j-1] = 255;
		dest += w;
	}
	memset (dest, 255, j);
}

static void LoadIMGZ (FILE *file, char *filename, UBYTE **data, int *width, int *height,
	int *srcwidth, int *cx, int *cy, UBYTE *palette)
{
	RawImageHeader header;
	int i;

	if (fread (&header.Width, sizeof(header)-4, 1, file) != 1)
	{
		fprintf (stderr, "%s is too short\n", filename);
		return;
	}

	header.Width = LittleShort (header.Width);
	header.Height = LittleShort (header.Height);
	header.LeftOffset = LittleShort (header.LeftOffset);
	header.TopOffset = LittleShort (header.TopOffset);

	fprintf (stderr, "Dimensions: %d x %d\nOrigin: (%d, %d)\n",
		header.Width, header.Height,
		header.LeftOffset, header.TopOffset);

	if (header.Width == 0 || header.Height == 0)
	{
		fprintf (stderr, "%s has bad size\n", filename);
		return;
	}

	*data = malloc (header.Width * header.Height);
	if (*data == NULL)
	{
		fprintf (stderr, "Out of memory\n");
	}

	switch (header.Compression)
	{
	case 0:
		i = fread (*data, 1, header.Width * header.Height, file);
		break;

	case 1:
		i = header.Width * header.Height;
		i -= Unpack (file, filename, *data, i);
		break;

	default:
		fprintf (stderr, "%s has unknown compression %d\n", filename, header.Compression);
		free (*data);
		*data = NULL;
		return;
	}

	if (i != header.Width * header.Height)
	{
		fprintf (stderr, "Read %d bytes of pixel data. Wanted %d.\n", i,
			header.Width * header.Height);
	}

	*width = *srcwidth = header.Width;
	*height = header.Height;
	*cx = header.LeftOffset;
	*cy = header.TopOffset;

	// Right now, IMGZ does not store a palette, so construct a simple
	// one that goes from white to black.
	for (i = 0; i < 256; ++i)
	{
		palette[i*3  ] = 255-i;
		palette[i*3+1] = 255-i;
		palette[i*3+2] = 255-i;
	}
}

static void LoadPatch (FILE *file, char *filename, UBYTE **data, int *width, int *height,
	int *srcwidth, int *cx, int *cy, UBYTE *palette)
{
	DoomPatch *patch;
	size_t patchSize;
	int x;

	fseek (file, 0, SEEK_END);
	patchSize = ftell (file);
	fseek (file, 0, SEEK_SET);

	patch = malloc (patchSize);
	if (patch == NULL)
	{
		fprintf (stderr, "out of memory\n");
		return;
	}

	x = fread (patch, 1, patchSize, file);

	if ((size_t)x != patchSize)
	{
		fprintf (stderr, "only read %d bytes of %s\n", x, filename);
		free (patch);
		return;
	}

	patch->Width = LittleShort (patch->Width);
	patch->Height = LittleShort (patch->Height);

	// Do some validity checks
	for (x = 0; x < patch->Width; ++x)
	{
		if ((size_t)x*4+8 >= patchSize ||
			(patch->ColumnOfs[x] = LittleLong(patch->ColumnOfs[x])) >= patchSize)
		{
			fprintf (stderr, "%s is not a Doom patch\n", filename);
			free (patch);
			return;
		}
	}

	*cx = LittleShort (patch->LeftOffset);
	*cy = LittleShort (patch->TopOffset);
	*srcwidth = patch->Width;
	*width = patch->Width;
	*height = patch->Height;
	fprintf (stderr, "Dimensions: %d x %d\n", patch->Width, patch->Height);

	*data = malloc (patch->Width * patch->Height);
	if (*data == NULL)
	{
		fprintf (stderr, "out of memory\n");
		free (patch);
		return;
	}
	memset (*data, 0, patch->Width * patch->Height);

	for (x = 0; x < patch->Width; ++x)
	{
		DoomPost *column = (DoomPost *)((BYTE *)patch + patch->ColumnOfs[x]);

		while (column->TopDelta != 255)
		{
			int y = column->Length;

			UBYTE *out = *data + x + column->TopDelta*patch->Width;
			UBYTE *in = (UBYTE *)column + 3;

			while (y != 0)
			{
				// Notice: 0 is our transparent color, not 247
				if (*in == 0)
					*out = 247;
				else
					*out = *in;
				out += patch->Width;
				in += 1;
				y -= 1;
			}

			column = (DoomPost *)((BYTE *)column + column->Length + 4);
		}
	}

	free (patch);

	if (palette != NULL)
	{
		memcpy (palette, DoomPalette, 768);
	}
}

static int Unpack (FILE *file, const char *filename, UBYTE *dest, int destSize)
{
	do
	{
		int code = fgetc (file);
		if (code == EOF)
		{
eof:
			fprintf (stderr, "%s is too short\n", filename);
			return destSize;
		}
		if (!(code & 0x80))
		{
			int left = code+1;
			destSize -= left;
			while (left > 0)
			{
				code = fgetc (file);
				if (code == EOF)
				{
					goto eof;
				}
				*dest++ = (UBYTE)code;
				left--;
			}
		}
		else if (code != 0x80)
		{
			int run = (256-code)+1;
			destSize -= run;
			code = fgetc (file);
			if (code == EOF)
			{
				goto eof;
			}
			memset (dest, code, run);
			dest += run;
		}
	} while (destSize > 0);

	return destSize;
}
