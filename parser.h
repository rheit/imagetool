#ifndef BISON_PARSER_H
# define BISON_PARSER_H

#ifndef YYSTYPE
typedef union
{
	int i;
	double f;
	char *s;
	ShadeType shade;
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif
# define	INT	257
# define	FLOAT	258
# define	STRING	259
# define	ELLIPSIS	260
# define	RIGHT_ASSIGN	261
# define	LEFT_ASSIGN	262
# define	ADD_ASSIGN	263
# define	SUB_ASSIGN	264
# define	MUL_ASSIGN	265
# define	DIV_ASSIGN	266
# define	MOD_ASSIGN	267
# define	AND_ASSIGN	268
# define	XOR_ASSIGN	269
# define	OR_ASSIGN	270
# define	RIGHT_OP	271
# define	LEFT_OP	272
# define	INC_OP	273
# define	DEC_OP	274
# define	PTR_OP	275
# define	AND_OP	276
# define	OR_OP	277
# define	LE_OP	278
# define	GE_OP	279
# define	EQ_OP	280
# define	NE_OP	281
# define	ID	282
# define	LOAD	283
# define	CONFONT	284
# define	XHAIR	285
# define	IMAGE	286
# define	FONT	287
# define	SHADING	288
# define	NORMAL	289
# define	CONSOLE	290
# define	TRANSPARENT	291
# define	ORIGIN	292


extern YYSTYPE yylval;

#endif /* not BISON_PARSER_H */
