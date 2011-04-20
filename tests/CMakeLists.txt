find_package( Boost REQUIRED COMPONENTS unit_test_framework )

# include subdirs for boost headers
include_directories( 
	${Boost_INCLUDE_DIRS}	
)

# create unit tests exec
add_executable (
	sipserver_tests
	#key include for boost.test
	TestRunner.cpp
	#test suites
	parse_tests.cpp
)

# link libraries
target_link_libraries (
	sipserver_tests
	sip
	${Boost_LIBRARIES}	
)
