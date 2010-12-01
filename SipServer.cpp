/****************************************************************************
*   Copyright (C) 2009 by Joshua weaver									*
*   joshuaweaver@gmail.com													*
*																			*
*   All rights reserved.													*
*																			*
*   Redistribution and use in source and binary forms, with or without		*
*	modification, are permitted provided that the following conditions are 	*
*	met:																	*
*		* Redistributions of source code must retain the above copyright 	*
*		  notice, this list of conditions and the following disclaimer.		*
*		* Redistributions in binary form must reproduce the above copyright	*
*		  notice, this list of conditions and the following disclaimer in 	*
*		  the documentation and/or other materials provided with the 		*
*		  distribution.														*
*     * Neither the name of the <ORGANIZATION> nor the names of its 		*
*		 contributors may be used to endorse or promote products derived 	*
*		 from this software without specific prior written permission.		*
*																			*
*	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS		*
*	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT		*
*	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR	*
*	A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 	*
*	OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 	*
*	SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 		*
*	LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 	*
*	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY	*
*	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 	*
*	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE	*
*	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.	*
***************************************************************************/

#include "SipServer.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <boost/regex.hpp>
#include <cctype> //for to(upper|lower) transform
#include <sstream>
#include <fcntl.h> //O_NONBLOCK and friends


//DEBUGGING
#include <iostream>

using namespace std;
namespace Sip
{
//
//Utility functions
//
/**
 * Generated a randoms string.
 * @warning Not cryptographically secure, should not be used for crypto
 * @param length The length of the string
 * @return A string of random, printable characters
 */
string RandomString( int length )
{
	ostringstream output;
	for ( int i = 0; i < length; ++i )
	{
		char nextChar = ( rand() % 26 ) + 97;
		output << nextChar;
	}

	return output.str();
}


//
// Via
//
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
	stringstream asString;
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
			transform( transportProtocolAsString.begin(), transportProtocolAsString.end(), transportProtocolAsString.begin(), (int(*)(int))tolower );
			try
			{
				m_transportProtocol = TransportProtocolTypes.Get( transportProtocolAsString );
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
//
// URI
//
URI::URI( const SipHeaderValue& srhv ) throw( URIException )
	:  has_displayName( false ), has_protocol( false ), has_user( false ), has_host( false ),has_port( false ), has_URIHeaders( false )
{
	try
	{
		this->ParseURI( srhv.Value() );
	}
	catch( URIException& e )
	{
		throw;
	}
}

URI::URI( const string& uri ) throw ( URIException )
	: has_displayName( false ), has_protocol( false ), has_user( false ), has_host( false ),has_port( false ), has_URIHeaders( false )
{
	try
	{
		this->ParseURI( uri );
	}
	catch( URIException& e )
	{
		throw;
	}
}

URI::URI( const Via& via ) throw()
	: m_protocol( "sip" ), has_displayName( false ), has_protocol( true ), has_user( false ), has_host( false ),has_port( false ), has_URIHeaders( false )
{
	ostringstream uriAsStringBuilder;
	uriAsStringBuilder << m_protocol << ":";

	if ( via.HasHost() )
	{
		this->has_host = true;
		m_host = via.Host();
		uriAsStringBuilder << m_host;
	}

	if ( via.HasPort() )
	{
		this->has_port = true;
		m_port = via.Port();
		uriAsStringBuilder << ":" << m_port;
	}

	this->ParseURI( uriAsStringBuilder.str() );
}

URI::URI() throw ()
	: has_displayName( false ), has_protocol( false ), has_user( false ), has_host( false ),has_port( false ), has_URIHeaders( false )
{ }

void 	 URI::URIFromString( const string& uri ) throw ( URIException )
{
	try
	{
		this->ParseURI( uri );
	}
	catch( URIException& e )
	{
		throw;
	}
}

string URI::URIAsString() const throw()
{
	ostringstream uriStringBuilder;

	if ( this->has_displayName )
	{
		uriStringBuilder << "\"" << m_displayName << "\" ";
	}

	uriStringBuilder << "<";

	if ( this->has_protocol )
		uriStringBuilder << m_protocol << ":";

	if ( this->has_user )
		uriStringBuilder << m_user << "@";

	uriStringBuilder << this->Host();

	if ( this->has_port )
		uriStringBuilder << ":" << m_port;

	uriStringBuilder << ">";

	return uriStringBuilder.str();
}

string URI::DisplayName() const throw ( URIException )
{
	if ( this->has_displayName )
		return m_displayName;
	else
		throw URIException( "Display name not set." );
}

string URI::Protocol() const throw ( URIException )
{
	if ( this->has_protocol )
		return m_protocol;
	else
		throw URIException( "Protocol not set." );
}

string URI::User() const throw ( URIException )
{
	if ( this->has_user )
		return m_user;
	else
		throw URIException( "User not set." + this->URIAsString() );
}

string URI::Host() const throw ( URIException )
{
	if ( this->has_host )
		return m_host;
	else
		throw URIException( "Host not set." );
}

int	 URI::Port() const throw( URIException )
{
	if ( this->has_port )
		return m_port;
	else
		throw URIException( "Port not set." );
}

void URI::SetUser ( const string& theValue )
{
	m_user = theValue;
	has_user = true;
}

void URI::SetHost ( const string& value )
{
	m_host = value;
	has_host = true;
}

const map<string, string>& URI::URIParameters() const throw()
{
		return m_URIParameters;
}

map<string, string>& URI::ModifyURIParameters() throw()
{
		return m_URIParameters;
}

string URI::URIHeaders() const throw ( URIException )
{
	if ( this->has_URIHeaders )
		return m_URIHeaders;
	else
		throw URIException( "URI headers not set" );
}

bool URI::HasDisplayName() const throw ()
{
	return this->has_displayName;
}

bool URI::HasProtocol() const throw ()
{
	return this->has_protocol;
}

bool URI::HasUser() const throw ()
{
	return this->has_user;
}

bool URI::HasHost() const throw ()
{
	return this->has_host;
}

bool URI::HasPort() const throw()
{
	return this->has_port;
}

bool URI::HasURIHeaders() const throw()
{
	return this->has_URIHeaders;
}

ostream& operator<< ( ostream &stream, const URI& uri )
{
	stream << "URI:";
	//has_displayName( false ), has_protocol( false ), has_user( false ), has_host( false ),has_port( false ), has_URIParameters( false ), has_URIHeaders( false )
	if ( uri.has_displayName )
		stream << " [Display-name: " << uri.m_displayName << "]";
	if ( uri.has_protocol )
		stream << " [Protocol: " << uri.m_protocol << "]";
	if ( uri.has_user )
		stream << " [User: " << uri.m_user << "]";
	if ( uri.has_host )
		stream << " [Host: " << uri.m_host << "]";
	if ( uri.has_port )
		stream << " [Port: " << uri.m_port << "]";
	if ( uri.m_URIParameters.size() > 0 )
	{
		stream << " [URI Paramaters: ";
		for ( map<string, string>::const_iterator parameter = uri.m_URIParameters.begin(); parameter != uri.m_URIParameters.end(); ++parameter)
		{
			stream << "(" << parameter->first << "=" << parameter->second << ") ";
		}
		stream << uri.m_displayName << "]";
	}
	if ( uri.has_URIHeaders )
		stream << " [URI Headers: " << uri.m_URIHeaders << "]";

	return stream;
}

void URI::ParseURI( const string& uriAsString ) throw( URIException )
{
	//TODO: Make regex work intelligently with request URI's
	//TODO: nakedURI MUST NOT MATCH SOMETHING WITH BRACKETS
	//TODO: match either host[:port] or user@host[:port] rather than current scheme for part before URI parameters

	//                                      1                         2          3           4        5                     6                   7
	//                           [Optional Name           ]     [Prot] [Opt. User     ][Host    ][Opt. Port ][Opt.URI tags ][Opt. hdrs ]
	boost::regex bracketedURI( "^((?:\".*?\")|(?:[^<\"]+))?\\s*<(\\w+):(?:([^<>@;]+)@)?([^<>;:]+)(?::(\\d+))?((?:;[^;?>]+)*)(?:\\?(.+))?>$" );

	//                               1         2             3           4                5                        6
	//								 [Proto  ] [Opt. User   ][Host    ][Opt.Port  ][Opt.URI tags][Opt. hdrs ]
	boost::regex nakedURI( "^(^\\w+):(?:([^<>@;]+)@)?([^<>;:]+)(?::(\\d+))?((?:;[^;?]+)*)(?:\\?(.+))?$" );
	boost::cmatch matches;

	if ( boost::regex_match( uriAsString.c_str(), matches, bracketedURI ) ) //Looks like a bracketed URI... might have a display name
	{
		string 	rawTags;

		for ( uint sub = 1; sub < matches.size(); ++sub )
		{
			switch ( sub )
			{
				case 1:	//Optional Name
					if ( matches[sub].matched )
					{
						boost::regex quotes( "\"" );
						string displayName = string( matches[sub].first, matches[sub].second );
						m_displayName = boost::regex_replace( displayName, quotes, "" );
						this->has_displayName = true;
					}
					break;
				case 2:	//Protocol
					m_protocol = string ( matches[sub].first, matches[sub].second );
					this->has_protocol = true;
					break;
				case 3:	//Optional User
					if ( matches[sub].matched )
					{
						m_user = string ( matches[sub].first, matches[sub].second );
						this->has_user = true;
					}
					break;
				case 4:	//Host
					m_host = string( matches[sub].first, matches[sub].second ) ;
					this->has_host = true;
					break;
				case 5:	//Optional host port
					if ( matches[sub].matched )
					{
						m_port = atoi( string( matches[sub].first, matches[sub].second ).c_str() );
						this->has_port = true;
					}
					break;
				case 6:	//Optional URI tags
					rawTags = string( matches[sub].first, matches[sub].second );
					if ( rawTags != "" )
					{
						SipRequest::FillTags( rawTags, m_URIParameters);
					}
					break;
				case 7:	//Optional URI headers
					if ( matches[sub].matched )
					{
						m_URIHeaders = string( matches[sub].first, matches[sub].second ) ;
						this->has_URIHeaders = true;
					}
					break;
			}
		}
	}
	else if ( boost::regex_match( uriAsString.c_str(), matches, nakedURI ) ) //Ok, this must be a non-bracketed URI
	{
		string 	rawTags;

		for ( uint sub = 1; sub < matches.size(); ++sub )
		{
			switch ( sub )
			{
				case 1:	//Protocol
					m_protocol = string ( matches[sub].first, matches[sub].second );
					this->has_protocol = true;
					break;
				case 2:	//Optional User
					if ( matches[sub].matched )
					{
						m_user = string ( matches[sub].first, matches[sub].second );
						this->has_user = true;
					}
					break;
				case 3:	//Host
					m_host = string( matches[sub].first, matches[sub].second ) ;
					this->has_host = true;
					break;
				case 4:	//Optional host port
					if ( matches[sub].matched )
					{
						m_port = atoi( string( matches[sub].first, matches[sub].second ).c_str() );
						this->has_port = true;
					}
					break;
				case 5:	//Optional URI tags
					rawTags = string( matches[sub].first, matches[sub].second );
					if ( rawTags != "" )
					{
						SipRequest::FillTags( rawTags, m_URIParameters);
					}
					break;
				case 6:	//Optional URI headers
					if ( matches[sub].matched )
					{
						m_URIHeaders = string( matches[sub].first, matches[sub].second ) ;
						this->has_URIHeaders = true;
					}
					break;
			}
		}
	}
	else
		throw URIException( string( "Invalid URI: " ) + uriAsString );
}

bool URI::IsURI( const string& uri ) throw()
{
	try
	{
		URI test( uri );
		return true;
	}
	catch ( URIException& e )
	{
		return false;
	}

}

bool URI::operator<( const URI& rhs ) const
{
 	return m_user < rhs.m_user && m_host < rhs.m_host;
}

bool URI::operator==( const URI& rhs ) const
{
	return m_user == rhs.m_user && m_host == rhs.m_host;
}


// bool URI::operator!=( const URI& rhs ) const
// {
// 	bool zomg = ( m_user != rhs.m_user || m_host != rhs.m_host );
// 	return zomg;
// }
//
// bool URI::operator==( const URI& rhs ) const
// {
// 	bool zomg = ( m_user == rhs.m_user && m_host == rhs.m_host );
// 	return zomg;
// }

//
// CSeq
//
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
	ostringstream cseqAsStringBuilder;

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
		transform( m_requestMethodString.begin(), m_requestMethodString.end(), m_requestMethodString.begin(), (int(*)(int))toupper ); //Go lowercase
		try
		{
			m_requestMethod = RequestTypes.Get( m_requestMethodString );
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

//
// SipHeaderValue
//

SipHeaderValue::SipHeaderValue( const string& value,  map<string, string> tags) throw()
	: m_hasTags( true ), m_tags( tags )
{
		//Trim whitespace from value
		boost::regex trimWS( "^\\s+|\\s+$" );
		m_value = boost::regex_replace( value, trimWS, "" );
}
SipHeaderValue::SipHeaderValue( const string& rawValue ) throw()
	: m_hasTags( false ), m_value( rawValue )
{
	boost::regex semiURICheck( "^\\s*(?:\".*?\")?\\s*<" );
	//Used to match with one or more tags
	boost::regex tagsAreaMatch( "((?:;[^;\\?]+)*)(?:\\?.*)?$" );
	//Ok, check if there is a uri that might so we can ignore inner semicolons
	boost::cmatch matches;
	string rawTags;
	try
	{
		if ( boost::regex_search( rawValue.c_str(), matches, semiURICheck ) )
		{ //Ok, lets weed out the area within the <>
			//Find position of >. If not found, throw and exception.
			//grab from > to first semicolon or end, that will be 'value'
			//if a semicolon is found, everything including and after that will be raw tags.
			string::size_type rightBracketPosition = rawValue.find( '>', 0 );
			if ( rightBracketPosition == string::npos )
				throw 1; //Just set m_value to rawValue...

			if ( boost::regex_search( (char * ) (rawValue.c_str() + rightBracketPosition), matches, tagsAreaMatch ) && string( matches[1].first, matches[1].second ) != "" )
			{
				rawTags = string( matches[1].first, matches[1].second );
				m_value = string( rawValue.c_str(), matches[1].first );
			}
			else
			{ //No tags found, whole rawValue is just a value
				rawTags = "";
				m_value = rawValue;
			}
		}
		else
		{ //Should be safe to assume semis only used for tags
			if ( boost::regex_search( rawValue.c_str(), matches, tagsAreaMatch )  && string( matches[1].first, matches[1].second ) != "" )
			{
				rawTags = string( matches[1].first, matches[1].second );
				m_value  = string( rawValue.c_str(), matches[1].first );
			}
			else
			{
				rawTags = "";
				m_value  = rawValue;
			}
		}
	}
	catch (...)
	{
		m_value  = rawValue;
	}
	//Trim whitespace from value
	boost::regex trimWS( "^\\s+|\\s+$" );
	m_value = boost::regex_replace( m_value, trimWS, "" );

	// At this point, we need thave the value and rawTags seperate and cleaned up

	if ( rawTags != "" )
	{
		SipMessage::FillTags( rawTags, m_tags );
		m_hasTags = true;
	}
}

SipHeaderValue::SipHeaderValue ( const Via& via )
{
	m_value = via.ToString();

	if ( via.HasBranch() ) {
		m_hasTags = true;
		m_tags.insert( std::pair<string, string>( "branch", via.Branch() ) );
	}
}


const map<string, string>& SipHeaderValue::Tags() const throw( SipHeaderValueException )
{
	if ( !m_hasTags )
		throw SipHeaderValueException( string( "No tags available for header value '" ) + m_value + "'" );
	return m_tags;
}

const string& SipHeaderValue::GetTagValue( const string& tagName ) const throw( SipHeaderValueException )
{
	if ( !m_hasTags )
		throw SipHeaderValueException( string( "No tags available for header value '" ) + m_value + "'" );
	map<string, string>::const_iterator theValue = m_tags.find( tagName );
	if ( theValue == m_tags.end() )
		throw SipHeaderValueException( "Tag not found: " + tagName );
	else
		return theValue->second;
}

const string& SipHeaderValue::Value() const throw()
{
	return m_value;
}

void SipHeaderValue::SetValue( const string& newValue ) throw()
{
	m_value = newValue;
}

string SipHeaderValue::ToString() const throw()
{
	ostringstream shvAsStringBuilder;

	shvAsStringBuilder << Value();

	for ( map<string, string>::const_iterator aPair = m_tags.begin();
			aPair != m_tags.end();
			++aPair )
	{
		shvAsStringBuilder << ';' << aPair->first << '=' << aPair->second;
	}

	return shvAsStringBuilder.str();
}


bool SipHeaderValue::HasTags() const throw()
{
	return m_hasTags;
}

bool SipHeaderValue::HasTag( const string& tagName ) const throw()
{
	if ( m_tags.find( tagName ) == m_tags.end() )
		return false;
	else
		return true;
}

void SipHeaderValue::AddTag( const string& key, const string& value ) throw()
{
	m_tags[key] = value;
	m_hasTags = true;
}
//
// SipServer
//

SipServer::SipServer( int UDPPort ) throw( SipServerException )
	: m_udpPort ( UDPPort )
{

	bzero( &m_serverAddr, sizeof( m_serverAddr ) );

	m_sock = socket( AF_INET, SOCK_DGRAM, 0 );
	if ( m_sock == -1 )
		throw SipServerException( string( "Could not create socket: " ) + strerror( errno ), "" );

	int on = 1;
	if ( setsockopt ( m_sock, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on, sizeof ( on ) ) == -1 )
	{
		close( m_sock );
		throw SipServerException( string( "Could not set SO_REUSEADDR on socket: " ) + strerror( errno ) , "");
	}

	// Set non-blocking
	long arg = 0 ;
	if ( ( arg = fcntl ( m_sock, F_GETFL, NULL ) ) < 0 )
		throw SipServerException( "Could not get socket opts: " +  string(strerror ( errno )), "" );
	arg |= O_NONBLOCK;
	if ( fcntl ( m_sock, F_SETFL, arg ) < 0 )
		throw SipServerException( "Could not set socket non-blocking:  " +  string(strerror ( errno )), "" );

	sockaddr& serverAddrCast = ( sockaddr& ) m_serverAddr;

	m_serverAddr.sin_family = AF_INET;
	m_serverAddr.sin_port= htons( m_udpPort );
	m_serverAddr.sin_addr.s_addr = INADDR_ANY;

	int bindResult = bind( m_sock, &serverAddrCast, sizeof( m_serverAddr ) );

	if ( bindResult == -1 )
	{
		ostringstream errorBuilder;
		errorBuilder << "Could not bind to UDP port " << m_udpPort << ", error: " << strerror( errno );
		close( m_sock );
		throw SipServerException( errorBuilder.str() , "");
	}
}

string SipServer::Accept( auto_ptr<SipMessage>& sipMessage )
{
	//SELECT() crap
	fd_set myset;
	struct timeval tv;
	int selectResult;
	tv.tv_sec = 3; //Wait 3 seconds for a datagram
	tv.tv_usec = 0;
	FD_ZERO ( &myset );
	FD_SET ( m_sock, &myset );
	// END SELECT() crap

	selectResult = select ( m_sock + 1, &myset, NULL, NULL, &tv );
	if ( selectResult < 0 )
	{
		throw SipServerException( string( "Socket error waiting for SIP data : " ) + strerror( errno ), "" );
	}
	else if ( selectResult == 0 )
	{
		throw SipServerTimeout( "SipServer::Accept() timeout" );
	}
	else //We have waiting data, we're good to go
	{
		return SipUtility::GetMessage( sipMessage, m_sock );
	}

	return string( "" );
}

void SipServer::Respond( SipResponse& response ) const
{
	SipUtility::Respond( response );
}

void SipServer::Request( SipRequest& request ) const
{
	SipUtility::Request( request );
}

int SipServer::UDPPort() const throw()
{
	return m_udpPort;
}

static void CSVSeperate( const string& rawString, queue<string>& elements )
{
	bool inQuotes = false;
	string currentString;

	for ( uint32_t i=0; i < rawString.length(); ++i )
	{
		if ( rawString[i] == '"' )
			inQuotes = !inQuotes;

		if ( !inQuotes && rawString[i] == ',' )
		{
			elements.push( currentString );
			currentString = "";
		}
		else
			currentString += rawString[i];
	}

	if ( currentString != "" )
		elements.push( currentString );
}

//
// SipMessage
//

void SipMessage::FillTags( const string& rawTags, map<string, string>& tagMap)
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

string SipMessage::ToString() const
{
	ostringstream stream;

	//TODO: Add some sanity checking while creating stream
	//TODO: Maybe explicitly process via's first?
	for ( map< string, vector<SipHeaderValue> >::const_iterator header = GetAllHeaders().begin(); header != GetAllHeaders().end(); ++header )
	{
		if ( header->first == "content-length" ) //This is calculated seperately and needs to be processed at the end
			continue;
		else if ( header->first == "via" ) //We handle via seperately because order is important, and we don't comma seperate multiple values, we put them on seperate lines
		{
			for ( vector<SipHeaderValue>::const_iterator value = header->second.begin(); value != header->second.end(); ++value  )
			{
				stream << "via: " << value->Value();

				if ( value->HasTags() )
				{
					for( map<string, string>::const_iterator tag = value->Tags().begin(); tag != value->Tags().end(); ++tag )
					{
						stream << ";" << tag->first;
						if ( tag->second != "" ) //Maybe completely omit tag if no value?!
 							stream << "=" << tag->second;
					}
				}

				stream << "\r\n";
			}
		}
		else //Process all other headers, seperating values with a comma SP combo
		{
			stream << header->first << ": ";
			bool firstValue = true;

			for ( vector<SipHeaderValue>::const_iterator value = header->second.begin(); value != header->second.end(); ++value  )
			{
				if ( firstValue )
					firstValue = false;
				else
					stream << ", ";

				stream << value->Value();

				if ( value->HasTags() )
				{
					for( map<string, string>::const_iterator tag = value->Tags().begin(); tag != value->Tags().end(); ++tag )
					{
						stream << ";" << tag->first << "=" << tag->second;
					}
				}
			}
			stream << "\r\n";
		}

	}

	//Do content length header and body

	if ( HasMessageBody() )
	{
		stream << "content-length: " << GetMessageBody().length() << "\r\n\r\n";
		stream << GetMessageBody();
	}
	else
	{
		stream << "content-length: 0" << "\r\n\r\n";
	}

	return stream.str();
}

const string& SipMessage::GetOriginalRawMessage() const
{
	return this->rawMessage;
}

const vector<SipHeaderValue>& SipMessage::GetHeaderValues( const string& headerName ) const throw( SipMessageException )
{
	map<string, vector<SipHeaderValue> >::const_iterator it;

	it = m_headers.find( headerName );
	if ( it == m_headers.end() )
		throw SipRequestException( string( "Header not present: " ) + headerName );
	else
	{
		return it->second;
	}
}

const map< string, vector<SipHeaderValue> >& SipMessage::GetAllHeaders() const throw()
{
	return m_headers;
}


const string& SipMessage::GetMessageBody() const throw( SipMessageException )
{
	if ( ! m_hasBody )
		throw SipMessageException( string( "GetMessageBody: Message has no body" ) + rawMessage  );
	else
		return this->messageBody;
}

bool SipMessage::HasHeader( const string& headerName ) const throw()
{
	return ( m_headers.find( headerName ) != m_headers.end() );
}

bool SipMessage::HasMessageBody ( ) const throw()
{
	return m_hasBody;
}

void SipMessage::SetMessageBody ( const string& body, const string& rtpMap ) throw( SipMessageException )
{
		if ( body == "" || rtpMap == "" )
			throw SipMessageException( "SipMessage::SetMessageBody: Bad arguments. Both are required." );

		ostringstream lengthAsStringBuilder;

		lengthAsStringBuilder << body.length();

		vector<SipHeaderValue> headerValues;
		headerValues.push_back( SipHeaderValue( rtpMap ) );
		m_headers[ "content-type" ] = headerValues;

		headerValues.clear();
		headerValues.push_back( SipHeaderValue(  lengthAsStringBuilder.str() ) );
		m_headers[ "content-length" ] = headerValues;

		messageBody = body;
		m_hasBody = true;
}

vector<SipHeaderValue>& SipMessage::ModifyHeader( const string& headerName) throw ( SipMessageException )
{
	if ( m_headers.find( headerName ) == m_headers.end() )
		throw SipRequestException( "Header not present: " + headerName );
	else
		return m_headers[headerName];
}

void SipMessage::SetHeader( const string& headerName, const vector<SipHeaderValue>& values ) throw()
{
	m_headers[headerName] = values;
}

void SipMessage::SetHeader( const string& headerName, const string& value ) throw()
{
	vector<SipHeaderValue> valueVector;
	valueVector.push_back( SipHeaderValue( value ) );

	m_headers[headerName] = valueVector;
}

void SipMessage::SetHeader( const string& headerName, const SipHeaderValue& value ) throw()
{
	vector<SipHeaderValue> valueVector;
	valueVector.push_back( value );

	m_headers[headerName] = valueVector;
}

string SipMessage::MassageHeaderKey( string headerName ) const throw()
{
	transform(headerName.begin(), headerName.end(), headerName.begin(), (int(*)(int))tolower ); //Go lowercase
	try
	{	//Transform if it matches a short version
		headerName = HeaderConversions.Get( headerName );
	}
	catch (...) {}

	return headerName;
}

void SipMessage::ProcessSipMessage( string::const_iterator start, string::const_iterator end ) throw( SipMessageException )
{
	boost::match_results<std::string::const_iterator> regResults;
	boost::regex headerExpression( "([\\w\\-]+)[ ]*:[ ]*(.+?)(?:(?:\\r\\n\\r\\n)|(?:\\r\\n(?=\\w)))" ); //(?s) is inline dotall modifier
	//boost::regex headerExpression( "([^:]+)\\s*:\\s*(.+?\\r\\n(?:[ \\t]+.+?\\r\\n)*)" ); //Used to grab headers, including multiline headers.
	boost::regex contentExpression( "\\r\\n\\r\\n(.+)" );
	string key, rawValue, messageBody;
	if ( boost::regex_search( start, end, regResults, contentExpression, boost::match_single_line ) == true )
	{
		messageBody = string( regResults[1].first, regResults[1].second );
		end = regResults[1].first;
	}

	while ( boost::regex_search( start, end, regResults, headerExpression, boost::match_single_line ) == true )
	{

		key = MassageHeaderKey( string( regResults[1].first, regResults[1].second ) );
		rawValue = string( regResults[2].first, regResults[2].second );

		ProcessSipHeaderValues( key, rawValue );
		start = regResults[0].second;
	}
	int contentLength = 0;

	if ( this->HasHeader( "content-length" ) )
		contentLength = atoi( m_headers["content-length"][0].Value().c_str() );

	if ( contentLength > 0 ) 	//Process content
	{

		if ( messageBody.length() != (unsigned)contentLength )
			throw SipMessageException( "Content-length != actual body length...message corrupt" );

		m_hasBody = true;
		this->messageBody = messageBody;
	}
}

void SipMessage::ProcessSipHeaderValues( const string& headerName, string& rawString ) throw( SipMessageException )
{


	boost::regex ws( "\\r\\n" );
	//Remove all CRLF's
	rawString = boost::regex_replace( rawString, ws, "" );

	//Will be used later to see if we need special handling to ignore semicolons in a bracket 'fence' - <>
	boost::regex semiURICheck( "^\\s*(?:\".*?\")?\\s*<" );
	//Used to match with one or more tags
	boost::regex tagsAreaMatch( "((?:;[^;\\?]+)*)(?:\\?.*)?$" );
	boost::match_results<std::string::const_iterator> regResults;
	queue<string> elements;
	CSVSeperate( rawString, elements );
	
	while ( !elements.empty() )
	{
		string value, rawTags;
		string element = elements.front();
		elements.pop();

		//Ok, check if there is a uri that might so we can ignore inner semicolons
		boost::cmatch matches;

		if ( boost::regex_search( element.c_str(), matches, semiURICheck ) )
		{ //Ok, lets weed out the area within the <>
			//Find position of >. If not found, throw and exception.
			//grab from > to first semicolon or end, that will be 'value'
			//if a semicolon is found, everything including and after that will be raw tags.
			string::size_type rightBracketPosition = element.find( '>', 0 );
			if ( rightBracketPosition == string::npos )
				throw SipRequestException( string( "Invalid data for (URI?) header: " ) + headerName );

			if ( boost::regex_search( (char * ) (element.c_str() + rightBracketPosition), matches, tagsAreaMatch ) && string( matches[1].first, matches[1].second ) != "" )
			{
				rawTags = string( matches[1].first, matches[1].second );
				value = string( element.c_str(), matches[1].first );
			}
			else
			{ //No tags found, whole element is just a value
				rawTags = "";
				value = element;
			}
		}
		else
		{ //Should be safe to assume semis only used for tags
			if ( boost::regex_search( element.c_str(), matches, tagsAreaMatch )  && string( matches[1].first, matches[1].second ) != "" )
			{
				rawTags = string( matches[1].first, matches[1].second );
				value = string( element.c_str(), matches[1].first );
			}
			else
			{
				rawTags = "";
				value = element;
			}
		}

		//Trim whitespace from value
		boost::regex trimWS( "^\\s+|\\s+$" );
		value = boost::regex_replace( value, trimWS, "" );

		// At this point, we need thave the value and rawTags seperate and cleaned up

		if ( rawTags != "" )
		{
			map<string, string> tagMap;
			FillTags( rawTags, tagMap );
			m_headers[headerName].push_back( SipHeaderValue( value, tagMap ) );
		}
		else
			m_headers[headerName].push_back( SipHeaderValue( value ) );
	}
}

//
// SipRequest
//

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
		transform(requestMethodString.begin(), requestMethodString.end(), requestMethodString.begin(), (int(*)(int))toupper ); //Go lowercase
		this->requestMethod = RequestTypes.Get( requestMethodString );
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

	if ( this->requestMethod == REQUEST_METHOD_INVITE && !m_headers[ "from" ][0].HasTag( "tag" ) )
	{
		throw SipRequestException( "'from' does not have a tag and request method is INVITE" ); //This should be an error: RFC 3261:8.1.1.3, Para. 4
		//cerr << "Warning: 'from' with no tag: " << m_headers[ "from" ][0].Value() << endl;
	}

	//CSeq method must match request method
	if ( static_cast<CSeq>(m_headers["cseq"][0]).RequestMethod() != this->requestMethod )
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

    if ( rhs.HasHeader( "via" ) )
    	m_headers["via"] = rhs.GetHeaderValues("via");

    if ( rhs.HasHeader( "cseq" ) )
        m_headers["cseq"] = rhs.GetHeaderValues("cseq");

    if ( rhs.HasHeader( "call-id" ) )
        m_headers["call-id"] = rhs.GetHeaderValues("call-id");

    if ( rhs.HasHeader( "to" ) )
        m_headers["to"] = rhs.GetHeaderValues("to");

    if ( rhs.HasHeader( "from" ) )
        m_headers["from"] = rhs.GetHeaderValues("from");

    if (rhs.HasHeader( "content-length" ) )
        m_headers["content-length"] = rhs.GetHeaderValues("content-length");

	if ( rhs.HasHeader( "contact" ) )
		m_headers["contact"] = rhs.GetHeaderValues("contact"); //May want to remove this to force proxying at some point

	if ( rhs.HasHeader( "content-type" ) )
		m_headers["content-type"] = rhs.GetHeaderValues("content-type");

	if ( rhs.HasHeader( "refer-to" ) )
		m_headers["refer-to"] = rhs.GetHeaderValues("refer-to");

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
		cseqBuilder << CSeq( m_headers["cseq"][0] ).Sequence()
						<< ' '
						<< requestMethod;

		m_headers["cseq"][0] = cseqBuilder.str();
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
		throw SipServerException( string( "Cannot create Request line from URI: " ) + e.what(), "" );
	}

	stream << " SIP/2.0" << "\r\n" << SipMessage::ToString();
	return stream.str(); // must return stream
}

//
// SipResponse		void Respond( SipResponse& response ) const throw ( SipServerException );
//

SipResponse::SipResponse( const int statusCode, const string& reasonPhrase, const SipRequest& request ) throw( SipResponseException )
	: SipMessage( MT_RESPONSE ), m_statusCode( statusCode ), m_reasonPhrase( reasonPhrase )
{ //See RFC 3261, 8.2.6.2
	try
	{
		m_headers["via"] = request.GetHeaderValues( "via" );
		m_headers["from"] = request.GetHeaderValues( "from" );
		m_headers["call-id"] = request.GetHeaderValues( "call-id" );
		m_headers["cseq"] = request.GetHeaderValues( "cseq" );
		m_headers["to"] = request.GetHeaderValues( "to" );
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

//
// SipUtility
//
string SipUtility::GetMessage( auto_ptr<SipMessage>& sipMessage, int sock )
{
	char buffer[ BUFFERSIZE ];
	boost::regex requestRegex( "^\\s*(\\w+)\\ssip:(.+?)\\sSIP/2.0$\\r\\n.*" );
	boost::regex responseRegex( "^\\s*SIP/2.0\\s(\\d{3})\\s(.*?)\\r\\n.*" );
	boost::cmatch matches;
	sockaddr_in clientAddr;
	bzero( &clientAddr, sizeof( clientAddr ) );
	sockaddr& clientAddrCast = ( sockaddr& ) clientAddr;
	
	int size = sizeof ( clientAddr );
	int received = recvfrom ( sock , buffer , BUFFERSIZE , 0, &clientAddrCast, ( socklen_t * ) &size );
	//cerr << "received: " << received << endl;
	if(received < 0)	//signal? errno will tell us
		throw SipServerException(string("No request found. ") + strerror(errno), "");

	string data( buffer, received );
	try
	{
		if ( boost::regex_match( data.c_str(), matches, requestRegex) )
		{
			//cout << ">>Request IN from " << inet_ntoa(clientAddr.sin_addr) << ":\n" << data << endl << endl;
			sipMessage.reset( new SipRequest( data ) );

		}
		else if ( boost::regex_match( data.c_str(), matches, responseRegex) )
		{
			//cout << ">>Response IN from " << inet_ntoa(clientAddr.sin_addr) << ":\n" << data << endl << endl;
			sipMessage.reset( new SipResponse( data ) );
		}
		else
		{
			throw SipServerException( string( "Invalid SIP message received from " ) + inet_ntoa( clientAddr.sin_addr ) , data );
		}
	}
	catch ( SipRequestException& e )
	{
		throw SipServerException( string( "Invalid request:\n" ) + e.what(), data );
	}
	catch ( SipResponseException& e )
	{
		throw SipServerException( string( "Invalid response:\n" ) + e.what(), data );
	}
	catch ( SipMessageException& e )
	{
		throw SipServerException( string( "Invalid SIP message:\n" ) + e.what(), data );
	}
		
	return string( inet_ntoa(clientAddr.sin_addr) );
}

void SipUtility::Respond( SipResponse& response )
{
	ostringstream message;
	string datagram;
	try
	{
		if ( response.HasHeader( "via" ) == false )
			 throw SipServerException( "No Via tag in response - response malformed.", response.rawMessage );
		URI recipient = Via( response.GetHeaderValues( "via" )[0] );

		//cout << "<<Response OUT to " << recipient.URIAsString() << ":\n" << datagram << endl << endl;
		DecrementForwards( response );

		SendSipMessage( response.ToString(), recipient );
	}
	catch ( ViaException& e )
	{
		throw SipServerException( e.what(), response.rawMessage );
	}
	catch ( SipHeaderValueException& e )
	{
		throw SipServerException( e.what(), response.rawMessage );
	}
}

void SipUtility::Request( SipRequest& request )
{
	ostringstream message;

	//cout << "<<Request OUT to " << recipient.URIAsString() << ":\n" << datagram << endl << endl;
	DecrementForwards( request );
	SendSipMessage( request.ToString(), request.RequestURI() );
}

void SipUtility::SendSipMessage( const string& rawMessage, const URI& recipient )
{
	if ( !recipient.HasHost() )
		throw SipServerException( "No host to send message to", rawMessage );

	string host = recipient.Host();
	int port = 5060;

	if ( recipient.Protocol() != "sip" )
		throw SipServerException( string( "Unsupported protocol: " ) + recipient.Protocol() , rawMessage );

	if ( recipient.HasPort() )
		port = recipient.Port();

	ostringstream portAsStream;
	portAsStream << port;
	//TODO: Support 'transport' tag, not just UDP
	struct addrinfo hints;
	struct addrinfo * result;
	struct addrinfo * res;
	int error, sock;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;    	// Allow only IPv4... shortsighted?
	hints.ai_socktype = SOCK_DGRAM; 	// Datagram socket
	hints.ai_flags = AI_PASSIVE;    	// For wildcard IP address
	hints.ai_protocol = 0;          // Any protocol


	error = getaddrinfo( host.c_str(), portAsStream.str().c_str(), &hints, &result);

	if ( error != 0 )
		throw SipServerException( string( "Could not resolve host: " ) + host, rawMessage );

	try
	{
		for (res = result; res != NULL; res = res->ai_next)
		{
			sock = socket( res->ai_family , res->ai_socktype, res->ai_protocol );
			if ( sock == -1 )
				continue;

			int sent = sendto ( sock , rawMessage.c_str() , rawMessage.length() , 0, res->ai_addr, res->ai_addrlen ) ;
			close( sock );

			if ( sent == -1 )
			{ //Uh-oh, we didn't send...
				ostringstream errorDescription;
				switch( errno )
				{
					case EMSGSIZE:
						throw SipServerException( "Could not send SIP message: too big", rawMessage );
						break;
					default:
						errorDescription << "Could not send SIP message: " << strerror( errno ) << '[' << errno << ']';
						throw SipServerException( errorDescription.str(), rawMessage );
				}
			}
		}
	}
	catch ( SipServerException& e )
	{
		freeaddrinfo( result );
		throw;
	}
	//Cleanup
	freeaddrinfo( result );
}

void SipUtility::DecrementForwards( SipMessage& mesg )
{
	if ( !mesg.HasHeader( "max-forwards" ) )
		return;

	int forwards = atoi( mesg.GetHeaderValues( "max-forwards" )[0].Value().c_str() );
	forwards--;

	if ( forwards < 0)
		throw SipServerException( "Cannot forward, Max-Forwards < 0", mesg.GetOriginalRawMessage() );
	else
	{
		stringstream newForwardsValue;

		newForwardsValue << forwards;
		mesg.ModifyHeader( "max-forwards" )[0].SetValue( newForwardsValue.str() );
	}

}

} //namespace Sip


