dnl aclocal.m4 generated automatically by aclocal 1.4-p4

dnl Copyright (C) 1994, 1995-8, 1999 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY, to the extent permitted by law; without
dnl even the implied warranty of MERCHANTABILITY or FITNESS FOR A
dnl PARTICULAR PURPOSE.


dnl This file is part of QXi, the Qt XInput Extension.
dnl
dnl Copyright (C) 2000 Daniel Schmitt <pnambic@unu.nu>
dnl
dnl This file may be distributed under the terms of the Q Public License as
dnl appearing in the file COPYING.QPL included in the packaging of this file.
dnl
dnl The portions of this file that deal with the detection of Qt are
dnl stripped-down versions of macros used in KDE (http://www.kde.org/).
dnl 
dnl The originals are copyrighted by
dnl     Copyright (C) 1997 Janos Farkas (chexum@shadow.banki.hu)
dnl               (C) 1997,98,99 Stephan Kulow (coolo@kde.org)

dnl ------------------------------------------------------------------------
dnl Find a file (or one of more files in a list of dirs)
dnl ------------------------------------------------------------------------
dnl
AC_DEFUN(AC_FIND_FILE,
[
$3=NO
for i in $2;
do
  for j in $1;
  do
    if test -r "$i/$j"; then
      $3=$i
      break 2
    fi
  done
done
])

dnl ------------------------------------------------------------------------
dnl Find the meta object compiler in the PATH, in $QTDIR/bin, and some
dnl more usual places
dnl ------------------------------------------------------------------------
dnl
AC_DEFUN(AC_PATH_QT_MOC,
[
   AC_PATH_PROG(MOC, moc, "", $ac_qt_bindir:$QTDIR/bin:$QTDIR/src/moc:/usr/bin /usr/X11R6/bin:/usr/lib/qt/bin:/usr/lib/qt2/bin:/usr/local/qt/bin:$PATH)
 
   if test -z "$MOC"; then
     if test -n "$ac_cv_path_moc"; then
       output=`eval "$ac_cv_path_moc --help 2>&1 | sed -e '1q' | grep Qt"`
     fi
     echo "configure:__oline__: tried to call $ac_cv_path_moc --help 2>&1 | sed -e '1q' | grep Qt" >&AC_FD_CC
     echo "configure:__oline__: moc output: $output" >&AC_FD_CC

     if test -z "$output"; then
       AC_MSG_ERROR([No Qt meta object compiler (moc) found!
Please check whether you installed Qt correctly. 
You need to have a running moc binary.
configure tried to run $ac_cv_path_moc and the test didn't
succeed. If configure shouldn't have tried this one, set
the environment variable MOC to the right one before running
configure.
])
    fi
   fi
   
   AC_SUBST(MOC)
])

AC_DEFUN(AC_PRINT_QT_PROGRAM,
[
cat > conftest.$ac_ext <<EOF
#include "confdefs.h"
#include <qglobal.h>
#include <qapplication.h>
#include <qstyle.h>

int main() {
    QStringList *t = new QStringList();

    return 0;
}
EOF
])

dnl ------------------------------------------------------------------------
dnl Try to find the Qt headers and libraries.
dnl $(QT_LDFLAGS) will be -Lqtliblocation (if needed)
dnl and $(QT_INCLUDES) will be -Iqthdrlocation (if needed)
dnl ------------------------------------------------------------------------
dnl
AC_DEFUN(AC_PATH_QT,
[
LIBQT="-lqt"
AC_MSG_CHECKING([for Qt])

ac_qt_includes=NO ac_qt_libraries=NO ac_qt_bindir=NO
qt_libraries=""
qt_includes=""
AC_ARG_WITH(qt-dir,
    [  --with-qt-dir=DIR       where the root of Qt is installed ],
    [  ac_qt_includes="$withval"/include
       ac_qt_libraries="$withval"/lib
       ac_qt_bindir="$withval"/bin
    ])

AC_ARG_WITH(qt-includes,
    [  --with-qt-includes=DIR  where the Qt includes are. ],
    [  
       ac_qt_includes="$withval"
    ])
    
ac_qt_libs_given=no

AC_ARG_WITH(qt-libraries,
    [  --with-qt-libraries=DIR where the Qt library is installed.],
    [  ac_qt_libraries="$withval"
       ac_qt_libs_given=yes
    ])

AC_CACHE_VAL(ac_cv_have_qt,
[#try to guess Qt locations

qt_incdirs="$QTINC /usr/lib/qt/include /usr/local/qt/include /usr/include/qt3 /usr/include/qt /usr/include /usr/lib/qt2/include /usr/X11R6/include/X11/qt $x_includes"
test -n "$QTDIR" && qt_incdirs="$QTDIR/include $QTDIR $qt_incdirs"
qt_incdirs="$ac_qt_includes $qt_incdirs"
AC_FIND_FILE(qstyle.h, $qt_incdirs, qt_incdir)
ac_qt_includes="$qt_incdir"

qt_libdirs="$QTLIB /usr/lib/qt/lib /usr/X11R6/lib /usr/lib /usr/local/qt/lib /usr/lib/qt /usr/lib/qt2/lib $x_libraries"
test -n "$QTDIR" && qt_libdirs="$QTDIR/lib $QTDIR $qt_libdirs"
if test ! "$ac_qt_libraries" = "NO"; then
  qt_libdirs="$ac_qt_libraries $qt_libdirs"
fi

test=NONE
qt_libdir=NONE
for dir in $qt_libdirs; do
  try="ls -1 $dir/libqt*"
  if test=`eval $try 2> /dev/null`; then qt_libdir=$dir; break; else echo "tried $dir" >&AC_FD_CC ; fi
done

ac_qt_libraries="$qt_libdir"

AC_LANG_SAVE
AC_LANG_CPLUSPLUS

ac_cxxflags_safe="$CXXFLAGS"
ac_ldflags_safe="$LDFLAGS"
ac_libs_safe="$LIBS"

CXXFLAGS="$CXXFLAGS -I$qt_incdir $all_includes"
LDFLAGS="-L$x_libraries -L$qt_libdir $all_libraries"
LIBS="$LIBS $LIBQT"

AC_PRINT_QT_PROGRAM

if AC_TRY_EVAL(ac_link) && test -s conftest; then
  rm -f conftest*
else
  echo "configure: failed program was:" >&AC_FD_CC
  cat conftest.$ac_ext >&AC_FD_CC
  ac_qt_libraries="NO"
fi
rm -f conftest*
CXXFLAGS="$ac_cxxflags_safe"
LDFLAGS="$ac_ldflags_safe"
LIBS="$ac_libs_safe"

AC_LANG_RESTORE
if test "$ac_qt_includes" = NO || test "$ac_qt_libraries" = NO; then
  ac_cv_have_qt="have_qt=no"
  ac_qt_notfound=""
  if test "$ac_qt_includes" = NO; then
    if test "$ac_qt_libraries" = NO; then
      ac_qt_notfound="(headers and libraries)";
    else
      ac_qt_notfound="(headers)";
    fi
  else
    ac_qt_notfound="(libraries)";
  fi

  AC_MSG_ERROR([Qt 2 not found. Please check your installation! ]);
else
  have_qt="yes"
fi
])

eval "$ac_cv_have_qt"

if test "$have_qt" != yes; then
  AC_MSG_RESULT([$have_qt]);
else
  ac_cv_have_qt="have_qt=yes \
    ac_qt_includes=$ac_qt_includes ac_qt_libraries=$ac_qt_libraries"
  AC_MSG_RESULT([libraries $ac_qt_libraries, headers $ac_qt_includes])
  
  qt_libraries="$ac_qt_libraries"
  qt_includes="$ac_qt_includes"
fi

AC_SUBST(qt_libraries)
AC_SUBST(qt_includes)

if test "$qt_includes" = "$x_includes" || test -z "$qt_includes"; then
 QT_INCLUDES="";
else
 QT_INCLUDES="-I$qt_includes"
 all_includes="$QT_INCLUDES $all_includes"
fi

if test "$qt_libraries" = "$x_libraries" || test -z "$qt_libraries"; then
 QT_LDFLAGS=""
else
 QT_LDFLAGS="-L$qt_libraries"
 all_libraries="$QT_LDFLAGS $all_libraries"
fi

AC_SUBST(QT_INCLUDES)
AC_SUBST(QT_LDFLAGS)
AC_PATH_QT_MOC

LIB_QT='-lqt'
AC_SUBST(LIB_QT)

])

dnl ------------------------------------------------------------------------
dnl Find libz (required by libpng)
dnl ------------------------------------------------------------------------
dnl
AC_DEFUN(AC_FIND_ZLIB,
[
AC_MSG_CHECKING([for libz])
AC_CACHE_VAL(ac_cv_lib_z,
[
AC_LANG_C
ac_save_LIBS="$LIBS"
LIBS="$all_libraries -lz $LIBSOCKET"
ac_save_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS $all_includes"
AC_TRY_LINK(dnl
[
#include<zlib.h>
],
            [return (zlibVersion() == ZLIB_VERSION); ],
            eval "ac_cv_lib_z='-lz'",
            eval "ac_cv_lib_z=no")
LIBS="$ac_save_LIBS"
CFLAGS="$ac_save_CFLAGS"
])dnl
if eval "test ! \"`echo $ac_cv_lib_z`\" = no"; then
  AC_DEFINE_UNQUOTED(HAVE_LIBZ, 1, [Define if you have libz])
  LIBZ="$ac_cv_lib_z"
  AC_SUBST(LIBZ)
  AC_MSG_RESULT($ac_cv_lib_z)
else
  AC_MSG_RESULT(no)
  LIBZ=""
  AC_SUBST(LIBZ)
fi
])

dnl ------------------------------------------------------------------------
dnl Find libpng (required by Qt 2)
dnl ------------------------------------------------------------------------
dnl
AC_DEFUN(AC_FIND_PNG,
[
AC_REQUIRE([AC_FIND_ZLIB])
AC_MSG_CHECKING([for libpng])
AC_CACHE_VAL(ac_cv_lib_png,
[
ac_save_LIBS="$LIBS"
LIBS="$LIBS $all_libraries -lpng $LIBZ -lm -lX11 $LIBSOCKET"
ac_save_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS $all_includes"
AC_LANG_C
AC_TRY_LINK(dnl
    [
    #include<png.h>
    ],
    [
    png_structp png_ptr = png_create_read_struct(  /* image ptr */
		PNG_LIBPNG_VER_STRING, 0, 0, 0 );
    return( png_ptr != 0 ); 
    ],
    eval "ac_cv_lib_png='-lpng $LIBZ -lm'",
    eval "ac_cv_lib_png=no"
)
LIBS="$ac_save_LIBS"
CFLAGS="$ac_save_CFLAGS"
])dnl
if eval "test ! \"`echo $ac_cv_lib_png`\" = no"; then
  AC_DEFINE_UNQUOTED(HAVE_LIBPNG, 1, [Define if you have libpng])
  LIBPNG="$ac_cv_lib_png"
  AC_SUBST(LIBPNG)
  AC_MSG_RESULT($ac_cv_lib_png)
else
  AC_MSG_RESULT(no)
  LIBPNG=""
  AC_SUBST(LIBPNG)
fi
])

