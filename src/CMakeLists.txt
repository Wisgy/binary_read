file(GLOB SOURCES "*.cc")

set(EXEC_FILE "main.cc")
list(REMOVE_ITEM SOURCES ${EXEC_FILE})

add_library(binread_lib STATIC ${SOURCES})

add_executable(main ${EXEC_FILE})

target_link_libraries(main binread_lib)
target_link_libraries(main ${Boost_LIBRARIES})
