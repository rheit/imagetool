/*
** makeimage.c
** Routines for creating IMGZ files.
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

int WriteImage (const char *name, UBYTE *data, int width, int height,
				int srcwidth, int cx, int cy, UBYTE *palette)
{
	FILE *f;
	RawImageHeader header;
	int i;
	int cprsize;
	BYTE *data_p;

	f = fopen (name, "wb");
	if (f == NULL)
	{
		printf ("could not open %s\n", name);
		return 1;
	}

	memset (&header, 0, sizeof(header));
	header.Magic[0] = 'I';
	header.Magic[1] = 'M';
	header.Magic[2] = 'G';
	header.Magic[3] = 'Z';
	header.Width = LittleShort(srcwidth);
	header.Height = LittleShort(height);
	header.LeftOffset = LittleShort(cx);
	header.TopOffset = LittleShort(cy);
	header.Compression = 1;

	cprsize = 0;
	data_p = (BYTE *)data;

	fwrite (&header, 1, sizeof(header), f);
	for (i = 0; i < height; i++)
	{
		cprsize += packrow (&data_p, f, srcwidth);
		data_p += width - srcwidth;
	}
	fclose (f);

	if (cprsize > srcwidth * height)
	{
		printf ("compressed to %d (%d larger than uncompressed)\n",
			cprsize, cprsize - srcwidth * height);
		fclose (f);

		f = fopen (name, "wb");
		if (f != NULL)
		{
			printf ("resaving as uncompressed\n");
			header.Compression = 0;
			fwrite (&header, 1, sizeof(header), f);
			data_p = data;
			for (i = 0; i < height; i++)
			{
				fwrite (data_p, srcwidth, 1, f);
				data_p += width;
			}
			fclose (f);
		}
		else
		{
			printf ("could not reopen to write uncompressed version\n");
		}
	}
	else
	{
		printf ("compressed to %d (%d smaller than uncompressed)\n",
			cprsize, srcwidth * height - cprsize);
	}

	return 0;
}
