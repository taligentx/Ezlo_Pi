file(GLOB_RECURSE src_files "src/*.c*")

idf_component_register(SRCS "${src_files}"
    INCLUDE_DIRS "include")