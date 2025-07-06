# The following variables contains the files used by the different stages of the build process.
set(test2_default_default_XC32_FILE_TYPE_assemble)
set_source_files_properties(${test2_default_default_XC32_FILE_TYPE_assemble} PROPERTIES LANGUAGE ASM)
set(test2_default_default_XC32_FILE_TYPE_assembleWithPreprocess)
set_source_files_properties(${test2_default_default_XC32_FILE_TYPE_assembleWithPreprocess} PROPERTIES LANGUAGE ASM)
set(test2_default_default_XC32_FILE_TYPE_compile
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/config/default/exceptions.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/config/default/initialization.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/config/default/interrupts.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/config/default/libc_syscalls.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/config/default/peripheral/clock/plib_clock.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/config/default/peripheral/cmcc/plib_cmcc.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/config/default/peripheral/evsys/plib_evsys.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/config/default/peripheral/nvic/plib_nvic.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/config/default/peripheral/nvmctrl/plib_nvmctrl.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/config/default/peripheral/port/plib_port.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/config/default/startup_xc32.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/config/default/stdio/xc32_monitor.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/main.c")
set_source_files_properties(${test2_default_default_XC32_FILE_TYPE_compile} PROPERTIES LANGUAGE C)
set(test2_default_default_XC32_FILE_TYPE_compile_cpp)
set_source_files_properties(${test2_default_default_XC32_FILE_TYPE_compile_cpp} PROPERTIES LANGUAGE CXX)
set(test2_default_default_XC32_FILE_TYPE_link)

# The linker script used for the build.
set(test2_default_LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/config/default/ATSAMD51J20A.ld")
set(test2_default_image_name "default.elf")


# The output directory of the final image.
set(test2_default_output_dir "${CMAKE_CURRENT_SOURCE_DIR}/../../../out/test2")
