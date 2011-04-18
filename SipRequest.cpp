#include "SipRequest.hpp"
#include <boost/regex.hpp>
#include <sstream>
#include "CSeq.hpp"
using std::ostringstream;
namespace Sip {

SipRequest::SipRequest( const string& rawRequestData ) throw( SipMessageException, SipRequestException ) : SipMessage( MT_REQUEST )
{
	boost::regex requestExpression( "^\\s*(\\w+)\\s(sips?:.+?)\\sSIP/2.0\\r\\n" ); //Matches request line 1
	boost::match_results<std::string::const_iterator> regResults;
	string::const_iterator start, end;

	//DEBUGGING
	//cerr << rawRequestData << endl;

	start = rawRequestData.begin();
	end = rawRequestData.end();

	//Read request header; check version is 2.0
	this->rawMessage = rawRequestData;
	if ( boost::regex_search( start, end, regResults, requestExpression, boost::match_default ) == false )
		throw SipRequestException( string( "Invalid request\nRequest:\n\t" ) + rawRequestData  );
	//TODO: Grap request and host (sanity check host is this one)
	string requestMethodString = string( regResults[1].first, regResults[1].second );
	try
	{
		m_requestURI = string( regResults[2].first, regResults[2].second );
		this->requestMethod = RequestTypes.GetCase( requestMethodString );
	}
	catch ( LookupTableException& e )
	{
		throw SipRequestException( string( "Unsupported Request Method: " ) + requestMethodString );
	}
	catch ( URIException& e )
	{
		throw SipRequestException( e.what() );
	}
	start = regResults[0].second; //Ok, ready for headers

	ProcessSipMessage( start, end );

	//Throw exception if To, From, CSeq, Call-ID, Max-Forwards, Contact and Via are not all there
	if ( 	! this->HasHeader( "to" ) ||
				 ! this->HasHeader( "from" ) ||
				 ! this->HasHeader( "cseq" ) ||
				 ! this->HasHeader( "call-id" ) ||
/*			! this->HasHeader( "max-forwards" ) || */ //Ambiguous requirement; MUST in RFC3261:8.1.1, but same RFC says it's optional elsewhere
				 ! this->HasHeader( "via" ) )
		throw SipRequestException( "Invalid SIP request recieved, critical headers missing." );

	if ( this->requestMethod == REQUEST_METHOD_INVITE && !GetHeaderValues( "from" )[0].HasTag( "tag" ) )
	{
		throw SipRequestException( "'from' does not have a tag and request method is INVITE" ); //This should be an error: RFC 3261:8.1.1.3, Para. 4
		//cerr << "Warning: 'from' with no tag: " << m_headers[ "from" ][0].Value() << endl;
	}

	//CSeq method must match request method
	if ( static_cast<CSeq>(GetHeaderValues( "cseq" )[0]).RequestMethod() != this->requestMethod )
		throw SipRequestException( "CSeq request method doesn't match request method of request." );


}

SipRequest::SipRequest() throw()
	: SipMessage( MT_REQUEST ), requestMethod( SipRequest::REQUEST_METHOD_BYE )
{ }

SipRequest::SipRequest( SipRequest::REQUEST_METHOD rm )
	: SipMessage( MT_REQUEST ), requestMethod ( rm )
{ }

SipRequest::SipRequest( const SipRequest& rhs ) : SipMessage( MT_REQUEST )
{
	this->rawMessage = rhs.rawMessage;

	this->requestMethod = rhs.requestMethod;
	m_requestURI = rhs.m_requestURI;

	m_headers = rhs.m_headers;
	m_hasBody = rhs.m_hasBody;
	this->messageBody = rhs.messageBody;

}

SipRequest::REQUEST_METHOD SipRequest::RequestMethod() const throw()
{
	return this->requestMethod;
}


const URI& SipRequest::RequestURI() const throw()
{
	return m_requestURI;
}

void SipRequest::SetRequestURI( const URI& uri ) throw()
{
	m_requestURI = uri;
}

void SipRequest::SetRequestMethod( const SipRequest::REQUEST_METHOD rm ) throw()
{
	this->requestMethod = rm;
	if ( this->HasHeader( "cseq" ) )
	{
		ostringstream cseqBuilder;
		string requestMethod = RequestTypes.ReverseGet( rm );
		transform( requestMethod.begin(), requestMethod.end(), requestMethod.begin(), (int(*)(int))toupper ); //Go uppercase
		cseqBuilder << CSeq( GetHeaderValues( "cseq" )[0] ).Sequence()
						<< ' '
						<< requestMethod;

		SetHeader( "cseq", cseqBuilder.str() );
	}
}

string SipRequest::ToString() const
{
	ostringstream stream;
	string requestMethod = RequestTypes.ReverseGet( RequestMethod() );

	transform(requestMethod.begin(), requestMethod.end(), requestMethod.begin(), (int(*)(int))toupper ); //Go uppercase
	try
	{

		stream << requestMethod << " " << m_requestURI.Protocol() << ":";

		if ( this->RequestURI().HasUser() )
			  stream << m_requestURI.User() << "@";

		stream << m_requestURI.Host();

		if ( m_requestURI.HasPort() )
				stream << ':' << m_requestURI.Port();

		for ( map<string, string>::const_iterator params = RequestURI().URIParameters().begin(); params != RequestURI().URIParameters().end(); ++params )
		{
			stream << ";" << params->first;
			if ( params->second != "" )
				stream << "=" << params->second;
		}
	}
	catch ( URIException& e )
	{
		throw SipRequestException( string( "Cannot create Request line from URI: " ) + e.what() );
	}

	stream << " SIP/2.0" << "\r\n" << SipMessage::ToString();
	return stream.str(); // must return stream
}

}; //namespace Sip
