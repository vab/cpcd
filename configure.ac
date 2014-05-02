dnl Process this file with autoconf to produce a configure script.
AC_INIT([cpcd], [1.1.0])
AC_CONFIG_SRCDIR([src/cpcd.c])
AM_INIT_AUTOMAKE

dnl Set of available languages.
ALL_LINGUAS="en"

dnl Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

dnl Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(limits.h unistd.h pthread.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_STRUCT_TM

dnl Checks for library installation
AC_HAVE_LIBRARY(pthread,,[AC_MSG_ERROR([Please install the pthread library.])])

dnl Checks for library functions.
AC_FUNC_MEMCMP
AC_CHECK_FUNCS(socket strstr)
AC_CHECK_LIB(pthread, pthread_create, [], [echo "Problem detected linking with the pthread library."; exit 1])

AC_SUBST(PKG_CPPFLAGS)
AC_SUBST(PKG_LIBS)
#AC_SUBST(PKG_LDFLAGS)
AC_SUBST(PKG_CFLAGS)
AC_SUBST(PKG_FFLAGS)

# Where to put cpcd.conf
if test [ \"NONE\" == \"$prefix\" ]; then
AC_SUBST(CONFDIR, "/usr/local/etc")
AC_ARG_WITH(conf,
	AC_HELP_STRING([--with-conf=FILE],
		[where config file lives [[/usr/local/etc]]]),
	CONF="${withval}")
else
	AC_SUBST(CONFDIR, "$prefix/etc")
	AC_ARG_WITH(conf,
        AC_HELP_STRING([--with-confdir=DIR],
		[where config file lives [[/usr/local/etc]]]),
	CONFDIR="${withval}")
fi
AC_MSG_NOTICE([where to store config... $CONFDIR])
AC_DEFINE_UNQUOTED(CONFDIR, "$CONFDIR", [Where to store config.])
AC_DEFINE_UNQUOTED(CONF, "$CONFDIR/cpcd.conf", [Where to find cpcd.conf.])

# Where the logs go
if test [ \"NONE\" == \"$prefix\" ]; then
AC_SUBST(LOGDIR,"/var/log/cpcd")
AC_ARG_WITH(logdir,
	AC_HELP_STRING([--with-logdir=DIR],
		[where logs are stored [[/var/log/cpcd]]]),
	LOGDIR="${withval}")
else
	AC_SUBST(LOGDIR, "$prefix/log")
	AC_ARG_WITH(logsdir,
        AC_HELP_STRING([--with-logdir=DIR],
		[where logs are stored [[/var/log/cpcd]]]),
	LOGDIR="${withval}")
fi
AC_MSG_NOTICE([where to store logs... $LOGDIR])
AC_DEFINE_UNQUOTED(LOGDIR, "$LOGDIR", [Where to store logs.])

# Where the data goes
if test [ \"NONE\" == \"$prefix\" ]; then
AC_SUBST(DATADIR,"/var/tmp/cpcd")
AC_ARG_WITH(datadir,
	AC_HELP_STRING([--with-datadir=DIR],
		[where datafiles are stored [[/var/tmp/cpcd]]]),
	DATADIR="${withval}")
else
	AC_SUBST(DATADIR, "$prefix/data")
	AC_ARG_WITH(datadir,
        AC_HELP_STRING([--with-datadir=DIR],
		[where data files are stored [[/var/tmp/cpcd]]]),
	DATADIR="${withval}")
fi
AC_MSG_NOTICE([where to store data... $DATADIR])
AC_DEFINE_UNQUOTED(DATADIR, "$DATADIR", [Where to store data.])

# Where the pid goes
if test [ \"NONE\" == \"$prefix\" ]; then
AC_SUBST(PIDDIR,"/var/run/cpcd")
AC_ARG_WITH(piddir,
	AC_HELP_STRING([--with-piddir=DIR],
		[where the pid is stored [[/var/run/cpcd]]]),
	PIDDIR="${withval}")
else
	AC_SUBST(PIDDIR, "$prefix/run/cpcd")
	AC_ARG_WITH(piddir,
        AC_HELP_STRING([--with-piddir=DIR],
		[where the pid is stored [[/var/run/cpcd]]]),
	PIDDIR="${withval}")
fi
AC_MSG_NOTICE([where to store pid... $PIDDIR])
AC_DEFINE_UNQUOTED(PIDDIR, "$PIDDIR", [Where to store the pid file.])

# Where the documentation goes
if test [ \"NONE\" == \"$prefix\" ]; then
AC_SUBST(DOCDIR,"/usr/local/doc")
AC_ARG_WITH(docdir,
	AC_HELP_STRING([--with-docdir=DIR],
		[where the docs are stored [[/usr/local/doc]]]),
	DOCDIR="${withval}")
else
	AC_SUBST(DOCDIR, "$prefix/doc")
	AC_ARG_WITH(docdir,
        AC_HELP_STRING([--with-docdir=DIR],
		[where the documentation is stored [[/usr/local/doc]]]),
	DOCDIR="${withval}")
fi
AC_MSG_NOTICE([where to store docs... $DOCDIR])
AC_DEFINE_UNQUOTED(DOCDIR, "$DOCDIR", [Where to store the documentation.])

dnl Allow the user to enable large file support
AC_ARG_ENABLE(lfs, AC_HELP_STRING([--enable-lfs],
       [Enable Large File Support. (default=no)]),
       [
               if test "x$enable_lfs" = xyes; then
                       AC_MSG_NOTICE([Large File Support Enabled.])
                       ifelse(AC_ACVERSION, [2.13],
                       [
                               CFLAGS="$CFLAGS -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE"
                       ],
                       [
                               AC_SYS_LARGEFILE
                       ])
               fi
       ],[])

AC_OUTPUT([Makefile etc/cpcd.conf etc/cpcd.init etc/Makefile doc/Makefile src/Makefile ])
