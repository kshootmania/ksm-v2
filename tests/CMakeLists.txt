cmake_minimum_required(VERSION 3.16)
project(ksm-v2-tests CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v2.13.6
)
FetchContent_MakeAvailable(Catch2)

set(TARGET_NAME ksm-v2-test)

# 出力ディレクトリをkshootmania/Appに設定
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../kshootmania/App)

file(GLOB TEST_SOURCES "*.cpp")

add_executable(${TARGET_NAME}
    ${TEST_SOURCES}
)

if(APPLE)
    set_target_properties(${TARGET_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "ksm-v2-test"
        MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
    )

    set(APP_BUNDLE_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET_NAME}.app)

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../kshootmania/App/engine")
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/../kshootmania/App/engine
            ${APP_BUNDLE_PATH}/Contents/Resources/engine
        )
    endif()
endif()

find_package(Siv3D REQUIRED)

# インクルードパスやコンパイルオプションはkshootmania_objectsのものを使用
target_link_libraries(${TARGET_NAME} PRIVATE
    kshootmania_objects
    Catch2::Catch2
)

if(CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(${TARGET_NAME} PRIVATE -fpie)
    target_link_options(${TARGET_NAME} PRIVATE -lpthread -ldl -pie)
endif()

enable_testing()
add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})
