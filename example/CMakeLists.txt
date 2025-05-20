cmake_minimum_required(VERSION 3.10.0) 

#设置要包含的库
set(SQLITE3_LIBS sqlite3)
#要包含的头文件目录
include_directories(${SYSROOT})

#设置要包含的库
set(BOOST_LIBS boost_thread boost_system)
#要包含的头文件目录
include_directories(${SYSROOT}/boost)

find_package(Threads REQUIRED)

#[[
#将什么源文件生成可执行文件
add_executable(lop2_test lop2_test.cpp) 
#生成这个可执行文件需要的头文件在哪里
target_include_directories(lop2_test PUBLIC ${CMAKE_SOURCE_DIR}/src/devices ${CMAKE_SOURCE_DIR}/src/parsedata ${CMAKE_SOURCE_DIR}/src/datatobase) 
#生成这个可执行文件需要依赖什么库
target_link_libraries(lop2_test libdevices ${SQLITE3_LIBS})
]]

#[[
#将什么源文件生成可执行文件
add_executable(lop1_422_test lop1_test.cpp) 
#生成这个可执行文件需要的头文件在哪里
target_include_directories(lop1_422_test PUBLIC ${CMAKE_SOURCE_DIR}/src/devices ${CMAKE_SOURCE_DIR}/src/parsedata ${CMAKE_SOURCE_DIR}/src/datatobase) 
#生成这个可执行文件需要依赖什么库
target_link_libraries(lop1_422_test libdevices ${SQLITE3_LIBS})

#将什么源文件生成可执行文件
add_executable(lop1_thread lop1_thread.cpp) 
#生成这个可执行文件需要的头文件在哪里
target_include_directories(lop1_thread PUBLIC ${CMAKE_SOURCE_DIR}/src/devices ${CMAKE_SOURCE_DIR}/src/parsedata ${CMAKE_SOURCE_DIR}/src/datatobase) 
#生成这个可执行文件需要依赖什么库
target_link_libraries(lop1_thread PRIVATE libdevices ${SQLITE3_LIBS} Threads::Threads)

#生成这个可执行文件需要的头文件在哪里
include_directories(${CMAKE_SOURCE_DIR}/include/json)
#将什么源文件生成可执行文件
add_executable(websocket_422_test websocket_test.cpp) 
#生成这个可执行文件需要依赖什么库
target_link_libraries(websocket_422_test libdevices ${BOOST_LIBS} ${SQLITE3_LIBS})
]]
#将什么源文件生成可执行文件
add_executable(lop1_thread_async lop1_thread_async.cpp) 
#生成这个可执行文件需要的头文件在哪里
target_include_directories(lop1_thread_async PUBLIC ${CMAKE_SOURCE_DIR}/src/devices ${CMAKE_SOURCE_DIR}/src/parsedata ${CMAKE_SOURCE_DIR}/src/datatobase) 
#生成这个可执行文件需要依赖什么库
target_link_libraries(lop1_thread_async PRIVATE libdevices ${SQLITE3_LIBS} Threads::Threads)

#生成这个可执行文件需要的头文件在哪里
include_directories(${CMAKE_SOURCE_DIR}/include/json)
#将什么源文件生成可执行文件
add_executable(server http_rest_realtime_main.cpp) 
#生成这个可执行文件需要依赖什么库
target_link_libraries(server libdevices ${BOOST_LIBS} ${SQLITE3_LIBS})


