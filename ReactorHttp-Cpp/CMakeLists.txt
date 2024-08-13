cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(ReactorHttp)

# 设置 C 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)

# 添加源文件
set(SOURCE_FILES
    Buffer.cpp
    Channel.cpp
    Dispatcher.cpp
    EpollDispatcher.cpp
    EventLoop.cpp
    HttpRequest.cpp
    HttpResponse.cpp
    main.cpp
    PollDispatcher.cpp
    SelectDispatcher.cpp
    TcpConnection.cpp
    TcpServer.cpp
    ThreadPool.cpp
    WorkerThread.cpp
)

# 添加头文件目录
include_directories(${CMAKE_SOURCE_DIR})

# 添加可执行文件
add_executable(ReactorHttp ${SOURCE_FILES})

# 如果需要链接其他库，可以在这里添加
# target_link_libraries(ReactorHttp some_library)