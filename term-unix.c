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

/* We determine if the terminal supports color by comparing the TERM
 * environment variable to a hardcoded list of terminals that are known
 * to support color.
 *
 * The list was created by choosing all the terminals in
 *   https://invisible-island.net/ncurses/terminfo.src.html
 * that have the "setaf" capability.
 *
 * Terminals whose name contains "color" have been omitted from the
 * list, because we assume all terminals whose name contains "color"
 * support color.  So the list is just terminals that support color
 * but don't have "color" in the name.
 *
 * The terminals are lexicographically sorted and stored in a
 * fixed-width format, so that they can easily be searched with the C
 * library function bsearch().  To optimize space a bit, there are two
 * different lists (colorTermsShort and colorTermsLong) with different
 * fixed widths.
 */

typedef struct TermNameShort {
    char name [10];
} TermNameShort;

static const TermNameShort colorTermsShort[/* 172 */] = {
    { "386at"     },
    { "Eterm"     },
    { "alacritty" },
    { "amiga-vnc" },
    { "ansi"      },
    { "ansi-emx"  },
    { "ansi80x25" },
    { "ansi80x30" },
    { "ansi80x43" },
    { "ansi80x50" },
    { "ansi80x60" },
    { "ansil"     },
    { "ansis"     },
    { "ansiw"     },
    { "arm100"    },
    { "arm100-am" },
    { "arm100-w"  },
    { "at386"     },
    { "att6386"   },
    { "beterm"    },
    { "bterm"     },
    { "cons25"    },
    { "cons25l1"  },
    { "cons25r"   },
    { "cons25w"   },
    { "cons30"    },
    { "cons43"    },
    { "cons50"    },
    { "cons50l1"  },
    { "cons50r"   },
    { "cons60"    },
    { "cons60l1"  },
    { "cons60r"   },
    { "crt"       },
    { "crt-vt220" },
    { "cx"        },
    { "cx100"     },
    { "cygwin"    },
    { "cygwinDBG" },
    { "d220"      },
    { "d220-7b"   },
    { "d220-dg"   },
    { "d230"      },
    { "d230-dg"   },
    { "d230c"     },
    { "d230c-dg"  },
    { "d430-unix" },
    { "d470"      },
    { "d470-7b"   },
    { "d470-dg"   },
    { "d470c"     },
    { "d470c-7b"  },
    { "d470c-dg"  },
    { "darwin"    },
    { "darwin-b"  },
    { "darwin-f"  },
    { "darwin-f2" },
    { "decansi"   },
    { "djgpp"     },
    { "djgpp204"  },
    { "domterm"   },
    { "dtterm"    },
    { "dvtm"      },
    { "emu"       },
    { "fbterm"    },
    { "hft"       },
    { "hft-c"     },
    { "hft-c-old" },
    { "hurd"      },
    { "i3164"     },
    { "iTerm.app" },
    { "ibm3164"   },
    { "ibm5081"   },
    { "ibm5154"   },
    { "ibm6154"   },
    { "ibm8503"   },
    { "ibm8507"   },
    { "ibm8512"   },
    { "ibm8513"   },
    { "ibm8514"   },
    { "ibm8604"   },
    { "ibmpc3r"   },
    { "interix"   },
    { "iterm"     },
    { "iterm2"    },
    { "kitty"     },
    { "kterm"     },
    { "kterm-co"  },
    { "linux-c"   },
    { "linux-m1"  },
    { "linux-m1b" },
    { "linux-m2"  },
    { "linux2.2"  },
    { "linux2.6"  },
    { "linux3.0"  },
    { "minitel1"  },
    { "minitel1b" },
    { "minix-3.0" },
    { "mintty"    },
    { "mlterm2"   },
    { "mlterm3"   },
    { "mvterm"    },
    { "ncsa"      },
    { "ncsa-ns"   },
    { "netbsd6"   },
    { "nsterm-7"  },
    { "nsterm-c"  },
    { "nsterm-s"  },
    { "ntconsole" },
    { "nxterm"    },
    { "old-st"    },
    { "opennt"    },
    { "opennt-25" },
    { "opennt-35" },
    { "opennt-50" },
    { "opennt-60" },
    { "opennt-w"  },
    { "pc-minix"  },
    { "pc3"       },
    { "pc3r"      },
    { "pcansi"    },
    { "pcansi-25" },
    { "pcansi-33" },
    { "pcansi-43" },
    { "pcansi25"  },
    { "pcansi33"  },
    { "pcansi43"  },
    { "pccon"     },
    { "pccon0"    },
    { "putty"     },
    { "putty-m1"  },
    { "putty-m1b" },
    { "putty-m2"  },
    { "putty-sco" },
    { "rxvt"      },
    { "rxvt-xpm"  },
    { "scoansi"   },
    { "screen"    },
    { "screen4"   },
    { "screen5"   },
    { "st-0.6"    },
    { "st-0.7"    },
    { "st-0.8"    },
    { "st-direct" },
    { "teken"     },
    { "termite"   },
    { "ti928"     },
    { "ti928-8"   },
    { "ti_ansi"   },
    { "tmux"      },
    { "tt52"      },
    { "tw52"      },
    { "uwin"      },
    { "vscode"    },
    { "vtnt"      },
    { "vv100"     },
    { "vwmterm"   },
    { "wsvt25"    },
    { "wsvt25m"   },
    { "xiterm"    },
    { "xnuppc"    },
    { "xnuppc-b"  },
    { "xnuppc-f"  },
    { "xnuppc-f2" },
    { "xterm-hp"  },
    { "xterm-new" },
    { "xterm-sco" },
    { "xterm-sun" },
    { "xterm-xi"  },
    { "xterm.js"  },
    { "xtermc"    },
    { "xwsh"      },
};

typedef struct TermNameLong {
    char name [19];
} TermNameLong;

static const TermNameLong colorTermsLong[/* 143 */] = {
    { "alacritty-direct"   },
    { "ansi.sys-old"       },
    { "ansi80x25-raw"      },
    { "arm100-wam"         },
    { "bsdos-pc-nobold"    },
    { "cons25-debian"      },
    { "cons25-iso8859"     },
    { "cons25-koi8-r"      },
    { "cons50-iso8859"     },
    { "cons50-koi8r"       },
    { "cons60-iso"         },
    { "cons60-koi8r"       },
    { "d430-unix-25"       },
    { "d430-unix-s"        },
    { "d430-unix-sr"       },
    { "d430-unix-w"        },
    { "d430c-unix"         },
    { "d430c-unix-25"      },
    { "d430c-unix-s"       },
    { "d430c-unix-sr"      },
    { "d430c-unix-w"       },
    { "darwin-100x37"      },
    { "darwin-112x37"      },
    { "darwin-128x40"      },
    { "darwin-128x48"      },
    { "darwin-144x48"      },
    { "darwin-160x64"      },
    { "darwin-200x64"      },
    { "darwin-200x75"      },
    { "darwin-256x96"      },
    { "darwin-80x25"       },
    { "darwin-80x30"       },
    { "darwin-90x30"       },
    { "dumb-emacs-ansi"    },
    { "iTerm2.app"         },
    { "interix-nti"        },
    { "iterm2-direct"      },
    { "kitty-direct"       },
    { "konsole-base"       },
    { "konsole-direct"     },
    { "konsole-linux"      },
    { "konsole-vt100"      },
    { "konsole-vt420pc"    },
    { "konsole-xf3x"       },
    { "konsole-xf4x"       },
    { "linux-basic"        },
    { "linux-c-nc"         },
    { "linux2.6.26"        },
    { "mintty-direct"      },
    { "mlterm-direct"      },
    { "ms-terminal"        },
    { "ncr260intan"        },
    { "ncr260intpp"        },
    { "ncr260intwan"       },
    { "ncr260intwpp"       },
    { "ncsa-vt220"         },
    { "nsterm-7-c"         },
    { "nsterm-7-c-s"       },
    { "nsterm-7-s"         },
    { "nsterm-acs"         },
    { "nsterm-acs-c"       },
    { "nsterm-acs-c-s"     },
    { "nsterm-acs-s"       },
    { "nsterm-build309"    },
    { "nsterm-build326"    },
    { "nsterm-build343"    },
    { "nsterm-build361"    },
    { "nsterm-build400"    },
    { "nsterm-c-7"         },
    { "nsterm-c-acs"       },
    { "nsterm-c-s"         },
    { "nsterm-c-s-7"       },
    { "nsterm-c-s-acs"     },
    { "nsterm-direct"      },
    { "nsterm-old"         },
    { "nsterm-s-7"         },
    { "nsterm-s-acs"       },
    { "ntconsole-100"      },
    { "ntconsole-100-nti"  },
    { "ntconsole-25"       },
    { "ntconsole-25-nti"   },
    { "ntconsole-25-w"     },
    { "ntconsole-25-w-vt"  },
    { "ntconsole-35"       },
    { "ntconsole-35-nti"   },
    { "ntconsole-50"       },
    { "ntconsole-50-nti"   },
    { "ntconsole-60"       },
    { "ntconsole-60-nti"   },
    { "ntconsole-w"        },
    { "ntconsole-w-vt"     },
    { "opennt-100"         },
    { "opennt-100-nti"     },
    { "opennt-25-nti"      },
    { "opennt-25-w"        },
    { "opennt-25-w-vt"     },
    { "opennt-35-nti"      },
    { "opennt-50-nti"      },
    { "opennt-60-nti"      },
    { "opennt-nti"         },
    { "opennt-w-vt"        },
    { "putty-noapp"        },
    { "putty-vt100"        },
    { "rxvt-cygwin"        },
    { "rxvt-cygwin-native" },
    { "scoansi-new"        },
    { "scoansi-old"        },
    { "screen.Eterm"       },
    { "screen.putty"       },
    { "simpleterm"         },
    { "teraterm2.3"        },
    { "teraterm4.59"       },
    { "teraterm4.97"       },
    { "terminator"         },
    { "terminology-0.6.1"  },
    { "terminology-1.0.0"  },
    { "vscode-direct"      },
    { "vte-direct"         },
    { "xnuppc-100x37"      },
    { "xnuppc-112x37"      },
    { "xnuppc-128x40"      },
    { "xnuppc-128x48"      },
    { "xnuppc-144x48"      },
    { "xnuppc-160x64"      },
    { "xnuppc-200x64"      },
    { "xnuppc-200x75"      },
    { "xnuppc-256x96"      },
    { "xnuppc-80x25"       },
    { "xnuppc-80x30"       },
    { "xnuppc-90x30"       },
    { "xterm-8bit"         },
    { "xterm-basic"        },
    { "xterm-direct"       },
    { "xterm-direct2"      },
    { "xterm-vt220"        },
    { "xterm-xf86-v32"     },
    { "xterm-xf86-v33"     },
    { "xterm-xf86-v333"    },
    { "xterm-xf86-v40"     },
    { "xterm-xf86-v43"     },
    { "xterm-xf86-v44"     },
    { "xterm-xfree86"      },
    { "xterms-sun"         },
};

Terminal stdoutTerminal;
Terminal stderrTerminal;
static bool initialized = false;

#define DO_BSEARCH(a) \
    (bsearch (term, (a), sizeof (a) / sizeof (a[0]), sizeof (a[0]), \
              (CmpFunc) strcmp))

static bool isColor (const char *term) {
    if (strstr (term, "color")) {
        return true;
    }

    const size_t len = strlen (term);

    if (len < sizeof (colorTermsShort[0])) {
        return (NULL != DO_BSEARCH (colorTermsShort));
    } else if (len < sizeof (colorTermsLong[0])) {
        return (NULL != DO_BSEARCH (colorTermsLong));
    } else {
        return false;
    }
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
