cmake_minimum_required(VERSION 3.15)
project(SerialKit VERSION 0.1.0 LANGUAGES CXX)

message(STATUS "[SerialKit] Configuring version ${PROJECT_VERSION}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SERIALKIT_BUILD_TESTS "Build tests" ON)
option(SERIALKIT_BUILD_COMPILER "Build compiler" ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "[SerialKit] Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "[SerialKit] C++ standard: C++${CMAKE_CXX_STANDARD}")

if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

find_package(Threads REQUIRED)

if(SERIALKIT_BUILD_COMPILER)
    message(STATUS "[SerialKit] Building compiler")
    add_subdirectory(compiler)
endif()

if(SERIALKIT_BUILD_TESTS)
    message(STATUS "[SerialKit] Building tests")
    enable_testing()
    add_subdirectory(tests)
endif()

if(SERIALKIT_BUILD_COMPILER)
    install(TARGETS serialkit_compiler
            RUNTIME DESTINATION bin
            COMPONENT Runtime)

    install(FILES 
            "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
            "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
            "${CMAKE_CURRENT_SOURCE_DIR}/CHANGELOG.md"
            DESTINATION .
            COMPONENT Documentation)

    install(DIRECTORY 
            "${CMAKE_CURRENT_SOURCE_DIR}/docs/"
            DESTINATION docs
            COMPONENT Documentation)

    install(DIRECTORY 
            "${CMAKE_CURRENT_SOURCE_DIR}/examples/"
            DESTINATION examples
            COMPONENT Examples)
endif()

set(CPACK_PACKAGE_NAME "SerialKit")
set(CPACK_PACKAGE_VENDOR "Kirill Khasanyanov")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "SerialKit - High-performance binary serialization compiler")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "SerialKit")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

set(CPACK_WIX_UPGRADE_GUID "5F7A6C8D-2E3B-4A1C-9F5D-8B2E4A1C6D3F")
set(CPACK_WIX_PRODUCT_GUID "*")
set(CPACK_WIX_PROPERTY_ARPURLINFOABOUT "https://github.com/khasanyanovk/serial-kit")
set(CPACK_WIX_UI_BANNER "${CMAKE_CURRENT_SOURCE_DIR}/installer/wix_banner.bmp")
set(CPACK_WIX_UI_DIALOG "${CMAKE_CURRENT_SOURCE_DIR}/installer/wix_dialog.bmp")
set(CPACK_WIX_PRODUCT_ICON "${CMAKE_CURRENT_SOURCE_DIR}/installer/serialkit.ico")
set(CPACK_WIX_PATCH_FILE "${CMAKE_CURRENT_SOURCE_DIR}/installer/path_patch.xml")

set(CPACK_NSIS_DISPLAY_NAME "SerialKit Compiler")
set(CPACK_NSIS_PACKAGE_NAME "SerialKit")
set(CPACK_NSIS_HELP_LINK "https://github.com/khasanyanovk/serial-kit")
set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/khasanyanovk/serial-kit")
set(CPACK_NSIS_CONTACT "khasanyanovk@gmail.com")
set(CPACK_NSIS_MODIFY_PATH ON)
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)

if(WIN32)
    set(CPACK_GENERATOR "WIX;NSIS")
else()
    set(CPACK_GENERATOR "TGZ")
endif()

include(CPack)
