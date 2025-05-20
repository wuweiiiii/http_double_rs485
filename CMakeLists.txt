cmake_minimum_required(VERSION 3.10.0) 

message(STATUS "cmake version ${CMAKE_VERSION}")

project(double_485)

#设置交叉编译工具链
set(TOOLCHAIN_DIR /opt/atk-dlrk356x-toolchain)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/bin/aarch64-buildroot-linux-gnu-g++)
set(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}/bin/aarch64-buildroot-linux-gnu-gcc)
#SET(CMAKE_CXX_COMPILER /opt/atk-dlrk356x-toolchain/usr/bin/aarch64-buildroot-linux-gnu-g++) 

#设置系统根目录(头文件目录)
set(SYSROOT ${TOOLCHAIN_DIR}/aarch64-buildroot-linux-gnu/sysroot/usr/include)
#设置系统根目录(库文件目录)
set(SYSROOT_LIB ${TOOLCHAIN_DIR}/aarch64-buildroot-linux-gnu/sysroot/usr/lib)
#设置系统根目录
set(CMAKE_SYSROOT ${TOOLCHAIN_DIR}/aarch64-buildroot-linux-gnu/sysroot)

SET(CMAKE_CXX_FLAGS "--std=c++11 -Wl,--no-as-needed -lpthread") 
add_compile_options("-D_GLIBCXX_USE_CXX11_ABI=0")



#先编译子目录
add_subdirectory(src) 
add_subdirectory(example) 