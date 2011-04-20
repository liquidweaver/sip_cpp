#include <boost/test/unit_test.hpp>
#include "../Registrar.cpp"
#include "registrar_sip_messages.h"

using namespace Sip;

BOOST_AUTO_TEST_CASE( registration ) {
	Sip::Registrar registrar;
	int i = 0;
	for ( const char* sip_message = sip_messages[i];
			sip_message != NULL; sip_message = sip_messages[++i] ) {

		auto_ptr<Sip::SipMessage> this_message;
		string original_message( sip_message );
		BOOST_REQUIRE_NO_THROW( SipUtility::ParseMessage( this_message, sip_message ) );			
		if ( this_message->Type == SipMessage::MT_REQUEST ) {
			SipRequest& request = static_cast<SipRequest&>( *this_message );
			auto_ptr<SipResponse> response( registrar.HandleRequest( request ) );
			BOOST_CHECK_EQ( response->ResponseCode(), 200 );
			//TODO: Check registratration in DB
		}
		else if ( this_message->Type == SipMessage::MT_RESPONSE ) {
			BOOST_FAIL( "Test Message not a REGISTER Request!" );
		}
		else {
			stringstream mesg;
			mesg << "SipMessage not MT_REQUEST or MT_RESPONSE\n"
					<< "i = " << i << "\n"
					<< sip_message;
			BOOST_FAIL( mesg.str() );
		}
		
	}
}
