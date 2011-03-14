cmake_minimum_required(VERSION 2.8)
SET( CMAKE_BUILD_TYPE Debug )

project(SipServerLibrary)
add_subdirectory( tests )
find_package( Boost COMPONENTS regex REQUIRED )
add_library( sipserver SipServer.cpp )
target_link_libraries( sipserver ${Boost_LIBRARIES} )
