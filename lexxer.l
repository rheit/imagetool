/*
** lexxer.l
** The lexxer used by the script engine. Recognizes way more keywords than I
** need it too. Mostly just an experiment in the use of flex, and adapted
** from some standard C lexer I found somewhere.
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

%option noyywrap

L						[_a-zA-Z]
D						[0-9]
O						[0-7]
H						[a-fA-F0-9]
E						[Ee][+-]?{D}+
FS						(f|F|l|L)
US						(u|U)
LS						(l|L)
IS						({US}{LS}?|{LS}{US}?)
singleop				[\[\]\(\)\{\}\.\&\*\+\-\~\!\/\%\^\|\?\:\=\,\#\;\<\>]
ws						[ \t\v\n\f]
ses						[abfnrtv\\\"\'\?]

%{
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "afx.h"
#include "parser.h"

int column = 0;
int lineno = 1;

static void comment(void);
static void cppcomment (void);
static void count(void);
static int intconst(void);
static int charconst(void);
static double floatconst(void);
static char *stringconst(void);
%}

%%
"/*"					{ comment(); }
"//"					{ cppcomment(); }

{L}({L}|{D})			{ count(); return(ID); }

0[xX]{H}+{IS}?			{ count(); yylval.i = intconst(); return(INT); }
0{O}+{IS}?				{ count(); yylval.i = intconst(); return(INT); }
{D}+{IS}?				{ count(); yylval.i = intconst(); return(INT); }
L?'(\\{ses}|[^\\'])'	{ count(); yylval.i = charconst(); return(INT); }

{D}+{E}{FS}?			{ count(); yylval.f = floatconst(); return(FLOAT); }
{D}*"."{D}+({E})?{FS}?	{ count(); yylval.f = floatconst(); return(FLOAT); }
{D}+"."{D}*({E})?{FS}?	{ count(); yylval.f = floatconst(); return(FLOAT); }

L?\"(\\.|[^\\"])*\" 	{ count(); yylval.s = stringconst(); return(STRING); }

"load"					{ count(); return(LOAD); }
"confont"				{ count(); return(CONFONT); }
"font"					{ count(); return(FONT); }
"xhair"					{ count(); return(XHAIR); }
"image"					{ count(); return(IMAGE); }
"shading"				{ count(); return(SHADING); }
"normal"				{ count(); return(NORMAL); }
"console"				{ count(); return(CONSOLE); }
"transparentcolor"		{ count(); return(TRANSPARENT); }
"origin"				{ count(); return(ORIGIN); }

"..."					{ count(); return(ELLIPSIS); }
">>="					{ count(); return(RIGHT_ASSIGN); }
"<<="					{ count(); return(LEFT_ASSIGN); }
"+="					{ count(); return(ADD_ASSIGN); }
"-="					{ count(); return(SUB_ASSIGN); }
"*="					{ count(); return(MUL_ASSIGN); }
"/="					{ count(); return(DIV_ASSIGN); }
"%="					{ count(); return(MOD_ASSIGN); }
"&="					{ count(); return(AND_ASSIGN); }
"^="					{ count(); return(XOR_ASSIGN); }
"|="					{ count(); return(OR_ASSIGN); }
">>"					{ count(); return(RIGHT_OP); }
"<<"					{ count(); return(LEFT_OP); }
"++"					{ count(); return(INC_OP); }
"--"					{ count(); return(DEC_OP); }
"->"					{ count(); return(PTR_OP); }
"&&"					{ count(); return(AND_OP); }
"||"					{ count(); return(OR_OP); }
"<="					{ count(); return(LE_OP); }
">="					{ count(); return(GE_OP); }
"=="					{ count(); return(EQ_OP); }
"!="					{ count(); return(NE_OP); }
{singleop}				{ count(); return(yytext[0]); }

{ws}		 			{ count(); }
.						{ /* ignore bad characters */ }

%%

static void comment (void)
{
	char c, c1;

	c1 = 0;
	do
	{
		c = c1;
		do
		{
			if (c == '\n')
				column = 0, lineno++;
			else if (c == '\t')
				column += 8 - (column & 7);
			else if (c)
				column++;
		} while ((c = input()) != '*' && c != 0);
	} while ((c1 = input()) != '/' && c != 0);
}

static void cppcomment (void)
{
	char c;

	while ((c = input()) != '\n' && c != 0)
		;

	column = 0;
	lineno++;
}

static void count(void)
{
	int i;

	for (i = 0; yytext[i] != '\0'; ++i)
	{
		if (yytext[i] == '\n')
			column = 0, lineno++;
		else if (yytext[i] == '\t')
			column += 8 - (column & 7);
		else
			column++;
	}
}

static int intconst(void)
{
	return strtol (yytext, NULL, 0);
}

static double floatconst(void)
{
	return strtod (yytext, NULL);
}

static char *stringconst(void)
{
	yytext[yyleng-1] = 0;
	return strdup (yytext[0] != '\"' ? yytext+2 : yytext+1);
}

static int charconst(void)
{
	if (yytext[1] == '\\')
	{
		switch (yytext[2])
		{
		case 'a': return '\a';
		case 'b': return '\b';
		case 'f': return '\f';
		case 'n': return '\n';
		case 'r': return '\r';
		case 't': return '\t';
		case 'v': return '\v';
		default:  return yytext[2];
		}
	}
	return yytext[1];
}
