cmake_minimum_required(VERSION 3.19)

# DEBUG CONFIG
set(CMAKE_VERBOSE_MAKEFILE on) 
# set_property(GLOBAL PROPERTY GLOBAL_DEPENDS_DEBUG_MODE 1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-g")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

include(FetchContent)
include(ExternalProject)
set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)

project(
    gv 
    LANGUAGES CXX
    VERSION 0.3.0)

# Git clone the ABC repo at build time
ExternalProject_Add(engine_abc 
    GIT_REPOSITORY https://github.com/berkeley-abc/abc.git
    GIT_TAG 6ca7eab
    GIT_PROGRESS TRUE
    PREFIX engine
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ""
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
    BUILD_COMMAND $(MAKE) libabc.a
    LOG_CONFIGURE ON
    LOG_INSTALL ON
    # LOG_BUILD ON
    LOG_OUTPUT_ON_FAILURE ON
)
ExternalProject_Get_property(engine_abc SOURCE_DIR)
# Include all the header files under the engine ABC
include_directories(${SOURCE_DIR}/src)

# Apply the patch to the Yosys repo
# set(yosys_patch git apply ${CMAKE_CURRENT_SOURCE_DIR}/patches/yosys.patch || git apply ${CMAKE_CURRENT_SOURCE_DIR}/patches/yosys.patch -R --check && echo already applied)
# Git clone the repo at build time 
# Log the exit value of git apply patch
# set(yosys_patch_check cd ${SOURCE_DIR} && git apply -q ${CMAKE_CURRENT_SOURCE_DIR}/patches/yosys.patch)
# set(yosys_patch "")
# execute_process(COMMAND ${yosys_patch_check} RESULT_VARIABLE ret)
# if(ret EQUAL "0")
#     message(FATAL_ERROR "PATCH has not been applied !!")
#     set(yosys_patch git apply ${CMAKE_CURRENT_SOURCE_DIR}/patches/yosys.patch)
# else()
#     message(FATAL_ERROR "PATCH has been applied !!")
#     set(yosys_patch "")
# endif()
set(yosys_patch git apply ${CMAKE_CURRENT_SOURCE_DIR}/patches/yosys.patch)
ExternalProject_Add(engine_yosys
    GIT_REPOSITORY https://github.com/YosysHQ/yosys.git
    GIT_TAG yosys-0.35
    GIT_PROGRESS TRUE
    PREFIX engine
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ""
    # CONFIGURE_COMMAND $(MAKE) config-gcc
    UPDATE_COMMAND ""
    INSTALL_COMMAND ""
    PATCH_COMMAND ${CMAKE_COMMAND} -DBUILD_STEP=patch -P ${CMAKE_SOURCE_DIR}/yosys_patch.cmake
    BUILD_COMMAND $(MAKE) libyosys.so -j 16
    UPDATE_DISCONNECTED TRUE
    LOG_CONFIGURE ON
    LOG_INSTALL ON
    # LOG_BUILD ON
)
ExternalProject_Get_property(engine_yosys SOURCE_DIR)
include_directories(${SOURCE_DIR})


# Include all header files under the src/**/
FILE(GLOB src_dir RELATIVE ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/*)
FOREACH(dir ${src_dir})
    IF(IS_DIRECTORY ${CMAKE_SOURCE_DIR}/${dir})
        include_directories(${CMAKE_SOURCE_DIR}/${dir})
    ENDIF()
ENDFOREACH()
# Link_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(/usr/include/readline)

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

# Link the libabc.a to GV
ExternalProject_Get_Property(engine_abc SOURCE_DIR)
target_link_libraries(${CMAKE_PROJECT_NAME} ${SOURCE_DIR}/libabc.a)
# message(${SOURCE_DIR}/libabc.a)
# Link the libyosys.so to GV
ExternalProject_Get_Property(engine_yosys SOURCE_DIR)
target_link_libraries(${CMAKE_PROJECT_NAME} ${SOURCE_DIR}/libyosys.so)
# message(${SOURCE_DIR}/libyosys.so)
# Link the system library to GV
target_link_libraries(${CMAKE_PROJECT_NAME} readline)
target_link_libraries(${CMAKE_PROJECT_NAME} gmp)

# Allow GV to be built only after all steps in the External project are completed.
add_dependencies(${CMAKE_PROJECT_NAME} engine_yosys)
add_dependencies(${CMAKE_PROJECT_NAME} engine_abc)

# Exclude from all
set_target_properties(engine_yosys PROPERTIES EXCLUDE_FROM_ALL TRUE)
set_target_properties(engine_abc PROPERTIES EXCLUDE_FROM_ALL TRUE)
