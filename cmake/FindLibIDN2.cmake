# Try to find the GNU Libidn2 library (libidn2)
#
# Once done this will define
#
#  LIBIDN2_FOUND - System has libidn2
#  LIBIDN2_INCLUDE_DIR - The libidn2 include directory
#  LIBIDN2_LIBRARIES - The libraries needed to use libidn2

# Adapted from FindGSasl.cmake

IF (LIBIDN2_INCLUDE_DIR AND LIBIDN2_LIBRARIES)
	SET(LibIDN2_FIND_QUIETLY TRUE)  # already in cache
ENDIF (LIBIDN2_INCLUDE_DIR AND LIBIDN2_LIBRARIES)

IF (NOT WIN32)
	# Use pkg-config to get the directories and then use these values
	# in the FIND_PATH() and FIND_LIBRARY() calls
	FIND_PACKAGE(PkgConfig)
	PKG_CHECK_MODULES(PC_LIBIDN2 QUIET libidn2)
ENDIF (NOT WIN32)

FIND_PATH(
	LIBIDN2_INCLUDE_DIR idn2.h
	HINTS
	${PC_LIBIDN2_INCLUDEDIR}
	${PC_LIBIDN2_INCLUDE_DIRS}
)

FIND_LIBRARY(
	LIBIDN2_LIBRARIES NAMES idn2 libidn2
	HINTS
	${PC_LIBIDN2_LIBDIR}
	${PC_LIBIDN2_LIBRARY_DIRS}
)

INCLUDE(FindPackageHandleStandardArgs)

# Handle the QUIETLY and REQUIRED arguments and set LIBIDN2_FOUND to TRUE if
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
	LibIDN2 DEFAULT_MSG LIBIDN2_LIBRARIES LIBIDN2_INCLUDE_DIR
)

MARK_AS_ADVANCED(LIBIDN2_INCLUDE_DIR LIBIDN2_LIBRARIES)
