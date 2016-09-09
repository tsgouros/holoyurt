PROJECT(holoyurt)

#SET BUILD-TYPE TO RELEASE IF NOT DEFINED
set(CMAKE_BUILD_TYPE Debug)

set(ARCH "x86_64")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC OR MSVC_IDE) 
  message( "VC11: use Microsoft Visual Studio 2013 " 
      "with Microsoft Visual C++ Compiler Nov 2012 CTP (v120p_xp)" ) 
    set(CMAKE_GENERATOR_TOOLSET   "v120_xp" CACHE STRING "Platform Toolset" FORCE) 
    set(CMAKE_VS_PLATFORM_TOOLSET  "v120_xp" CACHE STRING "Platform Toolset" FORCE) 
endif()

if(APPLE AND CMAKE_INSTALL_PREFIX MATCHES "/usr/local")
  set(CMAKE_INSTALL_PREFIX "/Applications")
endif()

if(APPLE)
       add_definitions( -Wno-deprecated-register)
endif()

set(CMAKE_C_FLAGS_RELEASE "-O3")

message(STATUS "Building ${PROJECT_NAME} in ${CMAKE_BUILD_TYPE} mode")
cmake_minimum_required(VERSION 2.8.8)

#ADD OPENCV SUPPORT
find_package( OpenCV REQUIRED )

# Set some Win32 Specific Settings
option(WITH_CONSOLE "Build with a command line for debugging" OFF)

if(WIN32)
  if(WITH_CONSOLE)
    add_definitions(-DWITH_CONSOLE)
  else()
    set(GUI_TYPE WIN32)
  endif()
endif(WIN32)

message(STATUS "opencv includes=${${OPENCV_INCLUDE_DIRS}}")
set(CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${OPENCV_INCLUDE_DIRS})

#SET GENERIC HEADER AND SOURCE FILES
set(HEADERS 
	src/Socket.h
)

set(SOURCES
	src/holoyurt.cpp
	src/Socket.cpp
)

source_group(_CORE FILES ${HEADERS} ${SOURCES})

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/bin)


#BUILD AND LINK
add_executable(${PROJECT_NAME} ${GUI_TYPE}
  ${SOURCES} 
  ${HEADERS}
  )

if (APPLE)
  add_dependencies(${PROJECT_NAME} osx_bundle_dirs )
endif(APPLE)

target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})
