cmake_minimum_required(VERSION 3.5)
project(aicup2020)

# OS and compiler checks.
if(WIN32)
    add_definitions(-DWIN32)
    SET(PROJECT_LIBS Ws2_32.lib)
endif()

set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined -fno-sanitize-recover=all -fsanitize-undefined-trap-on-error -g -O0 -fno-omit-frame-pointer -g")

file(GLOB HEADERS "*.hpp" "model/*.hpp")
SET_SOURCE_FILES_PROPERTIES(${HEADERS} PROPERTIES HEADER_FILE_ONLY TRUE)
file(GLOB SRC "*.cpp" "model/*.cpp")
add_executable(aicup2020 ${HEADERS} ${SRC})
TARGET_LINK_LIBRARIES(aicup2020 ${PROJECT_LIBS})