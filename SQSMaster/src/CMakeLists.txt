find_package(PkgConfig)

pkg_check_modules(LIBEVENT "libevent >= 2.0")

set(SQS_SRCS
   main.cpp
   MsgLock.cpp
   SQSMaster.cpp
   Logger.cpp
)

add_executable(SQSMaster ${SQS_SRCS})
target_link_libraries(SQSMaster ${LIBEVENT_LIBRARIES})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
install(TARGETS SQSMaster DESTINATION bin)