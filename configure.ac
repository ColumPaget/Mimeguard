

AC_INIT([main.c])

VERSION="1.0" 
AC_SUBST(VERSION) 

AC_CONFIG_SUBDIRS([libUseful-4])

AC_PROG_CC 
AC_LANG_C 
AC_PROG_MAKE_SET 
AC_PROG_INSTALL 
AC_HEADER_STDC 
AC_SYS_LARGEFILE

cf_use_zlib="yes"

AC_ARG_ENABLE(zlib, [  --enable-zlib       use ZLib Compression], cf_use_zlib=$enableval )
AC_ARG_ENABLE(ssl, [  --enable-ssl    use Secure Sockets Layer (default=no. Implies --enable-zlib)], cf_use_ssl=$enableval )


if test "$cf_use_ssl" = "yes"
then
AC_CHECK_LIB(ssl,SSL_library_init,,)
cf_have_libssl=$ac_cv_lib_ssl_SSL_library_init

AC_CHECK_LIB(crypto,EVP_CIPHER_CTX_init,,)
cf_have_libcrypto=$ac_cv_lib_crypto_EVP_CIPHER_CTX_init
cf_use_zlib="yes"
fi


if test "$cf_use_zlib" = "yes"
then
AC_CHECK_LIB(z,deflate,,)
cf_have_libz=$ac_cv_lib_z_deflate
fi

dnl read Makefile.in and write Makefile 
AC_OUTPUT(Makefile)
