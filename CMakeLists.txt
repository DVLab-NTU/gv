cmake_minimum_required(VERSION 3.19)

# Debug config
# set(CMAKE_VERBOSE_MAKEFILE on) 
# set_property(GLOBAL PROPERTY GLOBAL_DEPENDS_DEBUG_MODE 1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

include(FetchContent)
include(ExternalProject)
set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
set(EP_UPDATES_DISCONNECTED TRUE)

project(
    gv 
    LANGUAGES CXX
    VERSION 0.3.0)

# Detect if current system is Darwin
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MACOSX TRUE)
endif()

# Git clone the ABC repo at build time
# Run the BUILD_COMMAND "make libabc.a" for creating the ABC static lib
ExternalProject_Add(engine_abc 
    GIT_REPOSITORY https://github.com/berkeley-abc/abc.git
    GIT_TAG 6ca7eab
    GIT_PROGRESS TRUE
    PREFIX engines
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ""
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
    BUILD_COMMAND $(MAKE) libabc.a
    LOG_CONFIGURE ON
    LOG_INSTALL ON
    # LOG_BUILD ON
)
ExternalProject_Get_property(engine_abc SOURCE_DIR)
set(ABC_DIR ${SOURCE_DIR})
set(LIBABC_PATH ${ABC_DIR}/libabc.a)
add_library(libabc STATIC IMPORTED)
set_target_properties(libabc PROPERTIES IMPORTED_LOCATION ${LIBABC_PATH})

# Git clone the Yosys repo at build time
# Run the PATCH_COMMAND for applying the modification for yosys (patches/yosys.patch) 
# Run the BUILD_COMMAND "make libyosys.so" for creating the Yosys shared lib
set(PATCH_DIR ${CMAKE_SOURCE_DIR}/patches)
set(YOSYS_PATCH_CMAKE yosys_patch.cmake)
ExternalProject_Add(engine_yosys
    GIT_REPOSITORY https://github.com/YosysHQ/yosys.git
    GIT_TAG yosys-0.35
    GIT_PROGRESS TRUE
    PREFIX engines
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ""
    UPDATE_COMMAND ""
    INSTALL_COMMAND ""
    PATCH_COMMAND ${CMAKE_COMMAND} -P ${PATCH_DIR}/${YOSYS_PATCH_CMAKE}
    BUILD_COMMAND $(MAKE) libyosys.so
    UPDATE_DISCONNECTED TRUE
    LOG_CONFIGURE ON
    LOG_INSTALL ON
    # LOG_BUILD ON
)
ExternalProject_Get_property(engine_yosys SOURCE_DIR)
set(YOSYS_DIR ${SOURCE_DIR})
set(LIBYOSYS_PATH ${YOSYS_DIR}/libyosys.so)
add_library(libyosys SHARED IMPORTED)
set_target_properties(libyosys PROPERTIES IMPORTED_LOCATION ${LIBYOSYS_PATH})

# Link the library installed by brew
if(${MACOSX})
    link_directories(/opt/homebrew/lib)
endif()
# Link the Yosys shared libray path
link_directories(${YOSYS_DIR})

file(
    GLOB_RECURSE SOURCES
    RELATIVE ${CMAKE_SOURCE_DIR}
    "src/**/*.cpp" "src/**/*.h")
add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})

add_compile_definitions(ABC_USE_STDINT_H)
add_compile_definitions(_YOSYS_)
add_compile_definitions(GV_SIMSO_PATH="src/ext/")
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE)
# target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall -Wextra -w)
# target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall -Wextra)
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -w)

# g++ is being too paranoid about missing field initializers
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    target_compile_options(${CMAKE_PROJECT_NAME}
        PRIVATE -Wno-missing-field-initializers)
endif()


# Include all header files under the src/**/
FILE(GLOB SRC_DIR RELATIVE ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/*)
foreach(DIR ${SRC_DIR})
    if(IS_DIRECTORY ${CMAKE_SOURCE_DIR}/${DIR})
        target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/${DIR})
    endif()
endforeach()
# Include all the header files under the engine Yosys
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${YOSYS_DIR})
# Include all the header files under the engine ABC
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${ABC_DIR}/src)
# Include the readline header files
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE /usr/include/readline)

target_link_libraries(${CMAKE_PROJECT_NAME} 
    libabc
    libyosys
    readline
    gmp
)

# Allow GV to be built only after all steps in the External project are completed
add_dependencies(${CMAKE_PROJECT_NAME} engine_yosys)
add_dependencies(${CMAKE_PROJECT_NAME} engine_abc)

# Exclude from "all"
set_target_properties(engine_yosys PROPERTIES EXCLUDE_FROM_ALL TRUE)
set_target_properties(engine_abc PROPERTIES EXCLUDE_FROM_ALL TRUE)
