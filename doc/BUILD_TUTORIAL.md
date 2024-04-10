# GV Building Process Tutorial

This document serves as a guide to understand the building process of GV, focusing on the CMakeLists.

## Introduction

This document outlines the steps involved in building GV using CMakeLists.

<p align="center">
<img src="image-1.png" alt="image" width="350"/>
</p>

## Get the Engines source

We utilize ExternalProject_Add to acquire third-party engines since Yosys doesn't currently support CMake.

### Example: ExternalProject_Add Engine ABC

Below is an example demonstrating how we utilize ExternalProject_Add to obtain the source files of the ABC engine:

-   **Git Integration**: We use Git to fetch the source files.
-   **Version Tag**: The `GIT_TAG` parameter ensures we stay on a specific version of ABC.
-   **Build Configuration**: Setting `BUILD_IN_SOURCE` to 1 allows us to compile ABC directly within its source tree.
-   **Command Configuration**: Customizing `CONFIGURE_COMMAND`, `INSTALL_COMMAND`, and `UPDATE_COMMAND` to ensure that only code compilation occurs, without configuring, installing, or updating.

```cmake
# * Git clone the ABC repo at build time
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
```

## Patching the Yosys Engine

When integrating Yosys into GV, it's necessary to apply patches to its Makefile. Below, we outline how to patch the Yosys engine during the build process.

### Using PATCH_COMMAND for Patching

To facilitate patching, we utilize CMake and its ExternalProject_Add module. The PATCH_COMMAND within ExternalProject_Add allows us to apply modifications to the Yosys Makefile before building it.

PATCH_COMMAND will run the patches/yosys_patch.cmake module.

```cmake
# Set the directory containing patch files
set(PATCH_DIR ${CMAKE_SOURCE_DIR}/patches)
# Specify the name of the CMake script for patching
set(YOSYS_PATCH_CMAKE yosys_patch.cmake)
# Define the name of the Yosys shared library
set(LIBYOSYS_NAME libyosys.so)

# Add the external project for Yosys, including patching step
ExternalProject_Add(
    engine-yosys
    // ...
    PATCH_COMMAND ${CMAKE_COMMAND} -P ${PATCH_DIR}/${YOSYS_PATCH_CMAKE}
)
```

### yosys_patch.cmake

The yosys_patch.cmake script is responsible for handling the patching process. It selects the appropriate patch file based on the operating system (Linux/MacOS) and applies it to the Yosys Makefile.

```cmake
# Path to the yosys patch
if(APPLE)
    message(STATUS "Using Mac Patch")
    set(YOSYS_PATCH_FILE yosys_mac.patch)
else()
    message(STATUS "Using Linux Patch")
    set(YOSYS_PATCH_FILE yosys_linux.patch)
endif()

set(OUTPUT_REDIRECT "/dev/null")

# Command for checking the patch is applied or not
set(YOSYS_PATCH_CHECK git apply --check ${CMAKE_CURRENT_LIST_DIR}/${YOSYS_PATCH_FILE})
# Command for applying the patch
set(YOSYS_PATCH git apply ${CMAKE_CURRENT_LIST_DIR}/${YOSYS_PATCH_FILE})

# * Get the exit status of "git apply --check"
# * Output the log message to the /dev/null
execute_process(
    COMMAND ${YOSYS_PATCH_CHECK}
    RESULT_VARIABLE STATUS
    OUTPUT_VARIABLE OUTPUT
    OUTPUT_FILE ${OUTPUT_REDIRECT}
    ERROR_FILE ${OUTPUT_REDIRECT})

# Patch has not been applied
if(STATUS EQUAL "0")
    # Apply the patch to the Yosys Makefile
    message(STATUS "Applying the yosys patch")
    execute_process(COMMAND ${YOSYS_PATCH})
else()
    # Patch has been applied
    message(STATUS "Skip the patch step\nPatch has been applied")
endif()
```

## Collecting GV Source Files

In this section, we outline the process of collecting all the cpp and header files from the `src/` folder of GV for later compilation.

### Gathering Source Files

We use `file(GLOB_RECURSE ...)` command to recursively search for all `.cpp` and `.h` files under the `src/` directory.

```cmake
# Collect all the cpp and header files under src/
file(
    GLOB_RECURSE SOURCES
    RELATIVE ${CMAKE_SOURCE_DIR}
    "src/**/*.cpp" "src/**/*.h")
add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})
```

### Include Header Files

To ensure proper inclusion of header files without using relative paths, we specify the header file paths explicitly. This approach simplifies header file inclusion throughout the project. Here's how we achieve this in CMake:

```cmake
# Collect all directories under src/
file(
    GLOB SRC_DIR
    RELATIVE ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/*)

# Include all header files under the src/ directory
foreach(DIR ${SRC_DIR})
    if(IS_DIRECTORY ${CMAKE_SOURCE_DIR}/${DIR})
        target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/${DIR})
    endif()
endforeach()

```

For example, including the header file becomes simpler, as shown below:

```cpp
// Instead of: #include "src/cir/cirMgr.h"
#include "cirMgr.h"
// ...
```

### Include Engines Header Files

Additionally, we need to specify the header file paths for the engine components:

-   ABC_DIR: build/engines/src/engine-abc/src/
-   YOSYS_DIR: build/engines/src/engine-yosys/

```cmake
# Include all the header files under the engine Yosys
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${YOSYS_DIR})
# Include all the header files under the engine ABC
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${ABC_DIR}/src)
```

## Compiling Yosys Extension

In this section, we describe the process of compiling Yosys extensions by adding the `src/ext/` directory as a subdirectory. This allows us to compile the `.cc` files under `src/ext/` into a `.so` shared library based on the CMakeLists file located in `src/ext/`.

```cmake
# Set the path to the Yosys extension CMake file
set(YOSYS_PLUGIN_PATH src/ext/)
# Add the Yosys extension directory as a subdirectory
add_subdirectory(${YOSYS_PLUGIN_PATH})
```

Below is the modified `CMakeLists.txt` file located at `src/ext/CMakeLists.txt`. In this file, we have incorporated changes to include platform-specific compile options based on the operating system. Additionally, we ensure that the Yosys engine dependency (`engine-yosys`) is built before building the extension.

```cmake
# Collect all source files
file(GLOB MY_SOURCES "./*.cc")

# Define the shared library suffix
set(CMAKE_SHARED_LIBRARY_SUFFIX ".so")

# Compile each .cc file into a .so file
foreach(source_file ${MY_SOURCES})
    # Extract the file name (without path and extension)
    get_filename_component(name ${source_file} NAME_WE)
    # Compile each .cc file into a .so file
    add_library(${name} SHARED ${source_file})
    # Ensure position independent code
    set_property(TARGET ${name} PROPERTY POSITION_INDEPENDENT_CODE ON)
    # Include Yosys engine directory
    target_include_directories(${name} PRIVATE ${YOSYS_DIR})

    # Platform-specific compile options
    if(APPLE)
        # macOS specific options
        target_link_options(${name} PRIVATE "-dynamic" "-undefined" "dynamic_lookup")
    else()
        # Linux specific options
        target_link_options(${name} PRIVATE "-rdynamic")
    endif()

    # Define _YOSYS_ symbol for compilation
    target_compile_definitions(${name} PRIVATE _YOSYS_)

    # Ensure Yosys engine dependency is built before building the extension
    add_dependencies(${name} engine-yosys)
endforeach()
```
