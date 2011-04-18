#include "SipHeaderValue.hpp"
#include <boost/regex.hpp>
#include "SipUtility.hpp"
#include "Via.hpp"
#include <sstream>

namespace Sip {

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
	using Utility::FillTags;

	boost::regex semiURICheck( "^((?:\".*?\")|(?:[^<\"]+))?\\s*<" );
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
		FillTags( rawTags, m_tags );
		m_hasTags = true;
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
	using std::ostringstream;

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
}; //namespace Sip
