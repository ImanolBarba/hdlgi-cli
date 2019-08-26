cmake_minimum_required(VERSION 3.10)
project(hdlgi_cli)
set(CMAKE_CXX_STANDARD 11)

add_executable(hdlgi-cli
        main.cpp HDLGIConnection.cpp HDLGIInstance.cpp ISO9660Image.cpp OSD.cpp ProgressBar.cpp aux_functions.cpp
        HDLGI_commands.h HDLGIConnection.h HDLGIInstance.h ISO9660Image.h OSD.h ProgressBar.h aux_functions.h errcodes.h)

find_package(Threads REQUIRED)
target_link_libraries(hdlgi-cli Threads::Threads)