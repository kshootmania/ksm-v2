cmake_minimum_required(VERSION 3.16)

# macOS: x86_64 build for Siv3D 0.6.x compatibility
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version" FORCE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE INTERNAL "" FORCE)
endif()

project(kshootmania VERSION 2.0.0)
set(PROJECT_VERSION_SUFFIX "-alpha7")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# macOSのみカスタムFindSiv3D.cmakeを使用
if(APPLE)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/kshootmania/ThirdParty/NocoUI/cmake/macos")
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++2a")
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
    
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.19.2")
        set(CMAKE_APPLE_SILICON_PROCESSOR "x86_64")
    endif()
    
    message(STATUS "Building for x86_64 architecture (Rosetta 2 will be used on Apple Silicon)")
endif()

if(NOT DEFINED Siv3D_ROOT)
    set(Siv3D_ROOT "$ENV{HOME}/Library/Siv3D" CACHE PATH "Path to Siv3D installation")
endif()

add_subdirectory(kson)
add_subdirectory(ksmaudio)
set(KSMAXIS_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
add_subdirectory(ksmaxis)
add_subdirectory(kshootmania/ThirdParty/NocoUI)

if(APPLE)
	add_subdirectory(ksmplatform_macos)
endif()

add_subdirectory(kshootmania)
add_subdirectory(tests)

if(WIN32)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT kshootmania)
elseif(APPLE AND CMAKE_GENERATOR MATCHES "Xcode")
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY XCODE_STARTUP_PROJECT kshootmania)
endif()
