cmake_minimum_required(VERSION 3.12)
project (audio_tool_api)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11")

set(SOURCE_FILES
	audio_tool_api/audio/audio_processor.cpp
	audio_tool_api/audio/processor.cpp
	audio_tool_api/handlers/auth.cpp
	audio_tool_api/handlers/base_handler.cpp
	audio_tool_api/handlers/decode_file.cpp
	audio_tool_api/handlers/files.cpp
	audio_tool_api/handlers/project.cpp
	audio_tool_api/handlers/test.cpp
	audio_tool_api/handlers/user.cpp
	audio_tool_api/http/http_connection.cpp
	audio_tool_api/http/http_exception.cpp
	audio_tool_api/http/http_request.cpp
	audio_tool_api/http/http_response.cpp
	audio_tool_api/http/http_server.cpp
	audio_tool_api/http/http_work_thread.cpp
	audio_tool_api/http/http_worker.cpp
	audio_tool_api/http/st_http_server.cpp
	audio_tool_api/http/st_http_worker.cpp
	audio_tool_api/http/tpc_http_server.cpp
	audio_tool_api/http/multipart_parser.c
	audio_tool_api/main.cpp
	audio_tool_api/models/file.cpp
	audio_tool_api/models/project.cpp
	audio_tool_api/models/session.cpp
	audio_tool_api/models/user.cpp
	audio_tool_api/utilities/base64.cpp
	audio_tool_api/utilities/config.cpp
	audio_tool_api/utilities/database.cpp
	audio_tool_api/utilities/stats_client.cpp)
	
link_directories(/usr/local/lib)

add_executable(audio_tool_api ${SOURCE_FILES})

target_link_libraries(audio_tool_api PRIVATE pthread curl mysqlcppconn glog swscale swresample avdevice avformat avresample avfilter avcodec avutil boost_thread boost_system ssl crypto boost_coroutine)
target_include_directories(audio_tool_api PRIVATE audio_tool_api /usr/local/include /usr/local/opt/openssl/include)

#link_directories(./audio_tool_api)
