#include "SipResponse.hpp"
#include <boost/regex.hpp>
#include <sstream>

using std::ostringstream;
namespace Sip {

SipResponse::SipResponse( const int statusCode, const string& reasonPhrase, const SipRequest& request ) throw( SipResponseException )
	: SipMessage( MT_RESPONSE ), m_statusCode( statusCode ), m_reasonPhrase( reasonPhrase )
{ //See RFC 3261, 8.2.6.2
	try
	{
		SetHeader( "via", request.GetHeaderValues( "via" ) );
		SetHeader( "from", request.GetHeaderValues( "from" ) );
		SetHeader( "call-id", request.GetHeaderValues( "call-id" ) );
		SetHeader( "cseq", request.GetHeaderValues( "cseq" ) );
		SetHeader( "to", request.GetHeaderValues( "to" ) );
	}
	catch( SipRequestException&e )
	{
		throw SipResponseException( "Malformed SIP request; cannot form response." );
	}

}

SipResponse::SipResponse( const int statusCode, const string& reasonPhrase )
	: SipMessage( MT_RESPONSE ), m_statusCode( statusCode ), m_reasonPhrase( reasonPhrase )
{ }

SipResponse::SipResponse( const string& rawResponseData ) throw ( SipResponseException ) : SipMessage( MT_RESPONSE )
{
	boost::regex responseRegex( "^\\s*SIP/2.0\\s(\\d{3})\\s(.*?)\\r\\n" ); //Matches request line 1
	boost::match_results<std::string::const_iterator> regResults;
	string::const_iterator start, end;

	start = rawResponseData.begin();
	end = rawResponseData.end();

	//Read response header; check version is 2.0
	this->rawMessage = rawResponseData;
	if ( boost::regex_search( start, end, regResults, responseRegex, boost::match_default ) == false )
		throw SipResponseException( string( "Invalid response\nResponse:\n\t" ) + rawResponseData  );
	//TODO: Grap response and host (sanity check reponse is valid from know requests that haven't timed out)
	m_statusCode = atoi( string( regResults[1].first, regResults[1].second ).c_str() );

	m_reasonPhrase = string( regResults[2].first, regResults[2].second );

	start = regResults[0].second; //Ok, ready for headers

	ProcessSipMessage( start, end );
}

int SipResponse::StatusCode( ) const throw()
{
	return m_statusCode;
}

const string& SipResponse::ReasonPhrase() const throw()
{
	return m_reasonPhrase;
}

void SipResponse::SetStatusCode( int newStatusCode )
{
	m_statusCode = newStatusCode;
}
void SipResponse::SetReasonPhrase ( const string& newReasonPhrase )
{
	m_reasonPhrase = newReasonPhrase;
}

string SipResponse::ToString() const
{
		ostringstream stream;
		stream << "SIP/2.0 " << StatusCode() << " " << ReasonPhrase() << "\r\n" << SipMessage::ToString();

		return stream.str();
}

}; //namespace Sip
