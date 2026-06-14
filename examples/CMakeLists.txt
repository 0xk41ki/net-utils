# Each .cpp file in examples/ becomes its own binary named after the file stem.
file(GLOB EXAMPLE_SOURCES CONFIGURE_DEPENDS *.cpp)

foreach(src ${EXAMPLE_SOURCES})
    get_filename_component(name ${src} NAME_WE)
    add_executable(${name} ${src})
    target_link_libraries(${name}
        PRIVATE
            net_utils
            net_utils_warnings
            net_utils_options
    )
endforeach()
