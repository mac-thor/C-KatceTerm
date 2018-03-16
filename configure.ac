dnl Process this file with autoconf to produce a configure script.
AC_INIT([katceterm],[0.1],[thor@internetteam.de])
AC_CONFIG_SRCDIR([katceterm.c])
AC_CONFIG_AUX_DIR(config)
AC_CANONICAL_TARGET([])
dnl AC_CANONICAL_HOST
AM_INIT_AUTOMAKE
AC_ARG_ENABLE([ncurses],
  AS_HELP_STRING([--enable-ncurses],
    [if you prefer ncurses and ncurses is not your default library]),
    [if test "$enableval" = yes
     then
       default_ncurses=yes
     else
       default_ncurses=no
     fi
    ], [default_ncurses=no])
AC_ARG_ENABLE([libintl],
  AS_HELP_STRING([--disable-libintl],
    [if you prefer not to use libintl even it is installed on your system]),
    [if test "$enableval" = no
     then
       default_libintl=no
     else
       default_libintl=yes
     fi
    ], [default_libintl=yes])
dnl Checks for programs.
AC_USE_SYSTEM_EXTENSIONS
AC_PROG_CC
AC_LANG([C])
AC_PROG_INSTALL

dnl Checks for libraries.
if test $default_ncurses = no; then
  AC_CHECK_LIB(curses, getch)
  if test $ac_cv_lib_curses_getch = no; then
    AC_CHECK_LIB(ncurses, getch)
  fi
else
  AC_CHECK_LIB(ncurses, getch)
  if test $ac_cv_lib_ncurses_getch = no; then
    AC_CHECK_LIB(curses, getch)
  fi
fi

dnl add targed specific flags
case "$target" in
  *-*-solaris*)
    CFLAGS="$CFLAGS -D__EXTENSIONS__"
    ;;
esac

dnl Checks for header files.
AM_CONFIG_HEADER([config.h])
AC_HEADER_STDC
AC_HEADER_TIME 
AC_HEADER_STAT
AC_HEADER_TIOCGWINSZ
AC_CHECK_HEADERS([fcntl.h inttypes.h limits.h memory.h stddef.h stdint.h stdlib.h string.h strings.h termio.h termios.h unistd.h errno.h sys/time.h])
if test $default_ncurses = no; then
  AC_CHECK_HEADERS([curses.h])
  if test $ac_cv_header_curses_h = no; then
    AC_CHECK_HEADERS([ncurses.h])
  fi
else
  AC_CHECK_HEADERS([ncurses.h])
  if test $ac_cv_header_ncurses_h = no; then
    AC_CHECK_HEADERS([curses.h])
  fi
fi
if test $default_libintl = yes
then
  AC_CHECK_HEADERS([libintl.h])
fi
dnl Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_C_BIGENDIAN
AC_C_CONST
AC_SYS_LONG_FILE_NAMES
AC_SYS_LARGEFILE
AC_SYS_POSIX_TERMIOS()

dnl check for bindtextdomain in default library
if test $default_libintl = yes
then
  AC_CHECK_FUNC(bindtextdomain,[:],[AC_CHECK_LIB(intl, bindtextdomain)])
fi

dnl Checks for library functions.
AC_CHECK_FUNCS([strerror usleep select])

dnl my use AC_REPLACE_FUNCS(strerror usleep) and supply the functions

AC_CONFIG_FILES([Makefile po/Makefile])
AC_OUTPUT
