/* Commonly used routines in a wide range of applications.
 * Strings, singly-linked lists, and a little file i/o.
 *
 * This file is copyright 2002 Jim Kent, but license is hereby
 * granted for all use - public, private or commercial. */

#include "common.h"
#include "errAbort.h"
#include "portable.h"
#include "linefile.h"
#include "hash.h"


void *cloneMem(void *pt, size_t size)
/* Allocate a new buffer of given size, and copy pt to it. */
{
void *newPt = needLargeMem(size);
memcpy(newPt, pt, size);
return newPt;
}

static char *cloneStringZExt(const char *s, int size, int copySize)
/* Make a zero terminated copy of string in memory */
{
char *d = needMem(copySize+1);
copySize = min(size,copySize);
memcpy(d, s, copySize);
d[copySize] = 0;
return d;
}

char *cloneStringZ(const char *s, int size)
/* Make a zero terminated copy of string in memory */
{
return cloneStringZExt(s, strlen(s), size);
}

char *cloneString(const char *s)
/* Make copy of string in dynamic memory */
{
int size = 0;
if (s == NULL)
    return NULL;
size = strlen(s);
return cloneStringZExt(s, size, size);
}

/* Reverse the order of the bytes. */
void reverseBytes(char *bytes, long length)
{
long halfLen = (length>>1);
char *end = bytes+length;
char c;
while (--halfLen >= 0)
    {
    c = *bytes;
    *bytes++ = *--end;
    *end = c;
    }
}


/** List managing routines. */

/* Count up elements in list. */
int slCount(const void *list)
{
struct slList *pt = (struct slList *)list;
int len = 0;

while (pt != NULL)
    {
    len += 1;
    pt = pt->next;
    }
return len;
}

void *slElementFromIx(void *list, int ix)
/* Return the ix'th element in list.  Returns NULL
 * if no such element. */
{
struct slList *pt = (struct slList *)list;
int i;
for (i=0;i<ix;i++)
    {
    if (pt == NULL) return NULL;
    pt = pt->next;
    }
return pt;
}

int slIxFromElement(void *list, void *el)
/* Return index of el in list.  Returns -1 if not on list. */
{
struct slList *pt;
int ix = 0;

for (pt = list, ix=0; pt != NULL; pt = pt->next, ++ix)
    if (el == (void*)pt)
	return ix;
return -1;
}

void *slLastEl(void *list)
/* Returns last element in list or NULL if none. */
{
struct slList *next, *el;
if ((el = list) == NULL)
    return NULL;
while ((next = el->next) != NULL)
    el = next;
return el;
}

/* Add new node to tail of list.
 * Usage:
 *    slAddTail(&list, node);
 * where list and nodes are both pointers to structure
 * that begin with a next pointer.
 */
void slAddTail(void *listPt, void *node)
{
struct slList **ppt = (struct slList **)listPt;
struct slList *n = (struct slList *)node;

while (*ppt != NULL)
    {
    ppt = &((*ppt)->next);
    }
n->next = NULL;
*ppt = n;
}

void *slPopHead(void *vListPt)
/* Return head of list and remove it from list. (Fast) */
{
struct slList **listPt = (struct slList **)vListPt;
struct slList *el = *listPt;
if (el != NULL)
    {
    *listPt = el->next;
    el->next = NULL;
    }
return el;
}


void *slCat(void *va, void *vb)
/* Return concatenation of lists a and b.
 * Example Usage:
 *   struct slName *a = getNames("a");
 *   struct slName *b = getNames("b");
 *   struct slName *ab = slCat(a,b)
 */
{
struct slList *a = va;
struct slList *b = vb;
struct slList *end;
if (a == NULL)
    return b;
for (end = a; end->next != NULL; end = end->next)
    ;
end->next = b;
return a;
}

void slReverse(void *listPt)
/* Reverse order of a list.
 * Usage:
 *    slReverse(&list);
 */
{
struct slList **ppt = (struct slList **)listPt;
struct slList *newList = NULL;
struct slList *el, *next;

next = *ppt;
while (next != NULL)
    {
    el = next;
    next = el->next;
    el->next = newList;
    newList = el;
    }
*ppt = newList;
}

void slFreeList(void *listPt)
/* Free list */
{
struct slList **ppt = (struct slList**)listPt;
struct slList *next = *ppt;
struct slList *el;

while (next != NULL)
    {
    el = next;
    next = el->next;
    freeMem((char*)el);
    }
*ppt = NULL;
}

void slSort(void *pList, int (*compare )(const void *elem1,  const void *elem2))
/* Sort a singly linked list with Qsort and a temporary array. */
{
struct slList **pL = (struct slList **)pList;
struct slList *list = *pL;
int count;
count = slCount(list);
if (count > 1)
    {
    struct slList *el;
    struct slList **array;
    int i;
    array = needLargeMem(count * sizeof(*array));
    for (el = list, i=0; el != NULL; el = el->next, i++)
        array[i] = el;
    qsort(array, count, sizeof(array[0]), compare);
    list = NULL;
    for (i=0; i<count; ++i)
        {
        array[i]->next = list;
        list = array[i];
        }
    freeMem(array);
    slReverse(&list);
    *pL = list;
    }
}

void slUniqify(void *pList, int (*compare )(const void *elem1,  const void *elem2), void (*free)())
/* Return sorted list with duplicates removed.
 * Compare should be same type of function as slSort's compare (taking
 * pointers to pointers to elements.  Free should take a simple
 * pointer to dispose of duplicate element, and can be NULL. */
{
struct slList **pSlList = (struct slList **)pList;
struct slList *oldList = *pSlList;
struct slList *newList = NULL, *el;

slSort(&oldList, compare);
while ((el = slPopHead(&oldList)) != NULL)
    {
    if ((newList == NULL) || (compare(&newList, &el) != 0))
        slAddHead(&newList, el);
    else if (free != NULL)
        free(el);
    }
slReverse(&newList);
*pSlList = newList;
}

boolean slRemoveEl(void *vpList, void *vToRemove)
/* Remove element from singly linked list.  Usage:
 *    slRemove(&list, el);
 * Returns TRUE if element in list.  */
{
struct slList **pList = vpList;
struct slList *toRemove = vToRemove;
struct slList *el, *next, *newList = NULL;
boolean didRemove = FALSE;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    if (el != toRemove)
	{
	slAddHead(&newList, el);
	}
    else
        didRemove = TRUE;
    }
slReverse(&newList);
*pList = newList;
return didRemove;
}

struct slInt *slIntNew(int x)
/* Return a new int. */
{
struct slInt *a;
AllocVar(a);
a->val = x;
return a;
}

static int doubleCmp(const void *va, const void *vb)
/* Compare function to sort array of doubles. */
{
const double *a = va;
const double *b = vb;
double diff = *a - *b;
if (diff < 0)
    return -1;
else if (diff > 0)
    return 1;
else
    return 0;
}

void doubleSort(int count, double *array)
/* Sort an array of doubles. */
{
if (count > 1)
qsort(array, count, sizeof(array[0]), doubleCmp);
}

double doubleMedian(int count, double *array)
/* Return median value in array.  This will sort
 * the array as a side effect. */
{
double median;
doubleSort(count, array);
if ((count&1) == 1)
    median = array[count>>1];
else
    {
    count >>= 1;
    median = (array[count] + array[count-1]) * 0.5;
    }
return median;
}

void doubleBoxWhiskerCalc(int count, double *array, double *retMin,
                          double *retQ1, double *retMedian, double *retQ3, double *retMax)
/* Calculate what you need to draw a box and whiskers plot from an array of doubles. */
{
if (count <= 0)
    errAbort("doubleBoxWhiskerCalc needs a positive number, not %d for count", count);
if (count == 1)
    {
    *retMin = *retQ1 = *retMedian = *retQ3 = *retMax = array[0];
    return;
    }
doubleSort(count, array);
double min = array[0];
double max = array[count-1];
double median;
int halfCount = count>>1;
if ((count&1) == 1)
    median = array[halfCount];
else
    {
    median = (array[halfCount] + array[halfCount-1]) * 0.5;
    }
double q1, q3;
if (count <= 3)
    {
    q1 = 0.5 * (median + min);
    q3 = 0.5 * (median + max);
    }
else
    {
    int q1Ix = count/4;
    int q3Ix = count - 1 - q1Ix;
    uglyf("count %d, q1Ix %d, q3Ix %d\n", count, q1Ix, q3Ix);
    q1 = array[q1Ix];
    q3 = array[q3Ix];
    }
*retMin = min;
*retQ1 = q1;
*retMedian = median;
*retQ3 = q3;
*retMax = max;
}

static int intCmp(const void *va, const void *vb)
/* Compare function to sort array of ints. */
{
const int *a = va;
const int *b = vb;
int diff = *a - *b;
if (diff < 0)
    return -1;
else if (diff > 0)
    return 1;
else
    return 0;
}

void intSort(int count, int *array)
/* Sort an array of ints. */
{
if (count > 1)
qsort(array, count, sizeof(array[0]), intCmp);
}

struct slName *newSlName(char *name)
/* Return a new name. */
{
struct slName *sn;
if (name != NULL)
    {
    int len = strlen(name);
    sn = needMem(sizeof(*sn)+len);
    strcpy(sn->name, name);
    return sn;
    }
else
    {
    AllocVar(sn);
    }
return sn;
}

struct slName *slNameNewN(char *name, int size)
/* Return new slName of given size. */
{
struct slName *sn = needMem(sizeof(*sn) + size);
memcpy(sn->name, name, size);
return sn;
}

int slNameCmpCase(const void *va, const void *vb)
/* Compare two slNames, ignore case. */
{
const struct slName *a = *((struct slName **)va);
const struct slName *b = *((struct slName **)vb);
return strcasecmp(a->name, b->name);
}

int slNameCmp(const void *va, const void *vb)
/* Compare two slNames. */
{
const struct slName *a = *((struct slName **)va);
const struct slName *b = *((struct slName **)vb);
return strcmp(a->name, b->name);
}


void slNameSort(struct slName **pList)
/* Sort slName list. */
{
slSort(pList, slNameCmp);
}

char *slNameStore(struct slName **pList, char *string)
/* Put string into list if it's not there already.
 * Return the version of string stored in list. */
{
struct slName *el;
for (el = *pList; el != NULL; el = el->next)
    {
    if (sameString(string, el->name))
	return el->name;
    }
el = newSlName(string);
slAddHead(pList, el);
return el->name;
}

struct slName *slNameAddHead(struct slName **pList, char *name)
/* Add name to start of list and return it. */
{
struct slName *el = slNameNew(name);
slAddHead(pList, el);
return el;
}


struct slName *slNameListFromString(char *s, char delimiter)
/* Return list of slNames gotten from parsing delimited string.
 * The final delimiter is optional. a,b,c  and a,b,c, are equivalent
 * for comma-delimited lists. */
{
char *e;
struct slName *list = NULL, *el;
while (s != NULL && s[0] != 0)
    {
    e = strchr(s, delimiter);
    if (e == NULL)
	el = slNameNew(s);
    else
	{
        el = slNameNewN(s, e-s);
	e += 1;
	}
    slAddHead(&list, el);
    s = e;
    }
slReverse(&list);
return list;
}

struct slRef *refOnList(struct slRef *refList, void *val)
/* Return ref if val is already on list, otherwise NULL. */
{
struct slRef *ref;
for (ref = refList; ref != NULL; ref = ref->next)
    if (ref->val == val)
        return ref;
return NULL;
}

struct slRef *slRefNew(void *val)
/* Create new slRef element. */
{
struct slRef *ref;
AllocVar(ref);
ref->val = val;
return ref;
}

void refAdd(struct slRef **pRefList, void *val)
/* Add reference to list. */
{
struct slRef *ref;
AllocVar(ref);
ref->val = val;
slAddHead(pRefList, ref);
}

void slRefFreeListAndVals(struct slRef **pList)
/* Free up (with simple freeMem()) each val on list, and the list itself as well. */
{
struct slRef *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    freeMem(el->val);
    freeMem(el);
    }
*pList = NULL;
}

struct slPair *slPairNew(char *name, void *val)
/* Allocate new name/value pair. */
{
struct slPair *el;
AllocVar(el);
el->name = cloneString(name);
el->val = val;
return el;
}

void slPairAdd(struct slPair **pList, char *name, void *val)
/* Add new slPair to head of list. */
{
struct slPair *el = slPairNew(name, val);
slAddHead(pList, el);
}

void slPairFree(struct slPair **pEl)
/* Free up struct and name.  (Don't free up values.) */
{
struct slPair *el = *pEl;
if (el != NULL)
    {
    freeMem(el->name);
    freez(pEl);
    }
}

void slPairFreeList(struct slPair **pList)
/* Free up list.  (Don't free up values.) */
{
struct slPair *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    slPairFree(&el);
    }
*pList = NULL;
}

void slPairFreeVals(struct slPair *list)
/* Free up all values on list. */
{
struct slPair *el;
for (el = list; el != NULL; el = el->next)
    freez(&el->val);
}

struct slPair *slPairFind(struct slPair *list, char *name)
/* Return list element of given name, or NULL if not found. */
{
struct slPair *el;
for (el = list; el != NULL; el = el->next)
    if (sameString(name, el->name))
        break;
return el;
}

struct slPair *slPairListFromString(char *str,boolean respectQuotes)
// Return slPair list parsed from list in string like:  [name1=val1 name2=val2 ...]
// if respectQuotes then string can have double quotes: [name1="val 1" "name 2"=val2 ...]
//    resulting pair strips quotes: {name1}={val 1},{name 2}={val2}
// Returns NULL if parse error.  Free this up with slPairFreeValsAndList.
{
char *s = skipLeadingSpaces(str);  // Would like to remove this and tighten up the standard someday.
if (isEmpty(s))
    return NULL;

struct slPair *list = NULL;
char name[1024];
char val[1024];
char buf[1024];
bool inQuote = FALSE;
char *b = buf;
char sep = '=';
char c = ' ';
int mode = 0;
while(1)
    {
    c = *s++;
    if (mode == 0 || mode == 2) // reading name or val
	{
	boolean term = FALSE;
	if (respectQuotes && b == buf && !inQuote && c == '"')
	    inQuote = TRUE;
	else if (inQuote && c == '"')
	    term = TRUE;
	else if ((c == sep || c == 0) && !inQuote)
	    {
	    term = TRUE;
	    --s;  // rewind
	    }
	else if (c == ' ' && !inQuote)
	    {
	    warn("slPairListFromString: Unexpected whitespace in %s", str);
	    return NULL;
	    }
	else if (c == 0 && inQuote)
	    {
	    warn("slPairListFromString: Unterminated quote in %s", str);
	    return NULL;
	    }
	else
	    {
	    *b++ = c;
	    if ((b - buf) > sizeof buf)
		{
		warn("slPairListFromString: pair name or value too long in %s", str);
		return NULL;
		}
	    }
	if (term)
	    {
	    inQuote = FALSE;
	    *b = 0;
	    if (mode == 0)
		{
		safecpy(name, sizeof name, buf);
		if (strlen(name)<1)
		    {
		    warn("slPairListFromString: Pair name cannot be empty in %s", str);
		    return NULL;
		    }
		// Shall we check for name being alphanumeric, at least for the respectQuotes=FALSE case?
		}
	    else // mode == 2
                {
		safecpy(val, sizeof val, buf);
		if (!respectQuotes && (hasWhiteSpace(name) || hasWhiteSpace(val))) // should never happen
		    {
		    warn("slPairListFromString() Unexpected white space in name=value pair: [%s]=[%s] in string=[%s]\n", name, val, str);
		    break;
		    }
		slPairAdd(&list, name, cloneString(val));
		}
	    ++mode;
	    }
	}
    else if (mode == 1) // read required "=" sign
	{
	if (c != '=')
	    {
	    warn("slPairListFromString: Expected character = after name in %s", str);
	    return NULL;
            }
	++mode;
	sep = ' ';
	b = buf;
	}
    else // (mode == 3) reading optional separating space
	{
	if (c == 0)
	    break;
	if (c != ' ')
	    {
	    mode = 0;
	    --s;
	    b = buf;
	    sep = '=';
	    }
	}
    }
slReverse(&list);
return list;
}

int slPairValCmpCase(const void *va, const void *vb)
/* Case insensitive compare two slPairs on their values (must be string). */
{
const struct slPair *a = *((struct slPair **)va);
const struct slPair *b = *((struct slPair **)vb);
return strcasecmp((char *)(a->val), (char *)(b->val));
}

int slPairValCmp(const void *va, const void *vb)
/* Compare two slPairs on their values (must be string). */
{
const struct slPair *a = *((struct slPair **)va);
const struct slPair *b = *((struct slPair **)vb);
return strcmp((char *)(a->val), (char *)(b->val));
}

int slPairIntCmp(const void *va, const void *vb)
// Compare two slPairs on their integer values.
{
const struct slPair *a = *((struct slPair **)va);
const struct slPair *b = *((struct slPair **)vb);
return ((char *)(a->val) - (char *)(b->val)); // cast works and val is 0 vased integer
}

int slPairAtoiCmp(const void *va, const void *vb)
// Compare two slPairs on their strings interpreted as integer values.
{
const struct slPair *a = *((struct slPair **)va);
const struct slPair *b = *((struct slPair **)vb);
return (atoi((char *)(a->val)) - atoi((char *)(b->val)));
}

int differentWord(char *s1, char *s2)
/* strcmp ignoring case - returns zero if strings are
 * the same (ignoring case) otherwise returns difference
 * between first non-matching characters. */
{
char c1, c2;
for (;;)
    {
    c1 = toupper(*s1++);
    c2 = toupper(*s2++);
    if (c1 != c2) /* Takes care of end of string in one but not the other too */
	return c2-c1;
    if (c1 == 0)  /* Take care of end of string in both. */
	return 0;
    }
}

int differentStringNullOk(char *a, char *b)
/* Returns 0 if two strings (either of which may be NULL)
 * are the same.  Otherwise it returns a positive or negative
 * number depending on the alphabetical order of the two
 * strings.
 * This is basically a strcmp that can handle NULLs in
 * the input.  If used in a sort the NULLs will end
 * up before any of the cases with data.   */
{
if (a == b)
    return FALSE;
else if (a == NULL)
    return -1;
else if (b == NULL)
    return 1;
else
    return strcmp(a,b) != 0;
}

boolean startsWith(const char *start, const char *string)
/* Returns TRUE if string begins with start. */
{
char c;
int i;

for (i=0; ;i += 1)
    {
    if ((c = start[i]) == 0)
        return TRUE;
    if (string[i] != c)
        return FALSE;
    }
}

boolean startsWithWord(char *firstWord, char *line)
/* Return TRUE if first white-space-delimited word in line
 * is same as firstWord.  Comparison is case sensitive. */
{
int len = strlen(firstWord);
int i;
for (i=0; i<len; ++i)
   if (firstWord[i] != line[i])
       return FALSE;
char c = line[len];
return c == 0 || isspace(c);
}

boolean endsWith(char *string, char *end)
/* Returns TRUE if string ends with end. */
{
int sLen, eLen, offset;
sLen = strlen(string);
eLen = strlen(end);
offset = sLen - eLen;
if (offset < 0)
    return FALSE;
return sameString(string+offset, end);
}

char lastChar(char *s)
/* Return last character in string. */
{
if (s == NULL || s[0] == 0)
    return 0;
return s[strlen(s)-1];
}

char *matchingCharBeforeInLimits(char *limit, char *s, char c)
/* Look for character c sometime before s, but going no further than limit.
 * Return NULL if not found. */
{
while (--s >= limit)
    if (*s == c)
        return s;
return NULL;
}

void toUpperN(char *s, int n)
/* Convert a section of memory to upper case. */
{
int i;
for (i=0; i<n; ++i)
    s[i] = toupper(s[i]);
}

void toLowerN(char *s, int n)
/* Convert a section of memory to lower case. */
{
int i;
for (i=0; i<n; ++i)
    s[i] = tolower(s[i]);
}

char *strUpper(char *s)
/* Convert entire string to upper case. */
{
char c;
char *ss=s;
for (;;)
    {
    if ((c = *ss) == 0) break;
    *ss++ = toupper(c);
    }
return s;
}

char *replaceChars(char *string, char *old, char *new)
/*
  Replaces the old with the new. The old and new string need not be of equal size
 Can take any length string.
 Return value needs to be freeMem'd.
*/
{
int numTimes = 0;
int oldLen = strlen(old);
int newLen = strlen(new);
int strLen = 0;
char *result = NULL;
char *ptr = strstr(string, old);
char *resultPtr = NULL;

while(NULL != ptr)
    {
    numTimes++;
    ptr += oldLen;
    ptr = strstr(ptr, old);
    }
strLen = max(strlen(string) + (numTimes * (newLen - oldLen)), strlen(string));
result = needMem(strLen + 1);

ptr = strstr(string, old);
resultPtr = result;
while(NULL != ptr)
    {
    strLen = ptr - string;
    strcpy(resultPtr, string);
    string = ptr + oldLen;

    resultPtr += strLen;
    strcpy(resultPtr, new);
    resultPtr += newLen;
    ptr = strstr(string, old);
    }

strcpy(resultPtr, string);
return result;
}

char *strLower(char *s)
/* Convert entire string to lower case */
{
char c;
char *ss=s;
for (;;)
    {
    if ((c = *ss) == 0) break;
    *ss++ = tolower(c);
    }
return s;
}

char * memSwapChar(char *s, int len, char oldChar, char newChar)
/* Substitute newChar for oldChar throughout memory of given length. */
{
int ix=0;
for (;ix<len;ix++)
    {
    if (s[ix] == oldChar)
        s[ix] =  newChar;
    }
return s;
}

void stripChar(char *s, char c)
/* Remove all occurences of c from s. */
{
char *in = s, *out = s;
char b;

for (;;)
    {
    b = *out = *in++;
    if (b == 0)
       break;
    if (b != c)
       ++out;
    }
}

int countChars(char *s, char c)
/* Return number of characters c in string s. */
{
char a;
int count = 0;
while ((a = *s++) != 0)
    if (a == c)
        ++count;
return count;
}

int countLeadingDigits(const char *s)
/* Return number of leading digits in s */
{
int count = 0;
while (isdigit(*s))
   {
   ++count;
   ++s;
   }
return count;
}

int countLeadingNondigits(const char *s)
/* Count number of leading non-digit characters in s. */
{
int count = 0;
char c;
while ((c = *s++) != 0)
   {
   if (isdigit(c))
       break;
   ++count;
   }
return count;
}

int countSeparatedItems(char *string, char separator)
/* Count number of items in string you would parse out with given
 * separator,  assuming final separator is optional. */
{
int count = 0;
char c, lastC = 0;
while ((c = *string++) != 0)
    {   
    if (c == separator)
       ++count;
    lastC = c;
    }
if (lastC != separator && lastC != 0)
    ++count;
return count;
}

int cmpStringsWithEmbeddedNumbers(const char *a, const char *b)
/* Compare strings such as gene names that may have embedded numbers,
 * so that bmp4a comes before bmp14a */
{
for (;;)
   {
   /* Figure out number of digits at start, and do numerical comparison if there
    * are any.  If numbers agree step over numerical part, otherwise return difference. */
   int aNum = countLeadingDigits(a);
   int bNum = countLeadingDigits(b);
   if (aNum >= 0 && bNum >= 0)
       {
       int diff = atoi(a) - atoi(b);
       if (diff != 0)
           return diff;
       a += aNum;
       b += bNum;
       }

   /* Count number of non-digits at start. */
   int aNonNum = countLeadingNondigits(a);
   int bNonNum = countLeadingNondigits(b);

   // If different sizes of non-numerical part, then don't match, let strcmp sort out how
   if (aNonNum != bNonNum)
        return strcmp(a,b);
   // If no characters left then they are the same!
   else if (aNonNum == 0)
       return 0;
   // Non-numerical part is the same length and non-zero.  See if it is identical.  Return if not.
   else
       {
        int diff = memcmp(a,b,aNonNum);
       if (diff != 0)
            return diff;
       a += aNonNum;
       b += bNonNum;
       }
   }
}

/* int chopString(in, sep, outArray, outSize); */
/* This chops up the input string (cannabilizing it)
 * into an array of zero terminated strings in
 * outArray.  It returns the number of strings.
 * If you pass in NULL for outArray, it will just
 * return the number of strings that it *would*
 * chop. 
 * GOTCHA: since multiple separators are skipped
 * and treated as one, it is impossible to parse 
 * a list with an empty string. 
 * e.g. cat\t\tdog returns only cat and dog but no empty string */
int chopString(char *in, char *sep, char *outArray[], int outSize)
{
int recordCount = 0;

for (;;)
    {
    if (outArray != NULL && recordCount >= outSize)
	break;
    /* Skip initial separators. */
    in += strspn(in, sep);
    if (*in == 0)
	break;
    if (outArray != NULL)
	outArray[recordCount] = in;
    recordCount += 1;
    in += strcspn(in, sep);
    if (*in == 0)
	break;
    if (outArray != NULL)
	*in = 0;
    in += 1;
    }
return recordCount;
}

int chopByWhite(char *in, char *outArray[], int outSize)
/* Like chopString, but specialized for white space separators. 
 * See the GOTCHA in chopString */
{
int recordCount = 0;
char c;
for (;;)
    {
    if (outArray != NULL && recordCount >= outSize)
	break;

    /* Skip initial separators. */
    while (isspace(*in)) ++in;
    if (*in == 0)
        break;

    /* Store start of word and look for end of word. */
    if (outArray != NULL)
        outArray[recordCount] = in;
    recordCount += 1;
    for (;;)
        {
        if ((c = *in) == 0)
            break;
        if (isspace(c))
            break;
        ++in;
        }
    if (*in == 0)
	break;

    /* Tag end of word with zero. */
    if (outArray != NULL)
	*in = 0;
    /* And skip over the zero. */
    in += 1;
    }
return recordCount;
}


int chopByChar(char *in, char chopper, char *outArray[], int outSize)
/* Chop based on a single character. */
{
int i;
char c;
if (*in == 0)
    return 0;
for (i=0; (i<outSize) || (outArray==NULL); ++i)
    {
    if (outArray != NULL)
        outArray[i] = in;
    for (;;)
	{
	if ((c = *in++) == 0)
	    return i+1;
	else if (c == chopper)
	    {
            if (outArray != NULL)
                in[-1] = 0;
	    break;
	    }
	}
    }
return i;
}

char crLfChopper[] = "\n\r";
char whiteSpaceChopper[] = " \t\n\r";


char *skipBeyondDelimit(char *s,char delimit)
/* Returns NULL or pointer to first char beyond one (or more contiguous) delimit char.
   If delimit is ' ' then skips beyond first patch of whitespace. */
{
if (s != NULL)
    {
    char *beyond = NULL;
    if (delimit == ' ')
        return skipLeadingSpaces(skipToSpaces(s));
    else
        beyond = strchr(s,delimit);
    if (beyond != NULL)
        {
        for (beyond++;*beyond == delimit;beyond++) ;
        if (*beyond != '\0')
            return beyond;
        }
    }
return NULL;
}

char *skipLeadingSpaces(char *s)
/* Return first non-white space. */
{
char c;
if (s == NULL) return NULL;
for (;;)
    {
    c = *s;
    if (!isspace(c))
	return s;
    ++s;
    }
}

char *skipToSpaces(char *s)
/* Return first white space or NULL if none.. */
{
char c;
if (s == NULL)
    return NULL;
for (;;)
    {
    c = *s;
    if (c == 0)
        return NULL;
    if (isspace(c))
	return s;
    ++s;
    }
}



void eraseTrailingSpaces(char *s)
/* Replace trailing white space with zeroes. */
{
int len = strlen(s);
int i;
char c;

for (i=len-1; i>=0; --i)
    {
    c = s[i];
    if (isspace(c))
	s[i] = 0;
    else
	break;
    }
}

/* Remove white space from a string */
void eraseWhiteSpace(char *s)
{
char *in, *out;
char c;

in = out = s;
for (;;)
    {
    c = *in++;
    if (c == 0)
	break;
    if (!isspace(c))
	*out++ = c;
    }
*out++ = 0;
}

char *trimSpaces(char *s)
/* Remove leading and trailing white space. */
{
if (s != NULL)
    {
    s = skipLeadingSpaces(s);
    eraseTrailingSpaces(s);
    }
return s;
}

void repeatCharOut(FILE *f, char c, int count)
/* Write character to file repeatedly. */
{
while (--count >= 0)
    fputc(c, f);
}

void spaceOut(FILE *f, int count)
/* Put out some spaces to file. */
{
repeatCharOut(f, ' ', count);
}

boolean hasWhiteSpace(char *s)
/* Return TRUE if there is white space in string. */
{
char c;
while ((c = *s++) != 0)
    if (isspace(c))
        return TRUE;
return FALSE;
}


char *nextWord(char **pLine)
/* Return next word in *pLine and advance *pLine to next
 * word. */
{
char *s = *pLine, *e;
if (s == NULL || s[0] == 0)
    return NULL;
s = skipLeadingSpaces(s);
if (s[0] == 0)
    return NULL;
e = skipToSpaces(s);
if (e != NULL)
    *e++ = 0;
*pLine = e;
return s;
}

char *nextWordRespectingQuotes(char **pLine)
// return next word but respects single or double quotes surrounding sets of words.
{
char *s = *pLine, *e;
if (s == NULL || s[0] == 0)
    return NULL;
s = skipLeadingSpaces(s);
if (s[0] == 0)
    return NULL;
if (s[0] == '"')
    {
    e = skipBeyondDelimit(s+1,'"');
    if (e != NULL && !isspace(e[0]))
        e = skipToSpaces(s);
    }
else if (s[0] == '\'')
    {
    e = skipBeyondDelimit(s+1,'\'');
    if (e != NULL && !isspace(e[0]))
        e = skipToSpaces(s);
    }
else
    e = skipToSpaces(s);
if (e != NULL)
    *e++ = 0;
*pLine = e;
return s;
}

char *cloneFirstWordByDelimiter(char *line,char delimit)
/* Returns a cloned first word, not harming the memory passed in */
{
if (line == NULL || *line == 0)
    return NULL;
line = skipLeadingSpaces(line);
if (*line == 0)
    return NULL;
int size=0;
char *e;
for (e=line;*e!=0;e++)
    {
    if (*e==delimit)
        break;
    else if (delimit == ' ' && isspace(*e))
        break;
    size++;
    }
if (size == 0)
    return NULL;
char *new = needMem(size + 2); // Null terminated by 2
memcpy(new, line, size);
return new;
}


char *nextStringInList(char **pStrings)
/* returns pointer to the first string and advances pointer to next in
   list of strings dilimited by 1 null and terminated by 2 nulls. */
{
if (pStrings == NULL || *pStrings == NULL || **pStrings == 0)
    return NULL;
char *p=*pStrings;
*pStrings += strlen(p)+1;
return p;
}



FILE *mustOpen(char *fileName, char *mode)
/* Open a file - or squawk and die. */
{
FILE *f;

if (sameString(fileName, "stdin"))
    return stdin;
if (sameString(fileName, "stdout"))
    return stdout;
if ((f = fopen(fileName, mode)) == NULL)
    {
    char *modeName = "";
    if (mode)
        {
        if (mode[0] == 'r')
            modeName = " to read";
        else if (mode[0] == 'w')
            modeName = " to write";
        else if (mode[0] == 'a')
            modeName = " to append";
        }
    errAbort("mustOpen: Can't open %s%s: %s", fileName, modeName, strerror(errno));
    }
return f;
}

void mustWrite(FILE *file, void *buf, size_t size)
/* Write to a file or squawk and die. */
{
if (size != 0 && fwrite(buf, size, 1, file) != 1)
    {
    errAbort("Error writing %lld bytes: %s\n", (long long)size, strerror(ferror(file)));
    }
}


void mustRead(FILE *file, void *buf, size_t size)
/* Read size bytes from a file or squawk and die. */
{
if (size != 0 && fread(buf, size, 1, file) != 1)
    {
    if (ferror(file))
	errAbort("Error reading %lld bytes: %s", (long long)size, strerror(ferror(file)));
    else
	errAbort("End of file reading %lld bytes", (long long)size);
    }
}

void writeString(FILE *f, char *s)
/* Write a 255 or less character string to a file.
 * This will write the length of the string in the first
 * byte then the string itself. */
{
UBYTE bLen;
int len = strlen(s);

if (len > 255)
    {
    warn("String too long in writeString (%d chars):\n%s", len, s);
    len = 255;
    }
bLen = len;
writeOne(f, bLen);
mustWrite(f, s, len);
}

char *readString(FILE *f)
/* Read a string (written with writeString) into
 * memory.  freeMem the result when done. */
{
UBYTE bLen;
int len;
char *s;

if (!readOne(f, bLen))
    return NULL;
len = bLen;
s = needMem(len+1);
if (len > 0)
    mustRead(f, s, len);
return s;
}

boolean fastReadString(FILE *f, char buf[256])
/* Read a string into buffer, which must be long enough
 * to hold it.  String is in 'writeString' format. */
{
UBYTE bLen;
int len;
if (!readOne(f, bLen))
    return FALSE;
if ((len = bLen)> 0)
    mustRead(f, buf, len);
buf[len] = 0;
return TRUE;
}

void mustGetLine(FILE *file, char *buf, int charCount)
/* Read at most charCount-1 bytes from file, but stop after newline if one is
 * encountered.  The string in buf is '\0'-terminated.  (See man 3 fgets.)
 * Die if there is an error. */
{
char *success = fgets(buf, charCount, file);
if (success == NULL && charCount > 0)
    buf[0] = '\0';
if (ferror(file))
    errAbort("mustGetLine: fgets failed: %s", strerror(ferror(file)));
}

int mustOpenFd(char *fileName, int flags)
/* Open a file descriptor (see man 2 open) or squawk and die. */
{
if (sameString(fileName, "stdin"))
    return STDIN_FILENO;
if (sameString(fileName, "stdout"))
    return STDOUT_FILENO;
// mode is necessary when O_CREAT is given, ignored otherwise
int mode = 00664;
int fd = open(fileName, flags, mode);
if (fd < 0)
    {
    char *modeName = "";
    if ((flags & (O_WRONLY | O_CREAT | O_TRUNC)) == (O_WRONLY | O_CREAT | O_TRUNC))
	modeName = " to create and truncate";
    else if ((flags & (O_WRONLY | O_CREAT)) == (O_WRONLY | O_CREAT))
	modeName = " to create";
    else if ((flags & O_WRONLY) == O_WRONLY)
	modeName = " to write";
    else if ((flags & O_RDWR) == O_RDWR)
	modeName = " to append";
    else
	modeName = " to read";
    errnoAbort("mustOpenFd: Can't open %s%s", fileName, modeName);
    }
return fd;
}

void mustReadFd(int fd, void *buf, size_t size)
/* Read size bytes from a file or squawk and die. */
{
ssize_t actualSize;
char *cbuf = buf;
// using a loop because linux was not returning all data in a single request when request size exceeded 2GB.
while (size > 0)
    {
    actualSize = read(fd, cbuf, size);
    if (actualSize < 0)
	errnoAbort("Error reading %lld bytes", (long long)size);
    if (actualSize == 0)
	errAbort("End of file reading %llu bytes (got %lld)", (unsigned long long)size, (long long)actualSize);
    cbuf += actualSize;
    size -= actualSize;
    }
}

void mustWriteFd(int fd, void *buf, size_t size)
/* Write size bytes to file descriptor fd or die.  (See man 2 write.) */
{
ssize_t result = write(fd, buf, size);
if (result < size)
    {
    if (result < 0)
	errnoAbort("mustWriteFd: write failed");
    else 
        errAbort("mustWriteFd only wrote %lld of %lld bytes. Likely the disk is full.",
	    (long long)result, (long long)size);
    }
}

off_t mustLseek(int fd, off_t offset, int whence)
/* Seek to given offset, relative to whence (see man lseek) in file descriptor fd or errAbort.
 * Return final offset (e.g. if this is just an (fd, 0, SEEK_CUR) query for current position). */
{
off_t ret = lseek(fd, offset, whence);
if (ret < 0)
    errnoAbort("lseek(%d, %lld, %s (%d)) failed", fd, (long long)offset,
	       ((whence == SEEK_SET) ? "SEEK_SET" : (whence == SEEK_CUR) ? "SEEK_CUR" :
		(whence == SEEK_END) ? "SEEK_END" : "invalid 'whence' value"), whence);
return ret;
}

void mustCloseFd(int *pFd)
/* Close file descriptor *pFd if >= 0, abort if there's an error, set *pFd = -1. */
{
if (pFd != NULL && *pFd >= 0)
    {
    if (close(*pFd) < 0)
	errnoAbort("close failed");
    *pFd = -1;
    }
}

char *addSuffix(char *head, char *suffix)
/* Return a needMem'd string containing "headsuffix". Should be free'd
 when finished. */
{
char *ret = NULL;
int size = strlen(head) + strlen(suffix) +1;
ret = needMem(sizeof(char)*size);
snprintf(ret, size, "%s%s", head, suffix);
return ret;
}

void chopSuffix(char *s)
/* Remove suffix (last . in string and beyond) if any. */
{
char *e = strrchr(s, '.');
if (e != NULL)
    *e = 0;
}


char *chopPrefixAt(char *s, char c)
/* Like chopPrefix, but can chop on any character, not just '.' */
{
char *e = strchr(s, c);
if (e == NULL) return s;
*e++ = 0;
return e;
}

boolean carefulCloseWarn(FILE **pFile)
/* Close file if open and null out handle to it.
 * Return FALSE and print a warning message if there
 * is a problem.*/
{
FILE *f;
boolean ok = TRUE;
if ((pFile != NULL) && ((f = *pFile) != NULL))
    {
    if (f != stdin && f != stdout)
        {
        if (fclose(f) != 0)
	    {
            errnoWarn("fclose failed");
	    ok = FALSE;
	    }
        }
    *pFile = NULL;
    }
return ok;
}

void carefulClose(FILE **pFile)
/* Close file if open and null out handle to it.
 * Warn and abort if there's a problem. */
{
if (!carefulCloseWarn(pFile))
    noWarnAbort();
}

char *firstWordInFile(char *fileName, char *wordBuf, int wordBufSize)
/* Read the first word in file into wordBuf. */
{
FILE *f = mustOpen(fileName, "r");
mustGetLine(f, wordBuf, wordBufSize);
fclose(f);
return trimSpaces(wordBuf);
}

void fileOffsetSizeFindGap(struct fileOffsetSize *list,
                           struct fileOffsetSize **pBeforeGap, struct fileOffsetSize **pAfterGap)
/* Starting at list, find all items that don't have a gap between them and the previous item.
 * Return at gap, or at end of list, returning pointers to the items before and after the gap. */
{
struct fileOffsetSize *pt, *next;
for (pt = list; ; pt = next)
    {
    next = pt->next;
    if (next == NULL || next->offset != pt->offset + pt->size)
	{
	*pBeforeGap = pt;
	*pAfterGap = next;
	return;
	}
    }
}


int  rangeIntersection(int start1, int end1, int start2, int end2)
/* Return amount of bases two ranges intersect over, 0 or negative if no
 * intersection. */
{
int s = max(start1,start2);
int e = min(end1,end2);
return e-s;
}

int positiveRangeIntersection(int start1, int end1, int start2, int end2)
/* Return number of bases in intersection of two ranges, or
 * zero if they don't intersect. */
{
int ret = rangeIntersection(start1,end1,start2,end2);
if (ret < 0)
    ret = 0;
return ret;
}

bits64 byteSwap64(bits64 a)
/* Return byte-swapped version of a */
{
union {bits64 whole; UBYTE bytes[8];} u,v;
u.whole = a;
v.bytes[0] = u.bytes[7];
v.bytes[1] = u.bytes[6];
v.bytes[2] = u.bytes[5];
v.bytes[3] = u.bytes[4];
v.bytes[4] = u.bytes[3];
v.bytes[5] = u.bytes[2];
v.bytes[6] = u.bytes[1];
v.bytes[7] = u.bytes[0];
return v.whole;
}

bits64 fdReadBits64(int fd, boolean isSwapped)
/* Read and optionally byte-swap 64 bit entity. */
{
bits64 val;
mustReadOneFd(fd, val);
if (isSwapped)
    val = byteSwap64(val);
return val;
}

bits32 byteSwap32(bits32 a)
/* Return byte-swapped version of a */
{
union {bits32 whole; UBYTE bytes[4];} u,v;
u.whole = a;
v.bytes[0] = u.bytes[3];
v.bytes[1] = u.bytes[2];
v.bytes[2] = u.bytes[1];
v.bytes[3] = u.bytes[0];
return v.whole;
}

bits32 readBits32(FILE *f, boolean isSwapped)
/* Read and optionally byte-swap 32 bit entity. */
{
bits32 val;
mustReadOne(f, val);
if (isSwapped)
    val = byteSwap32(val);
return val;
}

bits32 fdReadBits32(int fd, boolean isSwapped)
/* Read and optionally byte-swap 32 bit entity. */
{
bits32 val;
mustReadOneFd(fd, val);
if (isSwapped)
    val = byteSwap32(val);
return val;
}

bits32 memReadBits32(char **pPt, boolean isSwapped)
/* Read and optionally byte-swap 32 bit entity from memory buffer pointed to by
 * *pPt, and advance *pPt past read area. */
{
bits32 val;
memcpy(&val, *pPt, sizeof(val));
if (isSwapped)
    val = byteSwap32(val);
*pPt += sizeof(val);
return val;
}

bits16 byteSwap16(bits16 a)
/* Return byte-swapped version of a */
{
union {bits16 whole; UBYTE bytes[2];} u,v;
u.whole = a;
v.bytes[0] = u.bytes[1];
v.bytes[1] = u.bytes[0];
return v.whole;
}

bits16 memReadBits16(char **pPt, boolean isSwapped)
/* Read and optionally byte-swap 16 bit entity from memory buffer pointed to by
 * *pPt, and advance *pPt past read area. */
{
bits16 val;
memcpy(&val, *pPt, sizeof(val));
if (isSwapped)
    val = byteSwap16(val);
*pPt += sizeof(val);
return val;
}

double byteSwapDouble(double a)
/* Return byte-swapped version of a */
{
union {double whole; UBYTE bytes[8];} u,v;
u.whole = a;
v.bytes[0] = u.bytes[7];
v.bytes[1] = u.bytes[6];
v.bytes[2] = u.bytes[5];
v.bytes[3] = u.bytes[4];
v.bytes[4] = u.bytes[3];
v.bytes[5] = u.bytes[2];
v.bytes[6] = u.bytes[1];
v.bytes[7] = u.bytes[0];
return v.whole;
}


float byteSwapFloat(float a)
/* Return byte-swapped version of a */
{
union {float whole; UBYTE bytes[4];} u,v;
u.whole = a;
v.bytes[0] = u.bytes[3];
v.bytes[1] = u.bytes[2];
v.bytes[2] = u.bytes[1];
v.bytes[3] = u.bytes[0];
return v.whole;
}


float memReadFloat(char **pPt, boolean isSwapped)
/* Read and optionally byte-swap single-precision floating point entity
 * from memory buffer pointed to by *pPt, and advance *pPt past read area. */
{
float val;
memcpy(&val, *pPt, sizeof(val));
if (isSwapped)
    val = byteSwapFloat(val);
*pPt += sizeof(val);
return val;
}

boolean fileExists(char *fileName)
/* Return TRUE if file exists (may replace this with non-
 * portable faster way some day). */
{
/* To make piping easier stdin and stdout always exist. */
if (sameString(fileName, "stdin")) return TRUE;
if (sameString(fileName, "stdout")) return TRUE;

return fileSize(fileName) != -1;
}

char *strstrNoCase(char *haystack, char *needle)
/*
  A case-insensitive strstr function
Will also robustly handle null strings
param haystack - The string to be searched
param needle - The string to look for in the haystack string

return - The position of the first occurence of the desired substring
or -1 if it is not found
 */
{
char *haystackCopy = NULL;
char *needleCopy = NULL;
int index = 0;
int haystackLen = 0;
int needleLen = 0;
char *p, *q;

if (NULL == haystack || NULL == needle)
    {
    return NULL;
    }

haystackLen = strlen(haystack);
needleLen = strlen(needle);

haystackCopy = (char*) needMem(haystackLen + 1);
needleCopy = (char*) needMem(needleLen + 1);

for(index = 0; index < haystackLen;  index++)
    {
    haystackCopy[index] = tolower(haystack[index]);
    }
haystackCopy[haystackLen] = 0; /* Null terminate */

for(index = 0; index < needleLen;  index++)
    {
    needleCopy[index] = tolower(needle[index]);
    }
needleCopy[needleLen] = 0; /* Null terminate */

p=strstr(haystackCopy, needleCopy);
q=haystackCopy;

freeMem(haystackCopy);
freeMem(needleCopy);

if(p==NULL) return NULL;

return p-q+haystack;
}

int vasafef(char* buffer, int bufSize, char *format, va_list args)
/* Format string to buffer, vsprintf style, only with buffer overflow
 * checking.  The resulting string is always terminated with zero byte. */
{
int sz = vsnprintf(buffer, bufSize, format, args);
/* note that some version return -1 if too small */
if ((sz < 0) || (sz >= bufSize))
    {
    buffer[bufSize-1] = (char) 0;
    errAbort("buffer overflow, size %d, format: %s, buffer: '%s'", bufSize, format, buffer);
    }
return sz;
}

int safef(char* buffer, int bufSize, char *format, ...)
/* Format string to buffer, vsprintf style, only with buffer overflow
 * checking.  The resulting string is always terminated with zero byte. */
{
int sz;
va_list args;
va_start(args, format);
sz = vasafef(buffer, bufSize, format, args);
va_end(args);
return sz;
}

void safecpy(char *buf, size_t bufSize, const char *src)
/* copy a string to a buffer, with bounds checking.*/
{
size_t slen = strlen(src);
if (slen > bufSize-1)
    errAbort("buffer overflow, size %lld, string size: %lld", (long long)bufSize, (long long)slen);
strcpy(buf, src);
}

void safencpy(char *buf, size_t bufSize, const char *src, size_t n)
/* copy n characters from a string to a buffer, with bounds checking.
 * Unlike strncpy, always null terminates the result */
{
if (n > bufSize-1)
    errAbort("buffer overflow, size %lld, substring size: %lld", (long long)bufSize, (long long)n);
// strlen(src) can take a long time when src is for example a pointer into a chromosome sequence.
// Instead of setting slen to max(strlen(src), n), just stop counting length at n.
size_t slen = 0;
while (src[slen] != '\0' && slen < n)
    slen++;
strncpy(buf, src, n);
buf[slen] = '\0';
}

void safecat(char *buf, size_t bufSize, const char *src)
/* Append  a string to a buffer, with bounds checking.*/
{
size_t blen = strlen(buf);
size_t slen = strlen(src);
if (blen+slen > bufSize-1)
    errAbort("buffer overflow, size %lld, new string size: %lld", (long long)bufSize, (long long)(blen+slen));
strcat(buf, src);
}

static char *naStr = "n/a";
static char *emptyStr = "";



char *skipToNumeric(char *s)
/* skip up to where numeric digits appear */
{
while (*s != 0 && !isdigit(*s))
    ++s;
return s;
}

#ifndef WIN32
time_t mktimeFromUtc (struct tm *t)
/* Return time_t for tm in UTC (GMT)
 * Useful for stuff like converting to time_t the
 * last-modified HTTP response header
 * which is always GMT. Returns -1 on failure of mktime */
{
    time_t time;
    char *tz;
    char save_tz[100];
    tz=getenv("TZ");
    if (tz)
        safecpy(save_tz, sizeof(save_tz), tz);
    setenv("TZ", "GMT0", 1);
    tzset();
    t->tm_isdst = 0;
    time=mktime(t);
    if (tz)
        setenv("TZ", save_tz, 1);
    else
        unsetenv("TZ");
    tzset();
    return (time);
}


time_t dateToSeconds(const char *date,const char*format)
// Convert a string date to time_t
{
struct tm storage={0,0,0,0,0,0,0,0,0};
if (strptime(date,format,&storage)==NULL)
    return 0;
else
    return mktime(&storage);
}

static int daysOfMonth(struct tm *tp)
/* Returns the days of the month given the year */
{
int days=0;
switch(tp->tm_mon)
    {
    case 3:
    case 5:
    case 8:
    case 10:    days = 30;
                break;
    case 1:     days = 28;
                if ( (tp->tm_year % 4) == 0
                &&  ((tp->tm_year % 20) != 0 || (tp->tm_year % 100) == 0) )
                    days = 29;
                break;
    default:    days = 31;
                break;
    }
return days;
}

static void dateAdd(struct tm *tp,int addYears,int addMonths,int addDays)
/* Add years,months,days to a date */
{
tp->tm_mday  += addDays;
tp->tm_mon   += addMonths;
tp->tm_year  += addYears;
int dom=28;
while ( (tp->tm_mon >11  || tp->tm_mon <0)
    || (tp->tm_mday>dom || tp->tm_mday<1) )
    {
    if (tp->tm_mon>11)   // First month: tm.tm_mon is 0-11 range
        {
        tp->tm_year += (tp->tm_mon / 12);
        tp->tm_mon  = (tp->tm_mon % 12);
        }
    else if (tp->tm_mon<0)
        {
        tp->tm_year += (tp->tm_mon / 12) - 1;
        tp->tm_mon  =  (tp->tm_mon % 12) + 12;
        }
    else
        {
        dom = daysOfMonth(tp);
        if (tp->tm_mday>dom)
            {
            tp->tm_mday -= dom;
            tp->tm_mon  += 1;
            dom = daysOfMonth(tp);
            }
        else if (tp->tm_mday < 1)
            {
            tp->tm_mon  -= 1;
            dom = daysOfMonth(tp);
            tp->tm_mday += dom;
            }
        }
    }
}

#endif
