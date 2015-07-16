/*
** imagetool.c
** Glue.
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

extern FILE *yyin;
extern int yyparse (void);

enum
{
	MODE_ConFont,
	MODE_Image,
	MODE_Crosshair,
	MODE_Font,
	MODE_PCX,
	MODE_BMP,
	MODE_ILBM,
};

UBYTE RetransImage = 0;

void usage (void)
{
	printf ("Usage: imagetool [-0] <type> <source> <output>\n"
			"<type> can be:\n"
			"\tconfont : Monospaced console font\n"
			"\tfont    : Normal font\n"
			"\timage   : Raw image\n"
			"\txhair   : Crosshair\n"
			"\tpcx     : Convert <source> to a PCX file\n"
			"\tbmp     : Convert <source> to a BMP file\n"
			"\tilbm    : Convert <source> to an ILBM file\n"
			"<source> can be an ILBM, BMP, PCX, IMGZ, FON1, FON2, or Doom patch.\n"
			"Specify -0 to swap colors 0 and 247 in <source>.\n\n"
			"Alternatively, to process a script file, in place of <type>, use:\n"
			"\timagetool script <file>\n"
			);
	exit (10);
}

int main (int argc, char **argv)
{
	UBYTE palette[768];
	int width, height, srcwidth;
	UBYTE *data;
	int mode;
	int failed;
	int cx, cy;
	int argstart;

	if (argc < 3)
	{
		usage ();
	}

	if (argv[1][0] == '-')
	{
		if (argv[1][1] == '0')
			RetransImage = 247;
		argstart = 2;
	}
	else
	{
		argstart = 1;
	}

	if (stricmp (argv[argstart], "confont") == 0)
	{
		mode = MODE_ConFont;
	}
	else if (stricmp (argv[argstart], "image") == 0)
	{
		mode = MODE_Image;
	}
	else if (stricmp (argv[argstart], "font") == 0)
	{
		mode = MODE_Font;
	}
	else if (stricmp (argv[argstart], "xhair") == 0)
	{
		mode = MODE_Crosshair;
	}
	else if (stricmp (argv[argstart], "script") == 0)
	{
		if (argc - argstart < 2)
		{
			usage ();
		}
		yyin = fopen (argv[argstart+1], "r");
		if (yyin == NULL)
		{
			fprintf (stderr, "Could not open %s\n", argv[argstart+1]);
			return 20;
		}
		return yyparse ();
	}
	else if (stricmp (argv[argstart], "pcx") == 0)
	{
		mode = MODE_PCX;
	}
	else if (stricmp (argv[argstart], "bmp") == 0)
	{
		mode = MODE_BMP;
	}
	else if (stricmp (argv[argstart], "ilbm") == 0)
	{
		mode = MODE_ILBM;
	}
	else
	{
		usage ();
	}

	if (argc - argstart < 3)
	{
		usage ();
	}

	LoadPic (argv[argstart+1], &data, &width, &height, &srcwidth, &cx, &cy, palette);
	if (data == NULL)
		return 20;

	switch (mode)
	{
	case MODE_ConFont:
		failed = WriteConFont (argv[argstart+2], data, width, height, srcwidth);
		break;

	case MODE_Font:
		StartFont (argv[argstart+2], data, width, height, srcwidth, palette);
		failed = EndFont ();
		break;

	case MODE_Crosshair:
		if (cx == 0x8000)
		{
			cx = srcwidth / 2;
			cy = height / 2;
		}
		/* intentional fallthrough */
	case MODE_Image:
		if (cx == 0x8000)
		{
			cx = 0;
			cy = 0;
		}
		failed = WriteImage (argv[argstart+2], data, width, height, srcwidth, cx, cy, palette);
		break;

	case MODE_PCX:
		failed = WritePCXfile (argv[argstart+2], data, width, height, srcwidth, palette);
		break;

	case MODE_BMP:
		failed = WriteBMPfile (argv[argstart+2], data, width, height, srcwidth, palette);
		break;

	case MODE_ILBM:
		failed = WriteILBMfile (argv[argstart+2], data, width, height, srcwidth, cx, cy, palette);
		break;
	}

	free (data);

	return failed;
}
