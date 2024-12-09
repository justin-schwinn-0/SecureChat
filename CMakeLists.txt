cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 17)

project(SecureChat)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_library(SCTP_LIB libsctp.so)
set(THREADS_PREFER_PTHREAD_FLAG ON)

set(CommonSources
    Utils.cpp
    NetServer.cpp
    NetCommon.cpp
    Crypto.cpp
)

set(CommonHeaders
    Utils.h
    NetServer.h
    NetCommon.h
    Crypto.h
)

set(ClientCode
    Client.cpp
)

set(ServerCode
    Server.cpp
    ServerData.h
    ServerData.cpp
)

set(Libraries
    ${SCTP_LIB}
    Threads::Threads
    OpenSSL::Crypto
)

#find_library(THREAD_LIB libpthread.so)


add_executable(${PROJECT_NAME} ${ClientCode} ${CommonSources} ${CommonHeaders})
add_executable(Server ${ServerCode} ${CommonSources} ${CommonHeaders})
#target_link_libraries(Server PRIVATE ${THREAD_LIB})

target_link_libraries(Server PRIVATE ${Libraries})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Libraries})
