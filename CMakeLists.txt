cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 17)

project(SecureChat)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(CommonSources
    Utils.cpp
    NetServer.cpp
    NetCommon.cpp
)

set(CommonHeaders
    Utils.h
    NetServer.h
    NetCommon.h
)

set(ClientCode
    Client.cpp
)

set(ServerCode
    Server.cpp
)

find_library(SCTP_LIB libsctp.so)
#find_library(THREAD_LIB libpthread.so)


add_executable(${PROJECT_NAME} ${ClientCode} ${CommonSources} ${CommonHeaders})
add_executable(Server ${ServerCode} ${CommonSources} ${CommonHeaders})
target_link_libraries(${PROJECT_NAME} PRIVATE ${SCTP_LIB})
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
#target_link_libraries(${PROJECT_NAME} PRIVATE ${THREAD_LIB})
