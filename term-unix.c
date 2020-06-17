/*
 * Copyright (c) 2020 Patrick Pelletier
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "whence.h"

#ifndef _WIN32

#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

typedef int (*CmpFunc) (const void*, const void*);

typedef struct TermName {
    char name [17];
} TermName;

static const TermName colorTerms[] = {
    { "amiga-vnc"        },
    { "ansi-color-2-emx" },
    { "ansi-color-3-emx" },
    { "ansi-emx"         },
    { "ansi80x25-raw"    },
    { "ansiw"            },
    { "at-color"         },
    { "atari-color"      },
    { "atari_st-color"   },
    { "beterm"           },
    { "bterm"            },
    { "cons25w"          },
    { "cygwin"           },
    { "cygwinDBG"        },
    { "decansi"          },
    { "dg+color"         },
    { "dg+color8"        },
    { "dgmode+color"     },
    { "dgmode+color8"    },
    { "dgunix+fixed"     },
    { "djgpp"            },
    { "djgpp204"         },
    { "dumb-emacs-ansi"  },
    { "dvtm"             },
    { "dvtm-256color"    },
    { "ecma+color"       },
    { "emu"              },
    { "eterm-color"      },
    { "fbterm"           },
    { "hft-c"            },
    { "hft-c-old"        },
    { "hurd"             },
    { "i3164"            },
    { "ibm+16color"      },
    { "ibm3164"          },
    { "ibm5154"          },
    { "klone+color"      },
    { "linux-16color"    },
    { "linux-m1"         },
    { "linux-m1b"        },
    { "linux-m2"         },
    { "mach-color"       },
    { "mach-gnu-color"   },
    { "minitel1"         },
    { "mlterm"           },
    { "mlterm2"          },
    { "mlterm3"          },
    { "mvterm"           },
    { "ncr260intan"      },
    { "ncr260intpp"      },
    { "ncr260intwan"     },
    { "ncr260intwpp"     },
    { "nsterm+c41"       },
    { "old-st"           },
    { "pccon+colors"     },
    { "putty"            },
    { "rcons-color"      },
    { "scoansi-old"      },
    { "simpleterm"       },
    { "st-0.6"           },
    { "st52-color"       },
    { "sun-color"        },
    { "ti_ansi"          },
    { "tw52"             },
    { "tw52-color"       },
    { "uwin"             },
    { "vv100"            },
    { "vwmterm"          },
    { "wsvt25"           },
    { "xnuppc+c"         },
    { "xterm+256color"   },
    { "xterm+256setaf"   },
    { "xterm+indirect"   },
    { "xterm-8bit"       },
    { "xterm-basic"      },
    { "xterm-direct2"    },
    { "xtermc"           },
};

#define NUM_TERMS (sizeof (colorTerms) / sizeof (colorTerms[0]))

Terminal stdoutTerminal;
Terminal stderrTerminal;
static bool initialized = false;

static bool isColor (const char *term) {
    return (NULL != bsearch (term, colorTerms,
                             NUM_TERMS, sizeof (TermName),
                             (CmpFunc) strcmp));
}

void detectConsole (void) {
    if (!initialized) {
        stdoutTerminal.is_terminal = isatty (STDOUT_FILENO);
        stderrTerminal.is_terminal = isatty (STDERR_FILENO);
        stdoutTerminal.supports_color = false;
        stderrTerminal.supports_color = false;

        if (stdoutTerminal.is_terminal || stderrTerminal.is_terminal) {
            const char *term = getenv ("TERM");

            if (term && *term && isColor (term)) {
                stdoutTerminal.supports_color = stdoutTerminal.is_terminal;
                stderrTerminal.supports_color = stderrTerminal.is_terminal;
            }
        }

        errno = 0;
        initialized = true;
    }
}

void writeUTF8 (FILE *f, const char *s) {
    fputs (s, f);
}

#endif  /* not _WIN32 */
