cmake_minimum_required(VERSION 3.16)

project(ksmplatform_linux)

add_library(ksmplatform_linux STATIC
	src/keyboard.cpp
)

target_include_directories(ksmplatform_linux
	PUBLIC
		include
)

find_package(X11 REQUIRED)

target_link_libraries(ksmplatform_linux
	PUBLIC
		X11::X11
)
