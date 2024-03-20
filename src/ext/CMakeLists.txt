project(YOSYS_EXTENSION)

# Collect all source files
file(GLOB MY_SOURCES "./*.cc")

set(CMAKE_SHARED_LIBRARY_SUFFIX ".so")
# message(FATAL_ERROR ${CMAKE_SHARED_LIBRARY_SUFFIX})

# Compile each .cc file into a .so file
foreach(source_file ${MY_SOURCES})
    # Extract the file name (without path and extension)
    get_filename_component(name ${source_file} NAME_WE)
    # Compile each .cc file into a .so file
    add_library(${name} SHARED ${source_file})
    set_property(TARGET ${name} PROPERTY POSITION_INDEPENDENT_CODE ON)
    # set_target_properties(OUTPUT_NAME ${name} SUFFIX ".so")
    target_include_directories(${name} PRIVATE ${YOSYS_DIR})
    target_link_options(${name} PRIVATE "-dynamic" "-undefined" "dynamic_lookup")
    target_compile_definitions(${name} PRIVATE _YOSYS_)
    # message(FATAL_ERROR ${name})
endforeach()
