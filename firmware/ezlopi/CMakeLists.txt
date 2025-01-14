cmake_minimum_required(VERSION 3.5)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# message(STATUS "current dir: ${CMAKE_CURRENT_SOURCE_DIR}")
execute_process(COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/update_device_list.sh"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/xyz.txt"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
)

# ADD_CUSTOM_COMMAND( OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/output_ff.txt"
# COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/update_device_list.sh
# DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/update_device_list.sh)

# Scans directories in 'curdir' and returns the list in 'result'
MACRO(SUBDIRLIST result curdir)
    FILE(GLOB children ${curdir}/*) # puts the list of all the components(files and/or folders) of 'curdir' in 'children'
    SET(dirlist "") # clear the list 'dirlist'

    FOREACH(child ${children}) # runs for loop for each components in 'children' and puts in 'child' one by one
        IF(IS_DIRECTORY ${child}) # check if the 'child' is directory or file
            LIST(APPEND dirlist ${child}) # if the 'child' is directory, append it to 'dirlist'
            message(STATUS "linking '${child}'.")
        ENDIF()
    ENDFOREACH()

    SET(${result} ${dirlist}) # copy the components of 'dirlist' into 'result'(return variable)
ENDMACRO()

set(external_components_list services devices protocols interface hub) # list of external components

set(EXTRA_COMPONENT_DIRS "") # clearing variable 'EXTRA_COMPONENT_DIRS'
set(ALL_COMPONENTS "") # clearing 'ALL_COMPONENTS' variable

foreach(component ${external_components_list})
    message(STATUS "  \r\n")
    message(STATUS "linking '${component}:: ")
    set(SUBDIRS, "") # clearing 'SUBDIRS'
    SUBDIRLIST(SUBDIRS ${component}) # scans and gets the list of directories from 'services' in 'SUBDIRS'
    LIST(APPEND ALL_COMPONENTS ${SUBDIRS})
endforeach()

set(EXTRA_COMPONENT_DIRS ${ALL_COMPONENTS}) # add the list of directories 'SUBDIRS' to 'EXTRA_COMPONENT_DIRS'(ESP32 VARIABLES), links the external components
message(STATUS "EXTRA_COMPONENTS_DIRS: ${EXTRA_COMPONENTS_DIRS}\r\n")

# # ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ ##
project(ezlopi)
