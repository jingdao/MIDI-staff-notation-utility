AC_INIT(a,0)
AC_PROG_CC
AC_CHECK_HEADERS(sqlite3.h)
AC_CHECK_LIB(sqlite3,sqlite3_prepare)

if test "$ac_cv_header_sqlite3_h" == yes && test "$ac_cv_lib_sqlite3_sqlite3_prepare" == yes
then
	AC_SUBST(USE_SQLITE3,-DUSE_SQLITE3)
	AC_SUBST(LIBSQLITE3,-lsqlite3)
fi

AC_OUTPUT(Makefile)
