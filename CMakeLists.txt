cmake_minimum_required(VERSION 3.6)
project(tiny-orm)

# protoc
set(PROTOC protoc)

# catch.hpp options
add_definitions(-DCATCH_PLATFORM_WINDOWS)

# CXX_FLAGS
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Werror -Wall -g")

set(SRC_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_EXAMPLE ${CMAKE_CURRENT_SOURCE_DIR}/example)
set(SRC_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/src)

include_directories(include)
include_directories(/usr/local/include)

link_directories(/usr/local/lib/)
link_directories(/usr/lib/)

# mysql
set(LIBS_MYSQL mysqlpp mysqlclient z ssl crypto)

#
#  SOCI ? Mysql++ ?
#
set(USE_ORM_SOCI 0)
set(ORM_LIBS "")
if (USE_ORM_SOCI)
    include_directories(/usr/local/include/soci
            /usr/local/include/soci/mysql
            /usr/include/mysq)
    link_directories(/usr/local/lib64)

    set(ORM_LIBS soci_core soci_mysql)
else ()
    add_definitions(-DUSE_ORM_MYSQLPP)
    add_definitions(-DMYSQLPP_MYSQL_HEADERS_BURIED)
    set(ORM_LIBS mysqlpp)
endif ()

execute_process(COMMAND ${PROTOC} -I=${SRC_INCLUDE} --cpp_out=${SRC_INCLUDE} ${SRC_INCLUDE}/archive.proto)
execute_process(COMMAND ${PROTOC} -I=${SRC_EXAMPLE} --cpp_out=${SRC_EXAMPLE} ${SRC_EXAMPLE}/player.proto)

add_library(tinyserializer
        STATIC
        ${SRC_INCLUDE}/archive.pb.h
        ${SRC_INCLUDE}/archive.pb.cc)

add_library(tinyorm
        STATIC
        ${SRC_DIR}/url.cpp
        ${SRC_DIR}/tinymysql.cpp
        ${SRC_DIR}/tinyorm.cpp
        ${SRC_DIR}/hashkit.cpp)

add_executable(demo_serialize example/demo_serialize.cpp example/player.pb.cc)
target_link_libraries(demo_serialize tinyserializer  protobuf)

add_executable(demo_serialize_dyn example/demo_serialize_dyn.cpp example/player.pb.cc)
target_link_libraries(demo_serialize_dyn tinyserializer protobuf)

add_executable(demo_orm example/demo_orm.cpp example/player.pb.cc)
target_link_libraries(demo_orm tinyserializer protobuf tinyorm ${LIBS_MYSQL})

add_executable(test_serialize test/test_serialize.cpp example/player.pb.cc)
target_link_libraries(test_serialize tinyserializer protobuf)

install(TARGETS tinyserializer DESTINATION lib)
install(TARGETS tinyorm DESTINATION lib)

install(FILES
        include/tinyreflection.h
        include/tinyserializer.h
        include/tinyserializer_proto.h
        include/tinyserializer_proto_dyn.h
        include/tinyserializer_proto_mapping.h
        include/tinyworld.h
        include/archive.pb.h
        include/archive.pb.cc
        DESTINATION include/tinyworld)