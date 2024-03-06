cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

include(FetchContent)
include(ExternalProject)
set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)

project(
    gv 
    LANGUAGES CXX
    VERSION 0.3.0)

# find_package(OpenMP REQUIRED)

# set(BLA_VENDER OpenBLAS)
# find_package(BLAS REQUIRED)
# find_package(LAPACK REQUIRED)

# --- Fetch the abc repo
# FetchContent_Declare(
#     abc 
#     GIT_REPOSITORY https://github.com/berkeley-abc/abc.git
#     GIT_TAG 6ca7eab
#     CONFIGURE_COMMAND ""
#     BUILD_COMMAND ""
#     INSTALL_COMMAND "")
# FetchContent_MakeAvailable(abc)

# --- Git clone the abc repo at build time
ExternalProject_Add(
    engine_abc 
    GIT_REPOSITORY https://github.com/berkeley-abc/abc.git
    GIT_TAG 6ca7eab
    PREFIX engine
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)
ExternalProject_Get_property(engine_abc SOURCE_DIR)
message(${SOURCE_DIR})
# --- include abc header files
include_directories(${SOURCE_DIR}/src)
message("Include header files under ${SOURCE_DIR}/src")
# --- include gv header files
include_directories(${CMAKE_SOURCE_DIR}/src/util)
include_directories(${CMAKE_SOURCE_DIR}/src/cir)
include_directories(${CMAKE_SOURCE_DIR}/src/abc)
include_directories(${CMAKE_SOURCE_DIR}/src/yosys)
include_directories(${CMAKE_SOURCE_DIR}/src/bdd)
include_directories(${CMAKE_SOURCE_DIR}/src/sat)
message("Include header files under ${CMAKE_SOURCE_DIR}/src")

# --- Git clone the repo at build time 
ExternalProject_Add(
    engine_yosys
    GIT_REPOSITORY https://github.com/YosysHQ/yosys.git
    GIT_TAG yosys-0.35
    PREFIX engine
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ""
    INSTALL_COMMAND ""
    # BUILD_COMMAND $(MAKE) -j 16
    BUILD_COMMAND ""
    LOG_DOWNLOAD 1
    LOG_UPDATE 1
    LOG_CONFIGURE 1
    LOG_BUILD 0
    LOG_TEST 1
    LOG_INSTALL 1
)
ExternalProject_Get_property(engine_yosys SOURCE_DIR)
message(${SOURCE_DIR})
add_library(libyosyss SHARED IMPORTED GLOBAL)
include_directories(${SOURCE_DIR})
# set_property(TARGET libyosyss 
#     PROPERTY IMPORTED_LOCATION 
#     $(SOURCE_DIR)
# )

# FetchContent_MakeAvailable(yosys)

# message(${CMAKE_BINARY_DIR}) # -> .../build
# message(${CMAKE_SOURCE_DIR}) # -> .../gv0
# message(${CMAKE_CURRENT_BINARY_DIR}) # -> .../gv0

file(
    GLOB_RECURSE SOURCES
    RELATIVE ${CMAKE_SOURCE_DIR}
    "src/**/*.cpp" "src/**/*.h")
add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})
# message(${SOURCES})

# target_compile_definitions(spdlog PUBLIC SPDLOG_FMT_EXTERNAL=1)
# target_compile_definitions(
#     spdlog
#     PUBLIC
#     SPDLOG_SHORT_LEVEL_NAMES={\"[trace]\ \ \ \ \",\"[debug]\ \ \ \ \",\"[info]\ \ \ \ \ \",\"[warn]\ \ \ \ \ \",\"[error]\ \ \ \ \",\"[critical]\ \",\"\"}
# )

# target_link_libraries(spdlog PUBLIC fmt::fmt)
add_compile_definitions(ABC_USE_STDINT_H)
# add_compile_definitions(_YOSYS_ YOSYS_ENABLE_PLUGINS YOSYS_ENABLE_GLOB YOSYS_ENABLE_ZLIB YOSYS_ENABLE_TCL YOSYS_ENABLE_ABC YOSYS_ENABLE_COVER)
add_compile_definitions(_YOSYS_)
add_compile_definitions(GV_SIMSO_PATH="src/ext/")
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE)
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall -Wextra)

# # g++ is being too paranoid about missing field initializers
# if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
#     target_compile_options(${CMAKE_PROJECT_NAME}
#         PRIVATE -Wno-missing-field-initializers)
# endif()

# target_include_directories(${CMAKE_PROJECT_NAME}
#     PRIVATE ${CMAKE_SOURCE_DIR}/src)



# --- Compile util
file (GLOB_RECURSE UTIL_SOURCES
      RELATIVE ${CMAKE_SOURCE_DIR}
      "src/util/*.cpp" "src/util/*.h")
add_library(util STATIC ${UTIL_SOURCES})
set_target_properties(util PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(${CMAKE_PROJECT_NAME} util)

# --- Compile cmd 
file (GLOB_RECURSE CMD_SOURCES
      RELATIVE ${CMAKE_SOURCE_DIR}
      "src/cmd/*.cpp" "src/cmd/*.h")
add_library(cmd STATIC ${UTIL_SOURCES})
set_target_properties(cmd PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(${CMAKE_PROJECT_NAME} cmd)

# --- Compile abc
file (GLOB_RECURSE ABC_SOURCES
      RELATIVE ${CMAKE_SOURCE_DIR}
      "src/abc/*.cpp" "src/abc/*.h")
add_library(abcc STATIC ${ABC_SOURCES})
target_include_directories(abcc PUBLIC ${CMAKE_SOURCE_DIR}/src/cmd)
set_target_properties(abcc PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(abcc util cmd)
target_link_libraries(${CMAKE_PROJECT_NAME} abcc)


# # specify SYSTEM to suppress warnings from vendor code
# target_include_directories(${CMAKE_PROJECT_NAME} SYSTEM
#     PRIVATE ${CMAKE_SOURCE_DIR}/vendor)

# # black magic to link libraries as if they were system libraries
# function(target_link_libraries_system target)
#     set(options PRIVATE PUBLIC INTERFACE)
#     cmake_parse_arguments(TLLS "${options}" "" "" ${ARGN})

#     foreach(op ${options})
#         if(TLLS_${op})
#             set(scope ${op})
#         endif()
#     endforeach(op)

#     set(libs ${TLLS_UNPARSED_ARGUMENTS})

#     foreach(lib ${libs})
#         get_target_property(lib_include_dirs ${lib} INTERFACE_INCLUDE_DIRECTORIES)

#         if(lib_include_dirs)
#             if(scope)
#                 target_include_directories(${target} SYSTEM ${scope}
#                     ${lib_include_dirs})
#             else()
#                 target_include_directories(${target} SYSTEM PRIVATE ${lib_include_dirs})
#             endif()
#         else()
#             message(
#                 "Warning: ${lib} doesn't set INTERFACE_INCLUDE_DIRECTORIES. No include_directories set."
#             )
#         endif()

#         if(scope)
#             target_link_libraries(${target} ${scope} ${lib})
#         else()
#             target_link_libraries(${target} ${lib})
#         endif()
#     endforeach()
# endfunction(target_link_libraries_system)

# target_link_libraries_system(
#     ${CMAKE_PROJECT_NAME}
#     PRIVATE
#     xtl
#     xtensor
#     xtensor-blas
#     libfort::fort
#     fmt::fmt
#     spdlog::spdlog
#     Microsoft.GSL::GSL)
# target_link_libraries(
#     ${CMAKE_PROJECT_NAME} PRIVATE lapack OpenMP::OpenMP_CXX ${BLAS_LIBRARIES}
#     ${LAPACK_LIBRARIES})

# default to ON
# option(COPY_EXECUTABLE " Copy executable to project root " ON)

# install(TARGETS ${CMAKE_PROJECT_NAME} DESTINATION bin)

# if(COPY_EXECUTABLE)
#     add_custom_command(
#         TARGET ${CMAKE_PROJECT_NAME}
#         POST_BUILD
#         COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
#         ${CMAKE_SOURCE_DIR}/${CMAKE_PROJECT_NAME}
#         COMMENT " Copied executable to ${CMAKE_SOURCE_DIR}/${CMAKE_PROJECT_NAME} ")
# endif()