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
    # Patch has been applied
else()
    message(STATUS "Skip the patch step\nPatch has been applied")
endif()
