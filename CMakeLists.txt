#
# CMake configuration file for VMime
#
# Usage:
#
# . 'cmake -LH' to list build settings variable
#
# . 'cmake -G <generator>' to generate makefiles for a build system
#   eg. cmake -G "Unix Makefiles"
#
# For more information, please visit:
# http://www.cmake.org
#

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.6 FATAL_ERROR)

INCLUDE(cmake/Utils.cmake)

INCLUDE(CheckIncludeFiles)
INCLUDE(CheckIncludeFileCXX)
INCLUDE(CheckFunctionExists)
INCLUDE(CheckSymbolExists)
INCLUDE(CheckTypeSize)
INCLUDE(CheckLibraryExists)
INCLUDE(CheckCXXSourceCompiles)


# CMake configuration
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY build/bin)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY build/lib)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY build/lib)

SET(CMAKE_VERBOSE_MAKEFILE ON)


# Project
PROJECT(vmime)

# Package version number
SET(VMIME_VERSION_MAJOR  0)
SET(VMIME_VERSION_MINOR  9)
SET(VMIME_VERSION_MICRO  2)

# API version number (libtool)
#
# Increment this number only immediately before a public release.
# This is independent from package version number.
#
# See: http://www.gnu.org/software/libtool/manual.html#Libtool-versioning
#
# . Implementation changed (eg. bug/security fix):  REVISION++
# . Interfaces added/removed/changed:               CURRENT++, REVISION=0
# . Interfaces added (upward-compatible changes):   AGE++
# . Interfaces removed:                             AGE=0
SET(VMIME_API_VERSION_CURRENT   0)
SET(VMIME_API_VERSION_REVISION  0)
SET(VMIME_API_VERSION_AGE       0)


SET(VMIME_VERSION ${VMIME_VERSION_MAJOR}.${VMIME_VERSION_MINOR}.${VMIME_VERSION_MICRO})
SET(VMIME_API_VERSION ${VMIME_API_VERSION_CURRENT}.${VMIME_API_VERSION_REVISION}.${VMIME_API_VERSION_AGE})


##############################################################################
# VMime Library

# Set base name
SET(VMIME_LIBRARY_NAME vmime)

# Source files
FILE(
	GLOB_RECURSE
	VMIME_LIBRARY_SRC_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/src/vmime/*.cpp
)

FILE(
	GLOB_RECURSE
	VMIME_LIBRARY_INCLUDE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/src/vmime/*.hpp
)

LIST(APPEND VMIME_LIBRARY_GENERATED_INCLUDE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/vmime/config.hpp")

INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_SOURCE_DIR}          # for "contrib/"
	${CMAKE_CURRENT_SOURCE_DIR}/src      # for "vmime/
)

INCLUDE(GenerateExportHeader)
ADD_COMPILER_EXPORT_FLAGS()

# Shared library
OPTION(
	VMIME_BUILD_SHARED_LIBRARY
	"Build shared library"
	ON
)

IF(VMIME_BUILD_SHARED_LIBRARY)

	ADD_LIBRARY(
		${VMIME_LIBRARY_NAME}
		SHARED
		${VMIME_LIBRARY_SRC_FILES}
		${VMIME_LIBRARY_INCLUDE_FILES}
	)

	GENERATE_EXPORT_HEADER(
		${VMIME_LIBRARY_NAME}
		BASE_NAME VMIME
		STATIC_DEFINE VMIME_STATIC
		EXPORT_FILE_NAME export-shared.hpp
	)

	FILE(
		COPY
		${CMAKE_CURRENT_BINARY_DIR}/export-shared.hpp
		DESTINATION
		${PROJECT_SOURCE_DIR}/src/vmime
	)

	LIST(APPEND VMIME_LIBRARY_GENERATED_INCLUDE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/vmime/export-shared.hpp")

	SET_TARGET_PROPERTIES(
		${VMIME_LIBRARY_NAME}
		PROPERTIES
		VERSION    "${VMIME_API_VERSION}"
		SOVERSION  "${VMIME_API_VERSION_CURRENT}"
		COMPILE_FLAGS -DVMIME_SHARED
	)

ENDIF()

# Static library
#
# Note: cannot have two targets with the same name so the static version has
# '-static' appended and then the name of the output file is set separately.
OPTION(
	VMIME_BUILD_STATIC_LIBRARY
	"Build static library"
	ON
)

IF(VMIME_BUILD_STATIC_LIBRARY)

	ADD_LIBRARY(
		${VMIME_LIBRARY_NAME}-static
		STATIC
		${VMIME_LIBRARY_SRC_FILES}
		${VMIME_LIBRARY_INCLUDE_FILES}
	)

	GENERATE_EXPORT_HEADER(
		${VMIME_LIBRARY_NAME}-static
		BASE_NAME VMIME
		STATIC_DEFINE VMIME_STATIC
		EXPORT_FILE_NAME export-static.hpp
	)

	FILE(
		COPY
		${CMAKE_CURRENT_BINARY_DIR}/export-static.hpp
		DESTINATION
		${PROJECT_SOURCE_DIR}/src/vmime
	)

	LIST(APPEND VMIME_LIBRARY_GENERATED_INCLUDE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/vmime/export-static.hpp")

	SET_TARGET_PROPERTIES(
		${VMIME_LIBRARY_NAME}-static
		PROPERTIES
		OUTPUT_NAME  ${VMIME_LIBRARY_NAME}
		COMPILE_FLAGS -DVMIME_STATIC
	)

ENDIF()

# Ensure at least one type of library (either shared or static) is build
IF(NOT(VMIME_BUILD_SHARED_LIBRARY OR VMIME_BUILD_STATIC_LIBRARY))
	MESSAGE(FATAL_ERROR "You should select at least one library to build (either VMIME_BUILD_SHARED_LIBRARY or VMIME_BUILD_STATIC_LIBRARY must be set to YES.")
ENDIF()

# These next two lines are required but it is unclear exactly what they do.
# The CMake FAQ mentions they are necessary and it does not work otherwise.
IF(VMIME_BUILD_SHARED_LIBRARY)
	SET_TARGET_PROPERTIES(${VMIME_LIBRARY_NAME} PROPERTIES CLEAN_DIRECT_OUTPUT 1)
ENDIF()
IF(VMIME_BUILD_STATIC_LIBRARY)
	SET_TARGET_PROPERTIES(${VMIME_LIBRARY_NAME}-static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
ENDIF()

SET(CMAKE_INSTALL_LIBDIR lib CACHE PATH "Output directory for libraries")

# Automatically set lib suffix
IF(UNIX AND NOT APPLE AND NOT CMAKE_CROSSCOMPILING AND NOT EXISTS "/etc/debian_version")
	IF(CMAKE_SIZEOF_VOID_P EQUAL 8 AND NOT LIB_SUFFIX)
		# ...only if LIB_SUFFIX is not defined...
		IF(NOT DEFINED LIB_SUFFIX)
			# ...and if CMAKE_INSTALL_LIBDIR does not already contain the suffix
			IF(NOT "${CMAKE_INSTALL_LIBDIR}" MATCHES ".*64.*")
				SET(LIB_SUFFIX 64)
			ENDIF()
		ENDIF()
	ENDIF()
ENDIF()

# Determine library installation dir, in this order:
#   1) use VMIME_INSTALL_LIBDIR if set
#   2) use LIB_INSTALL_DIR if defined
#   3) use CMAKE_INSTALL_LIBDIR if set
#      if it is a relative path, prepend CMAKE_INSTALL_PREFIX to it
#   4) use CMAKE_INSTALL_PREFIX/lib
IF(NOT DEFINED VMIME_INSTALL_LIBDIR)
	IF(DEFINED LIB_INSTALL_DIR)
		SET(VMIME_INSTALL_LIBDIR ${LIB_INSTALL_DIR})
	# respect CMAKE_INSTALL_LIBDIR if set
	ELSEIF(DEFINED CMAKE_INSTALL_LIBDIR)
		SET(VMIME_INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR})
		IF(IS_ABSOLUTE ${VMIME_INSTALL_LIBDIR})
			SET(VMIME_INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR}${LIB_SUFFIX})
		ELSE()
			SET(VMIME_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}${LIB_SUFFIX})
		ENDIF()
	ELSE()
		SET(VMIME_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})
	ENDIF()
ENDIF()

# Installation of libraries
IF(VMIME_BUILD_SHARED_LIBRARY)
	INSTALL(
		TARGETS ${VMIME_LIBRARY_NAME}
		LIBRARY DESTINATION "${VMIME_INSTALL_LIBDIR}" COMPONENT sharedlibs
		ARCHIVE DESTINATION "${VMIME_INSTALL_LIBDIR}" COMPONENT sharedlibs
	)
ENDIF()

IF(VMIME_BUILD_STATIC_LIBRARY)
	INSTALL(
		TARGETS ${VMIME_LIBRARY_NAME}-static
		LIBRARY DESTINATION "${VMIME_INSTALL_LIBDIR}" COMPONENT staticlibs
		ARCHIVE DESTINATION "${VMIME_INSTALL_LIBDIR}" COMPONENT staticlibs
	)
ENDIF()

# Installation of header files
INSTALL_HEADERS_WITH_DIRECTORY(VMIME_LIBRARY_INCLUDE_FILES headers src/)
INSTALL_HEADERS_WITH_DIRECTORY(VMIME_LIBRARY_GENERATED_INCLUDE_FILES headers src/)

#INSTALL(
#	FILES ${VMIME_LIBRARY_INCLUDE_FILES}
#	DESTINATION include
#	COMPONENT headers
#)


##############################################################################
# Tests

OPTION(
	VMIME_BUILD_TESTS
	"Build unit tests (this will create a 'run-tests' binary)"
	OFF
)

IF(VMIME_BUILD_TESTS)

	ENABLE_TESTING()

	INCLUDE(cmake/FindCppUnit.cmake)
	INCLUDE_DIRECTORIES(${CPPUNIT_INCLUDE_DIR})

	FILE(
		GLOB_RECURSE
		VMIME_TESTS_SRC_FILES
		${CMAKE_SOURCE_DIR}/tests/*Test.cpp
	)

	ADD_LIBRARY(
		test-runner
		${CMAKE_SOURCE_DIR}/tests/testRunner.cpp ${CMAKE_SOURCE_DIR}/tests/testUtils.cpp
	)

	# Build one file for each test
	FOREACH(VMIME_TEST_SRC_FILE ${VMIME_TESTS_SRC_FILES})

		# "/path/to/vmime/tests/module/testFile.cpp" --> "module_testFile"
		GET_FILENAME_COMPONENT(VMIME_TEST_SRC_PATH "${VMIME_TEST_SRC_FILE}" PATH)
		STRING(REPLACE "${CMAKE_SOURCE_DIR}" "" VMIME_TEST_SRC_PATH "${VMIME_TEST_SRC_PATH}")
		GET_FILENAME_COMPONENT(VMIME_TEST_NAME "${VMIME_TEST_SRC_FILE}" NAME_WE)
		SET(VMIME_TEST_NAME "${VMIME_TEST_SRC_PATH}/${VMIME_TEST_NAME}")
		STRING(REPLACE "/" "_" VMIME_TEST_NAME "${VMIME_TEST_NAME}")
		STRING(REPLACE "_tests_" "" VMIME_TEST_NAME "${VMIME_TEST_NAME}")

		ADD_EXECUTABLE(
			${VMIME_TEST_NAME}
			${VMIME_TEST_SRC_FILE}
		)

		TARGET_LINK_LIBRARIES(
			${VMIME_TEST_NAME}
			${VMIME_LIBRARY_NAME}
			${CPPUNIT_LIBRARY} test-runner
		)

		ADD_DEPENDENCIES(
			${VMIME_TEST_NAME}
			${VMIME_LIBRARY_NAME}
		)

		ADD_TEST(
			${VMIME_TEST_NAME}
			${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${VMIME_TEST_NAME}
		)

	ENDFOREACH()

	# Build one file for all tests
	ADD_EXECUTABLE(
		"run-tests"
		${VMIME_TESTS_SRC_FILES}
	)

	TARGET_LINK_LIBRARIES(
		"run-tests"
		${VMIME_LIBRARY_NAME}
		${CPPUNIT_LIBRARY} test-runner
	)

	ADD_DEPENDENCIES(
		"run-tests"
		${VMIME_LIBRARY_NAME}
	)

ENDIF()


##############################################################################
# Examples

OPTION(
	VMIME_BUILD_SAMPLES
	"Build samples (in 'examples' directory)"
	ON
)


##############################################################################
# Packaging / Distribution

# Package information
SET(VMIME_PACKAGE_NAME         ${VMIME_LIBRARY_NAME})
SET(VMIME_PACKAGE_VERSION      ${VMIME_VERSION})
SET(VMIME_PACKAGE_CONTACT      "Vincent Richard <vincent@vmime.org>")
SET(VMIME_PACKAGE_DESCRIPTION  "VMime C++ Mail Library (http://www.vmime.org)")

SET(CPACK_PACKAGE_NAME                 "${VMIME_PACKAGE_NAME}")
SET(CPACK_PACKAGE_CONTACT              "${VMIME_PACKAGE_CONTACT}")
SET(CPACK_PACKAGE_DESCRIPTION          "${VMIME_PACKAGE_DESCRIPTION}")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY  "${CPACK_PACKAGE_DESCRIPTION}")
SET(CPACK_PACKAGE_VERSION              "${VMIME_PACKAGE_VERSION}")

# Package settings
SET(CPACK_RESOURCE_FILE_LICENSE        ${CMAKE_SOURCE_DIR}/COPYING)

IF(APPLE)
	# CPack/PackageManager won't allow file without recognized extension
	# to be used as license file.
	CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/COPYING" "${CMAKE_BINARY_DIR}/COPYING.txt" COPYONLY)
	SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_BINARY_DIR}/COPYING.txt")
ELSE()
	SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING")
ENDIF(APPLE)

SET(CPACK_SOURCE_GENERATOR             TGZ)
SET(CPACK_SOURCE_IGNORE_FILES          "\\\\.git;~$;build/")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME     ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION})

# Set components
SET(CPACK_COMPONENTS_ALL sharedlibs staticlibs headers)

SET(CPACK_COMPONENT_SHAREDLIBS_DISPLAY_NAME "Shared libraries")
SET(CPACK_COMPONENT_SHAREDLIBS_DESCRIPTION
	"Shared library for general use.")

SET(CPACK_COMPONENT_STATICLIBS_DISPLAY_NAME "Static libraries")
SET(CPACK_COMPONENT_STATICLIBS_DESCRIPTION
	"Static library, good if you want to embed VMime in your application.")

SET(CPACK_COMPONENT_HEADERS_DISPLAY_NAME "C++ Headers")
SET(CPACK_COMPONENT_HEADERS_DESCRIPTION
	"C/C++ header files.")

SET(CPACK_COMPONENT_SHAREDLIBS_GROUP "Development")
SET(CPACK_COMPONENT_STATICLIBS_GROUP "Development")
SET(CPACK_COMPONENT_HEADERS_GROUP    "Development")
SET(CPACK_COMPONENT_GROUP_DEVELOPMENT_DESCRIPTION "")

# Make a target "dist" to generate tarball
SET(ARCHIVE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION})

ADD_CUSTOM_TARGET(
	dist
	COMMAND git archive --prefix=${ARCHIVE_NAME}/ HEAD
	| bzip2 > ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.bz2
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# PkgConfig
SET(prefix      ${CMAKE_INSTALL_PREFIX})
SET(exec_prefix ${CMAKE_INSTALL_PREFIX}/bin)
SET(includedir  ${CMAKE_INSTALL_PREFIX}/include/vmime)
SET(libdir      ${VMIME_INSTALL_LIBDIR})

SET(VMIME_PKGCONFIG_LIBS     "")
SET(VMIME_PKGCONFIG_CFLAGS   "")
SET(VMIME_PKGCONFIG_REQUIRES "")


##############################################################################
# Build type

IF(NOT CMAKE_BUILD_TYPE)
	SET(
		CMAKE_BUILD_TYPE
		"Debug"
		CACHE STRING
		"Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
		FORCE
	)
ENDIF(NOT CMAKE_BUILD_TYPE)

# Set a default build type for single-configuration
# CMake generators if no build type is set.
IF(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE Debug)
ENDIF()

# Debug build
MESSAGE("-- Build type: ${CMAKE_BUILD_TYPE}")
IF(${CMAKE_BUILD_TYPE} STREQUAL Debug)
	ADD_DEFINITIONS(-DDEBUG)
ENDIF(${CMAKE_BUILD_TYPE} STREQUAL Debug)


IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
	SET(VMIME_DEBUG 1)
ELSE()
	SET(VMIME_DEBUG 0)
ENDIF()


##############################################################################
# Test endianness and basic type sizes

INCLUDE(TestBigEndian)
TEST_BIG_ENDIAN(BIGENDIAN)

IF(BIGENDIAN EQUAL 0)
	set(VMIME_BYTE_ORDER_BIG_ENDIAN     0)
	set(VMIME_BYTE_ORDER_LITTLE_ENDIAN  1)
ELSE(BIGENDIAN EQUAL 0)
	SET(VMIME_BYTE_ORDER_BIG_ENDIAN     1)
	SET(VMIME_BYTE_ORDER_LITTLE_ENDIAN  0)
ENDIF(BIGENDIAN EQUAL 0)


CHECK_INCLUDE_FILE_CXX(cstdint VMIME_HAVE_CSTDINT)

CHECK_TYPE_SIZE("char"      SIZEOF_CHAR)
CHECK_TYPE_SIZE("short"     SIZEOF_SHORT)
CHECK_TYPE_SIZE("int"       SIZEOF_INT)
CHECK_TYPE_SIZE("long"      SIZEOF_LONG)
CHECK_TYPE_SIZE("long long" SIZEOF_LONG_LONG)
CHECK_TYPE_SIZE("__int64"   SIZEOF___INT64)

SET(CMAKE_EXTRA_INCLUDE_FILES cstdint)
CHECK_TYPE_SIZE("int64_t"   SIZEOF_INT64_T)
SET(CMAKE_EXTRA_INCLUDE_FILES)


IF(SIZEOF_CHAR EQUAL 1)
	SET(VMIME_8BIT_TYPE "char")
ELSE()
	MESSAGE(FATAL_ERROR "Cannot determine 8-bit type")
ENDIF()

IF(SIZEOF_INT EQUAL 2)
	SET(VMIME_16BIT_TYPE "int")
ELSE()
	IF(SIZEOF_SHORT EQUAL 2)
		SET(VMIME_16BIT_TYPE "short")
	ELSE()
		MESSAGE(FATAL_ERROR "Cannot determine 16-bit type")
	ENDIF()
ENDIF()

IF(SIZEOF_INT EQUAL 4)
	SET(VMIME_32BIT_TYPE "int")
ELSE()
	IF(SIZEOF_LONG EQUAL 4)
		SET(VMIME_32BIT_TYPE "long")
	ELSE()
		IF(SIZEOF_LONG_LONG EQUAL 4)
			SET(VMIME_32BIT_TYPE "long long")
		ELSE()
			MESSAGE(FATAL_ERROR "Cannot determine 32-bit type")
		ENDIF()
	ENDIF()
ENDIF()

IF(SIZEOF_INT EQUAL 8)
	SET(VMIME_64BIT_TYPE "int")
ELSE()
	IF(SIZEOF_LONG EQUAL 8)
		SET(VMIME_64BIT_TYPE "long")
	ELSE()
		IF(SIZEOF_LONG_LONG EQUAL 8)
			SET(VMIME_64BIT_TYPE "long long")
		ELSE()
			IF(SIZEOF_INT64_T EQUAL 8)
				SET(VMIME_64BIT_TYPE "int64_t")
			ELSE()
				IF(SIZEOF___INT64 EQUAL 8)
					SET(VMIME_64BIT_TYPE "__int64")
				ELSE()
					MESSAGE(FATAL_ERROR "Cannot determine 64-bit type")
				ENDIF()
			ENDIF()
		ENDIF()
	ENDIF()
ENDIF()


INCLUDE(cmake/TargetArch.cmake)
TARGET_ARCHITECTURE(CMAKE_TARGET_ARCHITECTURES)

CHECK_TYPE_SIZE(size_t VMIME_HAVE_SIZE_T)


##############################################################################
# Sendmail path

FOREACH (SENDMAIL_PATH /usr/sbin/sendmail /usr/lib/sendmail /usr/bin/sendmail /bin/sendmail /var/qmail/bin/qmail-inject /bin/cgimail)
	IF(EXISTS ${SENDMAIL_PATH})
		MESSAGE(STATUS "Sendmail binary found at ${SENDMAIL_PATH}")
		SET(VMIME_DEFAULT_SENDMAIL_PATH ${SENDMAIL_PATH})
	ENDIF()
ENDFOREACH(SENDMAIL_PATH)

SET(
	VMIME_SENDMAIL_PATH
	${VMIME_DEFAULT_SENDMAIL_PATH}
	CACHE
	STRING
	"Specifies the path to sendmail binary"
)


##############################################################################
# Messaging features

# Module
OPTION(
	VMIME_HAVE_MESSAGING_FEATURES
	"Enable messaging features (connection to IMAP, POP3, SMTP...)"
	ON
)

# Protocols
OPTION(
	VMIME_HAVE_MESSAGING_PROTO_POP3
	"Enable POP3 protocol"
	ON
)

OPTION(
	VMIME_HAVE_MESSAGING_PROTO_SMTP
	"Enable SMTP protocol"
	ON
)

OPTION(
	VMIME_HAVE_MESSAGING_PROTO_IMAP
	"Enable IMAP protocol"
	ON
)

OPTION(
	VMIME_HAVE_MESSAGING_PROTO_MAILDIR
	"Enable Maildir protocol"
	ON
)

OPTION(
	VMIME_HAVE_MESSAGING_PROTO_SENDMAIL
	"Enable Sendmail protocol"
	ON
)


##############################################################################
# File-system features

OPTION(
	VMIME_HAVE_FILESYSTEM_FEATURES
	"Enable file-system features (required for file attachments and Maildir)"
	ON
)


##############################################################################
# SASL support

INCLUDE(cmake/FindGSasl.cmake)

OPTION(
	VMIME_HAVE_SASL_SUPPORT
	"Enable SASL support (requires GNU SASL library)"
	ON
)

IF(VMIME_HAVE_SASL_SUPPORT)

	INCLUDE_DIRECTORIES(
		${INCLUDE_DIRECTORIES}
		${GSASL_INCLUDE_DIR}
	)

	IF(VMIME_BUILD_SHARED_LIBRARY)
		TARGET_LINK_LIBRARIES(
			${VMIME_LIBRARY_NAME}
			${TARGET_LINK_LIBRARIES}
			${GSASL_LIBRARIES}
		)
	ENDIF()

	SET(VMIME_PKGCONFIG_REQUIRES "${VMIME_PKGCONFIG_REQUIRES} libgsasl")

ENDIF()


##############################################################################
# SSL/TLS support

INCLUDE(FindGnuTLS)
INCLUDE(FindOpenSSL)

SET(CMAKE_REQUIRED_LIBRARIES "${GNUTLS_LIBRARY}")
CHECK_FUNCTION_EXISTS(gnutls_priority_set_direct VMIME_HAVE_GNUTLS_PRIORITY_FUNCS)


OPTION(
	VMIME_HAVE_TLS_SUPPORT
	"SSL/TLS support (requires either GNU TLS or OpenSSL library)"
	ON
)

SET(
	VMIME_TLS_SUPPORT_LIB
	"gnutls"
	CACHE
	STRING
	"Library to use for SSL/TLS conversion"
)
SET_PROPERTY(
	CACHE
	VMIME_TLS_SUPPORT_LIB
	PROPERTY STRINGS gnutls openssl
)


IF(VMIME_HAVE_TLS_SUPPORT)

	IF(VMIME_TLS_SUPPORT_LIB STREQUAL "gnutls")

		INCLUDE_DIRECTORIES(
			${INCLUDE_DIRECTORIES}
			${GNUTLS_INCLUDE_DIR}
		)

		LINK_DIRECTORIES(
			${LINK_DIRECTORIES}
			${GNUTLS_LIBRARY_DIRS}
		)

		IF(VMIME_BUILD_SHARED_LIBRARY)
			TARGET_LINK_LIBRARIES(
				${VMIME_LIBRARY_NAME}
				${TARGET_LINK_LIBRARIES}
				${GNUTLS_LIBRARY}
			)
		ENDIF()

		SET(VMIME_PKGCONFIG_REQUIRES "${VMIME_PKGCONFIG_REQUIRES} gnutls")

		SET(VMIME_TLS_SUPPORT_LIB_IS_GNUTLS "ON")
		SET(VMIME_TLS_SUPPORT_LIB_IS_OPENSSL "OFF")

	ELSEIF(VMIME_TLS_SUPPORT_LIB STREQUAL "openssl")

		INCLUDE_DIRECTORIES(
			${INCLUDE_DIRECTORIES}
			${OPENSSL_INCLUDE_DIR}
		)

		IF(VMIME_BUILD_SHARED_LIBRARY)
			TARGET_LINK_LIBRARIES(
				${VMIME_LIBRARY_NAME}
				${TARGET_LINK_LIBRARIES}
				${OPENSSL_LIBRARIES}
			)
		ENDIF()

		SET(VMIME_PKGCONFIG_REQUIRES "${VMIME_PKGCONFIG_REQUIRES} openssl")

		SET(VMIME_TLS_SUPPORT_LIB_IS_GNUTLS "OFF")
		SET(VMIME_TLS_SUPPORT_LIB_IS_OPENSSL "ON")

	ELSE()

		MESSAGE(FATAL_ERROR "TLS support is enabled, but no TLS/SSL library was selected/found")

	ENDIF()

ENDIF(VMIME_HAVE_TLS_SUPPORT)


##############################################################################
# Charset conversion library

INCLUDE(cmake/FindIconv.cmake)
INCLUDE(cmake/FindICU.cmake)

FIND_PACKAGE(ICU QUIET)

IF(WIN32)
	SET(VMIME_CHARSETCONV_LIB_DETECTED "win")
ELSEIF(ICU_LIBRARIES)
	SET(VMIME_CHARSETCONV_LIB_DETECTED "icu")
ELSEIF(ICONV_FOUND)
	SET(VMIME_CHARSETCONV_LIB_DETECTED "iconv")
ENDIF()

SET(
	VMIME_CHARSETCONV_LIB
	"${VMIME_CHARSETCONV_LIB_DETECTED}"
	CACHE
	STRING
	"Library to use for charset conversion"
)
SET_PROPERTY(
	CACHE
	VMIME_CHARSETCONV_LIB
	PROPERTY STRINGS win iconv icu
)


IF(VMIME_CHARSETCONV_LIB STREQUAL "iconv")

	INCLUDE_DIRECTORIES(
		${INCLUDE_DIRECTORIES}
		${ICONV_INCLUDE_DIR}
	)

	IF(VMIME_BUILD_SHARED_LIBRARY)
		TARGET_LINK_LIBRARIES(
			${VMIME_LIBRARY_NAME}
			${TARGET_LINK_LIBRARIES}
			${ICONV_LIBRARIES}
		)
	ENDIF()

	SET(VMIME_PKGCONFIG_LIBS "${VMIME_PKGCONFIG_LIBS} ${ICONV_LIBRARIES}")
	SET(VMIME_PKGCONFIG_CFLAGS "${VMIME_PKGCONFIG_CFLAGS} -I${ICONV_INCLUDE_DIR}")

	SET(VMIME_CHARSETCONV_LIB_IS_ICONV "ON")
	SET(VMIME_CHARSETCONV_LIB_IS_ICU "OFF")
	SET(VMIME_CHARSETCONV_LIB_IS_WIN "OFF")

ELSEIF(VMIME_CHARSETCONV_LIB STREQUAL "icu")

	INCLUDE_DIRECTORIES(
		${INCLUDE_DIRECTORIES}
		${ICU_INCLUDE_DIRS}
	)

	IF(VMIME_BUILD_SHARED_LIBRARY)
		TARGET_LINK_LIBRARIES(
			${VMIME_LIBRARY_NAME}
			${TARGET_LINK_LIBRARIES}
			${ICU_LIBRARIES}
		)
	ENDIF()

	SET(VMIME_PKGCONFIG_LIBS "${VMIME_PKGCONFIG_LIBS} ${ICU_LIBRARIES}")
	SET(VMIME_PKGCONFIG_CFLAGS "${VMIME_PKGCONFIG_CFLAGS} -I${ICU_INCLUDE_DIRS}")

	SET(VMIME_CHARSETCONV_LIB_IS_ICONV "OFF")
	SET(VMIME_CHARSETCONV_LIB_IS_ICU "ON")
	SET(VMIME_CHARSETCONV_LIB_IS_WIN "OFF")

ELSEIF(VMIME_CHARSETCONV_LIB STREQUAL "win")

	SET(VMIME_CHARSETCONV_LIB_IS_ICONV "OFF")
	SET(VMIME_CHARSETCONV_LIB_IS_ICU "OFF")
	SET(VMIME_CHARSETCONV_LIB_IS_WIN "ON")

ELSE()

	MESSAGE(FATAL_ERROR "No charset conversion library was selected/found")

ENDIF()


##############################################################################
# Language features

# C++11
INCLUDE(cmake/cmake-cxx11/Modules/CheckCXX11Features.cmake)

# Smart pointers
#
# If a C++11-compliant compiler is available and supports std::shared_ptr<>,
# use the standard implementation. Else, use boost::shared_ptr<>.
# In any case, let the user override the choice with VMIME_SHARED_PTR_USE_CXX
# and VMIME_SHARED_PTR_USE_BOOST variables.

CHECK_CXX_SOURCE_COMPILES(
	"
	#include <memory>
	struct A { int foo; };
	int main() {
		std::shared_ptr <A> a = std::make_shared <A>();
		return 0;
	}
	"
	VMIME_HAS_CXX11_SHARED_PTR
)

IF(NOT VMIME_SHARED_PTR_USE_CXX AND NOT VMIME_SHARED_PTR_USE_BOOST)
	IF(CXX11_COMPILER_FLAGS AND VMIME_HAS_CXX11_SHARED_PTR)
		# If std::shared_ptr<> is available, use it by default
		SET(VMIME_SHARED_PTR_USE_CXX_DEFAULT     ON)
		SET(VMIME_SHARED_PTR_USE_BOOST_DEFAULT  OFF)
	ELSE()
		# Else, set default to boost::shared_ptr<>
		SET(VMIME_SHARED_PTR_USE_CXX_DEFAULT    OFF)
		SET(VMIME_SHARED_PTR_USE_BOOST_DEFAULT   ON)
	ENDIF()
ENDIF()

OPTION(
	VMIME_SHARED_PTR_USE_CXX
	"Use standard std::shared_ptr<> (requires a C++11 compiler)"
	${VMIME_SHARED_PTR_USE_CXX_DEFAULT}
)

OPTION(
	VMIME_SHARED_PTR_USE_BOOST
	"Use boost::shared_ptr<> (requires Boost)"
	${VMIME_SHARED_PTR_USE_BOOST_DEFAULT}
)

IF(VMIME_SHARED_PTR_USE_CXX AND VMIME_SHARED_PTR_USE_BOOST)
	MESSAGE(FATAL_ERROR "Options VMIME_SHARED_PTR_USE_CXX and VMIME_SHARED_PTR_USE_BOOST are mutually exclusive (select one or the other, but not both!)")
ENDIF()

IF(VMIME_SHARED_PTR_USE_CXX)

	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX11_COMPILER_FLAGS}")

	IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
	ENDIF()

	MESSAGE(STATUS "Checking support for shared_ptr<>: built-in (C++11)")

ELSEIF(VMIME_SHARED_PTR_USE_BOOST)

	# Depends on Boost library if C++11 is not supported
	FIND_PACKAGE(Boost)

	IF(Boost_FOUND)
		INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
	ELSE()
		MESSAGE(FATAL_ERROR "Boost library is required for shared_ptr<>, unless you compile using C++11")
	ENDIF()

	MESSAGE(STATUS "Checking support for shared_ptr<>: boost library")

ELSE()

	MESSAGE(FATAL_ERROR "No implementation for shared_ptr<> was selected/found")

ENDIF()


##############################################################################
# Platform

SET(VMIME_PLATFORM_IS_POSIX OFF)
SET(VMIME_PLATFORM_IS_WINDOWS OFF)

IF(WIN32)
	SET(VMIME_PLATFORM_IS_WINDOWS ON)
ELSE()
	SET(VMIME_PLATFORM_IS_POSIX ON)
ENDIF()


##############################################################################
# Platform-specific checks

CHECK_FUNCTION_EXISTS(gmtime_s VMIME_HAVE_GMTIME_S)
CHECK_FUNCTION_EXISTS(gmtime_r VMIME_HAVE_GMTIME_R)
CHECK_FUNCTION_EXISTS(localtime_s VMIME_HAVE_LOCALTIME_S)
CHECK_FUNCTION_EXISTS(localtime_r VMIME_HAVE_LOCALTIME_R)
CHECK_FUNCTION_EXISTS(strcpy_s VMIME_HAVE_STRCPY_S)


##############################################################################
# Windows-specific checks

IF(WIN32)

	# Winsock
	CHECK_LIBRARY_EXISTS("ws2_32" getch "${CMAKE_LIBRARY_PATH}" VMIME_HAVE_LIBWS2_32)

	IF(VMIME_HAVE_LIBWS2_32)
		IF(VMIME_BUILD_SHARED_LIBRARY)
			TARGET_LINK_LIBRARIES(
				${VMIME_LIBRARY_NAME}
				${TARGET_LINK_LIBRARIES}
				"ws2_32"
			)
		ELSE()
			SET(VMIME_PKGCONFIG_LIBS "${VMIME_PKGCONFIG_LIBS} -lws2_32")
		ENDIF()
	ENDIF()

	# MLang
	CHECK_INCLUDE_FILES(Mlang.h VMIME_HAVE_MLANG_H)
	CHECK_LIBRARY_EXISTS("mlang" ConvertINetString "${CMAKE_LIBRARY_PATH}" VMIME_HAVE_MLANG_LIB)

	IF(VMIME_HAVE_MLANG_H)
		IF(VMIME_HAVE_MLANG_LIB)
			SET(VMIME_HAVE_MLANG ON)

			IF(VMIME_BUILD_SHARED_LIBRARY)
				TARGET_LINK_LIBRARIES(
					${VMIME_LIBRARY_NAME}
					${TARGET_LINK_LIBRARIES}
					"mlang"
				)
			ENDIF()
		ENDIF()
	ENDIF()

ENDIF()


##############################################################################
# POSIX-specific checks

CHECK_FUNCTION_EXISTS(getaddrinfo VMIME_HAVE_GETADDRINFO)
CHECK_FUNCTION_EXISTS(getnameinfo VMIME_HAVE_GETNAMEINFO)

CHECK_FUNCTION_EXISTS(gettid VMIME_HAVE_GETTID)
CHECK_FUNCTION_EXISTS(syscall VMIME_HAVE_SYSCALL)
CHECK_SYMBOL_EXISTS(SYS_gettid sys/syscall.h VMIME_HAVE_SYSCALL_GETTID)

FIND_PACKAGE(Threads)

IF(VMIME_BUILD_SHARED_LIBRARY)
	TARGET_LINK_LIBRARIES(
		${VMIME_LIBRARY_NAME}
		${TARGET_LINK_LIBRARIES}
		${CMAKE_THREAD_LIBS_INIT}
	)
ENDIF()

FIND_LIBRARY(PTHREAD_LIB pthread)

IF(PTHREAD_LIB)
	SET(VMIME_HAVE_PTHREAD 1)

	IF(VMIME_BUILD_SHARED_LIBRARY)
		TARGET_LINK_LIBRARIES(
			${VMIME_LIBRARY_NAME}
			${TARGET_LINK_LIBRARIES}
			${PTHREAD_LIB}
		)
	ENDIF()
ELSE(PTHREAD_LIB)
	SET(VMIME_HAVE_PTHREAD 0)
ENDIF(PTHREAD_LIB)


##############################################################################
# Additional compiler flags

IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")

	SET(
		CMAKE_CXX_FLAGS
		"${CMAKE_CXX_FLAGS} -D_REENTRANT=1 -W -Wall -pedantic -Warray-bounds-pointer-arithmetic -Wold-style-cast -Wconversion -Wcast-align -Wno-sign-conversion"
		CACHE STRING
		"g++ Compiler Flags"
		FORCE
	)

	SET(CMAKE_CXX_FLAGS_RELEASE "-O2")
	SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
	SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")

	#SET(CMAKE_EXE_LINKER_FLAGS "-s")

ELSE()

	IF(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)

		SET(
			CMAKE_CXX_FLAGS
			"${CMAKE_CXX_FLAGS} -D_REENTRANT=1 -W -Wall -pedantic -Wpointer-arith -Wold-style-cast -Wconversion -Wcast-align -Wno-long-long"
			CACHE STRING
			"g++ Compiler Flags"
			FORCE
		)

		SET(CMAKE_CXX_FLAGS_RELEASE "-O2")
		SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
		SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")

		#SET(CMAKE_EXE_LINKER_FLAGS "-s")

	ENDIF()

ENDIF()


##############################################################################
# Documentation

OPTION(
	VMIME_BUILD_DOCUMENTATION
	"Build documentation"
	ON
)

IF(VMIME_BUILD_DOCUMENTATION)
	FIND_PACKAGE(Doxygen)

	IF(DOXYGEN_FOUND)
		CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/Doxyfile.in ${CMAKE_BINARY_DIR}/Doxyfile @ONLY)

		# Make a target so that documentation can be generated by running "make doc"
		ADD_CUSTOM_TARGET(
			doc
			${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile
			WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
			COMMENT "Generating API documentation with Doxygen" VERBATIM
		)
	ENDIF(DOXYGEN_FOUND)
ENDIF(VMIME_BUILD_DOCUMENTATION)

##############################################################################
# Sanity checks

# Maildir protocol is available only if file-system features are enabled
IF(VMIME_HAVE_MESSAGING_FEATURES AND VMIME_HAVE_MESSAGING_PROTO_MAILDIR AND NOT VMIME_HAVE_FILESYSTEM_FEATURES)
	MESSAGE(FATAL_ERROR "Maildir protocol requires file-system support (VMIME_HAVE_FILESYSTEM_FEATURES must be set to ON).")
ENDIF()

# Path to 'sendmail' must be specified if Sendmail protocol is enabled
IF(VMIME_HAVE_MESSAGING_PROTO_SENDMAIL)
	IF(NOT VMIME_SENDMAIL_PATH OR VMIME_SENDMAIL_PATH STREQUAL "")
		MESSAGE(FATAL_ERROR "Enabling Sendmail protocol requires that you specify path to 'sendmail' binary.")
	ENDIF()
ENDIF()


##############################################################################
# Build examples

IF(VMIME_BUILD_SAMPLES)
	ADD_SUBDIRECTORY(examples)
ENDIF()


# Set our configure file
CONFIGURE_FILE(cmake/config.hpp.cmake ${CMAKE_SOURCE_DIR}/src/vmime/config.hpp)

# PkgConfig post-configuration
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/vmime.pc.in ${CMAKE_BINARY_DIR}/vmime.pc @ONLY)
INSTALL(FILES ${CMAKE_BINARY_DIR}/vmime.pc DESTINATION "${VMIME_INSTALL_LIBDIR}/pkgconfig" COMPONENT headers)

INCLUDE(CPack)

