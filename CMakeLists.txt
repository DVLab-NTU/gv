cmake_minimum_required(VERSION 3.19)

# * Macro to exclude the directory from source
function(EXCLUDE_FROM_LIST REMOVE_LIST TARGET_LIST)
    message(STATUS "Exclude the directory - ${${REMOVE_LIST}}")
    foreach(REMOVE_DIR ${${REMOVE_LIST}})
        foreach(TMP_PATH ${${TARGET_LIST}})
            string(FIND ${TMP_PATH} ${REMOVE_DIR} EXCLUDE_DIR_FOUND)
            if(NOT ${EXCLUDE_DIR_FOUND} EQUAL -1)
                message("Removing ${TMP_PATH}")
                # list(REMOVE_ITEM TARGET_LIST ${TMP_PATH})
                list(REMOVE_ITEM ${TARGET_LIST} ${TMP_PATH})
            endif()
        endforeach(TMP_PATH)
    endforeach(REMOVE_DIR)
    # message(STATUS ${${TARGET_LIST}})
    set(${TARGET_LIST}
        ${${TARGET_LIST}}
        PARENT_SCOPE)
endfunction()

# * Debug config
# * set(CMAKE_VERBOSE_MAKEFILE on)
# * set_property(GLOBAL PROPERTY GLOBAL_DEPENDS_DEBUG_MODE 1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")


include(ExternalProject)
include(FetchContent)

project(
    gv
    LANGUAGES CXX
    VERSION 0.3.0)

# * Git clokne the ABC repo at build time
# * Run the BUILD_COMMAND "make libabc.a" for creating the ABC static library
set(LIBABC_NAME libabc.a)
ExternalProject_Add(
    engine-abc
    GIT_REPOSITORY https://github.com/berkeley-abc/abc.git
    GIT_TAG 6ca7eab
    GIT_PROGRESS TRUE
    PREFIX engines
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ""
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
    UPDATE_DISCONNECTED TRUE
    BUILD_COMMAND $(MAKE) ${LIBABC_NAME}
    LOG_CONFIGURE ON
    LOG_INSTALL ON
    # LOG_BUILD ON
)
ExternalProject_Get_Property(engine-abc SOURCE_DIR)
set(ABC_DIR ${SOURCE_DIR})
set(LIBABC_PATH ${ABC_DIR}/libabc.a)
add_library(libabc STATIC IMPORTED)
set_target_properties(libabc PROPERTIES IMPORTED_LOCATION ${LIBABC_PATH})

# * Git clone the Yosys repo at build time
# * Run the PATCH_COMMAND for applying the modification for yosys (patches/yosys.patch)
# * Run the BUILD_COMMAND "make libyosys.so" for creating the Yosys shared lib
set(PATCH_DIR ${CMAKE_SOURCE_DIR}/patches)
set(YOSYS_PATCH_CMAKE yosys_patch.cmake)
set(LIBYOSYS_NAME libyosys.so)
ExternalProject_Add(
    engine-yosys
    GIT_REPOSITORY https://github.com/YosysHQ/yosys.git
    GIT_TAG yosys-0.35
    GIT_PROGRESS TRUE
    PREFIX engines
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ""
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
    UPDATE_DISCONNECTED TRUE
    PATCH_COMMAND ${CMAKE_COMMAND} -P ${PATCH_DIR}/${YOSYS_PATCH_CMAKE}
    BUILD_COMMAND $(MAKE) ${LIBYOSYS_NAME} && $(MAKE)
    LOG_CONFIGURE ON
    LOG_INSTALL ON
    # LOG_BUILD ON
)
ExternalProject_Get_Property(engine-yosys SOURCE_DIR)
set(YOSYS_DIR ${SOURCE_DIR})
set(LIBYOSYS_PATH ${YOSYS_DIR}/libyosys.so)
add_library(libyosys SHARED IMPORTED)
set_target_properties(libyosys PROPERTIES IMPORTED_LOCATION ${LIBYOSYS_PATH})

# * Git clone the Verilog-VCD-Parser repo at build time
set(LIB_VCD_PARSER_NAME libverilog-vcd-parser.a)
set(VCD_PARSER_PATCH_CMAKE vcd_parser_patch.cmake)
ExternalProject_Add(
    vcd-parser
    GIT_REPOSITORY https://github.com/ben-marshall/verilog-vcd-parser.git
    # GIT_TAG yosys-0.35
    GIT_PROGRESS TRUE
    PREFIX engines
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ""
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
    UPDATE_DISCONNECTED TRUE
    PATCH_COMMAND ${CMAKE_COMMAND} -P ${PATCH_DIR}/${VCD_PARSER_PATCH_CMAKE}
    BUILD_COMMAND $(MAKE) all
    LOG_CONFIGURE ON
    LOG_INSTALL ON
    # LOG_BUILD ON
)
ExternalProject_Get_Property(vcd-parser SOURCE_DIR)
set(VCD_PARSER_DIR ${SOURCE_DIR})
set(LIB_VCD_PARSER_PATH ${VCD_PARSER_DIR}/build/libverilog-vcd-parser.a)
add_library(libvcd-parser STATIC IMPORTED)
set_target_properties(libvcd-parser PROPERTIES IMPORTED_LOCATION ${LIB_VCD_PARSER_PATH})

# * FMT Library
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 10.2.1)
FetchContent_MakeAvailable(fmt)

# * Glucose
FetchContent_Declare(
    glucose 
    GIT_REPOSITORY https://github.com/audemard/glucose.git
    GIT_TAG 4.2.1)
FetchContent_MakeAvailable(glucose)

# Link the library installed by brew
if(APPLE)
    link_directories(/opt/homebrew/lib)
    link_directories(/usr/local/opt/gmp/lib)
endif()
# Link the Yosys shared libray path
link_directories(${YOSYS_DIR})

# Set the install rpath to the YOSYS_DIR
set(CMAKE_MACOSX_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "${YOSYS_DIR}")

# Collect all the cpp and header files under src/
file(
    GLOB_RECURSE SOURCES
    RELATIVE ${CMAKE_SOURCE_DIR}
    "src/*/*.cpp" "src/*/*.h" "${CMAKE_SOURCE_DIR}/satsolvers/minisat/*")
# * Exlcude the src/verilator/** from SOURCES
# * list(APPEND EXCLUDE_DIR "verilator")
# * exclude_from_list(EXCLUDE_DIR SOURCES)
add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})

# Add the definitions for ABC
add_compile_definitions(ABC_USE_STDINT_H)
# Add the definitions for Yosys
add_compile_definitions(_YOSYS_)
# Add the definitions for Yosys Extension add_compile_definitions(GV_SIMSO_PATH="src/ext/")
add_compile_definitions(GV_PATH="${CMAKE_SOURCE_DIR}/")
add_compile_definitions(GV_SIMSO_PATH="${CMAKE_SOURCE_DIR}/build/src/ext/")
add_compile_definitions(GV_TEMPLATE_PATH="${CMAKE_SOURCE_DIR}/simulators/verilator/template/")
add_compile_definitions(GV_VERILATOR_PATH="${CMAKE_SOURCE_DIR}/simulators/verilator/")
add_compile_definitions(GV_VERILATOR_BUILD_PATH="${CMAKE_SOURCE_DIR}/simulators/verilator/build/")
add_compile_definitions(GV_CXXRTL_PATH="${CMAKE_SOURCE_DIR}/simulators/cxxrtl/")
add_compile_definitions(GV_YOSYS_BIN_PATH="${YOSYS_DIR}/yosys")
# target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE)
# target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall -Wextra)
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -w)

# g++ is being too paranoid about missing field initializers
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wno-missing-field-initializers)
endif()

file(
    GLOB SRC_DIR
    RELATIVE ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/* ${CMAKE_SOURCE_DIR}/src/*/*)

# Include all header files under the src/*/ and src/*/*/
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
# Include the gmp header files
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE /usr/local/include)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE /opt/homebrew/include)
# Include the VCD parser header files
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${VCD_PARSER_DIR}/build)
# Include the Glucose header files
# message(FATAL_ERROR ${glucose_SOURCE_DIR}/core)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${glucose_SOURCE_DIR})
# Include the satsolver directory
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/satsolvers)

# target_link_libraries(${CMAKE_PROJECT_NAME} libabc libyosys readline gmp)
find_package(Threads REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} libabc readline gmp dl Threads::Threads)
target_link_libraries(${CMAKE_PROJECT_NAME} ${LIBYOSYS_PATH})
# * Link the filesystem library
# target_link_libraries(${CMAKE_PROJECT_NAME} stdc++fs)
# * Link the verilog-vcd-parser library
target_link_libraries(${CMAKE_PROJECT_NAME} libvcd-parser)
# * Link the fmt library
target_link_libraries(${CMAKE_PROJECT_NAME} fmt::fmt)
# * Link the glucose library
target_link_libraries(${CMAKE_PROJECT_NAME} glucose)

# Add the Yosys extension CMake file
# set(YOSYS_PLUGIN_PATH src/ext/)
# add_subdirectory(${YOSYS_PLUGIN_PATH})

# Allow GV to be built only after all steps in the External project are completed
add_dependencies(${CMAKE_PROJECT_NAME} engine-yosys)
add_dependencies(${CMAKE_PROJECT_NAME} engine-abc)
add_dependencies(${CMAKE_PROJECT_NAME} vcd-parser)

install(TARGETS ${CMAKE_PROJECT_NAME} DESTINATION)

# default to ON
option(SYMBLINK_EXECUTABLE " Symbolic link executable to project root" ON)
if(SYMBLINK_EXECUTABLE)
    add_custom_command(
        TARGET ${CMAKE_PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
                ${CMAKE_SOURCE_DIR}/${CMAKE_PROJECT_NAME}
        COMMENT " Symbolic link executable to ${CMAKE_SOURCE_DIR}/${CMAKE_PROJECT_NAME}")
endif()
