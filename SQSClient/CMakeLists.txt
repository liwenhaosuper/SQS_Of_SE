find_package(PkgConfig)

pkg_check_modules(LIBEVENT "libevent >= 2.0")

set(Client_SRCS
   Main.cpp
   SQSClient.cpp
)

add_executable(SQSClient ${Client_SRCS})
target_link_libraries(SQSClient ${LIBEVENT_LIBRARIES})
install(TARGETS SQSClient DESTINATION bin)