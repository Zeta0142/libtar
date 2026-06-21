cmake_minimum_required(VERSION 3.5)

project(libtar VERSION 1.2.20 LANGUAGES C)

set(COMPAT_SRC
    compat/basename.c
    compat/dirname.c
    # compat/fnmatch.c
    # compat/gethostbyname_r.c
    # compat/gethostname.c
    # compat/getservbyname_r.c
    # compat/glob.c
    # compat/inet_aton.c
    compat/snprintf.c
    # compat/strdup.c
    # compat/strlcat.c
    # compat/strlcpy.c
    compat/strmode.c
    # compat/strrstr.c
    # compat/strsep.c
)

set(LISTHASH_SRC
    listhash/libtar_hash.c
    listhash/libtar_list.c
)

set(LIBTAR_SRC
    lib/append.c
    lib/block.c
    lib/decode.c
    lib/encode.c
    lib/extract.c
    lib/handle.c
    lib/output.c
    lib/util.c
    lib/wrapper.c
)

set(TAR_SRC
    ${COMPAT_SRC}
    ${LISTHASH_SRC}
    ${LIBTAR_SRC}
)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG)
endif()

# static target

add_library(tar_static STATIC ${TAR_SRC})

set_target_properties(tar_static PROPERTIES 
    OUTPUT_NAME "tar"
    EXPORT_NAME "tar_static"
)

target_compile_definitions(tar_static PRIVATE PIC)

target_include_directories(tar_static PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/compat/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/listhash/>
    $<INSTALL_INTERFACE:include>
)

# shared target

add_library(tar_shared SHARED ${TAR_SRC})

set_target_properties(tar_shared PROPERTIES 
    OUTPUT_NAME "tar"
    EXPORT_NAME "tar_shared"
)

target_compile_definitions(tar_shared PRIVATE PIC)

target_include_directories(tar_shared PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/compat/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/listhash/>
    $<INSTALL_INTERFACE:include>
)

# exporting

include(CMakePackageConfigHelpers)

export(TARGETS tar_shared tar_static
    NAMESPACE libtar::
    FILE "${CMAKE_CURRENT_BINARY_DIR}/libtar_target.cmake"
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/libtarConfig.cmake
    INSTALL_DESTINATION .
    NO_SET_AND_CHECK_MACRO
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

# installing

if (CMAKE_INSTALL_PREFIX)
include(GNUInstallDirs)

install(TARGETS tar_shared tar_static
    EXPORT libtar_target
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES include/libtar.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT libtar_target
    FILE libtar_target.cmake
    NAMESPACE libtar::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libtar
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/libtarConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libtar
)

endif ()