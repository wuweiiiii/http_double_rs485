cmake_minimum_required(VERSION 3.10.0)

#参与编译的源文件.c和.cpp
aux_source_directory(linux_uart SRC_CPP_LISTS)
aux_source_directory(devices SRC_CPP_LISTS)
aux_source_directory(jsoncpp SRC_CPP_LISTS)
aux_source_directory(parsedata SRC_CPP_LISTS)
aux_source_directory(basetoweb SRC_CPP_LISTS)
aux_source_directory(datatobase SRC_CPP_LISTS)


#将源文件编译成一个静态库 
add_library(libdevices STATIC ${SRC_CPP_LISTS})

#引入头文件 
target_include_directories(libdevices PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(libdevices PUBLIC ${CMAKE_SOURCE_DIR}/src/linux_uart)
target_include_directories(libdevices PUBLIC ${CMAKE_SOURCE_DIR}/src/devices)
target_include_directories(libdevices PUBLIC ${CMAKE_SOURCE_DIR}/src/jsoc)
target_include_directories(libdevices PUBLIC ${CMAKE_SOURCE_DIR}/src/parsedata)
target_include_directories(libdevices PUBLIC ${CMAKE_SOURCE_DIR}/src/basetoweb)
target_include_directories(libdevices PUBLIC ${CMAKE_SOURCE_DIR}/src/datatobase)




