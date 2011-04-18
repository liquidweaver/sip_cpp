#include "CSeq.hpp"
#include <boost/regex.hpp>
#include <sstream>

namespace Sip {
CSeq::CSeq( const SipHeaderValue& srhv ) throw( CSeqException )
{
	this->ParseCSeq( srhv.Value() );
}

CSeq::CSeq ( int sequence, SipRequest::REQUEST_METHOD rm ) throw( CSeqException )
{
	m_requestMethod = rm;
	m_sequence = sequence;
	try
	{
		m_requestMethodString = RequestTypes.ReverseGet( rm );
	}
	catch ( LookupTableException& e )
	{
		throw CSeqException( "CSeq::CSeq - Invalid request method" );
	}
}

CSeq::CSeq()
{
	//Stub
}

string CSeq::ToString() const throw()
{
	std::ostringstream cseqAsStringBuilder;

	string upperRequestMethodString = this->m_requestMethodString;	//because some phones can be picky...
	transform( upperRequestMethodString.begin(), upperRequestMethodString.end(), upperRequestMethodString.begin(), (int(*)(int))toupper ); //go uppercase
	cseqAsStringBuilder << m_sequence << ' ' << upperRequestMethodString;

	return cseqAsStringBuilder.str();
}

void CSeq::ParseCSeq( const string& rawValue ) throw( CSeqException )
{
	//								  [seq]    [RM]
	boost::regex expression( "(.*?)\\s+(.*)" );

	boost::cmatch matches;

	if ( boost::regex_match( rawValue.c_str(), matches, expression ) )
	{
		m_sequence = atoi( string( matches[1].first, matches[1].second ).c_str() );
		if (m_sequence == 0 || m_sequence < 0 ) //Or just <1...
			throw CSeqException( string( "Invalid CSeq: " ) + rawValue );

		m_requestMethodString = string( matches[2].first, matches[2].second );
		try
		{
			m_requestMethod = RequestTypes.GetCase( m_requestMethodString );
		}
		catch ( LookupTableException& e )
		{
			throw CSeqException( string( "Request method not supported: " ) + m_requestMethodString );
		}
	}
	else
		throw CSeqException( string( "Invalid CSeq: " ) + rawValue );
}

int 		CSeq::Sequence() const throw()
{
	return m_sequence;
}

SipRequest::REQUEST_METHOD 	CSeq::RequestMethod() const throw()
{
	return m_requestMethod;
}

string								CSeq::RequestMethodAsString() const throw()
{
	return RequestTypes.ReverseGet( m_requestMethod );	
}

CSeq&									CSeq::Increment() throw()
{
	m_sequence++;

	return *this;
}
}; //namespace Sip
