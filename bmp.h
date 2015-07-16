#pragma pack(1)

typedef struct
{
	UBYTE id[2];
	ULONG size;
	UWORD reserved1;
	UWORD reserved2;
	ULONG offBits;
} BitmapFileHeader;

#pragma pack()

typedef struct
{
	ULONG size;
	ULONG w, h;
	UWORD nPlanes;
	UWORD bitCount;
	ULONG compression;
	ULONG sizeImage;
	ULONG xPelsPerMeter;
	ULONG yPelsPerMeter;
	ULONG clrUsed;
	ULONG clrImportant;
} BitmapInfoHeader;
