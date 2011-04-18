#include "Via.hpp"
#include <boost/regex.hpp>
#include <sstream>
namespace Sip {
Via::Via ( const SipHeaderValue& shv ) throw ( ViaException )
	: has_port( false ), has_host( false ), has_transportProtocol( false ), m_rfc3261compliant( false ), has_branch( false )
{
	ParseFromSHV( shv );
}

Via::Via ( const string& viaString ) throw ( ViaException )
{
	try {
		SipHeaderValue shvForVia( viaString );
		ParseFromSHV( shvForVia );
	}
	catch( SipHeaderValueException& e ) {
		throw ViaException( string( "Invalid 'Via' value: " ) + viaString + " (From exception: " + e.what() + ")" );
	}
}

Via::Via()
	: has_port( false ), has_host( false ), has_transportProtocol( false ), m_rfc3261compliant( false ), has_branch( false )
{}

int	Via::Port() const throw( ViaException )
{
	if ( this->has_port )
		return m_port;
	else
		throw ViaException( "Port not set." );
}

const string& Via::Host() const throw( ViaException )
{
	if ( this->has_host )
		return m_host;
	else
		throw ViaException( "Host not set." );
}

TRANSPORT_PROTOCOL	Via::TransportProtocol() const throw( ViaException )
{
	if ( this->has_transportProtocol )
		return m_transportProtocol;
	else
		throw ViaException( "Transport protocol not set." );
}

bool	Via::RFC3261Compliant() const throw()
{
	return m_rfc3261compliant;
}

const string& Via::Branch() const throw( ViaException )
{
	if ( this->has_branch )
		return m_branch;
	else
		throw ViaException( "Branch not set." );
}

bool Via::HasPort() const throw()
{
	return this->has_port;
}

bool Via::HasHost() const throw()
{
	return this->has_host;
}

bool Via::HasTransportProtocol() const throw()
{
	return this->has_transportProtocol;
}

bool Sip::Via::HasBranch() const throw()
{
	return has_branch;
}

string Sip::Via::ToString() const 
{
	std::ostringstream asString;
	asString << "SIP/2.0/";
	if ( has_transportProtocol ) {
		string transportProtocolAsString( TransportProtocolTypes.ReverseGet( m_transportProtocol ) );
		transform( transportProtocolAsString.begin(), transportProtocolAsString.end(), transportProtocolAsString.begin(), (int(*)(int))toupper );
		asString << transportProtocolAsString;
	}
	else
		asString << "UDP";
	asString << ' ';
	
	if ( has_host ) {
		asString << m_host;
		
		if ( has_port )
			asString << ':' << m_port;
	}
	else
		throw ViaException( "Via::ToString() : No host!" );

	return asString.str();
}

SipHeaderValue Via::ToSHV() {
	SipHeaderValue shv( this->ToString() );	

	if ( HasBranch() ) 
		shv.AddTag( "branch", Branch() );

	return shv;
}

void Via::ParseFromSHV( const SipHeaderValue& shv )
{
	//                                                     1                          2         3
	//                                    [Protocol                        ]   [Host       ][Opt. Port ]
	boost::regex viaExpression( "^SIP/2.0/((?:UDP)|(?:TCP)|(?:TLS)|(?:SCTP))\\s([^:\\?<>;]+)(?::(\\d+))?.*$" );
	boost::cmatch matches;

	try
	{

		if ( ! shv.HasTag( "branch" ) )
		{
			//throw ViaException( "Via does not have a branch tag." ); //We should do this; ...Via header field MUST contain a branch parameter... ( RFC 3261:8.1.1.7 Para. 2 )
			//cerr << "Warning: 'Via' without branch tag: " << shv.Value() << endl;
		}
		else if ( shv.Tags().find( "branch" )->second.find( "z9hG4bK" ) == 0  ) //Cannot use [], discards cv
		{
			m_rfc3261compliant = true;
			has_branch = true;
			m_branch = shv.Tags().find( "branch" )->second;
		}

		if ( boost::regex_match( shv.Value().c_str(), matches, viaExpression ) )
		{
			string transportProtocolAsString( matches[1].first, matches[1].second );
			try
			{
				m_transportProtocol = TransportProtocolTypes.GetCase( transportProtocolAsString );
				this->has_transportProtocol = true;
			}
			catch ( LookupTableException& e )
			{
				throw ViaException( string( "Uknown protocol specified in Via: " ) + string( matches[1].first, matches[1].second ) );
			}

			m_host = string( matches[2].first, matches[2].second );
			this->has_host = true;

			if ( matches[3].matched ) //Port?
			{
				m_port = atoi( string( matches[3].first, matches[3].second ).c_str() );
				this->has_port = true;
			}
		}
		else
			throw ViaException( string( "Invalid 'Via' value: " ) + shv.Value() );
	}
	catch( SipHeaderValueException& e )
	{
		throw ViaException( string( "Invalid 'Via' value: " ) + shv.Value() + " (From exception: " + e.what() + ")" );
	}
}
}; //namespace Sip
