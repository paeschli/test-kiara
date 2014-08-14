cmake_minimum_required (VERSION 2.6)
project (KIARA_SDK_Example)

include_directories("${PROJECT_SOURCE_DIR}/../include")
link_directories("${PROJECT_SOURCE_DIR}/../lib")

add_executable(structtest structtest.c)
target_link_libraries(structtest KIARA)

add_executable(structtest_server structtest_server.c)
target_link_libraries(structtest_server KIARA)
