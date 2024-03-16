cmake_minimum_required(VERSION 3.10)
project(accuracyCalculator)

set(CMAKE_CXX_STANDARD 17)

cmake_policy(SET CMP0074 NEW)
set(BOOST_ROOT "C:/boost_installed")
set(BOOST_LIBRARYDIR "${BOOST_ROOT}/lib")

find_package(Boost 1.83.0 REQUIRED COMPONENTS system filesystem)
include_directories(${Boost_INCLUDE_DIRS})
add_executable(accuracyCalculator main.cpp)

target_link_libraries(accuracyCalculator ${Boost_LIBRARIES} ws2_32)
