dnl Initialise autoconf

AC_INIT([process], [0.0.0], [alriddoch@googlemail.com])
AC_CONFIG_SRCDIR([src/process.cpp])
AC_CONFIG_HEADERS([config.h])

dnl Detect the canonical host environment

AC_CANONICAL_HOST

dnl Initialise automake

AM_INIT_AUTOMAKE([nostdinc dist-bzip2 silent-rules])
AM_SILENT_RULES([yes])

dnl Test for C++ compiler

AC_PROG_CXX
AC_PROG_CPP

CXXFLAGS="$CXXFLAGS -Wall -Wno-unknown-pragmas"

AC_MSG_CHECKING(whether gcc needs -fno-strict-aliasing)
AC_PREPROC_IFELSE([
#ifdef __GNUC__
# if defined(__GNUC_MINOR__) && __GNUC__ == 3 && (__GNUC_MINOR__ >= 3)
#   error -fno-strict-aliasing needed
# elif __GNUC__ > 3
#   error -fno-strict-aliasing needed
# endif
#endif
],[
  GCC_EXTRA_OPT=
  AC_MSG_RESULT(no)
],[
  GCC_EXTRA_OPT=-fno-strict-aliasing
  AC_MSG_RESULT(yes)
])

AC_ARG_ENABLE(debug,
    [  --enable-debug          enable debug information [default=no]],
    [
        if test "$enableval" = "yes"; then
            CXXFLAGS="$CXXFLAGS $GCC_EXTRA_OPT -Wall -DDEBUG"
        else
            CXXFLAGS="$CXXFLAGS $GCC_EXTRA_OPT -Wall -DNDEBUG"
        fi
    ],[
        CXXFLAGS="$CXXFLAGS $GCC_EXTRA_OPT -Wall -DNDEBUG"
    ]
)

PKG_PROG_PKG_CONFIG

dnl Test for WorldForge Atlas Libraries

PKG_CHECK_MODULES(WF, skstream-0.3 atlascpp-0.6 ,
    [
        CPPFLAGS="$CPPFLAGS $WF_CFLAGS"
        LIBS="$LIBS $WF_LIBS"
    ],
    [
        AC_MSG_ERROR(Couldn't find WF libs.)
    ])

PKG_CHECK_MODULES(ERIS, eris-1.3, [ AC_MSG_NOTICE(Found Eris.)],
    [
        AC_MSG_ERROR(Couldn't find Eris.)
    ])

AC_LANG(C++)

dnl allow the user to provide the directory where python is installed in
dnl
python_prefix=/usr
AC_ARG_WITH(python,
[  --with-python=DIR       Prefix where python is installed in [default=/usr]],
[
    if test $withval != yes; then
        python_prefix=$withval
    fi
])

dnl then check for the header file Python.h and set
dnl python_include_path and python_version
dnl appropriately to what we have found
dnl


AC_CHECK_HEADER(python2.7/Python.h,
[
    python_include_path=-I${python_prefix}/include/python2.7
    python_version=2.7
],[
    AC_CHECK_HEADER(python2.6/Python.h,
    [
        python_include_path=-I${python_prefix}/include/python2.6
        python_version=2.6
    ],[
        AC_CHECK_HEADER(python2.5/Python.h,
        [
            python_include_path=-I${python_prefix}/include/python2.5
            python_version=2.5
        ],[
            AC_CHECK_HEADER(python2.4/Python.h,
            [
                python_include_path=-I${python_prefix}/include/python2.4
                python_version=2.4
            ],[
                AC_CHECK_HEADER(python2.3/Python.h,
                [
                    python_include_path=-I${python_prefix}/include/python2.3
                    python_version=2.3
                ],[
                    AC_CHECK_HEADER(python2.2/Python.h,
                    [
                        python_include_path=-I${python_prefix}/include/python2.2
                        python_version=2.2
                    ],[
                        AC_MSG_ERROR([
Cannot find python headers for Python 2.2 or later.
Please see http://www.python.org/ for details of how to download and install
Python. If Python is installed somewhere other than in /usr the please use the
--with-python=DIR option to point to the prefix where Python is installed.])
                    ])
                ])
            ])
        ])
    ])
])

PYTHON_INCLUDES=${python_include_path}
CPPFLAGS="$CPPFLAGS ${PYTHON_INCLUDES}"

dnl build the library path from the found version
python_lib_path=${python_prefix}/lib/${python_version}/config

AC_CHECK_LIB(dl,main)
AC_CHECK_LIB(util,openpty)

AC_CHECK_FUNC(pthread_mutex_trylock, ,
[
    ac_save_CXXFLAGS="$CXXFLAGS"
    CXXFLAGS="$CXXFLAGS -pthread"
    AC_CHECK_FUNC(pthread_create, ,
    [
        CXXFLAGS="$ac_save_CXXFLAGS"
        AC_CHECK_LIB(pthread, pthread_create, ,
        [
            AC_MSG_WARN([Couldn't find pthread library. Python lib check may fail])
        ])
    ])
])

AC_CHECK_LIB(python${python_version}, Py_Initialize,
[
    python_libs="-lpython${python_version}"
    PYTHON_UTIL_LIBS=
],
[
    AC_CHECK_LIB(python${python_version}, Py_Initialize,
    [
        python_libs="-L${python_lib_path} -lpython${python_version}"
        PYTHON_LINKER_FLAGS="-export-dynamic"
    ],AC_MSG_ERROR([
Cannot find python libraries. Do you have python development installed? Please
see http://www.python.org/ for details of how to download and install Python.]),
    [-L${python_lib_path} ${PYTHON_UTIL_LIBS}])
])

PYTHON_LIBS=${python_libs}
LIBS="$LIBS $PYTHON_LIBS"

dnl Generate files
AC_CONFIG_FILES([
        Makefile
        src/Makefile
        perf/Makefile
        loadtool/Makefile
])
AC_OUTPUT
