cmake_minimum_required(VERSION 2.8.3 FATAL_ERROR)
project(Cxx11Features CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../Modules")

include(CheckCXX11Features)

foreach (flag IN ITEMS
        HAS_CXX11_AUTO
        HAS_CXX11_AUTO_RET_TYPE
        HAS_CXX11_CLASS_OVERRIDE
        HAS_CXX11_CONSTEXPR
        HAS_CXX11_CSTDINT_H
        HAS_CXX11_DECLTYPE
        HAS_CXX11_FUNC
        HAS_CXX11_INITIALIZER_LIST
        HAS_CXX11_LAMBDA
        HAS_CXX11_LIB_REGEX
        HAS_CXX11_LONG_LONG
        HAS_CXX11_NULLPTR
        HAS_CXX11_RVALUE_REFERENCES
        HAS_CXX11_SIZEOF_MEMBER
        HAS_CXX11_STATIC_ASSERT
        HAS_CXX11_VARIADIC_TEMPLATES
        )
    if (${flag})
        add_definitions("-D${flag}")
        message(STATUS "Compiler C++11 support flag ${flag} set")
    endif ()
endforeach (flag)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${CXX11_COMPILER_FLAGS})
add_executable(CXX11Features cxx11features.cxx)
