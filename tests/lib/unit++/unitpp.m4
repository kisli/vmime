dnl AM_PATH_UNITPP([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl note to self - reference vars with $1, $2 etc

AC_DEFUN(AM_PATH_UNITPP,
[
	AC_LANG_PUSH(C++)

	AC_ARG_ENABLE(unitpp,
	    AC_HELP_STRING([--enable-unitpp-test],
	        [enable unit tests (default is YES if you have libunit++, else NO)]),
		, 
		[enable_unitpp="yes"] ) 

	AC_ARG_WITH(unitpp-prefix,
		AC_HELP_STRING([--with-unitpp-prefix=DIR],
			[specify the location of unit++ for unit tests]) )
	
	if test "x$with_unitpp_prefix"="x"; then
		UNITPP_CXXFLAGS="-I$with_unitpp_prefix/include"
		UNITPP_LIBS="-L$with_unitpp_prefix/lib -lunit++"
	else
#		UNITPP_CXXFLAGS="-I/usr/local/include"
		UNITPP_LIBS="-lunit++"
	fi
	CXXFLAGS="$CXXFLAGS $UNITPP_CXXFLAGS"
	LIBS="$LIBS $UNITPP_LIBS"
	
	if test $enable_unitpp="yes"; then
		AC_CHECK_HEADER(unit++/unit++.h,
				[ac_check_unitpp="yes"],
				[ac_check_unitpp="no"] )
		
		if test $ac_check_unitpp="yes"; then
			AC_MSG_CHECKING(if unit++ test program compiles)
			AC_TRY_LINK(
				[ #include<unit++/unit++.h> ],
				[ unitpp::test* test_test = new unitpp::suite("Test Suite"); ],
				[ac_check_unitpp="yes"
				AC_MSG_RESULT(yes) ],
				[ac_check_unitpp="no"
				AC_MSG_RESULT(no) ] )
		fi
	else
		ac_check_unitpp="yes"
	fi

	if test $ac_check_unitpp="yes"; then
		ifelse([$1], , :, [$1])
	else
		UNITPP_CXXFLAGS=""
		UNITPP_LIBS=""
		ifelse([$2], , :, [$2])
	fi
	
	AC_LANG_POP(C++)
	
	AC_SUBST(UNITPP_CXXFLAGS)
	AC_SUBST(UNITPP_LIBS)
])
