cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(ReactorHttp)

# 设置 C 标准
set(CMAKE_C_STANDARD 99)

# 添加源文件
set(SOURCE_FILES
    Buffer.c
    Channel.c
    ChannelMap.c
    EpollDispatcher.c
    EventLoop.c
    HttpRequest.c
    HttpResponse.c
    main.c
    PollDispatcher.c
    SelectDispatcher.c
    TcpConnection.c
    TcpServer.c
    ThreadPool.c
    WorkerThread.c
)

# 添加头文件目录
include_directories(${CMAKE_SOURCE_DIR})

# 添加可执行文件
add_executable(ReactorHttp ${SOURCE_FILES})

# 如果需要链接其他库，可以在这里添加
# target_link_libraries(ReactorHttp some_library)