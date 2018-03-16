/*
 * default includes
 */

#include <stdio.h>
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#if STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
# include <stdarg.h>
#else
# if HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#if HAVE_STRING_H
# if !STDC_HEADERS && HAVE_MEMORY_H
#  include <memory.h>
# endif
# include <string.h>
#endif
#if HAVE_STRINGS_H
# include <strings.h>
#endif
#if HAVE_INTTYPES_H
# include <inttypes.h>
#else
# if HAVE_STDINT_H
#  include <stdint.h>
# endif
#endif
#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_LIBINTL_H
#include <libintl.h>
#else
#define bindtextdomain(x,y)
#define gettext(x) x
#endif

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#if HAVE_FCNTL_H
#include <fcntl.h>
#endif

#if HAVE_TERMIOS_H
# include <termios.h>
#else
# if HAVE_TERMIO_H
#  include <termio.h>
# else
// #  error "no termio[s] header found"
# endif
#endif

#if HAVE_CURSES_H
# include <curses.h>
#else
# if HAVE_NCURSES_H
#  include <ncurses.h>
# else
// #  error "no curses header found"
# endif
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

/*
 * debug macros
 */
#ifdef TRACE
#define FTRACE(func)	fprintf(stderr, (func))
#else
#define FTRACE(func)
#endif

#ifdef READTRACE
#define RTRACE(rchar)	fprintf(stderr, "in: 0x%x\n", (unsigned int)(rchar))
#else
#define RTRACE(rchar)
#endif

#ifdef WRITETRACE
#define WTRACE(rchar)	fprintf(stderr, "out: 0x%x\n", (unsigned int)(rchar))
#else
#define WTRACE(rchar)
#endif

/*
 * set misc info's if not defined by autoconf
 */
#ifndef PACKAGE
#define PACKAGE "katceterm"
#endif

#ifndef LOCALEDIR
#define LOCALEDIR "/usr/local/share/locale"
#endif

#ifndef PATH_MAX
#define PATH_MAX 255U		/* if PATH_MAX is not defined set to minimum */
#endif

/*
 * redefine system defines for splint
 */
#ifdef S_SPLINT_S
/*@-incondefs@*//*@-redecl@ *//*@-isoreserved@ */
extern int tcgetattr (int, /*@out@ */ struct termios *);
extern ssize_t read (int fildes, /*@out@ */ void *buf, size_t nbyte);
int getopt (int argc, char *const *argv, const char *optstring) /*@warn legacy@ */ /*@globals optarg, optind, optopt, opterr@ */ /*@modifies optarg, optind, optopt@ */ /*@requires maxRead(argv) >= (argc - 1) @ */ ;
size_t strlcpy ( /*@unique@ *//*@out@ */ char *s1, char *s2, size_t s1size) /*@modifies *s1@ */ ;
/*@=incondefs@*//*@=redecl@ *//*@=isoreserved@ */
#endif

