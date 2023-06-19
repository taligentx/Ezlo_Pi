# Find all source files recursively
file(GLOB_RECURSE src_files "*.c*")

# Register the component with IDF
idf_component_register(
    SRCS "data.c" "${src_files}"
    INCLUDE_DIRS "."
    REQUIRES
        ezlopi_factory_info
        trace
        json
        ezlopi_devices
)
