# file( GLOB_RECURSE provisioning_src "*.c*" "actions/*.c*" )
file(GLOB_RECURSE src_files "*.c")

idf_component_register(SRCS "${src_files}"
    INCLUDE_DIRS "."
    REQUIRES driver freertos devices_common interface_common debug dht11 web_provisioning items)