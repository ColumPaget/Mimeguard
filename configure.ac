

AC_INIT([main.c])

VERSION="1.0" 
AC_SUBST(VERSION) 

AC_CONFIG_SUBDIRS([libUseful-2.8])

AC_PROG_CC 
AC_LANG_C 
AC_PROG_MAKE_SET 
AC_PROG_INSTALL 
AC_HEADER_STDC 

cf_use_zlib="yes"

AC_ARG_ENABLE(zlib, [  --enable-zlib       use ZLib Compression], cf_use_zlib=$enableval )

if test "$cf_use_zlib" = "yes"
then
AC_CHECK_LIB(z,deflate,,)
cf_have_libz=$ac_cv_lib_z_deflate
fi

dnl read Makefile.in and write Makefile 
AC_OUTPUT(Makefile)
