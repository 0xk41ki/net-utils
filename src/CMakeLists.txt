file(GLOB_RECURSE NET_UTILS_SOURCES CONFIGURE_DEPENDS *.cpp)

add_library(net_utils ${NET_UTILS_SOURCES})

target_include_directories(net_utils
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(net_utils
    PUBLIC
        OpenSSL::SSL
        OpenSSL::Crypto
    PRIVATE
        net_utils_warnings
        net_utils_options
)

target_compile_features(net_utils PUBLIC cxx_std_23)
