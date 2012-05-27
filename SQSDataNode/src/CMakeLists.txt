find_package(PkgConfig)

pkg_check_modules(LIBEVENT "libevent >= 2.0")
pkg_check_modules(SQLITE "sqlite3 >= 3.0")

set(DataNode_SRCS
   main.cpp
   Logger.cpp
   Database.cpp
)

add_executable(SQSDataNode ${DataNode_SRCS})
target_link_libraries(SQSDataNode ${LIBEVENT_LIBRARIES} ${SQLITE_LIBRARIES})
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")