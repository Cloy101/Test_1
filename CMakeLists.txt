cmake_minimum_required(VERSION 3.17)

set(PROJECT_NAME Task_1)
project(${PROJECT_NAME})

add_executable(${PROJECT_NAME} Main.cpp config.json)

find_package(PostgreSQL REQUIRED)

if(PostgreSQL_FOUND)
    include_directories(${PostgreSQL_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${PostgreSQL_LIBRARIES})
endif()
