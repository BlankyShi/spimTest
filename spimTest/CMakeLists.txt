project(spimTest)

cmake_minimum_required(VERSION 3.10)

set(INC_DIR /usr/local/include)

set(LINK_DIR /usr/local/lib)

include_directories(${INC_DIR}) 
link_directories(${LINK_DIR})


add_executable(spimTest src/spimTest.c)
target_link_libraries(spimTest libft4222.so)

add_executable(spimTest2 src/spimTest2.c)
target_link_libraries(spimTest2 libft4222.so)

add_executable(spimTest1 src/spimTest1.c)
target_link_libraries(spimTest1 libft4222.so)

add_executable(spimTest3 src/spimTest3.c)
target_link_libraries(spimTest3 libft4222.so)

add_executable(cpp src/cpp.cpp)