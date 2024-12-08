cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 17)

project(SecureChat)

find_package(Threads REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)

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
    ServerData.h
    ServerData.cpp
)

find_library(SCTP_LIB libsctp.so)
find_library(THREAD_LIB libpthread.so)


add_executable(${PROJECT_NAME} ${ClientCode} ${CommonSources} ${CommonHeaders})
add_executable(Server ${ServerCode} ${CommonSources} ${CommonHeaders})
target_link_libraries(${PROJECT_NAME} PRIVATE ${SCTP_LIB})
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
#target_link_libraries(${PROJECT_NAME} PRIVATE ${THREAD_LIB})

find_package(OpenSSL REQUIRED)
target_link_libraries(Server PRIVATE OpenSSL::Crypto)
target_include_directories(${PROJECT_NAME} PRIVATE ${OPENSSL_INCLUDE_DIR})
