#include <boost/test/unit_test.hpp>
#include <sstream>
#include "../SipUtility.hpp"
#include "../SipRequest.hpp"
#include "../SipResponse.hpp"
//http://code.google.com/p/dtl-cpp/
#include "dtl/dtl.hpp"

#include "sip_messages.h"
using namespace Sip;
using namespace std;
void check_differences( const string& original, const string& render, const int& i ) {
	dtl::Diff< char, string > d( original, render );
	d.compose();
	stringstream output;
	dtl::Ses<char> ses = d.getSes();
	bool good = true;
	bool has_whitespace_diff = false;
	for ( vector< pair< char, dtl::elemInfo > >::const_iterator elem = ses.getSequence().begin();
			elem != ses.getSequence().end(); ++elem ) {
		if ( ( elem->second.type == dtl::SES_ADD || elem->second.type == dtl::SES_DELETE ) ) {
			if ( elem->first != ' ' && elem->first != '\t' ) {
				good = false;
				break;
			}
			else {
				has_whitespace_diff = true;	
			}
		}
	}
	if ( !good ) {
		output << "SipMessage @ " << i << " does not match!\n";
		d.composeUnifiedHunks();
		d.printUnifiedFormat( output );
		BOOST_ERROR( output.str() );
	}
	else if ( has_whitespace_diff ) {
		output << "SipMessage @ " << i << " has whitespace differences.";
		BOOST_WARN_MESSAGE( false, output.str() );
	}
}

BOOST_AUTO_TEST_CASE( invites ) {
	int i = 0;
	for ( const char* sip_message = sip_messages[i];
			sip_message != NULL; sip_message = sip_messages[++i] ) {

		auto_ptr<Sip::SipMessage> this_message;
		string original_message( sip_message );
		BOOST_REQUIRE_NO_THROW( Utility::ParseMessage( this_message, sip_message ) );			
		if ( this_message->Type == SipMessage::MT_REQUEST ) {
			SipRequest& request = static_cast<SipRequest&>( *this_message );
			check_differences( original_message, request.ToString(), i );
		}
		else if ( this_message->Type == SipMessage::MT_RESPONSE ) {
			SipResponse& response = static_cast<SipResponse&>( *this_message );
			check_differences( original_message, response.ToString(), i );
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
