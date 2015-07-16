/*
** imagewrite.c
** Routines for writing ILBM, PCX and BMP.
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

#include "afx.h"
#include "pcx.h"
#include "bmp.h"
#include "ilbm.h"

#define MAXPLANEWIDTH		(1600/8)

static const char Anno[] = "Created with the ZDoom imagetool.";

static void c2p (UBYTE *planes, int planewidth, UBYTE *src, int width);

int WritePCXfile (const char *filename, UBYTE *data, int pitch, int height,
	int width, UBYTE *palette)
{
	int x, y;
	int runlen;
	UBYTE color;
	pcxHeader pcx;
	FILE *file;

	file = fopen (filename, "wb");
	if (file == NULL)
	{
		fprintf (stderr, "Could not open %s\n", filename);
		return 1;
	}

	pcx.manufacturer = 10;				// PCX id
	pcx.version = 5;					// 256 color
	pcx.encoding = 1;
	pcx.bits_per_pixel = 8;				// 256 color
	pcx.xmin = 0;
	pcx.ymin = 0;
	pcx.xmax = LittleShort(width-1);
	pcx.ymax = LittleShort(height-1);
	pcx.hdpi = LittleShort(75);
	pcx.vdpi = LittleShort(75);
	memset (pcx.palette, 0, sizeof(pcx.palette));
	pcx.reserved = 0;
	pcx.color_planes = 1;				// chunky image
	pcx.bytes_per_line = width + (width & 1);
	pcx.palette_type = 1;				// not a grey scale
	memset (pcx.filler, 0, sizeof(pcx.filler));

	fwrite (&pcx, 128, 1, file);

	// pack the image
	for (y = height; y > 0; y--)
	{
		color = *data++;
		runlen = 1;

		for (x = width - 1; x > 0; x--)
		{
			if (*data == color)
			{
				runlen++;
			}
			else
			{
				if (runlen > 1 || color >= 0xc0)
				{
					while (runlen > 63)
					{
						putc (0xff, file);
						putc (color, file);
						runlen -= 63;
					}
					if (runlen > 0)
					{
						putc (0xc0 + runlen, file);
					}
				}
				if (runlen > 0)
				{
					putc (color, file);
				}
				runlen = 1;
				color = *data;
			}
			data++;
		}

		if (runlen > 1 || color >= 0xc0)
		{
			while (runlen > 63)
			{
				putc (0xff, file);
				putc (color, file);
				runlen -= 63;
			}
			if (runlen > 0)
			{
				putc (0xc0 + runlen, file);
			}
		}
		if (runlen > 0)
		{
			putc (color, file);
		}

		if (width & 1)
			putc (0, file);

		data += pitch - width;
	}

	// write the palette
	putc (12, file);		// palette ID byte
	fwrite (palette, 3, 256, file);

	fclose (file);

	return 0;
}

int WriteBMPfile (const char *filename, UBYTE *data, int pitch, int height,
	int width, UBYTE *palette)
{
	BitmapInfoHeader header;
	FILE *file;
	ULONG temp;
	UBYTE zeros[8];
	int i;
	int padwidth;

	file = fopen (filename, "wb");
	if (file == NULL)
	{
		fprintf (stderr, "Cannot open %s\n", filename);
		return 1;
	}

	padwidth = (width + 3) & (~3);
	memset (zeros, 0, sizeof(zeros));

	fwrite ("BM", 2, 1, file);
	temp = LittleLong (sizeof(BitmapInfoHeader) + 1024 + 14 + padwidth * height);
	fwrite (&temp, 4, 1, file);
	temp = 0;
	fwrite (&temp, 4, 1, file);
	temp = sizeof(BitmapInfoHeader) + 1024 + 14;
	fwrite (&temp, 4, 1, file);

	header.size = LittleLong (sizeof(header));
	header.w = LittleLong (width);
	header.h = LittleLong (height);
	header.nPlanes = LittleShort (1);
	header.bitCount = LittleShort (8);
	header.compression = 0;
	header.sizeImage = padwidth * height;
	header.xPelsPerMeter = LittleLong (5039);
	header.yPelsPerMeter = LittleLong (5039);
	header.clrUsed = 0;
	header.clrImportant = 0;
	fwrite (&header, sizeof(header), 1, file);

	for (i = 0; i < 256; ++i)
	{
		temp = MAKE_ID (palette[i*3+2], palette[i*3+1], palette[i*3], 0);
		fwrite (&temp, 4, 1, file);
	}

	padwidth = padwidth - width;

	for (i = height-1; i >= 0; --i)
	{
		if (fwrite (data+i*pitch, 1, width, file) != (unsigned)width ||
			(padwidth && fwrite (zeros, 1, padwidth, file) != (unsigned)padwidth))
		{
			fclose (file);
			return 1;
		}
	}

	fclose (file);
	return 0;
}

int WriteILBMfile (const char *filename, UBYTE *data, int pitch, int height,
	int width, int cx, int cy, UBYTE *palette)
{
	BitmapHeader header;
	ULONG temp1, temp2;
	FILE *file;
	int padwidth, planewidth;
	UBYTE planes[8][MAXPLANEWIDTH];
	int i;

	padwidth = (width + 15) & ~15;
	planewidth = ((width + 15) / 16) * 2;

	if (planewidth > MAXPLANEWIDTH)
	{
		fprintf (stderr, "%s is too wide. (Max is %d pixels.)\n",
			filename, MAXPLANEWIDTH*8);
		return 1;
	}

	file = fopen (filename, "wb");
	if (file == NULL)
	{
		fprintf (stderr, "Could not open %s for writing\n", filename);
		return 1;
	}

	temp1 = ID_FORM;
	fwrite (&temp1, 4, 1, file);
	fwrite (&temp1, 4, 1, file);
	temp1 = ID_ILBM;
	fwrite (&temp1, 4, 1, file);

	temp1 = ID_ANNO;
	temp2 = BigLong (sizeof(Anno));
	fwrite (&temp1, 4, 1, file);
	fwrite (&temp2, 4, 1, file);
	fwrite (Anno, 1, sizeof(Anno), file);

	if (cx != 0x8000)
	{
		WORD val;

		temp1 = ID_GRAB;
		temp2 = BigLong (4);
		fwrite (&temp1, 4, 1, file);
		fwrite (&temp2, 4, 1, file);
		val = BigShort (cx);
		fwrite (&val, 2, 1, file);
		val = BigShort (cy);
		fwrite (&val, 2, 1, file);
	}

	temp1 = ID_CMAP;
	temp2 = BigLong (768);
	fwrite (&temp1, 4, 1, file);
	fwrite (&temp2, 4, 1, file);
	fwrite (palette, 3, 256, file);

	temp1 = ID_BMHD;
	temp2 = BigLong (sizeof(header));
	header.w = BigShort (width);
	header.h = BigShort (height);
	header.x = 0;
	header.y = 0;
	header.nPlanes = 8;
	header.masking = mskHasTransparentColor;
	header.compression = cmpByteRun1;
	header.pad1 = 0;
	header.transparentColor = 0;
	header.xAspect = 44;
	header.yAspect = 44;
	header.pageWidth = header.w;
	header.pageHeight = header.h;
	fwrite (&temp1, 4, 1, file);
	fwrite (&temp2, 4, 1, file);
	fwrite (&header, sizeof(header), 1, file);

	temp1 = ID_BODY;
	fwrite (&temp1, 4, 1, file);
	temp1 = ftell (file);
	fwrite (&temp1, 4, 1, file);

	for (i = 0; i < height; ++i)
	{
		int plane;

		memset (planes, 0, sizeof(planes));
		c2p (planes[0], MAXPLANEWIDTH, data + i*pitch, width);
		for (plane = 0; plane < 8; ++plane)
		{
			BYTE *source_p = &planes[plane][0];
			packrow (&source_p, file, planewidth);
		}
	}

	temp2 = ftell (file);
	if (temp2 & 1)
	{
		putc (0, file);
	}
	fseek (file, temp1, SEEK_SET);
	temp1 = BigLong (temp2 - temp1 - 4);
	fwrite (&temp1, 4, 1, file);
	temp1 = BigLong (temp2 + (temp2&1) - 8);
	fseek (file, 4, SEEK_SET);
	fwrite (&temp1, 4, 1, file);
	fclose (file);

	return 0;
}

static void c2p (UBYTE *planes, int planewidth, UBYTE *src, int width)
{
	int i, j;

	for (i = 0; i < width; ++i)
	{
		UBYTE chunk = src[i];
		int bit = 7-(i&7);
		int byt = i>>3;

		for (j = 0; j < 8; ++j)
		{
			planes[j*planewidth+byt] |= (chunk&1) << bit;
			chunk >>= 1;
		}
	}
}
