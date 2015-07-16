typedef struct
{
	UWORD	Width;
	UWORD	Height;
	WORD	LeftOffset;
	WORD	TopOffset;
	ULONG	ColumnOfs[8];
} DoomPatch;

typedef struct
{
	UBYTE	TopDelta;
	UBYTE	Length;
} DoomPost;
