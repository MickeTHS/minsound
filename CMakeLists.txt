cmake_minimum_required(VERSION 2.4)
PROJECT(minsound)
SET (CMAKE_BUILD_TYPE Release)

if(APPLE)
  link_libraries(${CMAKE_SOURCE_DIR}/lib/win32)

elseif(WIN32)
  add_definitions(-DWIN32)
  set(CMAKE_LIBRARY_PATH "${CMAKE_SOURCE_DIR}/lib/win32/Release/")

  ADD_CUSTOM_COMMAND (TARGET testminsound POST_BUILD
    # DEBUG
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/lib/win32/Debug/OpenAL32.dll ${CMAKE_BINARY_DIR}/Debug

    # RELEASE
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/lib/win32/Release/OpenAL32.dll ${CMAKE_BINARY_DIR}/Release

    COMMENT "Copying DLLs" VERBATIM
  
  )

endif()

IF(WIN32)
set(CMAKE_CXX_FLAGS "-std=c++11")
ELSE(WIN32)
set(CMAKE_CXX_FLAGS "-std=c++11 -pthread -O3")
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined -pthread -std=c++11 -O3")
ENDIF(WIN32)

include_directories(
  lib/include
  include
)

SET(minsound_src
    src/minsound.cpp
)

SET(testminsound_src
    src/test.cpp
)

add_library(libminsound
    ${minsound_src}
)

link_libraries(${CMAKE_SOURCE_DIR}/lib/win32/Release/OpenAL32.lib)

target_link_libraries(libminsound OpenAL32)

add_executable(testminsound ${testminsound_src})

target_link_libraries(testminsound OpenAL32 libminsound)