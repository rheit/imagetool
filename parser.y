%{
/*
** parser.y
** A Yacc/Bison grammar that specifies the recognized scripting language.
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

#include <malloc.h>
#include "afx.h"

void yyerror (char *);
int yylex (void);
%}

%union
{
	int i;
	double f;
	char *s;
	ShadeType shade;
}

%token <i> INT
%token <f> FLOAT
%token <s> STRING

%token ELLIPSIS
%token RIGHT_ASSIGN
%token LEFT_ASSIGN
%token ADD_ASSIGN
%token SUB_ASSIGN
%token MUL_ASSIGN
%token DIV_ASSIGN
%token MOD_ASSIGN
%token AND_ASSIGN
%token XOR_ASSIGN
%token OR_ASSIGN
%token RIGHT_OP
%token LEFT_OP
%token INC_OP
%token DEC_OP
%token PTR_OP
%token AND_OP
%token OR_OP
%token LE_OP
%token GE_OP
%token EQ_OP
%token NE_OP
%token ID

%token LOAD
%token CONFONT
%token XHAIR
%token IMAGE
%token FONT
%token SHADING
%token NORMAL
%token CONSOLE
%token TRANSPARENT
%token ORIGIN

%type <shade> shadetype

%%

start: | commands { if (ImageData != NULL) free (ImageData); };

/* An image must be loaded before any file-creators are used */

commands: commands1 image commands2;

commands1: | commands1 nonimage;
commands2: | commands2 command2;
command2: nonimage | image;

/* A nonimage is a command that does not require an image to be loaded */

nonimage:
	LOAD STRING ';'
	{
		if (ImageData != NULL)
			free (ImageData);
		LoadPic ($2, &ImageData, &ImageWidth, &ImageHeight, &ImageSrcWidth,
			&ImageCX, &ImageCY, ImagePalette);
		free ($2);
	}

	| TRANSPARENT INT ';'
	{
		if ($2 >= 0 && $2 <= 255)
			RetransImage = $2;
		else
			yyerror ("Transparent color out of range\n");
	}
;

/* An image is a command that does require an image to be loaded */

image:
	ORIGIN '(' INT ',' INT ')' ';'
	{
		ImageCX = $3;
		ImageCY = $5;
	}

	| CONFONT STRING ';'
	{
		if (ImageData)
			WriteConFont ($2, ImageData, ImageWidth, ImageHeight, ImageSrcWidth);
		free ($2);
	}

	| XHAIR STRING ';'
	{
		if (ImageData)
		{
			if (ImageCX == 0x8000)
			{
				ImageCX = ImageSrcWidth / 2;
				ImageCY = ImageHeight / 2;
			}
			WriteImage ($2, ImageData, ImageWidth, ImageHeight, ImageSrcWidth,
						ImageCX, ImageCY, ImagePalette);
		}
		free ($2);
	}

	| IMAGE STRING ';'
	{
		if (ImageData)
		{
			if (ImageCX == 0x8000)
			{
				ImageCX = 0;
				ImageCY = 0;
			}
			WriteImage ($2, ImageData, ImageWidth, ImageHeight, ImageSrcWidth,
						ImageCX, ImageCY, ImagePalette);
		}
		free ($2);
	}

	| FONT STRING { StartFont ($2, ImageData, ImageWidth, ImageHeight, ImageSrcWidth, ImagePalette); }
	  fontchunks ';' { EndFont (); free ($2); }
;

fontchunks: | fontchunks fontchunk;

fontchunk:
	INT INT INT { GrabFont ($1, $2, $3); }		/* x y startchar */

	| SHADING '=' shadetype { SetFontShading ($3); }
;

shadetype:
	NORMAL		{ $$ = SHADING_Normal; }
	| CONSOLE	{ $$ = SHADING_Console; }
;

%%

extern int column, lineno;

void yyerror (char *s)
{
	fprintf (stderr, "line %d, col %d: %s\n", lineno, column+1, s);
}
