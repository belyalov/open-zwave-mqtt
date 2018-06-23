# CMake project to build DPI parsers on different platforms.

cmake_minimum_required(VERSION 2.8)

project(zwave2mqtt)

## Default directories ##
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(SRC_TEST_DIR "${CMAKE_SOURCE_DIR}/test")

# App source files
set(SRCS
   ${SRC_DIR}/node_value.cpp
   ${SRC_DIR}/command_classes.cpp
   ${SRC_DIR}/mqtt.cpp
   ${SRC_DIR}/process_notification.cpp
   ${SRC_DIR}/options.cpp
   ${SRC_DIR}/polling.cpp
)

# Tests source files
set(SRCS_TESTS
   ${SRC_TEST_DIR}/mock_manager.cpp
   ${SRC_TEST_DIR}/mock_mosquitto.cpp
   ${SRC_TEST_DIR}/test_mqtt.cpp
   ${SRC_TEST_DIR}/test_node.cpp
   ${SRC_TEST_DIR}/test_options.cpp
   ${SRC_TEST_DIR}/test_value.cpp
   ${SRC_TEST_DIR}/test_polling.cpp
)

set(CMAKE_MACOSX_RPATH 1)

# In case of GNU CPP compiler - check version before
if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8)
    message(FATAL_ERROR "ZWave-MQTT requires at least gcc-4.8")
endif()

# pthreads
find_package(Threads REQUIRED)

# google test
find_package(GTest)

# openzwave library
find_library(OPENZWAVE_LIB PATH_SUFFIXES lib lib64 NAMES libopenzwave.so libopenzwave.dylib)
if(NOT OPENZWAVE_LIB)
  message(FATAL_ERROR "open_zwave library not found.")
endif()

# openzwave headers
find_path(OPENZWAVE_INCLUDES openzwave/Manager.h)
if(NOT OPENZWAVE_INCLUDES)
  message(FATAL_ERROR "open_zwave library headers not found")
endif()

# mosquitto library
find_library(MOSQUITTO_LIB NAMES libmosquitto.so libmosquitto.dylib)
if(NOT MOSQUITTO_LIB)
  message(FATAL_ERROR "mosquitto library not found.")
endif()

# mosquitto headers
find_path(MOSQUITTO_INCLUDES mosquitto.h)
if(NOT MOSQUITTO_INCLUDES)
  message(FATAL_ERROR "mosquitto library headers not found")
endif()

# valgrind
find_program(VALGRIND valgrind)
if(NOT VALGRIND)
    message(STATUS "Valgrind DISABLED: not installed")
else()
    message(STATUS "Valgrind ENABLED. (${VALGRIND})")
endif()

message(STATUS "OpenZwave Includes:  ${OPENZWAVE_INCLUDES}")
message(STATUS "OpenZwave Libraries: ${OPENZWAVE_LIB}")
message(STATUS "Mosquitto Includes:  ${MOSQUITTO_INCLUDES}")
message(STATUS "Mosquitto Libraries: ${MOSQUITTO_LIB}")
message(STATUS "GTest Includes:      ${GTEST_INCLUDE_DIRS}")
message(STATUS "GTest Libraries:     ${GTEST_BOTH_LIBRARIES}")
message(STATUS "C++ compiler:        ${CMAKE_CXX_COMPILER}")

# global include directories

include_directories(${SRC_DIR})
include_directories(${OPENZWAVE_INCLUDES} ${OPENZWAVE_INCLUDES}/openzwave)
include_directories(${MOSQUITTO_INCLUDES})

# Common CPP flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++0x -Wall -Wreturn-type -fvisibility=hidden")

# Compile app objects before making executables (otherwise - they'll be compiled twice)
add_library(ozw-mqtt-objs OBJECT ${SRCS})

# Main application

add_executable(ozw-mqtt ${SRC_DIR}/ozw-mqtt.cpp $<TARGET_OBJECTS:ozw-mqtt-objs>)
target_link_libraries(ozw-mqtt ${CMAKE_THREAD_LIBS_INIT} ${OPENZWAVE_LIB} ${MOSQUITTO_LIB})

# Unit testing
if (GTEST_FOUND)
  message(STATUS "Testing enabled")
  enable_testing()
  add_executable(tests ${SRCS_TESTS} $<TARGET_OBJECTS:ozw-mqtt-objs>)
  target_link_libraries(tests ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
  # Test 1: Unittests
  add_test(UnitTests tests)
  # Test 2: Valgrind on Unittests, if available
  if (VALGRIND)
    add_test(Valgrind valgrind ./tests)
  endif()
  # To make run unittests easier
  add_custom_target(check COMMAND tests)
endif()
