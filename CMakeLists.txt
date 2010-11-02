cmake_minimum_required(VERSION 2.8)

project(SipServerLibrary)
find_package( Boost COMPONENTS regex REQUIRED )
add_library( sipserver SipServer.cpp )
target_link_libraries( sipserver ${bOOsT_LiBRArIES} )
