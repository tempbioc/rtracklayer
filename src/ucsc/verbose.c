/* verbose.c - write out status messages according to the
 * current verbosity level.  These messages go to stderr. */

/* Copyright (C) 2011 The Regents of the University of California 
 * See README in this or parent directory for licensing information. */

#include "common.h"
#include "portable.h"
#include "verbose.h"


static int logVerbosity = 1;	/* The level of log verbosity.  0 is silent. */
static FILE *logFile;	/* File to log to. */

static boolean checkedDotsEnabled = FALSE;  /* have we check for dot output
                                             * being enabled? */
static boolean dotsEnabled = FALSE;         /* is dot output enabled? */

void verboseVa(int verbosity, char *format, va_list args)
/* Log with at given verbosity vprintf formatted args. */
{
if (verbosity <= logVerbosity)
    {
    if (logFile == NULL)
        logFile = stderr;
    vfprintf(logFile, format, args);
    fflush(logFile);
    }
}

void verbose(int verbosity, char *format, ...)
/* Write printf formatted message to log (which by
 * default is stderr) if global verbose variable
 * is set to verbosity or higher. */
{
va_list args;
va_start(args, format);
verboseVa(verbosity, format, args);
va_end(args);
}

static long lastTime = -1;  // previous call time.

void verboseTimeInit(void)
/* Initialize or reinitialize the previous time for use by verboseTime. */
{
lastTime = clock1000();
}

void verboseTime(int verbosity, char *label, ...)
/* Print label and how long it's been since last call.  Start time can be
 * initialized with verboseTimeInit, otherwise the elapsed time will be
 * zero. */
{
assert(label != NULL);  // original version allowed this, but breaks some GCCs
if (lastTime < 0)
    verboseTimeInit();
long time = clock1000();
va_list args;
va_start(args, label);
verboseVa(verbosity, label, args);
verbose(verbosity, ": %ld millis\n", time - lastTime);
lastTime = time;
va_end(args);
}


boolean verboseDotsEnabled()
/* check if outputting of happy dots are enabled.  They will be enabled if the
 * verbosity is > 0, stderr is a tty and we don't appear to be running an
 * emacs shell. */
{
if (!checkedDotsEnabled)
    {
    if (logFile == NULL)
        logFile = stderr;
    dotsEnabled = (logVerbosity > 0) && isatty(fileno(logFile));
    if (dotsEnabled)
        {
        /* check for an possible emacs shell */
        char *emacs = getenv("emacs");
        char *term = getenv("TERM");
        if ((emacs != NULL) && (emacs[0] == 't'))
            dotsEnabled = FALSE;
        else if ((term != NULL) && sameString(term, "dumb"))
            dotsEnabled = FALSE;
        }
    checkedDotsEnabled = TRUE;
    }
return dotsEnabled;
}

int verboseLevel(void)
/* Get verbosity level. */
{
return logVerbosity;
}
