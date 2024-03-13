# Path to the yosys patch
set(YOSYS_PATCH_FILE yosys.patch)
# Command for checking the patch is applied or not
set(YOSYS_PATCH_CHECK git apply --check ${CMAKE_CURRENT_LIST_DIR}/${YOSYS_PATCH_FILE})
# Command for applying the patch
set(YOSYS_PATCH git apply ${CMAKE_CURRENT_LIST_DIR}/${YOSYS_PATCH_FILE})

# Get the exit status of "git apply --check"
execute_process(COMMAND ${YOSYS_PATCH_CHECK}
    RESULT_VARIABLE STATUS
    OUTPUT_VARIABLE OUTPUT)

# Patch has not been applied
if(STATUS EQUAL "0")
    # Apply the patch to the Yosys repo
    message(WARNING "Applying the yosys patch !!")
    execute_process(COMMAND ${YOSYS_PATCH})
# Patch has been applied
else()
    message(WARNING "Skip the patch step !!\n Patch has been applied !!")
endif()