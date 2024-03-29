/* tokenizer - A tokenizer structure that will chop up file into
 * tokens.  It is aware of quoted strings and otherwise tends to return
 * white-space or punctuated-separated words, with punctuation in
 * a separate token.  This is used by autoSql. */

#ifndef TOKENIZER_H
#define TOKENIZER_H

struct tokenizer
/* This handles reading in tokens. */
    {
    bool reuse;	         /* True if want to reuse this token. */
    bool eof;            /* True at end of file. */
    int leadingSpaces;	 /* Number of leading spaces before token. */
    struct lineFile *lf; /* Underlying file. */
    char *curLine;       /* Current line of text. */
    char *linePt;        /* Start position within current line. */
    char *string;        /* String value of token */
    int sSize;           /* Size of string. */
    int sAlloc;          /* Allocated string size. */
      /* Some variables set after tokenizerNew to control details of
       * parsing. */
    bool leaveQuotes;	 /* Leave quotes in string. */
    bool uncommentC;	 /* Take out C (and C++) style comments. */
    bool uncommentShell; /* Take out # style comments. */
    };

struct tokenizer *tokenizerOnLineFile(struct lineFile *lf);
/* Create a new tokenizer on open lineFile. */

void tokenizerFree(struct tokenizer **pTkz);
/* Tear down a tokenizer. */

int tokenizerLineCount(struct tokenizer *tkz);
/* Return line of current token. */

char *tokenizerFileName(struct tokenizer *tkz);
/* Return name of file. */

char *tokenizerNext(struct tokenizer *tkz);
/* Return token's next string (also available as tkz->string) or
 * NULL at EOF. This string will be overwritten with the next call
 * to tokenizerNext, so cloneString if you need to save it. */

void tokenizerErrAbort(struct tokenizer *tkz, char *format, ...)
/* Print error message followed by file and line number and
 * abort. */
#if defined(__GNUC__)
__attribute__((format(printf, 2, 3)))
#endif
;

char *tokenizerMustHaveNext(struct tokenizer *tkz);
/* Get next token, which must be there. */

void tokenizerMustMatch(struct tokenizer *tkz, char *string);
/* Require next token to match string.  Return next token
 * if it does, otherwise abort. */

#endif /* TOKENIZER_H */

