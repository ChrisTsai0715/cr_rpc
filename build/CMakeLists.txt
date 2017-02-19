cmake_minimum_required(VERSION 2.8)  # CMake 版本要求
PROJECT(cr_rpc)                      # 项目名称

set(CMAKE_SYSTEM_NAME Linux)

set(SRC "../src")
set(COMMON "${SRC}/common")
set(RPC "${SRC}/rpc_lib")

include_directories(${COMMON}
		    ${RPC}
		    ${COMMON}/json/include
		    ${SRC}
)

set(SOURCE_LIST ${COMMON}/base_thread.cpp
	${COMMON}/base_thread.cpp
	${COMMON}/json/src/lib_json/json_reader.cpp  
	${COMMON}/json/src/lib_json/json_value.cpp
	${COMMON}/json/src/lib_json/json_writer.cpp
	${COMMON}/base_thread.cpp
	${RPC}/base_rpc_interface.cpp
	${RPC}/socket_comm.cpp
	${RPC}/select_task.cpp
	${RPC}/fifo_comm.cpp
	${RPC}/base_rpc_client.cpp
	${RPC}/base_rpc_server.cpp
)

set(CMAKE_CXX_FLAGS "-O3 -fPIC -s")
set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -s")
link_libraries(
pthread
stdc++
rt
) 

install(FILES ${RPC}/base_rpc_interface.h  DESTINATION ../lib)
install(FILES ${RPC}/base_rpc_service.h    DESTINATION ../lib)
install(FILES cr_rpc_lib DESTINATION ../lib)

#add_library(cr_rpc_lib SHARED ${SOURCE_LIST})
add_library(cr_rpc_lib STATIC ${SOURCE_LIST})

