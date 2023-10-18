cmake_minimum_required(VERSION 3.17)

set(PROJECT_NAME Task_1)
project(${PROJECT_NAME})

add_executable(${PROJECT_NAME} Main.cpp)

set(CONFIG_DESTINATION "{CMAKE_CURRENT_BINARY_DIR}")
configure_file(config.json ${CMAKE_BINARY_DIR}/config.json COPYONLY)

find_package(PostgreSQL REQUIRED)

if(PostgreSQL_FOUND)
    include_directories(${PostgreSQL_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${PostgreSQL_LIBRARIES})
endif()


