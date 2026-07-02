cmake_minimum_required(VERSION 3.16)
project(kshootmania CXX)

if (NOT CMAKE_CONFIGURATION_TYPES AND
    NOT CMAKE_NO_BUILD_TYPE AND
    NOT CMAKE_BUILD_TYPE AND
    CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    message(STATUS "[!] Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release)
endif()

option(OUTPUT_LICENSE_TXT "Output license.txt on startup" OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/App)

# ゲーム本体
file(GLOB_RECURSE GAME_SRC_FILES src/Scenes/*.cpp)

# 共通
file(GLOB_RECURSE ALL_SRC_FILES src/*.cpp)
list(REMOVE_ITEM ALL_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/stdafx.cpp)
set(COMMON_SRC_FILES ${ALL_SRC_FILES})
list(REMOVE_ITEM COMMON_SRC_FILES
	${GAME_SRC_FILES}
	${CMAKE_CURRENT_SOURCE_DIR}/src/Main.cpp
)

add_executable(kshootmania
	${COMMON_SRC_FILES}
	${GAME_SRC_FILES}
	src/Main.cpp
)

if(APPLE)
    set_target_properties(kshootmania PROPERTIES
        OUTPUT_NAME "K-Shoot MANIA"
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "K-Shoot MANIA"
        MACOSX_BUNDLE_BUNDLE_VERSION "${CMAKE_PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${CMAKE_PROJECT_VERSION}${PROJECT_VERSION_SUFFIX}"
        MACOSX_BUNDLE_ICON_FILE icon.icns
    )

    set(APP_BUNDLE_PATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/K-Shoot MANIA.app")

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/App/engine")
        # engineディレクトリ全体をコピー
        add_custom_command(TARGET kshootmania POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/App/engine
            ${APP_BUNDLE_PATH}/Contents/Resources/engine
        )

        # engine/font/min以外のフォントフォルダを削除
        add_custom_command(TARGET kshootmania POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E rm -rf
            ${APP_BUNDLE_PATH}/Contents/Resources/engine/font/fontawesome
            ${APP_BUNDLE_PATH}/Contents/Resources/engine/font/materialdesignicons
            ${APP_BUNDLE_PATH}/Contents/Resources/engine/font/mplus
            ${APP_BUNDLE_PATH}/Contents/Resources/engine/font/noto-cjk
            ${APP_BUNDLE_PATH}/Contents/Resources/engine/font/noto-emoji
        )

        # engine/soundfontフォルダを削除
        add_custom_command(TARGET kshootmania POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E rm -rf
            ${APP_BUNDLE_PATH}/Contents/Resources/engine/soundfont
        )
    endif()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/App/assets")
        add_custom_command(TARGET kshootmania POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/App/assets
            ${APP_BUNDLE_PATH}/Contents/Resources/assets
        )
    endif()

    # imgs, se, ui, shaders, langフォルダをコピー
    # (POST_BUILDではソース変更がないビルドでコピーされないため、毎回実行されるターゲットとして登録)
    set(RESOURCE_COPY_COMMANDS)
    foreach(RESOURCE_FOLDER imgs se ui shaders lang)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/App/${RESOURCE_FOLDER}")
            list(APPEND RESOURCE_COPY_COMMANDS
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/App/${RESOURCE_FOLDER}
                ${APP_BUNDLE_PATH}/Contents/Resources/${RESOURCE_FOLDER}
            )
        endif()
    endforeach()
    add_custom_target(kshootmania_copy_resources ALL
        ${RESOURCE_COPY_COMMANDS}
        COMMENT "Copying resource folders to app bundle"
    )
    add_dependencies(kshootmania_copy_resources kshootmania)

    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/App/icon.icns)
        set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/App/icon.icns PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources"
        )
        target_sources(kshootmania PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/App/icon.icns)
    endif()
    
    add_custom_command(TARGET kshootmania POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
        ${APP_BUNDLE_PATH}/Contents/Frameworks
    )
    
    add_custom_command(TARGET kshootmania POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/ksmaudio/ThirdParty_macOS/bass/libbass.dylib
        ${APP_BUNDLE_PATH}/Contents/Frameworks/libbass.dylib
    )
    
    add_custom_command(TARGET kshootmania POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/ksmaudio/ThirdParty_macOS/bass_fx/libbass_fx.dylib
        ${APP_BUNDLE_PATH}/Contents/Frameworks/libbass_fx.dylib
    )
    
    set_target_properties(kshootmania PROPERTIES
        BUILD_RPATH "@executable_path/../Frameworks"
        INSTALL_RPATH "@executable_path/../Frameworks"
    )
endif()

target_include_directories(kshootmania PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty
    ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/CoTaskLib/include
)

find_package(Siv3D REQUIRED)
target_link_libraries(kshootmania PRIVATE
    Siv3D::Siv3D
    kson
    ksmaudio
    ksmaxis
    NocoUI
    SQLiteCpp
)

if(APPLE)
	target_link_libraries(kshootmania PRIVATE ksmplatform_macos)
endif()

target_compile_features(kshootmania PRIVATE cxx_std_20)

target_compile_definitions(kshootmania PRIVATE)

if(OUTPUT_LICENSE_TXT)
	target_compile_definitions(kshootmania PRIVATE OUTPUT_LICENSE_TXT)
endif()

if(APPLE)
    # Siv3DのmacOS/Linux版はプリコンパイル済みヘッダ非対応のため通常のヘッダとしてインクルード
    target_compile_options(kshootmania PRIVATE
        -Wall
        -Wextra
        -Wno-unknown-pragmas
        -include ${CMAKE_CURRENT_SOURCE_DIR}/src/stdafx.h
    )

    target_compile_definitions(kshootmania PRIVATE
        SIV3D_PLATFORM_MACOS
    )
elseif(EMSCRIPTEN)
    # Siv3DのWeb版はプリコンパイル済みヘッダ非対応のため通常のヘッダとしてインクルード
    target_compile_options(kshootmania PRIVATE
        -Wall
        -Wextra
        -Wno-unknown-pragmas
        -include ${CMAKE_CURRENT_SOURCE_DIR}/src/stdafx.h
    )

    # 出力はHTML形式(kshootmania.html, .js, .wasm, .data)
    set_target_properties(kshootmania PROPERTIES
        SUFFIX ".html"
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/AppWeb
    )

    set(KSM_WEB_SIV3D_RESOURCES "" CACHE PATH "Path to Siv3D Web app resources directory")
    if(NOT KSM_WEB_SIV3D_RESOURCES)
        message(FATAL_ERROR "KSM_WEB_SIV3D_RESOURCES must be set to the Siv3D Web App resources directory")
    endif()
    if(NOT EXISTS "${KSM_WEB_SIV3D_RESOURCES}/engine")
        message(FATAL_ERROR "KSM_WEB_SIV3D_RESOURCES must contain an engine directory: ${KSM_WEB_SIV3D_RESOURCES}")
    endif()

    target_link_options(kshootmania PRIVATE
        -fexceptions
        -sASYNCIFY=1
        -sTOTAL_STACK=32MB
        -sINITIAL_MEMORY=256MB
        -sFORCE_FILESYSTEM=1
        --preload-file=${KSM_WEB_SIV3D_RESOURCES}/engine@/resources/engine
        --preload-file=${CMAKE_CURRENT_SOURCE_DIR}/App/assets@/resources/assets
        --preload-file=${CMAKE_CURRENT_SOURCE_DIR}/App/imgs@/imgs
        --preload-file=${CMAKE_CURRENT_SOURCE_DIR}/App/se@/se
        --preload-file=${CMAKE_CURRENT_SOURCE_DIR}/App/ui@/ui
        --preload-file=${CMAKE_CURRENT_SOURCE_DIR}/App/shaders@/shaders
        --preload-file=${CMAKE_CURRENT_SOURCE_DIR}/App/lang@/lang
        --exclude-file=*/.DS_Store
    )

    # Asyncify対象の関数リスト(OpenSiv3D/Web/App/CMakeLists.txtの設定に準拠)
    set(KSM_ASYNCIFY_ADD
        main
        "Main()"
        dynCall_v
        dynCall_vi
        "s3d::TryMain()"
        "s3d::CSystem::init()"
        "s3d::System::Update()"
        "s3d::AACDecoder::decode(*) const"
        "s3d::MP3Decoder::decode(*) const"
        "s3d::CAudioDecoder::decode(*)"
        "s3d::AudioDecoder::Decode(*)"
        "s3d::Wave::Wave(*)"
        "s3d::Audio::Audio(*)"
        "s3d::GenericDecoder::decode(*) const"
        "s3d::CImageDecoder::decode(*)"
        "s3d::Image::Image(*)"
        "s3d::Texture::Texture(*)"
        "s3d::ImageDecoder::Decode(*)"
        "s3d::ImageDecoder::GetImageInfo(*)"
        "s3d::Model::Model(*)"
        "s3d::CModel::create(*)"
        "s3d::CRenderer2D_GLES3::init()"
        "s3d::Clipboard::GetText(*)"
        "s3d::CClipboard::getText(*)"
        "s3d::SimpleHTTP::Save(*)"
        "s3d::SimpleHTTP::Load(*)"
        "s3d::SimpleHTTP::Get(*)"
        "s3d::SimpleHTTP::Post(*)"
        "s3d::VideoReader::VideoReader(*)"
        "s3d::VideoReader::open(*)"
        "s3d::Platform::Web::FetchFile(*)"
    )
    set(KSM_ASYNCIFY_IMPORTS
        siv3dRequestAnimationFrame
        siv3dGetClipboardText
        siv3dDecodeImageFromFile
        siv3dSleepUntilWaked
        invoke_v
        invoke_vi
    )
    list(TRANSFORM KSM_ASYNCIFY_ADD REPLACE ".+" "'\\0'")
    string(REPLACE ";" "," KSM_ASYNCIFY_ADD "${KSM_ASYNCIFY_ADD}")
    target_link_options(kshootmania PRIVATE "SHELL:-sASYNCIFY_ADD=[${KSM_ASYNCIFY_ADD}]")
    list(TRANSFORM KSM_ASYNCIFY_IMPORTS REPLACE ".+" "'\\0'")
    string(REPLACE ";" "," KSM_ASYNCIFY_IMPORTS "${KSM_ASYNCIFY_IMPORTS}")
    target_link_options(kshootmania PRIVATE "SHELL:-sASYNCIFY_IMPORTS=[${KSM_ASYNCIFY_IMPORTS}]")
elseif(UNIX)
    # Siv3DのmacOS/Linux版はプリコンパイル済みヘッダ非対応のため通常のヘッダとしてインクルード
    target_compile_options(kshootmania PRIVATE
        -Wall
        -Wextra
        -Wno-unknown-pragmas
        -include ${CMAKE_CURRENT_SOURCE_DIR}/src/stdafx.h
    )

    # BASSライブラリへのRPATHを設定
    set_target_properties(kshootmania PROPERTIES
        BUILD_RPATH "${CMAKE_SOURCE_DIR}/ksmaudio/ThirdParty_Linux/bass:${CMAKE_SOURCE_DIR}/ksmaudio/ThirdParty_Linux/bass_fx"
        INSTALL_RPATH "$ORIGIN/../lib"
    )

    # resourcesフォルダがない場合は作成
    add_custom_command(TARGET kshootmania POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources
    )

    # engineディレクトリをコピー
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/App/engine")
        add_custom_command(TARGET kshootmania POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/App/engine
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources/engine
        )
    endif()

    # assetsディレクトリをコピー
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/App/assets")
        add_custom_command(TARGET kshootmania POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/App/assets
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources/assets
        )
    endif()
endif()
