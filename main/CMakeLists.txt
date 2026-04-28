idf_component_register(SRCS "zigbee_light.c"
                    INCLUDE_DIRS "."
                    REQUIRES led_strip driver)

execute_process(
    COMMAND git -C ${CMAKE_SOURCE_DIR} rev-parse --short HEAD
    OUTPUT_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

execute_process(
    COMMAND git -C ${CMAKE_SOURCE_DIR} diff --quiet --exit-code
    RESULT_VARIABLE GIT_DIRTY
    ERROR_QUIET
)

if("${GIT_HASH}" STREQUAL "")
    set(FIRMWARE_VERSION "unknown")
elseif(GIT_DIRTY EQUAL 0)
    set(FIRMWARE_VERSION "${GIT_HASH}")
else()
    set(FIRMWARE_VERSION "${GIT_HASH}-dirty")
endif()

target_compile_definitions(${COMPONENT_LIB} PRIVATE FIRMWARE_VERSION=\"${FIRMWARE_VERSION}\")
message(STATUS "Firmware version: ${FIRMWARE_VERSION}")
