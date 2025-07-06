# cmake files support debug production
include("${CMAKE_CURRENT_LIST_DIR}/rule.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/file.cmake")

set(test2_default_library_list )

# Handle files with suffix s, for group default-XC32
if(test2_default_default_XC32_FILE_TYPE_assemble)
add_library(test2_default_default_XC32_assemble OBJECT ${test2_default_default_XC32_FILE_TYPE_assemble})
    test2_default_default_XC32_assemble_rule(test2_default_default_XC32_assemble)
    list(APPEND test2_default_library_list "$<TARGET_OBJECTS:test2_default_default_XC32_assemble>")
endif()

# Handle files with suffix S, for group default-XC32
if(test2_default_default_XC32_FILE_TYPE_assembleWithPreprocess)
add_library(test2_default_default_XC32_assembleWithPreprocess OBJECT ${test2_default_default_XC32_FILE_TYPE_assembleWithPreprocess})
    test2_default_default_XC32_assembleWithPreprocess_rule(test2_default_default_XC32_assembleWithPreprocess)
    list(APPEND test2_default_library_list "$<TARGET_OBJECTS:test2_default_default_XC32_assembleWithPreprocess>")
endif()

# Handle files with suffix [cC], for group default-XC32
if(test2_default_default_XC32_FILE_TYPE_compile)
add_library(test2_default_default_XC32_compile OBJECT ${test2_default_default_XC32_FILE_TYPE_compile})
    test2_default_default_XC32_compile_rule(test2_default_default_XC32_compile)
    list(APPEND test2_default_library_list "$<TARGET_OBJECTS:test2_default_default_XC32_compile>")
endif()

# Handle files with suffix cpp, for group default-XC32
if(test2_default_default_XC32_FILE_TYPE_compile_cpp)
add_library(test2_default_default_XC32_compile_cpp OBJECT ${test2_default_default_XC32_FILE_TYPE_compile_cpp})
    test2_default_default_XC32_compile_cpp_rule(test2_default_default_XC32_compile_cpp)
    list(APPEND test2_default_library_list "$<TARGET_OBJECTS:test2_default_default_XC32_compile_cpp>")
endif()


add_executable(${test2_default_image_name} ${test2_default_library_list})
set_target_properties(${test2_default_image_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${test2_default_output_dir})

target_link_libraries(${test2_default_image_name} PRIVATE ${test2_default_default_XC32_FILE_TYPE_link})

# Add the link options from the rule file.
test2_default_link_rule(${test2_default_image_name})

# Add bin2hex target for converting built file to a .hex file.
string(REGEX REPLACE [.]elf$ .hex test2_default_image_name_hex ${test2_default_image_name})
add_custom_target(test2_default_Bin2Hex ALL
    COMMAND ${MP_BIN2HEX} ${test2_default_output_dir}/${test2_default_image_name}
    BYPRODUCTS ${test2_default_output_dir}/${test2_default_image_name_hex}
    COMMENT Convert built file to .hex)
add_dependencies(test2_default_Bin2Hex ${test2_default_image_name})



