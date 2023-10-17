cmake_minimum_required(VERSION 3.17)

set(PROJECT_NAME Task_1)
project(${PROJECT_NAME})

add_executable(${PROJECT_NAME} Main.cpp config.json)

set(nlohmann_json_DIR "O:/C++/Libraries/json-3.11.2/build")

find_package(PostgreSQL REQUIRED)

find_package(nlohmann_json 3.11.2 REQUIRED)

if(PostgreSQL_FOUND)
    include_directories(${PostgreSQL_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${PostgreSQL_LIBRARIES})
endif()

target_link_libraries(${PROJECT_NAME}  nlohmann_json::nlohmann_json)

