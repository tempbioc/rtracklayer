/* dystring - dynamically resizing string.
 *
 * This file is copyright 2002 Jim Kent, but license is hereby
 * granted for all use - public, private or commercial. */

#ifndef DYSTRING_H	/* Wrapper to avoid including this twice. */
#define DYSTRING_H

#include "common.h"

struct dyString
/* Dynamically resizable string that you can do formatted
 * output to. */
    {
    struct dyString *next;	/* Next in list. */
    char *string;		/* Current buffer. */
    int bufSize;		/* Size of buffer. */
    int stringSize;		/* Size of string. */
    };

struct dyString *newDyString(int initialBufSize);
/* Allocate dynamic string with initial buffer size.  (Pass zero for default) */

#define dyStringNew newDyString

void freeDyString(struct dyString **pDs);
/* Free up dynamic string. */

#define dyStringFree(a) freeDyString(a);

void dyStringAppend(struct dyString *ds, char *string);
/* Append zero terminated string to end of dyString. */

void dyStringAppendN(struct dyString *ds, char *string, int stringSize);
/* Append string of given size to end of string. */

char dyStringAppendC(struct dyString *ds, char c);
/* Append char to end of string. */

#define dyStringWriteOne(dy, var) dyStringAppendN(dy, (char *)(&var), sizeof(var))
/* Write one variable (binary!) to dyString - for cases when want to treat string like
 * a file stream. */

void dyStringVaPrintf(struct dyString *ds, char *format, va_list args);
/* VarArgs Printf to end of dyString. */

void dyStringPrintf(struct dyString *ds, char *format, ...)
/*  Printf to end of dyString. */
#ifdef __GNUC__
__attribute__((format(printf, 2, 3)))
#endif
    ;

#define dyStringClear(ds) (ds->string[0] = ds->stringSize = 0)
/* Clear string. */

char *dyStringCannibalize(struct dyString **pDy);
/* Kill dyString, but return the string it is wrapping
 * (formerly dy->string).  This should be free'd at your
 * convenience. */

#define dyStringContents(ds) (ds)->string
/* return raw string. */

#define dyStringLen(ds) ds->stringSize
/* return raw string length. */

#endif /* DYSTRING_H */

