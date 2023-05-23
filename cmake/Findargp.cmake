#[=======================================================================[.rst:
Findargp
-------

Finds the argp library.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``Foo_FOUND``
  True if the system has the Foo library.
``Foo_INCLUDE_DIRS``
  Include directories needed to use Foo.
``Foo_LIBRARIES``
  Libraries needed to link to Foo.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``Foo_INCLUDE_DIR``
  The directory containing ``foo.h``.
``Foo_LIBRARY``
  The path to the Foo library.

#]=======================================================================]

include(FindPackageHandleStandardArgs)
include(CheckCXXSymbolExists)

set(CMAKE_REQUIRED_QUITE TRUE)

# Check if header file exists
find_path(argp_INCLUDE_DIR 
    NAMES argp.h
    HINTS /usr/include /usr/local/include)

set(CMAKE_REQUIRED_INCLUDES ${argp_INCLUDE_DIR})

if(argp_INCLUDE_DIR)
    # Check for function argp_parse in default libraries (libc)
    check_cxx_symbol_exists(argp_parse "argp.h" ARGP_LIBC)
    if(ARGP_LIBC)
        set(argp_LIBRARY c CACHE STRING
            "argp library file")
    else()
        find_library(argp_LIBRARY "argp"
            HINTS /usr/local/lib
            DOC "argp library file")
        if (argp_LIBRARY)
            set(CMAKE_REQUIRED_LIBRARIES ${argp_LIBRARY})

            # Check for function argp_parse
            check_cxx_symbol_exists(argp_parse "argp.h" ARGP_EXTERNAL)
            if(NOT ARGP_EXTERNAL)
                message(FATAL_ERROR 
                    "The argp library was found in ${argp_LIBRARY},"
                    "but it doesn't contain a symbol named argp_parse.")
            endif()
        endif()
    endif()
endif()

find_package_handle_standard_args(argp 
    DEFAULT_MSG argp_LIBRARY argp_INCLUDE_DIR)

if(argp_FOUND)
    set(argp_LIBRARIES ${argp_LIBRARY})
    set(argp_INCLUDE_DIRS ${argp_INCLUDE_DIR})
endif()

mark_as_advanced(
    argp_LIBRARY 
    argp_INCLUDE_DIR
)