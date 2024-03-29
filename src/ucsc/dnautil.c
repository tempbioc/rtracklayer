/* Some stuff that you'll likely need in any program that works with
 * DNA.  Includes stuff for amino acids as well. 
 *
 * Assumes that DNA is stored as a character.
 * The DNA it generates will include the bases 
 * as lowercase tcag.  It will generally accept
 * uppercase as well, and also 'n' or 'N' or '-'
 * for unknown bases. 
 *
 * Amino acids are stored as single character upper case. 
 *
 * This file is copyright 2002 Jim Kent, but license is hereby
 * granted for all use - public, private or commercial. */

#include "common.h"
#include "dnautil.h"


struct codonTable
/* The dread codon table. */
    {
    DNA *codon;		/* Lower case. */
    AA protCode;	/* Upper case. The "Standard" code */
    AA mitoCode;	/* Upper case. Vertebrate Mitochondrial translations */
    };

struct codonTable codonTable[] = 
/* The master codon/protein table. */
{
    {"ttt", 'F', 'F',},
    {"ttc", 'F', 'F',},
    {"tta", 'L', 'L',},
    {"ttg", 'L', 'L',},

    {"tct", 'S', 'S',},
    {"tcc", 'S', 'S',},
    {"tca", 'S', 'S',},
    {"tcg", 'S', 'S',},

    {"tat", 'Y', 'Y',},
    {"tac", 'Y', 'Y',},
    {"taa", 0, 0,},
    {"tag", 0, 0,},

    {"tgt", 'C', 'C',},
    {"tgc", 'C', 'C',},
    {"tga", 0, 'W',},
    {"tgg", 'W', 'W',},


    {"ctt", 'L', 'L',},
    {"ctc", 'L', 'L',},
    {"cta", 'L', 'L',},
    {"ctg", 'L', 'L',},

    {"cct", 'P', 'P',},
    {"ccc", 'P', 'P',},
    {"cca", 'P', 'P',},
    {"ccg", 'P', 'P',},

    {"cat", 'H', 'H',},
    {"cac", 'H', 'H',},
    {"caa", 'Q', 'Q',},
    {"cag", 'Q', 'Q',},

    {"cgt", 'R', 'R',},
    {"cgc", 'R', 'R',},
    {"cga", 'R', 'R',},
    {"cgg", 'R', 'R',},


    {"att", 'I', 'I',},
    {"atc", 'I', 'I',},
    {"ata", 'I', 'M',},
    {"atg", 'M', 'M',},

    {"act", 'T', 'T',},
    {"acc", 'T', 'T',},
    {"aca", 'T', 'T',},
    {"acg", 'T', 'T',},

    {"aat", 'N', 'N',},
    {"aac", 'N', 'N',},
    {"aaa", 'K', 'K',},
    {"aag", 'K', 'K',},

    {"agt", 'S', 'S',},
    {"agc", 'S', 'S',},
    {"aga", 'R', 0,},
    {"agg", 'R', 0,},


    {"gtt", 'V', 'V',},
    {"gtc", 'V', 'V',},
    {"gta", 'V', 'V',},
    {"gtg", 'V', 'V',},

    {"gct", 'A', 'A',},
    {"gcc", 'A', 'A',},
    {"gca", 'A', 'A',},
    {"gcg", 'A', 'A',},

    {"gat", 'D', 'D',},
    {"gac", 'D', 'D',},
    {"gaa", 'E', 'E',},
    {"gag", 'E', 'E',},

    {"ggt", 'G', 'G',},
    {"ggc", 'G', 'G',},
    {"gga", 'G', 'G',},
    {"ggg", 'G', 'G',},
};

/* A table that gives values 0 for t
			     1 for c
			     2 for a
			     3 for g
 * (which is order aa's are in biochemistry codon tables)
 * and gives -1 for all others. */
int ntVal[256];
int ntValLower[256];	/* NT values only for lower case. */
int ntValUpper[256];	/* NT values only for upper case. */
int ntVal5[256];
int ntValNoN[256]; /* Like ntVal, but with T_BASE_VAL in place of -1 for nonexistent ones. */
DNA valToNt[(N_BASE_VAL|MASKED_BASE_BIT)+1];

/* convert tables for bit-4 indicating masked */
int ntValMasked[256];
DNA valToNtMasked[256];

static boolean inittedNtVal = FALSE;

static void initNtVal()
{
if (!inittedNtVal)
    {
    int i;
    for (i=0; i<ArraySize(ntVal); i++)
        {
	ntValUpper[i] = ntValLower[i] = ntVal[i] = -1;
        ntValNoN[i] = T_BASE_VAL;
	if (isspace(i) || isdigit(i))
	    ntVal5[i] = ntValMasked[i] = -1;
	else
            {
	    ntVal5[i] = N_BASE_VAL;
	    ntValMasked[i] = (islower(i) ? (N_BASE_VAL|MASKED_BASE_BIT) : N_BASE_VAL);
            }
        }
    ntVal5['t'] = ntVal5['T'] = ntValNoN['t'] = ntValNoN['T'] = ntVal['t'] = ntVal['T'] = 
    	ntValLower['t'] = ntValUpper['T'] = T_BASE_VAL;
    ntVal5['u'] = ntVal5['U'] = ntValNoN['u'] = ntValNoN['U'] = ntVal['u'] = ntVal['U'] = 
    	ntValLower['u'] = ntValUpper['U'] = U_BASE_VAL;
    ntVal5['c'] = ntVal5['C'] = ntValNoN['c'] = ntValNoN['C'] = ntVal['c'] = ntVal['C'] = 
    	ntValLower['c'] = ntValUpper['C'] = C_BASE_VAL;
    ntVal5['a'] = ntVal5['A'] = ntValNoN['a'] = ntValNoN['A'] = ntVal['a'] = ntVal['A'] = 
    	ntValLower['a'] = ntValUpper['A'] = A_BASE_VAL;
    ntVal5['g'] = ntVal5['G'] = ntValNoN['g'] = ntValNoN['G'] = ntVal['g'] = ntVal['G'] = 
    	ntValLower['g'] = ntValUpper['G'] = G_BASE_VAL;

    valToNt[T_BASE_VAL] = valToNt[T_BASE_VAL|MASKED_BASE_BIT] = 't';
    valToNt[C_BASE_VAL] = valToNt[C_BASE_VAL|MASKED_BASE_BIT] = 'c';
    valToNt[A_BASE_VAL] = valToNt[A_BASE_VAL|MASKED_BASE_BIT] = 'a';
    valToNt[G_BASE_VAL] = valToNt[G_BASE_VAL|MASKED_BASE_BIT] = 'g';
    valToNt[N_BASE_VAL] = valToNt[N_BASE_VAL|MASKED_BASE_BIT] = 'n';

    /* masked values */
    ntValMasked['T'] = T_BASE_VAL;
    ntValMasked['U'] = U_BASE_VAL;
    ntValMasked['C'] = C_BASE_VAL;
    ntValMasked['A'] = A_BASE_VAL;
    ntValMasked['G'] = G_BASE_VAL;

    ntValMasked['t'] = T_BASE_VAL|MASKED_BASE_BIT;
    ntValMasked['u'] = U_BASE_VAL|MASKED_BASE_BIT;
    ntValMasked['c'] = C_BASE_VAL|MASKED_BASE_BIT;
    ntValMasked['a'] = A_BASE_VAL|MASKED_BASE_BIT;
    ntValMasked['g'] = G_BASE_VAL|MASKED_BASE_BIT;

    valToNtMasked[T_BASE_VAL] = 'T';
    valToNtMasked[C_BASE_VAL] = 'C';
    valToNtMasked[A_BASE_VAL] = 'A';
    valToNtMasked[G_BASE_VAL] = 'G';
    valToNtMasked[N_BASE_VAL] = 'N';

    valToNtMasked[T_BASE_VAL|MASKED_BASE_BIT] = 't';
    valToNtMasked[C_BASE_VAL|MASKED_BASE_BIT] = 'c';
    valToNtMasked[A_BASE_VAL|MASKED_BASE_BIT] = 'a';
    valToNtMasked[G_BASE_VAL|MASKED_BASE_BIT] = 'g';
    valToNtMasked[N_BASE_VAL|MASKED_BASE_BIT] = 'n';

    inittedNtVal = TRUE;
    }
}

/* Returns one letter code for protein, 
 * 0 for stop codon or X for bad input,
 * The "Standard" Code */
AA lookupCodon(DNA *dna)
{
int ix;
int i;
char c;

if (!inittedNtVal)
    initNtVal();
ix = 0;
for (i=0; i<3; ++i)
    {
    int bv = ntVal[(int)dna[i]];
    if (bv<0)
	return 'X';
    ix = (ix<<2) + bv;
    }
c = codonTable[ix].protCode;
return c;
}


/* Returns one letter code for protein, 
 * 0 for stop codon or X for bad input,
 * Vertebrate Mitochondrial Code */
AA lookupMitoCodon(DNA *dna)
{
int ix;
int i;
char c;

if (!inittedNtVal)
    initNtVal();
ix = 0;
for (i=0; i<3; ++i)
    {
    int bv = ntVal[(int)dna[i]];
    if (bv<0)
	return 'X';
    ix = (ix<<2) + bv;
    }
c = codonTable[ix].mitoCode;
c = toupper(c);
return c;
}

/* A little array to help us decide if a character is a 
 * nucleotide, and if so convert it to lower case. */
char ntChars[256];

static void initNtChars()
{
static boolean initted = FALSE;

if (!initted)
    {
    zeroBytes(ntChars, sizeof(ntChars));
    ntChars['a'] = ntChars['A'] = 'a';
    ntChars['c'] = ntChars['C'] = 'c';
    ntChars['g'] = ntChars['G'] = 'g';
    ntChars['t'] = ntChars['T'] = 't';
    ntChars['n'] = ntChars['N'] = 'n';
    ntChars['u'] = ntChars['U'] = 'u';
    ntChars['-'] = 'n';
    initted = TRUE;
    }
}

char ntMixedCaseChars[256];

static void initNtMixedCaseChars()
{
static boolean initted = FALSE;

if (!initted)
    {
    zeroBytes(ntMixedCaseChars, sizeof(ntMixedCaseChars));
    ntMixedCaseChars['a'] = 'a';
    ntMixedCaseChars['A'] = 'A';
    ntMixedCaseChars['c'] = 'c';
    ntMixedCaseChars['C'] = 'C';
    ntMixedCaseChars['g'] = 'g';
    ntMixedCaseChars['G'] = 'G';
    ntMixedCaseChars['t'] = 't';
    ntMixedCaseChars['T'] = 'T';
    ntMixedCaseChars['n'] = 'n';
    ntMixedCaseChars['N'] = 'N';
    ntMixedCaseChars['u'] = 'u';
    ntMixedCaseChars['U'] = 'U';
    ntMixedCaseChars['-'] = 'n';
    initted = TRUE;
    }
}

/* Another array to help us do complement of DNA */
DNA ntCompTable[256];
static boolean inittedCompTable = FALSE;

static void initNtCompTable()
{
zeroBytes(ntCompTable, sizeof(ntCompTable));
ntCompTable[' '] = ' ';
ntCompTable['-'] = '-';
ntCompTable['='] = '=';
ntCompTable['a'] = 't';
ntCompTable['c'] = 'g';
ntCompTable['g'] = 'c';
ntCompTable['t'] = 'a';
ntCompTable['u'] = 'a';
ntCompTable['n'] = 'n';
ntCompTable['-'] = '-';
ntCompTable['.'] = '.';
ntCompTable['A'] = 'T';
ntCompTable['C'] = 'G';
ntCompTable['G'] = 'C';
ntCompTable['T'] = 'A';
ntCompTable['U'] = 'A';
ntCompTable['N'] = 'N';
ntCompTable['R'] = 'Y';
ntCompTable['Y'] = 'R';
ntCompTable['M'] = 'K';
ntCompTable['K'] = 'M';
ntCompTable['S'] = 'S';
ntCompTable['W'] = 'W';
ntCompTable['V'] = 'B';
ntCompTable['H'] = 'D';
ntCompTable['D'] = 'H';
ntCompTable['B'] = 'V';
ntCompTable['X'] = 'N';
ntCompTable['r'] = 'y';
ntCompTable['y'] = 'r';
ntCompTable['s'] = 's';
ntCompTable['w'] = 'w';
ntCompTable['m'] = 'k';
ntCompTable['k'] = 'm';
ntCompTable['v'] = 'b';
ntCompTable['h'] = 'd';
ntCompTable['d'] = 'h';
ntCompTable['b'] = 'v';
ntCompTable['x'] = 'n';
ntCompTable['('] = ')';
ntCompTable[')'] = '(';
inittedCompTable = TRUE;
}

/* Complement DNA (not reverse). */
void complement(DNA *dna, long length)
{
int i;

if (!inittedCompTable) initNtCompTable();
for (i=0; i<length; ++i)
    {
    *dna = ntCompTable[(int)*dna];
    ++dna;
    }
}


/* Reverse complement DNA. */
void reverseComplement(DNA *dna, long length)
{
reverseBytes(dna, length);
complement(dna, length);
}

boolean isAllNt(char *seq, int size)
/* Return TRUE if all letters in seq are ACGTNU-. */
{
    int i;
    dnaUtilOpen();
    for (i=0; i<size-1; ++i)
	{
	    if (ntChars[(int)seq[i]] == 0)
		return FALSE;
	}
    return TRUE;
}

long dnaOrAaFilteredSize(char *raw, char filter[256])
/* Return how long DNA will be after non-DNA is filtered out. */
{
char c;
long count = 0;
dnaUtilOpen();
while ((c = *raw++) != 0)
    {
    if (filter[(int)c]) ++count;
    }
return count;
}

void dnaOrAaFilter(char *in, char *out, char filter[256])
/* Run chars through filter. */
{
char c;
dnaUtilOpen();
while ((c = *in++) != 0)
    {
    if ((c = filter[(int)c]) != 0) *out++ = c;
    }
*out++ = 0;
}

UBYTE packDna4(DNA *in)
/* Pack 4 bases into a UBYTE */
{
UBYTE out = 0;
int count = 4;
int bVal;
while (--count >= 0)
    {
    bVal = ntValNoN[(int)*in++];
    out <<= 2;
    out += bVal;
    }
return out;
}

static void checkSizeTypes()
/* Make sure that some of our predefined types are the right size. */
{
assert(sizeof(UBYTE) == 1);
assert(sizeof(WORD) == 2);
assert(sizeof(bits32) == 4);
assert(sizeof(bits16) == 2);
}

int intronOrientationMinSize(DNA *iStart, DNA *iEnd, int minIntronSize)
/* Given a gap in genome from iStart to iEnd, return 
 * Return 1 for GT/AG intron between left and right, -1 for CT/AC, 0 for no
 * intron.  Assumes DNA is lower cased. */
{
if (iEnd - iStart < minIntronSize)
    return 0;
if (iStart[0] == 'g' && iStart[1] == 't' && iEnd[-2] == 'a' && iEnd[-1] == 'g')
    {
    return 1;
    }
else if (iStart[0] == 'c' && iStart[1] == 't' && iEnd[-2] == 'a' && iEnd[-1] == 'c')
    {
    return -1;
    }
else
    return 0;
}

int  dnaOrAaScoreMatch(char *a, char *b, int size, int matchScore, int mismatchScore, 
	char ignore)
/* Compare two sequences (without inserts or deletions) and score. */
{
int i;
int score = 0;
for (i=0; i<size; ++i)
    {
    char aa = a[i];
    char bb = b[i];
    if (aa == ignore || bb == ignore)
        continue;
    if (aa == bb)
        score += matchScore;
    else
        score += mismatchScore;
    }
return score;
}

static int findTailPolyAMaybeMask(DNA *dna, int size, boolean doMask,
				  boolean loose)
/* Identify PolyA at end; mask to 'n' if specified.  This allows a few 
 * non-A's as noise to be trimmed too.  Returns number of bases trimmed.  
 * Leaves first two bases of PolyA in case there's a taa stop codon. */
{
int i;
int score = 10;
int bestScore = 10;
int bestPos = -1;
int trimSize = 0;

for (i=size-1; i>=0; --i)
    {
    DNA b = dna[i];
    if (b == 'n' || b == 'N')
        continue;
    if (score > 20) score = 20;
    if (b == 'a' || b == 'A')
	{
        score += 1;
	if (score >= bestScore)
	    {
	    bestScore = score;
	    bestPos = i;
	    }
	else if (loose && score >= (bestScore - 8))
	    {
	    /* If loose, keep extending even if score isn't back up to best. */
	    bestPos = i;
	    }
	}
    else
	{
        score -= 10;
	}
    if (score < 0)
	{
        break;
	}
    }
if (bestPos >= 0)
    {
    trimSize = size - bestPos - 2;	// Leave two for aa in taa stop codon
    if (trimSize > 0)
        {
	if (doMask)
	    for (i=size - trimSize; i<size; ++i)
		dna[i] = 'n';
	}
    else
        trimSize = 0;
    }
return trimSize;
}

static int findHeadPolyTMaybeMask(DNA *dna, int size, boolean doMask,
				  boolean loose)
/* Return size of PolyT at start (if present); mask to 'n' if specified.  
 * This allows a few non-T's as noise to be trimmed too, but skips last
 * two tt for revcomp'd taa stop codon. */
{
int i;
int score = 10;
int bestScore = 10;
int bestPos = -1;
int trimSize = 0;

for (i=0; i<size; ++i)
    {
    DNA b = dna[i];
    if (b == 'n' || b == 'N')
        continue;
    if (score > 20) score = 20;
    if (b == 't' || b == 'T')
	{
        score += 1;
	if (score >= bestScore)
	    {
	    bestScore = score;
	    bestPos = i;
	    }
	else if (loose && score >= (bestScore - 8))
	    {
	    /* If loose, keep extending even if score isn't back up to best. */
	    bestPos = i;
	    }
	}
    else
	{
        score -= 10;
	}
    if (score < 0)
	{
        break;
	}
    }
if (bestPos >= 0)
    {
    trimSize = bestPos+1 - 2;	// Leave two for aa in taa stop codon
    if (trimSize > 0)
	{
	if (doMask)
	    memset(dna, 'n', trimSize);
	}
    else
        trimSize = 0;
    }
return trimSize;
}

boolean isDna(char *poly, int size)
/* Return TRUE if letters in poly are at least 90% ACGTNU- */
{
int i;
int dnaCount = 0;

dnaUtilOpen();
for (i=0; i<size; ++i)
    {
    if (ntChars[(int)poly[i]])
	dnaCount += 1;
    }
return (dnaCount >= round(0.9 * size));
}


/* Tables to convert from 0-20 to ascii single letter representation
 * of proteins. */
int aaVal[256];
AA valToAa[21];

AA aaChars[256];	/* 0 except for value aa characters.  Converts to upper case rest. */

struct aminoAcidTable
/* A little info about each amino acid. */
    {
    int ix;
    char letter;
    char abbreviation[3];
    char *name;
    };

struct aminoAcidTable aminoAcidTable[] = 
{
    {0, 'A', "ala", "alanine"},
    {1, 'C', "cys", "cysteine"},
    {2, 'D', "asp",  "aspartic acid"},
    {3, 'E', "glu",  "glutamic acid"},
    {4, 'F', "phe",  "phenylalanine"},
    {5, 'G', "gly",  "glycine"},
    {6, 'H', "his",  "histidine"},
    {7, 'I', "ile",  "isoleucine"},
    {8, 'K', "lys",  "lysine"},
    {9, 'L', "leu",  "leucine"},
    {10, 'M',  "met", "methionine"},
    {11, 'N',  "asn", "asparagine"},
    {12, 'P',  "pro", "proline"},
    {13, 'Q',  "gln", "glutamine"},
    {14, 'R',  "arg", "arginine"},
    {15, 'S',  "ser", "serine"},
    {16, 'T',  "thr", "threonine"},
    {17, 'V',  "val", "valine"},
    {18, 'W',  "trp", "tryptophan"},
    {19, 'Y',  "tyr", "tyrosine"},
    {20, 'X',  "ter", "termination"},
};

char *aaAbbr(int i)
/* return pointer to AA abbrevation */
{
return(aminoAcidTable[i].abbreviation);
}

char aaLetter(int i)
/* return AA letter */
{
return(aminoAcidTable[i].letter);
}

static void initAaVal()
/* Initialize aaVal and valToAa tables. */
{
int i;
char c, lowc;

for (i=0; i<ArraySize(aaVal); ++i)
    aaVal[i] = -1;
for (i=0; i<ArraySize(aminoAcidTable); ++i)
    {
    c = aminoAcidTable[i].letter;
    lowc = tolower(c);
    aaVal[(int)c] = aaVal[(int)lowc] = i;
    aaChars[(int)c] = aaChars[(int)lowc] = c;
    valToAa[i] = c;
    }
aaChars['x'] = aaChars['X'] = 'X';
}

void dnaUtilOpen()
/* Initialize stuff herein. */
{
static boolean opened = FALSE;
if (!opened)
    {
    checkSizeTypes();
    initNtVal();
    initAaVal();
    initNtChars();
    initNtMixedCaseChars();
    initNtCompTable();
    opened = TRUE;
    }
}
