############################################################
#  Created by Lloyd Hilaiel on May 1 2006
#  Copyright (c) Yahoo!, Inc 2006
#
#  A CMake include file (pulled in by BuildTemplates.cmake)
#  that implements and documents all macros available for use
############################################################

# private macros not intended for public consumption
INCLUDE("${YBT_PATH}/lib/PrivateMacros.cmake")
INCLUDE("${YBT_PATH}/lib/PrivateLibraryMacros.cmake")
INCLUDE("${YBT_PATH}/lib/PrivateTestMacros.cmake")
INCLUDE("${YBT_PATH}/lib/PrivateSampleMacros.cmake")
INCLUDE("${YBT_PATH}/lib/PrivateBinaryMacros.cmake")

############################################################
# YBT_BUILD(type name)
#
# The do-it-all workhorse.  Specify an object to be built.
#
# Input parameters:
#   type - one of LIBRARY, BINARY, TEST, TEST_STATIC, or SAMPLE
#   name - unique name for the object
#
# Input variables
#   ${name}_SOURCES - an optional explicit list of sources to use rather
#                     than dynamic discovery
#   ${name}_IGNORE_PATTERNS - For dynamic discovery, all files matching
#                             this list of patterns will be ignored
#                             (ignored when ${name}_SOURCES is defined)
#   ${name}_LINK_STATIC - libraries to link the object against statically
#   ${name}_LINK_SHARED - libraries to link the object against dynamically
#   ${name}_REQUIRES - Includes and link directories will be added to the
#                      target and build dependencies will be set up.
#                      On unix, no linking will be perform, on windows the
#                      shared version of the library produced will link
#                      to the shared version of the requirement, and
#                      likewise the static version will link static...
#   ${name}_MAJOR_VERSION - the major version of the library/executable
#   ${name}_MINOR_VERSION - the minor version of the library/executable
#   ${name}_MICRO_VERSION - the micro version of the library/executable
#
# Input variables specific to the type of object you're building
#   LIBRARY:
#     ${name}_PRIVATE_HEADERS - an optional explicit list of private headers
#     ${name}_PUBLIC_HEADERS - an optional explicit list of public headers
#
# Specifying sources:
# 
#   There are two different ways that a cmake file writer may
#   specify sources:
#     1. with an explicit list
#     2. via implicit discovery of sources starting at the cwd.  Implicit
#        discovery works thus:
#        a. all sources found starting at cwd
#        b. all sources matching optional IGNORE_PATTERNS are filtered
#        c. all platform specific sources (filename_PLATFORM.ext) not
#           pertinent on this platform are filtered.
#           (Warning, when using implicit source discovery, '_' in a
#            filename implies a platform specific source file.  The
#            string which delimits filename from platform name ('_')
#            can be overridden by setting YBT_PLATFORM_SPERATOR
#            _before_ including BuildTemplates.cmake.
#
#            If you're not careful, files will unexpectedly be excluded
#            from the build.  This is mostly an issue when migrating an
#            existing project to BuildTemplates.)
#
#           The "platform name" is CMAKE_SYSTEM_NAME.  On UNIX platforms,
#           sources named <filename>_UNIX.{cpp,h} will be included as
#           well.  You may add arbitrary strings to the list of platforms
#           by defining YBT_ALLOWED_PLATFORMS.
#           
#
#   Implicit source file specification is, at first glance, more complex. 
#   The common case (combine all the files into this directory and all
#   subdirectories into a binary of some form) is terse and simple.  
#   Give it a try! 
#
#   Explicit source lists are simple.  build up the variable with
#   desired values.  
#
#   NOTE: libraries have a notion of "public" and "private" headers
#         where public headers are shipping headers that form the libraries
#         public API.  other objects have no such notion
############################################################
MACRO(YBT_BUILD type name)
  # validate the type
  IF (NOT ${type} STREQUAL "LIBRARY")
    IF (NOT ${type} STREQUAL "BINARY")
      IF (NOT ${type} STREQUAL "SAMPLE")
        IF (NOT ${type} STREQUAL "TEST")
          IF (NOT ${type} STREQUAL "TEST_STATIC")
            IF (NOT ${type} STREQUAL "LIBRARY_STATIC")
              IF (NOT ${type} STREQUAL "LIBRARY_SHARED")
                MESSAGE(FATAL_ERROR
				        "!! Error: I don't know how to build a ${type}")
              ENDIF ()
            ENDIF ()
          ENDIF ()
        ENDIF ()
      ENDIF ()
    ENDIF ()
  ENDIF ()          

  # default version to 1.0
  IF (NOT DEFINED ${name}_MAJOR_VERSION)
    SET(${name}_MAJOR_VERSION 1)
  ENDIF ()

  IF (NOT DEFINED ${name}_MINOR_VERSION)
    SET(${name}_MINOR_VERSION 0)
  ENDIF ()

  IF (NOT DEFINED ${name}_MICRO_VERSION)
    SET(${name}_MICRO_VERSION 0)
  ENDIF ()

  IF ("${YBT_WIN32_LEAN}" STREQUAL "NO")
    # Do nothing!
  ELSE ()
    # Set one of the macros.
    IF ("${YBT_WIN32_LEAN}" STREQUAL "SOME")
      # LESS restrictive macro.
      ADD_DEFINITIONS(-DWIN32_LEAN_AND_MEAN)
    ELSE ()
      # Set one of the macros.
      IF ("${YBT_WIN32_LEAN}" STREQUAL "YES")
        # MORE restrictive macro.
        # For performance reasons, this is the default.
        ADD_DEFINITIONS(-DVC_EXTRALEAN)
      ELSE ()
        # MORE restrictive macro.
        # For performance reasons, this is the default.
        ADD_DEFINITIONS(-DVC_EXTRALEAN)
      ENDIF ()
    ENDIF ()
  ENDIF ()

  SET(origType ${type})
  SET (types ${type})
  IF (${type} STREQUAL "LIBRARY")
    SET(types LIBRARY_STATIC LIBRARY_SHARED)
  ENDIF()

  SET(type)
  FOREACH(objType ${types})
    SET(thisTarget ${name}) 

	# for static libraries we append _s
    IF (${objType} STREQUAL "LIBRARY_STATIC")
      # copy all input variables      
      SET(${name}_s_SOURCES ${${name}_SOURCES})
      SET(${name}_s_IGNORE_PATTERNS ${${name}_IGNORE_PATTERNS})
      SET(${name}_s_LINK_STATIC ${${name}_LINK_STATIC})
      SET(${name}_s_LINK_SHARED ${${name}_LINK_SHARED})
      SET(${name}_s_REQUIRES ${${name}_REQUIRES})
      SET(${name}_s_MAJOR_VERSION ${${name}_MAJOR_VERSION})
      SET(${name}_s_MINOR_VERSION ${${name}_MINOR_VERSION})
      SET(${name}_s_MICRO_VERSION ${${name}_MICRO_VERSION})
      SET(${name}_s_PRIVATE_HEADERS ${${name}_PRIVATE_HEADERS})
      SET(${name}_s_PUBLIC_HEADERS ${${name}_PUBLIC_HEADERS})
      SET(${name}_s_MICRO_VERSION ${${name}_MICRO_VERSION})

      SET(thisTarget ${thisTarget}_s)
    ENDIF ()  

    IF(DEFINED YBT_VERBOSE)
      MESSAGE("** Building ${thisTarget} which is a ${objType}")
    ENDIF ()

    # we determine the sources that will compose the project in different
    # ways for libraries than we do for samples, tests, or binaries
    # samples and binaries need not distinguish between headers and
    # implementation files.
    IF (${objType} MATCHES "LIBRARY_.*")
      _YBT_DETERMINE_LIBRARY_SOURCE_FILES(${thisTarget})
      # a nicety for win32.  pretty source groups for libraries
      _YBT_DEFINE_SOURCE_GROUPS(${${thisTarget}_SOURCES} ${${thisTarget}_HEADERS})
    ELSE ()
      _YBT_DETERMINE_SOURCE_FILES(${thisTarget})
    ENDIF ()
  
  #  IF (WIN32 AND NOT DEFINED ${thisTarget}_NO_VERSION_RESOURCES)
  #    _YBT_HANDLE_WIN32_VERSION_RESOURCES(${thisTarget})
  #  ENDIF ()
  
    IF(DEFINED YBT_DEBUG)
      MESSAGE("DD   Sources for ${thisTarget}: ${${thisTarget}_SOURCES}")
      IF (${objType} MATCHES LIBRARY_.*)
        MESSAGE("DD   Private headers for ${thisTarget}: ${${thisTarget}_PRIVATE_HEADERS}")
        MESSAGE("DD   Public headers for ${thisTarget}: ${${thisTarget}_PUBLIC_HEADERS}")
      ELSE ()
        MESSAGE("DD   Headers for ${thisTarget}: ${${thisTarget}_HEADERS}")
      ENDIF ()
    ENDIF()
  
    # special case for TEST, add YCPTest to the set of libraries to link
    IF (${objType} STREQUAL "TEST")
      SET(${thisTarget}_LINK_SHARED YCPTest ${${thisTarget}_LINK_SHARED}) 
    ENDIF ()
  
    IF (${objType} STREQUAL "TEST_STATIC")
      SET(${thisTarget}_LINK_STATIC YCPTest ${${thisTarget}_LINK_STATIC}) 
    ENDIF ()
  
    # handle local project include directories
    _YBT_SET_INCLUDE_DIRECTORIES(${${thisTarget}_HEADERS})
  
    # handle include directories for dependencies
    FOREACH (dep ${${thisTarget}_LINK_SHARED} ${${thisTarget}_REQUIRES}) 
      IF (DEFINED ${dep}_INCLUDE_DIR)
        INCLUDE_DIRECTORIES (${${dep}_INCLUDE_DIR})
        IF(DEFINED YBT_DEBUG)      
          MESSAGE("DD   ${thisTarget} include for ${dep} from var: ${${dep}_INCLUDE_DIR}")
        ENDIF ()      
      ELSE ()
        IF(DEFINED YBT_DEBUG)      
          MESSAGE("DD   (${thisTarget}) ${dep} has no include path, not adding anything")
        ENDIF ()
      ENDIF ()
  
      IF (DEFINED ${dep}_LINK_DIR)
        LINK_DIRECTORIES(${${dep}_LINK_DIR})
        IF(DEFINED YBT_DEBUG)      
          MESSAGE("DD   ${thisTarget} link dir for ${dep} from var: ${${dep}_LINK_DIR}")
        ENDIF ()      
      ELSE ()
        IF(DEFINED YBT_DEBUG)      
          MESSAGE("DD   (${thisTarget}) ${dep} has no link path, not adding anything")
        ENDIF ()
      ENDIF ()
    ENDFOREACH () 
  
    # handle include directories for static library dependencies
    FOREACH (dep ${${thisTarget}_LINK_STATIC}) 
      SET(dep ${dep}_s)
      IF (DEFINED ${dep}_INCLUDE_DIR)
        INCLUDE_DIRECTORIES (${${dep}_INCLUDE_DIR})
        IF(DEFINED YBT_DEBUG)      
          MESSAGE("DD   ${thisTarget} include for ${dep} from var: ${${dep}_INCLUDE_DIR}")
        ENDIF ()      
      ELSE ()
        IF(DEFINED YBT_DEBUG)      
          MESSAGE("DD   (${thisTarget}) ${dep} has no include path, not adding anything")
        ENDIF ()
      ENDIF ()
  
      IF (DEFINED ${dep}_LINK_DIR)
        LINK_DIRECTORIES(${${dep}_LINK_DIR})
        IF(DEFINED YBT_DEBUG)      
          MESSAGE("DD   ${thisTarget} link dir for ${dep} from var: ${${dep}_LINK_DIR}")
        ENDIF ()      
      ELSE ()
        IF(DEFINED YBT_DEBUG)      
          MESSAGE("DD   (${thisTarget}) ${dep} has no link path, not adding anything")
        ENDIF ()
      ENDIF ()
    ENDFOREACH () 

    # we know type is library, binary, or sample, call the correct function
    IF (${objType} STREQUAL "LIBRARY_STATIC")
      _YBT_BUILD_STATIC_LIBRARY(${thisTarget})    
    ELSEIF (${objType} STREQUAL "LIBRARY_SHARED")
      _YBT_BUILD_SHARED_LIBRARY(${thisTarget})    
    ELSEIF (${objType} STREQUAL "BINARY")
      _YBT_BUILD_BINARY(${thisTarget} ${ARGN})    
    ELSEIF (${objType} STREQUAL "SAMPLE")        
      _YBT_BUILD_SAMPLE(${thisTarget} ${ARGN})    
    ELSE ()
      _YBT_BUILD_TEST(${thisTarget} ${ARGN})
    ENDIF ()

    # now make our object link against all dependencies
    _YBT_LINK_LIBRARIES(${thisTarget} SHARED ${${thisTarget}_LINK_SHARED})
    _YBT_LINK_LIBRARIES(${thisTarget} STATIC ${${thisTarget}_LINK_STATIC})
  
    # on windows a shared library uses an import lib to compile (same as
    # for shared.  On other platforms, compiling a shared library does not
    # required resolution of undefined symbols, which happens at link time.
    # This avoids embedding dependant library information in shared objects
    # and allows mixed mode linking (shared/static) on all platforms.
    # see the documentation and implementation of _YBT_REQUIRE_LIBRARIES
    # for more information.
    IF (DEFINED ${thisTarget}_REQUIRES)
      IF (${objType} STREQUAL "LIBRARY_STATIC")
        _YBT_REQUIRE_LIBRARIES_STATIC(${thisTarget} ${${thisTarget}_REQUIRES})
      ELSEIF (${objType} STREQUAL "LIBRARY_SHARED")
        _YBT_REQUIRE_LIBRARIES_SHARED(${thisTarget} ${${thisTarget}_REQUIRES})
      ELSE ()
        MESSAGE("!! attempt to use _REQUIRES on \"${name}\" which is a ${objType}")
        MESSAGE(FATAL_ERROR "!! _REQUIRES is only allowed for LIBRARY objects")
      ENDIF ()    
    ENDIF ()    
  ENDFOREACH ()
ENDMACRO ()

############################################################
# YBT_DISTRIBUTE_FILES(relativePath [PRESERVE] <files>)
#
# copy files into the resultant dist/ hierarchy
#
# Input parameters:
#   relativePath - The path underneath dist into which you wish to install
#   [PRESERVE] - an optional argument which means that directory hierarchy
#                should be preserved. 
#   <files> - files to be installed
#
############################################################
MACRO (YBT_DISTRIBUTE_FILES relativePath)
  _YBT_SYMLINK_FILES(${relativePath} ${ARGN})
ENDMACRO ()

############################################################
# YBT_DISTRIBUTE_FILE_AS(source destination)
#
# copy files into the resultant dist/ hierarchy
#
# Input parameters:
#   source - The source file that you wish to copy into the dist
#            directory.
#   destination - the full path under dist/ where you wish to copy the
#                 file
############################################################
MACRO (YBT_DISTRIBUTE_FILE_AS src dstPath)
  SET (dst ${CMAKE_INSTALL_PREFIX}/${dstPath})
  _YBT_SYMLINK_OR_COPY(${src} ${dst})
ENDMACRO ()

############################################################
# YBT_COMPILE_RESOURCEBUNDLE(bundleName <files>)
#
# Compile a resource bundle into a binary resource format
#
# Warning:  to compile resource bundles, the "genrb" program from
#           ICU must be in your path
#
# Input parameters:
#   bundleName - the name of the resource bundle to build
#   path - the relative path under dist/ to install the resource bundle to
#   <files> - files to be compiled into the bundle
############################################################
MACRO (YBT_COMPILE_RESOURCEBUNDLE name path)
  FIND_PROGRAM(GENRB_PROGRAM NAMES genrb genrb.exe
               DOC "Path to genrb executable")
  IF (NOT GENRB_PROGRAM)
    MESSAGE(FATAL_ERROR "!!: cannot find genrb for YBT_COMPILE_RESOURCEBUNDLE")
  ENDIF ()      

  # now figure out the output path
  SET (outputPath "${CMAKE_INSTALL_PREFIX}/${path}")
  IF (DEFINED YBT_VERBOSE)
    MESSAGE("**   Writing resource bundle ${name} to \"${outputPath}\"")
  ENDIF ()

  FILE(MAKE_DIRECTORY ${outputPath})

  _YBT_ABSOLUTIFY_PATHS(files ${ARGN})

  FOREACH (file ${files}) 
    GET_FILENAME_COMPONENT(sourcePath ${file} PATH)
    EXECUTE_PROCESS(COMMAND
                    ${GENRB_PROGRAM} -d ${outputPath} -p ${name} ${file}
                    WORKING_DIRECTORY ${sourcePath})
  ENDFOREACH () 
ENDMACRO ()

############################################################
# YBT_GENERATE_INCLUDEFILE(filename)
#
############################################################
MACRO (YBT_GENERATE_INCLUDE_FILE filename)
  # first find all of the cachefiles written by the various built
  # targets
  GET_FILENAME_COMPONENT(fname ${filename} NAME)
  GET_FILENAME_COMPONENT(tag ${filename} NAME_WE)

  # remove the include file
  FILE(WRITE ${filename}
"#########################################
#  Include file, generated by BuildTemplates.  To use the libraries 
#  here add the following line to your cmake file:
#  INCLUDE("some/path/${fname}")
#
#  After this you must include BuildTemplates, and should be able to
#  use _LINK_XXX directives to link libraries from this distribution.
#########################################

# reduce redundancy in the cmake language
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS 1)

")

  # determine the include file location
  FILE(APPEND ${filename} "
# determine the include file location
GET_FILENAME_COMPONENT(${tag}_PATH \"\${CMAKE_CURRENT_LIST_FILE}\" PATH)
MESSAGE(\"** Path to ${fname}: \${${tag}_PATH}\")

")

  # get the path to the include file we're generating for relative
  # filename calculatioons
  GET_FILENAME_COMPONENT(pathToInclude ${filename} PATH)

  FOREACH(lib ${YBT_ALL_LIBRARIES})
    FILE(RELATIVE_PATH relincpath ${pathToInclude} ${${lib}_INCLUDE_DIR})
    FILE(APPEND ${filename} "SET(${lib}_INCLUDE_DIR \"\${${tag}_PATH}/${relincpath}\")
")
    FILE(RELATIVE_PATH rellibpath ${pathToInclude} ${${lib}_LINK_DIR})
    FILE(APPEND ${filename} "SET(${lib}_LINK_DIR \"\${${tag}_PATH}/${rellibpath}\")
")
    # eliminate the full path of each lib dependency
    SET(alldeps )
    FOREACH (dep ${${lib}_LIB_DEPENDS})
      # to support mixed mode linking, we should never have libraries
      # we build in the depends list.  We require that clients explicitly
      # specify the libraries they want to include, to give them the freedom
      # to specify which to link shared, which static
      FOREACH(ourlib ${YBT_ALL_LIBRARIES})   
        IF ("${dep}" STREQUAL "${ourlib}")
          SET(dep "")  
        ENDIF ()
      ENDFOREACH ()
      IF (NOT "${dep}" STREQUAL "") 
        GET_FILENAME_COMPONENT(shortname ${dep} NAME)
        LIST(APPEND alldeps ${shortname})
      ENDIF ()
    ENDFOREACH ()
    FILE(RELATIVE_PATH rellibpath ${pathToInclude} ${${lib}_LINK_DIR})
    FILE(APPEND ${filename} "SET(${lib}_LIB_DEPENDS \"${alldeps};\")
")

    #separate with a newline
    FILE(APPEND ${filename} "
")

  ENDFOREACH ()
  FILE(APPEND ${filename} "
")

ENDMACRO ()
