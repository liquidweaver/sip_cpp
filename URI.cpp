#include "URI.hpp"
#include "Via.hpp"
#include "SipUtility.hpp"
#include <boost/regex.hpp>
#include <sstream>

namespace Sip {

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
	std::ostringstream uriAsStringBuilder;
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
	std::ostringstream uriStringBuilder;

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

	if ( this->m_URIParameters.size() > 0 ) {
		uriStringBuilder << ';';

		for ( map<string, string>::const_iterator param = m_URIParameters.begin();
				param != m_URIParameters.end(); ++param ) {
			uriStringBuilder << param->first << '=' << param->second;
		}
	}

	if ( this->has_URIHeaders )
		uriStringBuilder << "?" << m_URIHeaders;
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
	//TODO: nakedURI MUST NOT MATCH SOMETHING WITH BRACKETS
	//TODO: match either host[:port] or user@host[:port] rather than current scheme for part before URI parameters

	//                                      1                         2          3           4        5                     6                   7
	//                           [Optional Name           ]     [Prot] [Opt. User     ][Host    ][Opt. Port ][Opt.URI tags ][Opt. hdrs ]
	boost::regex bracketedURI( "^((?:\".*?\")|(?:[^<\"]+))?\\s*<(\\w+):(?:([^<>@;\\?]+)@)?([^<>;:\\?]+)(?::(\\d+))?((?:;[^;?>]+)*)(?:\\?(.+))?>$" );

	//                               1         2          3         4            5            6
	//								 [Proto  ] [Opt. User   ][Host    ][Opt.Port  ][Opt.URI tags][Opt. hdrs ]
	boost::regex nakedURI( "^(^\\w+):(?:([^<>@;]+)@)?([^<>;:\\?]+)(?::(\\d+))?((?:;[^;?]+)*)(?:\\?(.+))?$" );
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
						Utility::FillTags( rawTags, m_URIParameters);
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
						Utility::FillTags( rawTags, m_URIParameters);
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

}; //namespace Sip
