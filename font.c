/*
** font.c
** Routines for producing FON1 and FON2 files.
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

#define FONT_BORDER		255

typedef struct
{
	UWORD x, y, w;
} CharRect;

static int compare (const void *arg1, const void *arg2);

FontHeader Font;
char *FontName;
UBYTE FontPalette[768];
CharRect Chars[256];
int FontWidth, FontHeight, FontPitch;
UBYTE *FontBitmap;
UBYTE Font255Remap;

void StartFont (const char *fontname, UBYTE *data, int width, int height,
				int srcwidth, UBYTE palette[768])
{
	UBYTE used[256];
	UBYTE remap[256], unmap[256];
	int x, y;
	UBYTE color255;
	int color255best;

	// Set up font book keeping
	memset (&Font, 0, sizeof(Font));
	memset (Chars, 0, sizeof(Chars));
	memcpy (FontPalette, palette, 768);
	FontName = strdup (fontname);
	FontWidth = srcwidth;
	FontPitch = width;
	FontHeight = height;
	FontBitmap = data;
	Font.FirstChar = 255;

	// Find out which of the image's colors are actually used
	memset (used, 0, 256);
	for (y = 0; y < height; ++y)
	{
		UBYTE *colors = data + y*width;
		for (x = 0; x < srcwidth; ++x)
		{
			used[*colors++] = 1;
		}
	}

	// Move those colors to the front of the palette.
	// Color 0 is left alone, because it is the transparent color.
	// Also leave 255 alone, because it is the glyph delimiter.
	memset (remap, 0, 256);
	memset (unmap, 0, 256);
	remap[255] = 255;
	used[0] = 1;
	for (x = y = 0; x < 255; ++x)
	{
		if (used[x])
		{
			remap[x] = y;
			unmap[y] = x;
			y++;
		}
	}

	// Sort the palette by increasing brightness
	Font.PaletteSize = y;
	qsort (unmap+1, y-1, 1, compare);
	color255 = 0;
	color255best = 256*256*4;
	for (x = 1; x < y; ++x)
	{
		int dist, rd, gd, bd;

		remap[unmap[x]] = x;
		rd = FontPalette[x*3  ] = palette[unmap[x]*3  ];
		gd = FontPalette[x*3+1] = palette[unmap[x]*3+1];
		bd = FontPalette[x*3+2] = palette[unmap[x]*3+2];

		rd -= palette[255*3  ];
		gd -= palette[255*3+1];
		bd -= palette[255*3+2];

		dist = rd*rd + gd*gd + bd*bd;
		if (dist < color255best)
		{
			color255best = dist;
			color255 = x;
		}
	}

	// Fix fonts that include palette entry 255 within
	// a glyph's bounding box.
	Font255Remap = color255;

	// Now remap the source image to use the new colors.
	for (y = 0; y < height; ++y)
	{
		UBYTE *colors = data + y*width;
		for (x = 0; x < srcwidth; ++x)
		{
			colors[x] = remap[colors[x]];
		}
	}
}

void GrabFont (int xl, int yl, int firstchar)
{
	int glyph = firstchar;
	int x, y, r, h;

	// Skip top and left box line
	yl++;
	xl++;

	for (y = yl; y < FontHeight; )
	{
		for (x = xl; x < FontWidth; )
		{
			if (glyph < 0 || glyph > 255)
			{
				fprintf (stderr, "%s has glyphs out of range\n", FontName);
				return;
			}

			if (FontBitmap[x+y*FontPitch] == FONT_BORDER)
			{ // Character is empty, so skip it
				glyph++;
				x++;
				continue;
			}

			// Determine glyph height
			for (r = y+1; r < FontHeight; ++r)
			{
				if (FontBitmap[x+r*FontPitch] == FONT_BORDER)
					break;
			}
			if (Font.FontHeight == 0)
			{
				if (r - y == 0)
				{
					fprintf (stderr, "Font has 0 height!\n");
					return;
				}
				Font.FontHeight = h = r - y;
			}
			else
			{
				h = r - y;
			}

			// Determine glyph width
			for (r = x+1; r < FontWidth; ++r)
			{
				if (FontBitmap[r+y*FontPitch] == FONT_BORDER)
					break;
			}

			if (FontBitmap[r-1+(y-1)*FontPitch] != FONT_BORDER)
			{ // If the top border was not present, skip to the next row
				break;
			}

			if (Font.FontHeight != h)
			{
				fprintf (stderr, "Char #%d (%c) has height %d instead of %d\n",
					glyph, glyph, h, Font.FontHeight);
				return;
			}

			// Store glyph position and size
			Chars[glyph].x = x;
			Chars[glyph].y = y;
			Chars[glyph].w = r-x;

			glyph++;
			x = r + 1;
		}
		// Advance to next row of image
		y += Font.FontHeight + 1;
		x = xl;
	}

	glyph--;

	if (glyph >= firstchar)
	{
		if (firstchar < Font.FirstChar)
		{
			Font.FirstChar = firstchar;
		}
		if (glyph > Font.LastChar)
		{
			Font.LastChar = glyph;
		}
	}
}

bool EndFont (void)
{
	BYTE *PackBytes;
	FILE *f;
	UWORD swizzle;
	int i;
	int totalwidth;

	if (Font.FirstChar > Font.LastChar)
	{
		GrabFont (0, 0, ' ');
		if (Font.FirstChar > Font.LastChar || Font.PaletteSize == 0)
		{
			fprintf (stderr, "%s: Nothing to save\n", FontName);
			free (FontName);
			return true;
		}
	}

	f = fopen (FontName, "wb");
	if (f == NULL)
	{
		fprintf (stderr, "Could not open %s\n", FontName);
		free (FontName);
		return true;
	}

	PackBytes = malloc (FontWidth * FontHeight);
	if (PackBytes == NULL)
	{
		fprintf (stderr, "Out of memory\n");
		free (FontName);
		fclose (f);
		return true;
	}

	Font.bConstantWidth = 1;

	for (i = Font.FirstChar+1; i <= Font.LastChar; ++i)
	{
		if (Chars[Font.FirstChar].w != Chars[i].w)
		{
			Font.bConstantWidth = 0;
			break;
		}
	}

	fwrite ("FON2", 4, 1, f);
	swizzle = LittleShort (Font.FontHeight);
	fwrite (&swizzle, 2, 1, f);
	fwrite (&Font.FirstChar, 1, 6, f);
	if (Font.bConstantWidth)
	{
		swizzle = LittleShort (Chars[Font.FirstChar].w);
		fwrite (&swizzle, 2, 1, f);
	}
	else
	{
		for (i = Font.FirstChar; i <= Font.LastChar; ++i)
		{
			swizzle = LittleShort (Chars[i].w);
			fwrite (&swizzle, 2, 1, f);
		}
	}

	fwrite (FontPalette, 3, Font.PaletteSize, f);

	// Write out the color of the delimiter (color 255) so that the source
	// image can be reconstructed with all used colors intact. This color
	// is not included in the PaletteSize count in the header.
	fwrite (FontPalette+255*3, 3, 1, f);

	totalwidth = 0;
	for (i = Font.FirstChar; i <= Font.LastChar; ++i)
	{
		if (Chars[i].w)
		{
			int y;
			UBYTE *glyph = FontBitmap + Chars[i].x + Chars[i].y*FontPitch;
			BYTE *pack_p = PackBytes;
			
			for (y = 0; y < Font.FontHeight; ++y)
			{
				int x;
				for (x = 0; x < Chars[i].w; ++x)
				{
					pack_p[x] = glyph[x] != 255 ? glyph[x] : Font255Remap;
				}
				pack_p += Chars[i].w;
				glyph += FontPitch;
			}
			pack_p = PackBytes;
			packrow (&pack_p, f, Chars[i].w*y);
			totalwidth += Chars[i].w;
		}
	}

	printf ("%s: %d pixels of font glyphs stored\n", FontName, totalwidth * Font.FontHeight);

	free (PackBytes);
	free (FontName);
	fclose (f);

	return false;
}

void SetFontShading (ShadeType shade)
{
	Font.ShadingType = shade;
}

static int compare (const void *arg1, const void *arg2)
{
	if (FontPalette[*((UBYTE *)arg1)*3  ] * 299 +
		FontPalette[*((UBYTE *)arg1)*3+1] * 587 +
		FontPalette[*((UBYTE *)arg1)*3+2] * 114  <
		FontPalette[*((UBYTE *)arg2)*3  ] * 299 +
		FontPalette[*((UBYTE *)arg2)*3+1] * 587 +
		FontPalette[*((UBYTE *)arg2)*3+2] * 114)
		return -1;
	else
		return 1;
}

int WriteConFont (const char *name, UBYTE *data, int width, int height, int srcwidth)
{
	int charwidth = srcwidth / 16;
	int charheight = height / 16;
	int x, y;
	BYTE *shifted = malloc (srcwidth * height);
	BYTE *shift_p;
	FILE *f;

	if (shifted == NULL)
	{
		printf ("out of memory\n");
		return 1;
	}

	shift_p = shifted;
	for (x = 0; x < 256; x++)
	{
		UBYTE *offset = data + (x>>4)*charheight*width + (x&15)*charwidth;
		for (y = 0; y < charheight; y++)
		{
			memcpy (shift_p, offset, charwidth);
			shift_p += charwidth;
			offset += width;
		}
	}

	f = fopen (name, "wb");
	if (f == NULL)
	{
		printf ("could not open %s\n", name);
		return 1;
	}

	fwrite ("FON1", 1, 4, f);
	charwidth = LittleShort(charwidth);
	fwrite (&charwidth, 2, 1, f);
	charwidth = LittleShort(charwidth);
	charheight = LittleShort(charheight);
	fwrite (&charheight, 2, 1, f);
	charheight = LittleShort(charheight);
	shift_p = shifted;
	for (x = 0; x < 256; x++)
	{
		packrow (&shift_p, f, charwidth*charheight);
	}
	fclose (f);
	free (shifted);
	return 0;
}
