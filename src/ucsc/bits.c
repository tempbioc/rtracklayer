/* bits - handle operations on arrays of bits. 
 *
 * This file is copyright 2002 Jim Kent, but license is hereby
 * granted for all use - public, private or commercial. */

#include "common.h"
#include "bits.h"



static Bits oneBit[8] = { 0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1};
static Bits leftMask[8] = {0xFF, 0x7F, 0x3F, 0x1F,  0xF,  0x7,  0x3,  0x1,};
static Bits rightMask[8] = {0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE, 0xFF,};
int bitsInByte[256];

static boolean inittedBitsInByte = FALSE;

void bitsInByteInit()
/* Initialize bitsInByte array. */
{
int i;

if (!inittedBitsInByte)
    {
    inittedBitsInByte = TRUE;
    for (i=0; i<256; ++i)
        {
	int count = 0;
	if (i&1)
	    count = 1;
	if (i&2)
	    ++count;
	if (i&4)
	    ++count;
	if (i&8)
	    ++count;
	if (i&0x10)
	    ++count;
	if (i&0x20)
	    ++count;
	if (i&0x40)
	    ++count;
	if (i&0x80)
	    ++count;
	bitsInByte[i] = count;
	}
    }
}

Bits *bitAlloc(int bitCount)
/* Allocate bits. */
{
int byteCount = ((bitCount+7)>>3);
return needLargeZeroedMem(byteCount);
}

Bits *bitClone(Bits* orig, int bitCount)
/* Clone bits. */
{
int byteCount = ((bitCount+7)>>3);
Bits* bits = needLargeZeroedMem(byteCount);
memcpy(bits, orig, byteCount);
return bits;
}

void bitFree(Bits **pB)
/* Free bits. */
{
freez(pB);
}

Bits *lmBitAlloc(struct lm *lm,int bitCount)
// Allocate bits.  Must supply local memory.
{
assert(lm != NULL);
int byteCount = ((bitCount+7)>>3);
return lmAlloc(lm,byteCount);
}

void bitSetOne(Bits *b, int bitIx)
/* Set a single bit. */
{
b[bitIx>>3] |= oneBit[bitIx&7];
}

void bitClearOne(Bits *b, int bitIx)
/* Clear a single bit. */
{
b[bitIx>>3] &= ~oneBit[bitIx&7];
}

void bitSetRange(Bits *b, int startIx, int bitCount)
/* Set a range of bits. */
{
if (bitCount <= 0)
    return;
int endIx = (startIx + bitCount - 1);
int startByte = (startIx>>3);
int endByte = (endIx>>3);
int startBits = (startIx&7);
int endBits = (endIx&7);
int i;

if (startByte == endByte)
    {
    b[startByte] |= (leftMask[startBits] & rightMask[endBits]);
    return;
    }
b[startByte] |= leftMask[startBits];
for (i = startByte+1; i<endByte; ++i)
    b[i] = 0xff;
b[endByte] |= rightMask[endBits];
}


boolean bitReadOne(Bits *b, int bitIx)
/* Read a single bit. */
{
return (b[bitIx>>3] & oneBit[bitIx&7]) != 0;
}

int bitCountRange(Bits *b, int startIx, int bitCount)
/* Count number of bits set in range. */
{
if (bitCount <= 0)
    return 0;
int endIx = (startIx + bitCount - 1);
int startByte = (startIx>>3);
int endByte = (endIx>>3);
int startBits = (startIx&7);
int endBits = (endIx&7);
int i;
int count = 0;

if (!inittedBitsInByte)
    bitsInByteInit();
if (startByte == endByte)
    return bitsInByte[b[startByte] & leftMask[startBits] & rightMask[endBits]];
count = bitsInByte[b[startByte] & leftMask[startBits]];
for (i = startByte+1; i<endByte; ++i)
    count += bitsInByte[b[i]];
count += bitsInByte[b[endByte] & rightMask[endBits]];
return count;
}

int bitFind(Bits *b, int startIx, boolean val, int bitCount)
/* Find the index of the the next set bit. */
{
unsigned char notByteVal = val ? 0 : 0xff;
int iBit = startIx;
int endByte = ((bitCount-1)>>3);
int iByte;

/* scan initial byte */
while (((iBit & 7) != 0) && (iBit < bitCount))
    {
    if (bitReadOne(b, iBit) == val)
        return iBit;
    iBit++;
    }

/* scan byte at a time, if not already in last byte */
iByte = (iBit >> 3);
if (iByte < endByte)
    {
    while ((iByte < endByte) && (b[iByte] == notByteVal))
        iByte++;
    iBit = iByte << 3;
    }

/* scan last byte */
while (iBit < bitCount)
    {
    if (bitReadOne(b, iBit) == val)
        return iBit;
    iBit++;
    }
 return bitCount;  /* not found */
}

int bitFindSet(Bits *b, int startIx, int bitCount)
/* Find the index of the the next set bit. */
{
return bitFind(b, startIx, TRUE, bitCount);
}

int bitFindClear(Bits *b, int startIx, int bitCount)
/* Find the index of the the next clear bit. */
{
return bitFind(b, startIx, FALSE, bitCount);
}
