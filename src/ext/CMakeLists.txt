# project(YOSYS_EXTENSION)

# Collect all source files
file(GLOB MY_SOURCES "./*.cc")

# Make the plugin library with the .so suffix
set(CMAKE_SHARED_LIBRARY_SUFFIX ".so")

# Compile each .cc file into a .so file
foreach(source_file ${MY_SOURCES})
    # Extract the file name (without path and extension)
    get_filename_component(name ${source_file} NAME_WE)
    # Compile each .cc file into a .so file
    add_library(${name} SHARED ${source_file})
    set_property(TARGET ${name} PROPERTY POSITION_INDEPENDENT_CODE ON)
    target_include_directories(${name} PRIVATE ${YOSYS_DIR})
    if(APPLE)
        target_link_options(${name} PRIVATE "-dynamic" "-undefined" "dynamic_lookup")
    else()
        target_link_options(${name} PRIVATE "-rdynamic")
    endif()
    target_compile_definitions(${name} PRIVATE _YOSYS_)
    add_dependencies(${name} engine-yosys)
endforeach()
