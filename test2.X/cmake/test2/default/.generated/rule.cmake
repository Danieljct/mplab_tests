# The following functions contains all the flags passed to the different build stages.

set(PACK_REPO_PATH "C:/Users/danie/.mchp_packs" CACHE PATH "Path to the root of a pack repository.")

function(test2_default_default_XC32_assemble_rule target)
    set(options
        "-g"
        "${ASSEMBLER_PRE}"
        "-mprocessor=ATSAMD51J20A"
        "-Wa,--defsym=__MPLAB_BUILD=1${MP_EXTRA_AS_POST},--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--gdwarf-2,-I${CMAKE_CURRENT_SOURCE_DIR}/../../.."
        "-mdfp=${PACK_REPO_PATH}/Microchip/SAMD51_DFP/3.8.246/samd51a")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target} PRIVATE "__DEBUG")
    target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../..")
endfunction()
function(test2_default_default_XC32_assembleWithPreprocess_rule target)
    set(options
        "-x"
        "assembler-with-cpp"
        "-g"
        "${MP_EXTRA_AS_PRE}"
        "${DEBUGGER_NAME_AS_MACRO}"
        "-mprocessor=ATSAMD51J20A"
        "-Wa,--defsym=__MPLAB_BUILD=1${MP_EXTRA_AS_POST},--defsym=__MPLAB_DEBUG=1,--gdwarf-2,--defsym=__DEBUG=1,-I${CMAKE_CURRENT_SOURCE_DIR}/../../..")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target}
        PRIVATE "__DEBUG"
        PRIVATE "XPRJ_default=default")
    target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../..")
endfunction()
function(test2_default_default_XC32_compile_rule target)
    set(options
        "-g"
        "${CC_PRE}"
        "-x"
        "c"
        "-c"
        "-mprocessor=ATSAMD51J20A"
        "-ffunction-sections"
        "-fdata-sections"
        "-O1"
        "-fno-common"
        "-Werror"
        "-Wall"
        "-mdfp=${PACK_REPO_PATH}/Microchip/SAMD51_DFP/3.8.246/samd51a")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target}
        PRIVATE "__DEBUG"
        PRIVATE "XPRJ_default=default")
    target_include_directories(${target}
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/config/default"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/packs/ATSAMD51J20A_DFP"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/packs/CMSIS"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/packs/CMSIS/CMSIS/Core/Include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../.."
        PRIVATE "${PACK_REPO_PATH}/ARM/CMSIS/6.2.0/CMSIS/Core/Include")
endfunction()
function(test2_default_default_XC32_compile_cpp_rule target)
    set(options
        "-g"
        "${CC_PRE}"
        "${DEBUGGER_NAME_AS_MACRO}"
        "-mprocessor=ATSAMD51J20A"
        "-frtti"
        "-fexceptions"
        "-fno-check-new"
        "-fenforce-eh-specs"
        "-ffunction-sections"
        "-O1"
        "-fno-common"
        "-mdfp=${PACK_REPO_PATH}/Microchip/SAMD51_DFP/3.8.246/samd51a")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target}
        PRIVATE "__DEBUG"
        PRIVATE "XPRJ_default=default")
    target_include_directories(${target}
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/config/default"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/packs/ATSAMD51J20A_DFP"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/packs/CMSIS"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/packs/CMSIS/CMSIS/Core/Include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../.."
        PRIVATE "${PACK_REPO_PATH}/ARM/CMSIS/6.2.0/CMSIS/Core/Include")
endfunction()
function(test2_default_link_rule target)
    set(options
        "-g"
        "${MP_EXTRA_LD_PRE}"
        "${DEBUGGER_OPTION_TO_LINKER}"
        "${DEBUGGER_NAME_AS_MACRO}"
        "-mprocessor=ATSAMD51J20A"
        "-mno-device-startup-code"
        "-Wl,--defsym=__MPLAB_BUILD=1${MP_EXTRA_LD_POST},--script=${test2_default_LINKER_SCRIPT},--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=_min_heap_size=512,--gc-sections,-L${CMAKE_CURRENT_SOURCE_DIR}/../../..,--memorysummary,memoryfile.xml"
        "-mdfp=${PACK_REPO_PATH}/Microchip/SAMD51_DFP/3.8.246/samd51a")
    list(REMOVE_ITEM options "")
    target_link_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target} PRIVATE "XPRJ_default=default")
endfunction()
