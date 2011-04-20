#include <iostream>
#include "redis-cplusplus-client/redisclient.h"


int main( int argc, char* argv[] ) {
	redis::client rc;

	rc.set( "Test", "Blah" );
	
	return 0;
}
