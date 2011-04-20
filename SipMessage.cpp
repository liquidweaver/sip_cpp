#include "SipMessage.hpp"
#include "SipDefines.hpp"
#include "SipUtility.hpp"
#include <sstream>
#include <boost/regex.hpp>
#include <queue>
using std::string;
using std::ostringstream;
using std::queue;
namespace Sip {


string SipMessage::ToString() const
{
	ostringstream stream;

	//TODO: Add some sanity checking while creating stream
	//TODO: Maybe explicitly process via's first?
	for ( vector<SipHeader>::const_iterator header = GetAllHeaders().begin(); header != GetAllHeaders().end(); ++header )
	{
		if ( strcasecmp( header->header_name.c_str(), "content-length" ) == 0 ) //This is calculated seperately and needs to be processed at the end
			continue;
		else if ( strcasecmp( header->header_name.c_str(), "via" ) == 0 ) //We handle via seperately because order is important, and we don't comma seperate multiple values, we put them on seperate lines
		{
			for ( vector<SipHeaderValue>::const_iterator value = header->shvs.begin(); value != header->shvs.end(); ++value  )
			{
				stream << header->header_name << ": " << value->Value();

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
			stream << header->header_name << ": ";
			bool firstValue = true;

			for ( vector<SipHeaderValue>::const_iterator value = header->shvs.begin(); value != header->shvs.end(); ++value  )
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
		stream << "Content-Length: " << GetMessageBody().length() << "\r\n\r\n";
		stream << GetMessageBody();
	}
	else
	{
		stream << "Content-Length: 0" << "\r\n\r\n";
	}

	return stream.str();
}

const string& SipMessage::GetOriginalRawMessage() const
{
	return this->rawMessage;
}

const vector<SipHeaderValue>& SipMessage::GetHeaderValues( const string& headerName ) const throw( SipMessageException )
{
	vector<SipHeader>::const_iterator it;

	it = std::find( m_headers.begin(), m_headers.end(), headerName );
	if ( it == m_headers.end() )
		throw SipMessageException( string( "Header not present: " ) + headerName );
	else
		return it->shvs;
}

const vector<SipHeader>& SipMessage::GetAllHeaders() const throw()
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

string& SipMessage::ModifyMessageBody() throw ( SipMessageException ) {
	if ( ! m_hasBody )
		throw SipMessageException( string( "ModifyMessageBody: Message has no body" ) + rawMessage  );
	else
		return this->messageBody;
}

bool SipMessage::HasHeader( const string& headerName ) const throw()
{
	return ( std::find( m_headers.begin(), m_headers.end(), headerName ) != m_headers.end() );
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
		SetHeader( "content-type",  headerValues );

		headerValues.clear();
		headerValues.push_back( SipHeaderValue(  lengthAsStringBuilder.str() ) );
		SetHeader( "content-length", headerValues );

		messageBody = body;
		m_hasBody = true;
}

vector<SipHeaderValue>& SipMessage::ModifyHeader( const string& headerName) 
{
	vector<SipHeader>::iterator header = std::find( m_headers.begin(), m_headers.end(), headerName );
	if ( header == m_headers.end() ) {
		m_headers.push_back( SipHeader() );
		header = m_headers.end() - 1;
		header->header_name = headerName;
	}
	return header->shvs;
}

void SipMessage::SetHeader( const string& headerName, const vector<SipHeaderValue>& values ) throw()
{
	vector<SipHeader>::iterator header = std::find( m_headers.begin(), m_headers.end(), headerName );
	if ( header == m_headers.end() ) {
		m_headers.push_back( SipHeader() );
		header = m_headers.end() - 1;
		header->header_name = headerName;
	}
	header->shvs = values;
}

void SipMessage::SetHeader( const string& headerName, const string& value ) throw()
{
	vector<SipHeaderValue> valueVector;
	valueVector.push_back( SipHeaderValue( value ) );

	SetHeader( headerName, valueVector );
}

void SipMessage::SetHeader( const string& headerName, const SipHeaderValue& value ) throw()
{
	vector<SipHeaderValue> valueVector;
	valueVector.push_back( value );

	SetHeader( headerName, valueVector );
}

void SipMessage::PushHeader( const string& headerName, const vector<SipHeaderValue>& values ) throw()
{
	vector<SipHeader>::iterator header = std::find( m_headers.begin(), m_headers.end(), headerName );
	if ( header == m_headers.end() ) {
		m_headers.push_back( SipHeader() );
		header = m_headers.end() - 1;
		header->header_name = headerName;
		header->shvs = values;
	}
	else {
		header->shvs.insert( header->shvs.end(), values.begin(), values.end() );
	}
}

void SipMessage::PushHeader( const string& headerName, const string& value ) throw()
{
	vector<SipHeaderValue> valueVector;
	valueVector.push_back( SipHeaderValue( value ) );

	PushHeader( headerName, valueVector );
}

void SipMessage::PushHeader( const string& headerName, const SipHeaderValue& value ) throw()
{
	vector<SipHeaderValue> valueVector;
	valueVector.push_back( value );

	PushHeader( headerName, valueVector );
}

void SipMessage::DeleteHeader( const string& headerName ) throw() {
	vector<SipHeader>::iterator header = std::find( m_headers.begin(), m_headers.end(), headerName );
	if ( header != m_headers.end() ) {
		m_headers.erase( header );
	}
}

string SipMessage::MassageHeaderKey( string headerName ) const throw()
{
	//transform(headerName.begin(), headerName.end(), headerName.begin(), (int(*)(int))tolower ); //Go lowercase
	if ( HeaderConversions.HasKey( headerName ) )
		headerName = HeaderConversions.GetCase( headerName );

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
		contentLength = atoi( GetHeaderValues("content-length")[0].Value().c_str() );

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
	using Utility::FillTags;
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
				throw SipMessageException( string( "Invalid data for (URI?) header: " ) + headerName );

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
			PushHeader( headerName, SipHeaderValue( value, tagMap ) );
		}
		else
			PushHeader( headerName, SipHeaderValue( value ) );
	}
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
// UTILITY
// 

}; //namespace Sip
