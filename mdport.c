/*
    mdport.c - Machine Dependent Code for Porting Unix/Curses games

    Copyright (C) 2005 Nicholas J. Kisseberth
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. Neither the name(s) of the author(s) nor the names of other contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) AND CONTRIBUTORS ``AS IS'' AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR(S) OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
*/

#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <Windows.h>
#include <Lmcons.h>
#include <io.h>
#include <conio.h>
#pragma warning( disable: 4201 )
#include <shlobj.h>
#pragma warning( default: 4201 )
#include <Shlwapi.h>
#undef MOUSE_MOVED
#endif

#include <curses.h>
#include "extern.h"

#if defined(HAVE_SYS_TYPES)
#include <sys/types.h>
#endif

#if defined(HAVE_PROCESS_H)
#include <process.h>
#endif

#if defined(HAVE_PWD_H)
#include <pwd.h>
#endif

#if defined(HAVE_SYS_UTSNAME)
#include <sys/utsname.h>
#endif

#if defined(HAVE_ARPA_INET_H)
#include <arpa/inet.h> /* Solaris 2.8 required this for htonl() and ntohl() */
#endif

#if defined(HAVE_TERMIOS_H)
#include <termios.h>
#endif

#if defined(HAVE_UNISTD_H)
#ifndef __USE_GNU
#define __USE_GNU
#include <unistd.h>
#undef __USE_GNU
#else
#include <unistd.h>
#endif
#endif

#include <curses.h> /* AIX requires curses.h be included before term.h */

#if defined(HAVE_TERM_H)
#include <term.h>
#elif defined(HAVE_NCURSES_TERM_H)
#include <ncurses/term.h>
#endif

#if defined(HAVE_WORKING_FORK)
#include <sys/wait.h>
#endif

#include <ctype.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <signal.h>
#include "extern.h"

#if !defined(PATH_MAX) && defined(_MAX_PATH)
#define PATH_MAX _MAX_PATH
#endif

#if !defined(PATH_MAX) && defined(_PATH_MAX)
#define PATH_MAX _PATH_MAX
#endif

#define NOOP(x) (x += 0)

void md_init()
{
#if defined(__INTERIX)
    char *term;

    term = getenv ("TERM");

    if (term == NULL)
    {
        setenv ("TERM", "interix");
    }

#elif defined(__DJGPP__)
    _fmode = _O_BINARY;
#elif defined(_WIN32)
    _fmode = _O_BINARY;
#endif

#if defined(HAVE_ESCDELAY) || defined(NCURSES_VERSION)
    ESCDELAY = 64;
#endif

#if defined(DUMP)
    md_onsignal_default();
#else
    md_onsignal_exit();
#endif
}

void md_onsignal_default()
{
#ifdef SIGHUP
    signal (SIGHUP, SIG_DFL);
#endif
#ifdef SIGQUIT
    signal (SIGQUIT, SIG_DFL);
#endif
#ifdef SIGILL
    signal (SIGILL, SIG_DFL);
#endif
#ifdef SIGTRAP
    signal (SIGTRAP, SIG_DFL);
#endif
#ifdef SIGIOT
    signal (SIGIOT, SIG_DFL);
#endif
#ifdef SIGEMT
    signal (SIGEMT, SIG_DFL);
#endif
#ifdef SIGFPE
    signal (SIGFPE, SIG_DFL);
#endif
#ifdef SIGBUS
    signal (SIGBUS, SIG_DFL);
#endif
#ifdef SIGSEGV
    signal (SIGSEGV, SIG_DFL);
#endif
#ifdef SIGSYS
    signal (SIGSYS, SIG_DFL);
#endif
#ifdef SIGTERM
    signal (SIGTERM, SIG_DFL);
#endif
}

void md_onsignal_exit()
{
#ifdef SIGHUP
    signal (SIGHUP, SIG_DFL);
#endif
#ifdef SIGQUIT
    signal (SIGQUIT, exit);
#endif
#ifdef SIGILL
    signal (SIGILL, exit);
#endif
#ifdef SIGTRAP
    signal (SIGTRAP, exit);
#endif
#ifdef SIGIOT
    signal (SIGIOT, exit);
#endif
#ifdef SIGEMT
    signal (SIGEMT, exit);
#endif
#ifdef SIGFPE
    signal (SIGFPE, exit);
#endif
#ifdef SIGBUS
    signal (SIGBUS, exit);
#endif
#ifdef SIGSEGV
    signal (SIGSEGV, exit);
#endif
#ifdef SIGSYS
    signal (SIGSYS, exit);
#endif
#ifdef SIGTERM
    signal (SIGTERM, exit);
#endif
}

void md_onsignal_autosave()
{
#ifdef SIGHUP
    signal (SIGHUP, auto_save);
#endif
#ifdef SIGQUIT
    signal (SIGQUIT, endit);
#endif
#ifdef SIGILL
    signal (SIGILL, auto_save);
#endif
#ifdef SIGTRAP
    signal (SIGTRAP, auto_save);
#endif
#ifdef SIGIOT
    signal (SIGIOT, auto_save);
#endif
#ifdef SIGEMT
    signal (SIGEMT, auto_save);
#endif
#ifdef SIGFPE
    signal (SIGFPE, auto_save);
#endif
#ifdef SIGBUS
    signal (SIGBUS, auto_save);
#endif
#ifdef SIGSEGV
    signal (SIGSEGV, auto_save);
#endif
#ifdef SIGSYS
    signal (SIGSYS, auto_save);
#endif
#ifdef SIGTERM
    signal (SIGTERM, auto_save);
#endif
#ifdef SIGINT
    signal (SIGINT, quit);
#endif
}

int md_hasclreol()
{
#if defined(clr_eol)
#ifdef NCURSES_VERSION

    if (cur_term == NULL)
    {
        return (0);
    }

    if (cur_term->type.Strings == NULL)
    {
        return (0);
    }

#endif
    return ((clr_eol != NULL) && (*clr_eol != 0));
#elif defined(__PDCURSES__)
    return (TRUE);
#else
    return ((CE != NULL) && (*CE != 0));
#endif
}

void md_putchar (int c)
{
    putchar (c);
}

#ifdef _WIN32
static int md_standout_mode = 0;
#endif

void md_raw_standout()
{
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
    HANDLE hStdout;
    WORD fgattr, bgattr;

    if (md_standout_mode == 0)
    {
        hStdout = GetStdHandle (STD_OUTPUT_HANDLE);
        GetConsoleScreenBufferInfo (hStdout, &csbiInfo);
        fgattr = (csbiInfo.wAttributes & 0xF);
        bgattr = (csbiInfo.wAttributes & 0xF0);
        SetConsoleTextAttribute (hStdout, (fgattr << 4) | (bgattr >> 4));
        md_standout_mode = 1;
    }

#elif defined(SO)
    tputs (SO, 0, md_putchar);
    fflush (stdout);
#endif
}

void md_raw_standend()
{
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
    HANDLE hStdout;
    WORD fgattr, bgattr;

    if (md_standout_mode == 1)
    {
        hStdout = GetStdHandle (STD_OUTPUT_HANDLE);
        GetConsoleScreenBufferInfo (hStdout, &csbiInfo);
        fgattr = (csbiInfo.wAttributes & 0xF);
        bgattr = (csbiInfo.wAttributes & 0xF0);
        SetConsoleTextAttribute (hStdout, (fgattr << 4) | (bgattr >> 4));
        md_standout_mode = 0;
    }

#elif defined(SE)
    tputs (SE, 0, md_putchar);
    fflush (stdout);
#endif
}

int md_unlink_open_file (char *file, FILE *inf)
{
#ifdef _WIN32
    fclose (inf);
    _chmod (file, 0600);
    return (_unlink (file));
#else
    return (unlink (file));
#endif
}

int md_unlink (char *file)
{
#ifdef _WIN32
    _chmod (file, 0600);
    return (_unlink (file));
#else
    return (unlink (file));
#endif
}

int md_chmod (char *filename, int mode)
{
#ifdef _WIN32
    return (_chmod (filename, mode));
#else
    return (chmod (filename, mode));
#endif
}

void md_normaluser()
{
#if defined(HAVE_GETGID) && defined(HAVE_GETUID)
    gid_t realgid = getgid();
    uid_t realuid = getuid();

#if defined(HAVE_SETRESGID)

    if (setresgid (-1, realgid, realgid) != 0)
    {
#elif defined (HAVE_SETREGID)

    if (setregid (realgid, realgid) != 0)
    {
#elif defined (HAVE_SETGID)

    if (setgid (realgid) != 0)
    {
#else

    if (0)
    {
#endif
        perror ("Could not drop setgid privileges.  Aborting.");
        exit (1);
    }

#if defined(HAVE_SETRESUID)

    if (setresuid (-1, realuid, realuid) != 0)
    {
#elif defined(HAVE_SETREUID)

    if (setreuid (realuid, realuid) != 0)
    {
#elif defined(HAVE_SETUID)

    if (setuid (realuid) != 0)
    {
#else

    if (0)
    {
#endif
        perror ("Could not drop setuid privileges.  Aborting.");
        exit (1);
    }

#endif
}

int md_getuid()
{
#ifdef HAVE_GETUID
    return (getuid());
#else
    return (42);
#endif
}

int md_getpid()
{
#ifdef _WIN32
    return (_getpid());
#else
    return (getpid());
#endif
}

char *md_getusername()
{
    static char login[80];
    char *l = NULL;
#ifdef _WIN32
    LPSTR mybuffer;
    DWORD size = UNLEN + 1;
    TCHAR buffer[UNLEN + 1];

    mybuffer = buffer;
    GetUserName (mybuffer, &size);
    l = mybuffer;
#elif defined(HAVE_GETPWUID)&& !defined(__DJGPP__)
    struct passwd *pw;

    pw = getpwuid (getuid());

    l = pw->pw_name;
#endif

    if ((l == NULL) || (*l == '\0'))
        if ((l = getenv ("USERNAME")) == NULL)
            if ((l = getenv ("LOGNAME")) == NULL)
                if ((l = getenv ("USER")) == NULL)
                {
                    l = "nobody";
                }

    strncpy (login, l, 80);
    login[79] = 0;

    return (login);
}

char *md_gethomedir()
{
    static char homedir[PATH_MAX];
    char *h = NULL;
    size_t len;
#if defined(_WIN32)
    TCHAR szPath[PATH_MAX];
#endif
#if defined(_WIN32) || defined(DJGPP)
    char slash = '\\';
#else
    char slash = '/';
    struct passwd *pw;
    pw = getpwuid (getuid());

    h = pw->pw_dir;

    if (strcmp (h, "/") == 0)
    {
        h = NULL;
    }

#endif
    homedir[0] = 0;
#ifdef _WIN32

    if (SUCCEEDED (SHGetFolderPath (NULL, CSIDL_PERSONAL, NULL, 0, szPath)))
    {
        h = szPath;
    }

#endif

    if ((h == NULL) || (*h == '\0'))
    {
        if ((h = getenv ("HOME")) == NULL)
        {
            if ((h = getenv ("HOMEDRIVE")) == NULL)
            {
                h = "";
            }
            else
            {
                strncpy (homedir, h, PATH_MAX - 1);
                homedir[PATH_MAX - 1] = 0;

                if ((h = getenv ("HOMEPATH")) == NULL)
                {
                    h = "";
                }
            }
        }
    }


    len = strlen (homedir);
    strncat (homedir, h, PATH_MAX - len - 1);
    len = strlen (homedir);

    if ((len > 0) && (homedir[len - 1] != slash))
    {
        homedir[len] = slash;
        homedir[len + 1] = 0;
    }

    return (homedir);
}

void md_sleep (int s)
{
#ifdef _WIN32
    Sleep (s);
#else
    sleep (s);
#endif
}

char *md_getshell()
{
    static char shell[PATH_MAX];
    char *s = NULL;
#ifdef _WIN32
    char *def = "C:\\WINDOWS\\SYSTEM32\\CMD.EXE";
#elif defined(__DJGPP__)
    char *def = "C:\\COMMAND.COM";
#else
    char *def = "/bin/sh";
    struct passwd *pw;
    pw = getpwuid (getuid());
    s = pw->pw_shell;
#endif

    if ((s == NULL) || (*s == '\0'))
        if ((s = getenv ("COMSPEC")) == NULL)
            if ((s = getenv ("SHELL")) == NULL)
                if ((s = getenv ("SystemRoot")) == NULL)
                {
                    s = def;
                }

    strncpy (shell, s, PATH_MAX);
    shell[PATH_MAX - 1] = 0;

    return (shell);
}

int md_shellescape()
{
#if defined(HAVE_WORKING_FORK)
    int ret_status;
    int pid;
    void (*myquit) (int);
    void (*myend) (int);
    char *sh;

    sh = md_getshell();

    while ((pid = fork()) < 0)
    {
        sleep (1);
    }

    if (pid == 0)   /* Shell Process */
    {
        /*
         * Set back to original user, just in case
         */
        md_normaluser();
        execl (sh == NULL ? "/bin/sh" : sh, "shell", "-i", NULL);
        perror ("No shelly");
        _exit (-1);
    }
    else     /* Application */
    {
        myend = signal (SIGINT, SIG_IGN);
#ifdef SIGQUIT
        myquit = signal (SIGQUIT, SIG_IGN);
#endif

        while (wait (&ret_status) != pid)
        {
            continue;
        }

        signal (SIGINT, myquit);
#ifdef SIGQUIT
        signal (SIGQUIT, myend);
#endif
    }

    return (ret_status);
#elif defined(HAVE__SPAWNL)
    return ((int) _spawnl (_P_WAIT, md_getshell(), "shell", NULL, 0));
#elif defined(HAVE_SPAWNL)
    return (spawnl (P_WAIT, md_getshell(), "shell", NULL, 0));
#else
    return (0);
#endif
}

int directory_exists (char *dirname)
{
    struct stat sb;

    if (stat (dirname, &sb) == 0)     /* path exists */
    {
        return (sb.st_mode & S_IFDIR);
    }

    return (0);
}

char *md_getrealname (int uid)
{
    static char uidstr[20];
#if !defined(_WIN32) && !defined(DJGPP)
    struct passwd *pp;

    if ((pp = getpwuid (uid)) == NULL)
    {
        sprintf (uidstr, "%d", uid);
        return (uidstr);
    }
    else
    {
        return (pp->pw_name);
    }

#else
    sprintf (uidstr, "%d", uid);
    return (uidstr);
#endif
}

extern char *xcrypt (char *key, char *salt);

char *md_crypt (char *key, char *salt)
{
    return (xcrypt (key, salt));
}

char *md_getpass (char *prompt)
{
#ifndef HAVE_GETPASS
    static char password_buffer[9];
    char *p = password_buffer;
    int c, count = 0;
    int max_length = 9;

    fflush (stdout);

    /* If we can't prompt, abort */
    if (fputs (prompt, stderr) < 0)
    {
        *p = '\0';
        return NULL;
    }

    for (;;)
    {
        /* Get a character with no echo */
        c = _getch();

        /* Exit on interrupt (^c or ^break) */
        if (c == '\003' || c == 0x100)
        {
            exit (1);
        }

        /* Terminate on end of line or file (^j, ^m, ^d, ^z) */
        if (c == '\r' || c == '\n' || c == '\004' || c == '\032')
        {
            break;
        }

        /* Back up on backspace */
        if (c == '\b')
        {
            if (count)
            {
                count--;
            }
            else if (p > password_buffer)
            {
                p--;
            }

            continue;
        }

        /* Ignore DOS extended characters */
        if ((c & 0xff) != c)
        {
            continue;
        }

        /* Add to password if it isn't full */
        if (p < password_buffer + max_length - 1)
        {
            *p++ = (char) c;
        }
        else
        {
            count++;
        }
    }

    *p = '\0';

    fputc ('\n', stderr);

    return password_buffer;
#else
    return ((char *) getpass (prompt));
#endif
}

int md_erasechar()
{
#ifdef HAVE_ERASECHAR
    return (erasechar());   /* process erase character */
#elif defined(VERASE)
    return (_tty.c_cc[VERASE]);   /* process erase character */
#else
    return (_tty.sg_erase);   /* process erase character */
#endif
}

int md_killchar()
{
#ifdef HAVE_KILLCHAR
    return (killchar());
#elif defined(VKILL)
    return (_tty.c_cc[VKILL]);
#else
    return (_tty.sg_kill);
#endif
}

int md_dsuspchar()
{
#if defined(VDSUSP)         /* POSIX has priority */
    struct termios attr;
    tcgetattr (STDIN_FILENO, &attr);
    return (attr.c_cc[VDSUSP]);
#elif defined(TIOCGLTC)
    struct ltchars ltc;
    ioctl (1, TIOCGLTC, &ltc);
    return (ltc.t_dsuspc);
#elif defined(_POSIX_VDISABLE)
    return (_POSIX_VDISABLE);
#else
    return (0);
#endif
}

int md_setdsuspchar (int c)
{
#if defined(VDSUSP)         /* POSIX has priority */
    struct termios attr;
    tcgetattr (STDIN_FILENO, &attr);
    attr.c_cc[VDSUSP] = c;
    tcgetattr (STDIN_FILENO, &attr);
#elif defined(TIOCSLTC)
    struct ltchars ltc;
    ioctl (1, TIOCGLTC, &ltc);
    ltc.t_dsuspc = c;
    ioctl (1, TIOCSLTC, &ltc);
#else
    NOOP (c);
#endif
    return (0);
}

int md_suspchar()
{
#if defined(VSUSP)          /* POSIX has priority */
    struct termios attr;
    tcgetattr (STDIN_FILENO, &attr);
    return (attr.c_cc[VSUSP]);
#elif defined(TIOCGLTC)
    struct ltchars ltc;
    ioctl (1, TIOCGLTC, &ltc);
    return (ltc.t_suspc);
#elif defined(_POSIX_VDISABLE)
    return (_POSIX_VDISABLE);
#else
    return (0);
#endif
}

#define M_NORMAL 0
#define M_ESC    1
#define M_KEYPAD 2
#define M_TRAIL  3

int md_readchar()
{
    int ch = 0;
    int lastch = 0;
    int mode = M_NORMAL;
    int mode2 = M_NORMAL;

    for (;;)
    {
        ch = getch();

        if (ch == ERR)      /* timed out waiting for valid sequence */
        {
            /* flush input so far and start over    */
            mode = M_NORMAL;
            nocbreak();
            raw();
            ch = 27;
            break;
        }

        if (mode == M_TRAIL)
        {
            if (ch == '^')      /* msys console  : 7,5,6,8: modified*/
            {
                ch = CTRL (toupper (lastch));
            }

            if (ch == '~')      /* cygwin console: 1,5,6,4: normal  */
            {
                ch = tolower (lastch);    /* windows telnet: 1,5,6,4: normal  */
            }

            /* msys console  : 7,5,6,8: normal  */

            if (mode2 == M_ESC)     /* cygwin console: 1,5,6,4: modified*/
            {
                ch = CTRL (toupper (ch));
            }

            break;
        }

        if (mode == M_ESC)
        {
            if (ch == 27)
            {
                mode2 = M_ESC;
                continue;
            }
            break;
        }

        if (ch == 27)
        {
            halfdelay (1);
            mode = M_ESC;
            continue;
        }

        break;
    }

    nocbreak();     /* disable halfdelay mode if on */
    raw();

    return (ch & 0x7F);
}

#if defined(LOADAV) && defined(HAVE_NLIST_H) && defined(HAVE_NLIST)
/*
 * loadav:
 *  Looking up load average in core (for system where the loadav()
 *  system call isn't defined
 */

#include <nlist.h>

struct nlist avenrun =
{
    "_avenrun"
};

void md_loadav (double *avg)
{
    int kmem;

    if ((kmem = open ("/dev/kmem", 0)) < 0)
    {
        goto bad;
    }

    nlist (NAMELIST, &avenrun);

    if (avenrun.n_type == 0)
    {
        close (kmem);
    bad:
        avg[0] = 0.0;
        avg[1] = 0.0;
        avg[2] = 0.0;
        return;
    }

    lseek (kmem, avenrun.n_value, 0);
    read (kmem, (char *) avg, 3 * sizeof (double));
    close (kmem);
}
#else
void md_loadav (double *avg)
{
#if defined(HAVE_LOADAV)
    loadav (avg);
#elif defined(HAVE_GETLOADAVG)
    getloadavg (avg, 3);
#else
    avg[0] = avg[1] = avg[2] = 0;
#endif
}
#endif

#ifndef NSIG
#define NSIG 32
#endif

void md_ignoreallsignals()
{
    int i;

    for (i = 0; i < NSIG; i++)
    {
        signal (i, SIG_IGN);
    }
}

void md_tstphold()
{
#ifdef SIGTSTP
    /*
     * If a process can be suspended, this code wouldn't work
     */
# ifdef SIG_HOLD
    signal (SIGTSTP, SIG_HOLD);
# else
    signal (SIGTSTP, SIG_IGN);
# endif
#endif
}

void md_tstpresume()
{
#ifdef SIGTSTP
    signal (SIGTSTP, tstp);
#endif
}

void md_tstpsignal()
{
#ifdef SIGTSTP
    kill (0, SIGTSTP);      /* send actual signal and suspend process */
#endif
}

#if defined(CHECKTIME)
void md_start_checkout_timer (int time)
{
    int  checkout();

#if defined(HAVE_ALARM) && defined(SIGALRM)
    signal (SIGALRM, checkout);
    alarm (time);
#endif
}

void md_stop_checkout_timer()
{
#if defined(SIGALRM)
    signal (SIGALRM, SIG_IGN);
#endif
}

#endif
