

AC_INIT([main.c])

VERSION="1.0" 
AC_SUBST(VERSION) 

AC_PROG_CC 
AC_LANG_C 
AC_PROG_MAKE_SET 
AC_PROG_INSTALL 
AC_HEADER_STDC 
AC_SYS_LARGEFILE

cf_use_zlib="yes"

AC_ARG_ENABLE(zlib, [  --enable-zlib       use ZLib Compression], cf_use_zlib=$enableval )
AC_ARG_ENABLE(ssl, [  --enable-ssl    use Secure Sockets Layer (default=no. Implies --enable-zlib)], cf_use_ssl=$enableval )
AC_ARG_WITH(system-libuseful, [  --with-system-libuseful       use systemwide libUseful.so rather than bundled version], cf_with_libuseful=$withval )

echo "LU: $cf_with_libuseful"
if test "$cf_with_libuseful" != "no" -a "$cf_with_libuseful" != ""
then
AC_CHECK_LIB([Useful],[OpenSSLVerifyCallback],
  [AC_CHECK_LIB([ssl],[SSL_new],,,[-lcrypto -lz])]
  [AC_CHECK_LIB([crypto],[ERR_load_CRYPTO_strings],,,[-lz])]
  [AC_CHECK_LIB([z],[inflate])]
  ,,[-lssl -lcrypto -lz])
AC_CHECK_LIB(Useful,GetToken)

LUSEFUL="system"
else
LUSEFUL="bundled"
LU_STATIC="libUseful-4/libUseful.a"
AC_CONFIG_SUBDIRS([libUseful-4])
fi



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


AC_SUBST(LU_STATIC)

dnl read Makefile.in and write Makefile 
AC_OUTPUT(Makefile)
