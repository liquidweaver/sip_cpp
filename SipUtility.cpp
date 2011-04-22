#include "SipUtility.hpp"
#include <boost/regex.hpp>
#include "SipRequest.hpp"
#include "SipResponse.hpp"
namespace Sip {
void Utility::FillTags( const string& rawTags, map<string, string>& tagMap)
{
			boost::regex tagSearch( ";([^;=]+)(?:=([^;]+))?" );
			boost::match_results<std::string::const_iterator> tags;
   		string::const_iterator start, end;
   		start = rawTags.begin();
   		end = rawTags.end();

			while ( boost::regex_search( start, end, tags, tagSearch, boost::match_default ) )
			{
				if ( tags[2].matched )
					tagMap[ string( tags[1].first, tags[1].second ) ] = string( tags[2].first, tags[2].second );
				else
					tagMap[ string( tags[1].first, tags[1].second ) ] = "";
				start = tags[0].second;
			}
}

void Utility::ParseMessage( auto_ptr<SipMessage>& sipMessage, const string& data ) {
	boost::regex requestRegex( "^\\s*(\\w+)\\ssip:(.+?)\\sSIP/2.0$\\r\\n.*" );
	boost::regex responseRegex( "^\\s*SIP/2.0\\s(\\d{3})\\s(.*?)\\r\\n.*" );
	boost::cmatch matches;

	try {
		if ( boost::regex_match( data.c_str(), matches, requestRegex) ) {
			sipMessage.reset( new SipRequest( data ) );
		}
		else if ( boost::regex_match( data.c_str(), matches, responseRegex) ) {
			sipMessage.reset( new SipResponse( data ) );
		}
		else
			throw SipMessageException( "SIP message not parseable" );
	}
	catch ( SipRequestException& e ) {
		throw SipMessageException( string( "Invalid request:\n" ) + e.what() );
	}
	catch ( SipResponseException& e ) {
		throw SipMessageException( string( "Invalid response:\n" ) + e.what() );
	}
	catch ( SipMessageException& e ) {
		throw SipMessageException( string( "Invalid SIP message:\n" ) + e.what() );
	}
}
};//namespace Sip
