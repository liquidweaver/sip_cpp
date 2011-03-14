#include <boost/test/unit_test.hpp>
#include "../SipServer.h"

#include "sip_messages.h"
BOOST_AUTO_TEST_CASE( invites ) {
	int i = 0;
	for ( const char* sip_message = sip_messages[i];
			sip_message != NULL; sip_message = sip_messages[++i] ) {
			auto_ptr<Sip::SipMessage> this_message;
		BOOST_REQUIRE_NO_THROW( Sip::SipUtility::ParseMessage( this_message, sip_message ) );			
		
	}
}
