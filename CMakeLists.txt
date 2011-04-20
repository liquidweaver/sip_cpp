cmake_minimum_required(VERSION 2.8)
SET( CMAKE_BUILD_TYPE Debug )

project(SipServer)
add_subdirectory( tests )
find_package( Boost COMPONENTS regex REQUIRED )

#add zeromq library
set(zmq_DIR ${CMAKE_SOURCE_DIR}/zmq)
add_custom_command(
  OUTPUT ${zmq_DIR}/configure
  COMMAND VERBATIM ${zmq_DIR}/autogen.sh
  #ARGS
  #DEPENDS ${zmq_DIR}/src/platform.hpp.in
  COMMENT "Automating zeromq configuration..."
  WORKING_DIRECTORY ${zmq_DIR}
  VERBATIM )
add_custom_command(
  OUTPUT ${zmq_DIR}/src/platform.hpp
  COMMAND  ${zmq_DIR}/configure
  COMMAND  ${CMAKE_COMMAND} -E touch ${zmq_DIR}/src/platform.hpp
  #ARGS
  DEPENDS ${zmq_DIR}/configure
  COMMENT "Configuring zeromq..."
  WORKING_DIRECTORY ${zmq_DIR}
  VERBATIM )

file(GLOB zmq_FILES ${zmq_DIR}/src/*.cpp ${zmq_DIR}/src/*.c)
add_library(zmq ${zmq_FILES} ${zmq_DIR}/src/platform.hpp)
target_link_libraries(zmq uuid)

#add redis library
set(redis_DIR ${CMAKE_SOURCE_DIR}/redis-cplusplus-client)

set(redis_FILES ${redis_DIR}/anet.c )
add_library(hiredis ${redis_FILES} )

#since we're providing zmq, set zmq variables as appropriate,
# so sub-projects will use our zmq and not fail when checking for it
set(zmq_INCLUDE_DIRS ${zmq_DIR}/include)
set(zmq_LIBRARIES zmq)
set(zmq_FOUND TRUE)
file(GLOB sip_FILES ${CMAKE_SOURCE_DIR}/*.cpp )
add_library( sip ${sip_FILES} )
#add_library( sipserver SipServer.cpp )
#target_link_libraries( sipserver ${Boost_LIBRARIES} )
