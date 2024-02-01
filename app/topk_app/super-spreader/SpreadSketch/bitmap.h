
/* These are structures for manipulating bitmaps of various
   sizes. Allocation and deallocation of these bitmaps is handled by
   others. The bitmap does not keep track of its size. Arguments
   indicating positions and length are always in bits, never in
   bytes. */

#ifndef BITMAP_H
#define BITMAP_H

typedef unsigned char* bitmap;

/* gives a 0 if the bit is not set something else otherwise */
#define getbit(n,bmp) ((bmp[(n)>>3])&(0x80>>((n)&0x07)))

/* sets the bit to 1 */
//TL: n>>3 <=> n/8,  n&0x07 <=> n%0x07
#define setbit(n,bmp) {bmp[(n)>>3]|=(0x80>>((n)&0x07));}

/* sets the bit to 0 */
#define clearbit(n,bmp) {bmp[(n)>>3]&=(~(0x80>>((n)&0x07)));}

/* initializes the bitmap with all zeroes */
#define fillzero(bmp,n) {int internali;for(internali=(((int)(n)-1)>>3);internali>=0;internali--){bmp[internali]=0x00;}}

/* initializes the bitmap with all ones */
#define fillone(bmp,n) {int internali;for(internali=(((int)(n)-1)>>3);internali>=0;internali--){bmp[internali]=0xFF;}}

/* computes the size in bytes of a bitmap */
#define bitmapsize(n) (((int)(n)+7)>>3)

/* allocating a bitmap of given size */
#define allocbitmap(n) (unsigned char*)malloc(bitmapsize(n))

int countzerobits(bitmap bmp, int from, int to);

#endif /* BITMAP_H */



