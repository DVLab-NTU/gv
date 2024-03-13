set(yosys_patch_check git apply --check -q /home/hchchiu/dvlab/gv0/patches/yosys.patch)
set(yosys_patch git apply /home/hchchiu/dvlab/gv0/patches/yosys.patch)

execute_process(COMMAND ${yosys_patch_check}
    RESULT_VARIABLE retval
    OUTPUT_VARIABLE path)
if(retval EQUAL "0")
    execute_process(COMMAND ${yosys_patch})
    message(WARNING "PATCH has not been applied !!")
    # set(yosys_patch git apply ${CMAKE_CURRENT_SOURCE_DIR}/patches/yosys.patch)
else()
    message(WARNING "PATCH has been applied !!")
    # set(yosys_patch "")
endif()

# if("${BUILD_STEP}" STREQUAL "patch")
#     message("BUILD_STEP: patch")
# endif()