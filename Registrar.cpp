#include "Registrar.hpp"
#include "SipRequest.hpp"
#include "redis-cplusplus-client/redisclient.h"

namespace Sip {

auto_ptr<SipResponse> Registrar::HandleRequest( const SipRequest& request ) {
	auto_ptr<SipResponse> response( new SipResponse( 200, "OK", request ) );
	try {
		redis::client rc;

		const string& endpoint_id = Sip::URI( request.GetHeaderValues( "to" )[0] ).User();
		const string& uri = Sip::URI( request.GetHeaderValues( "contact" )[0] ).URIAsString();
		ostringstream path;
		path << "registrar:" << endpoint_id;
		time_t expiration = time( NULL );
		int toExpire = 3600;
		if ( request.GetHeaderValues( "via" )[0].HasTag( "expires" ) )
			toExpire = atoi( request.GetHeaderValues( "via" )[0].Tags().find( "expires" )->second.c_str() );
		else if ( request.HasHeader( "expires" )  )
			toExpire = atoi( request.GetHeaderValues( "expires" )[0].Value().c_str() );
		if ( request.GetHeaderValues( "contact" )[0].Value() == "*" )  //Erase registration
			toExpire = 0;

		if ( toExpire == 0 ) {	//Unregister contact
			rc.del( path.str() );  
		}
		else {	//Register (as long as expiration > SIP_MIN_EXPIRE )
			if ( toExpire < atoi( SIP_MIN_EXPIRE ) && toExpire != 0) { //0 has informally become the way to de-register it seems
				vector<SipHeaderValue> minExpires;
				minExpires.push_back( SipHeaderValue( SIP_MIN_EXPIRE ) );
				response->SetStatusCode( 423 );
				response->SetReasonPhrase( "Interval too brief" );
				response->SetHeader( "min-expires", minExpires );

				return response;
			}
			//TODO: Set keys
			rc.hset( path.str(), "uri", uri );
			stringstream expire;
			expire << toExpire;
			rc.hset( path.str(), "expiration", expire.str() );
			rc.expire( path.str(), toExpire );
		}
	} catch( exception& err ) {
		response->SetStatusCode( "500" );
		response->SetReasonPhrase( err.what() );

		return response;
	}

	return response;
}

};

