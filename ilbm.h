typedef UBYTE Masking;		/* Choice of masking technique. */

#define mskNone					0
#define mskHasMask				1
#define mskHasTransparentColor	2
#define mskLasso				3

typedef UBYTE Compression;	/* Choice of compression algorithm
	applied to the rows of all source and mask planes. "cmpByteRun1"
	is the byte run encoding described in Appendix C. Do not compress
	across rows! */
#define cmpNone			0
#define cmpByteRun1		1

typedef struct {
	UWORD		w, h;			/* raster width & height in pixels			*/
	WORD		x, y;			/* pixel position for this image			*/
	UBYTE		nPlanes;		/* # source bitplanes						*/
	Masking		masking;
	Compression	compression;
	UBYTE		pad1;			/* unused; ignore on read, write as 0		*/
	UWORD		transparentColor; /* transparent "color number" (sort of)	*/
	UBYTE		xAspect, yAspect; /* pixel aspect, a ratio width : height	*/
	WORD		pageWidth, pageHeight; /* source "page" size in pixels		*/
} BitmapHeader;

typedef struct {
	UBYTE red, green, blue;			/* color intensities 0..255 */
} ColorRegister;					/* size = 3 bytes			*/

typedef struct {
	UBYTE depth;		/* # bitplanes in the original source				*/
	UBYTE pad1;			/* unused; for consistency put 0 here				*/
	UWORD planePick;	/* how to scatter source bitplanes into destination	*/
	UWORD planeOnOff;	/* default bitplane data for planePick				*/
	UWORD planeMask;	/* selects which bitplanes to store into			*/
} Destmerge;

typedef UWORD SpritePrecedence;	/* relative precedence, 0 is the highest	*/

#define MAKE_ID(a,b,c,d)	((a)|((b)<<8)|((c)<<16)|((d)<<24))

#define ID_FORM		MAKE_ID('F','O','R','M')
#define ID_ILBM		MAKE_ID('I','L','B','M')
#define ID_BMHD		MAKE_ID('B','M','H','D')
#define ID_CMAP		MAKE_ID('C','M','A','P')
#define ID_GRAB		MAKE_ID('G','R','A','B')
#define ID_DEST		MAKE_ID('D','E','S','T')
#define	ID_SPRT		MAKE_ID('S','P','R','T')
#define ID_CAMG		MAKE_ID('C','A','M','G')
#define ID_BODY		MAKE_ID('B','O','D','Y')
#define ID_ANNO		MAKE_ID('A','N','N','O')

#define SwapShort(x)	((((x)>>8)&0xff)|(((x)<<8))&0xff00)
#define SwapLong(x)		((((x)>>24) & 0xff)|(((x)>>8)&0xff00)|(((x)<<8)&0xff0000)|((x)<<24))
