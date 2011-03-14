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

#ifndef SIPSERVER_H
#define SIPSERVER_H

#include <string>
#include <exception>
#include <map>
#include <vector>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>
#include <queue>
#include <memory>
#include "lookuptable.h"
#include <string.h> //strcasecmp

using namespace std;


namespace Sip
{

//In milliseconds, the basic T1 retry interval and other intervals ( RFC 3261, 17.1.1.1 )
#define SIP_T1 500
#define SIP_T2 SIP_T1 * 4
#define SIP_T4 4000

#define SIP_TB SIP_T1 * 64
#define SIP_TH SIP_T1 * 64
#define SIP_TD SIP_T1 * 64
#define SIP_TF SIP_T1 * 64
#define SIP_TK SIP_T4
#define SIP_TI SIP_T4
#define SIP_TJ SIP_T1 * 64


const int DEFAULT_UDP_LISTENPORT = 5060;
const int BUFFERSIZE = 2048; // Shouldn't need more than 1500 bytes for a UDP datagram, but whatever...

enum TRANSPORT_PROTOCOL
{
	TRANSPORT_PROTOCOL_UDP,
	TRANSPORT_PROTOCOL_TCP,
	TRANSPORT_PROTOCOL_TLS,
	TRANSPORT_PROTOCOL_SCTP
};

//
// Utility functions
//

string RandomString( int length );

//
// Exception Classes
//

/**
* \class SipServerException
* \brief standard exception class for SipServer
*/
class SipServerException : public exception
{
	public:
		SipServerException ( std::string what, std::string message ) throw()
			: m_s ( what ), m_m( message) {}
		~SipServerException () throw() {}

		virtual const char * what() const throw()  { return m_s.c_str(); }

		string originalMessage() throw() { return m_m; }

	private:

		string m_s, m_m;

};

/**
 * \class SipServerException
 * \brief standard exception class for SipServer
 */
class SipServerTimeout : public exception
{
	public:
		SipServerTimeout ( std::string what ) throw()
	: m_s ( what ) {}
		~SipServerTimeout () throw() {}

		virtual const char * what() const throw()  { return m_s.c_str(); }

	private:

		string m_s;

};

/**
* \class SipProtocolException
* \brief standard exception class for SipProtocol
*/
class SipProtocolException : public std::exception
{
	public:
		SipProtocolException ( std::string code, std::string description ) throw()
				: m_code ( code ), m_desc ( description ) {};
		~SipProtocolException () throw() {}

		std::string code() throw() { return m_code; }
		virtual const char * what() const throw() { return m_desc.c_str(); }

	private:

		std::string m_code;
		std::string m_desc;

};

/**
* \class SipMessageException
* \brief standard exception class for SipMessage
*/
class SipMessageException : public std::exception
{
	public:
		SipMessageException ( std::string what ) throw() : m_s ( what ) {};
		~SipMessageException () throw() {}

		const char * what() const throw() { return m_s.c_str(); }

	private:

		std::string m_s;
};

/**
* \class SipRequestException
* \brief standard exception class for SipRequest
*/
class SipRequestException : public SipMessageException
{
	public:
		SipRequestException ( std::string what ) throw() : SipMessageException ( what ) {};
};

/**
* \class SipResponseException
* \brief standard exception class for SipResponse
*/
class SipResponseException: public SipMessageException
{
	public:
		SipResponseException ( std::string what ) throw() : SipMessageException ( what ) {};
};

/**
* \class SipHeaderValueException
* \brief standard exception class for SipHeaderValue
*/
class SipHeaderValueException : public SipMessageException
{
	public:
		SipHeaderValueException ( std::string what ) throw() : SipMessageException ( what ) {};
};

/**
* \class URIException
* \brief standard exception class for URI
*/
class URIException : public SipHeaderValueException
{
	public:
		URIException ( std::string what ) throw() : SipHeaderValueException ( what ) {};

};

/**
* \class CSeqException
* \brief standard exception class for CSeq
*/
class CSeqException : public SipHeaderValueException
{
	public:
		CSeqException ( std::string what ) throw() : SipHeaderValueException ( what ) {};

};

/**
* \class ViaException
* \brief standard exception class for Via
*/
class ViaException : public SipHeaderValueException
{
	public:
		ViaException ( std::string what ) throw() : SipHeaderValueException ( what ) {};

};


class Via; //Forward declaration for SipHeaderValue-from-Via ctor
/**
* \class SipHeaderValue
* \brief A container for a single value from a SIP header.
*/
class SipHeaderValue
{
	public:
		SipHeaderValue ( const string& value,  map<string, string> tags ) throw();
		SipHeaderValue ( const string& value ) throw();
		SipHeaderValue ( const Via& via ); 
		/**
		 *     Allows you to access this value tags, if applicable
		 * @return A const reference to the tags
		 * @throw SipHeaderValueException if not tags available
		 * @sa SipHeaderValue::HasTags()
		 */
		const map<string, string>& Tags() const throw ( SipHeaderValueException );

		/**
		 *     Retrieves the value of a specific tag.
		 * @param tagName The name of the tag
		 * @return The value of the tag
		 * @throw SipHeaderValueException if tag doesn't exist
		 */
		const string& GetTagValue( const string& tagName ) const throw( SipHeaderValueException );

		/**
		 *     Allows you to access the value, not including any tags.
		 * @return A const reference to the value.
		 */
		const string& Value() const throw();

		/**
		 * \brief Allows you to change the value.
		 * \param newValue The new value to set it to.
		 */
		void SetValue( const string& newValue ) throw();

		/**
		 *     Represents value and any tags
		 * @return
		 */
		string ToString() const throw();

		/**
		 *     Indicates whether this value had any tags. You should call this before calling SipHeaderValue::Tags()
		 * @return True if tags available, false otherwise
		 * @sa SipHeaderValue::Tags()
		 */
		bool HasTags() const throw();

		/**
		 *     Indicates whethor a specific tag is present. If there aren't any tags, always returns false.
		 * @param tagName The name of the tag in question
		 * @return True if tag exists, false otherwise
		 */
		bool HasTag( const string& tagName ) const throw();

		/**
		 *     Adds a tag to the value
		 * @param key The key
		 * @param value The value
		 */
		void AddTag( const string& key, const string& value ) throw();

	protected:
		bool m_hasTags;
		string m_value;
		map<string, string> m_tags;
};

/** 
 * \class SipHeader
* \brief A single sip header.
*/
class SipHeader
{
	public:
		string header_name;
		vector<SipHeaderValue> shvs;

		bool operator==( const string& header_name ) const {
			return !( strcasecmp( this->header_name.c_str(), header_name.c_str() ) );
		}
};

/**
* \class Via
* \brief Creates a usable Via object from a generic SipHeaderValue
* \sa SipHeaderValue
*/
class Via
{
	public:
		Via ( const SipHeaderValue& shv ) throw ( ViaException );
		Via ( const string& viaString ) throw ( ViaException );
		Via ();

		int	Port() const throw( ViaException );
		const string& Host() const throw( ViaException );
		TRANSPORT_PROTOCOL	TransportProtocol() const throw( ViaException );
		bool	RFC3261Compliant() const throw();
		const string& Branch() const throw( ViaException );

		bool HasPort() const throw();
		bool HasHost() const throw();
		bool HasTransportProtocol() const throw();
		bool HasBranch() const throw();

		//Warning: Will not return branch
		string ToString() const;
	protected:
		void ParseFromSHV( const SipHeaderValue& shv );
		int m_port;
		string m_host;
		string m_branch;
		TRANSPORT_PROTOCOL m_transportProtocol;
		bool has_port, has_host, has_transportProtocol, m_rfc3261compliant, has_branch;

};

/**
* \class URI
* \brief Creates a usable URI object from a generic SipHeaderValue
* \warning Passwords in the URI are not supported. Cry me a river.
* \sa SipHeaderValue
*/
class URI
{
	public:
		/**
		 *     Parse a new URI
		 * @param srhv The value to transform into a URI
		 */
		URI( const SipHeaderValue& srhv ) throw ( URIException );
		URI( const string& uri ) throw ( URIException );
		URI( const Via& via ) throw();
		URI() throw ();

		void 	 URIFromString( const string& uri ) throw ( URIException );
		string URIAsString() const throw();

		string DisplayName() const throw ( URIException );
		string Protocol() const throw ( URIException );
		string User() const throw ( URIException );
		string Host() const throw ( URIException );
		int	 Port() const throw( URIException );

		void SetUser ( const string& theValue );
		void SetHost ( const string& value );

		const map<string, string>& URIParameters() const throw();
		map<string, string>& ModifyURIParameters() throw();
		string URIHeaders() const throw ( URIException );

		bool HasDisplayName() const throw ();
		bool HasProtocol() const throw ();
		bool HasUser() const throw ();
		bool HasHost() const throw ();
		bool HasPort() const throw();
		bool HasURIHeaders() const throw();

		friend ostream &operator<< ( ostream &stream, const URI& uri );

		//Utility
		static bool IsURI( const string& uri ) throw();
		bool operator<( const URI& rhs ) const;
 		bool operator==( const URI& rhs ) const;



	private:
		void ParseURI ( const string& uriAsString ) throw ( URIException );
		map<string, string> m_URIParameters;
		string m_displayName, m_protocol, m_user, m_host, m_URIHeaders;
		int m_port;
		bool has_displayName, has_protocol, has_user, has_host, has_port, has_URIHeaders;
};

/**
* \class SipMessage
* \brief The basic pattern for SIP datagrams.
*/
class SipMessage
{

	public:

		enum MESSAGE_TYPE
		{
			MT_UNDEFINED,
			MT_REQUEST,
			MT_RESPONSE
		};

		SipMessage( MESSAGE_TYPE type ) throw() : Type( type ), m_hasBody( false ) {}
		virtual ~SipMessage() {}
		/**
		*     Returns a vector<SipHeaderValue> corresponding to the header key
		* @param key The header key. For example, 'via'
		* @return vector<SipHeaderValue>
		* @throw SipMessageException if key doesn't exist. See HasHeader()
		*/
		const vector<SipHeaderValue>& GetHeaderValues ( const string& headerName ) const throw ( SipMessageException );

		/**
		 *     Allows you to enumerate all headers
		 * @return A const reference to the vector of SipHeader's
		 */
		const vector<SipHeader>& GetAllHeaders() const throw();

		/**
		 *     Returns the message body, if there is one->messageQueue.push_back( message );
		 * @return A const reference to the message body
		 * @throw SipMessageException is there isn't a body to get
		 */
		const string& GetMessageBody() const throw( SipMessageException );

		/**
		*     Does a specific SIP header exist
		* @param key Header in question. Remember, it's all lcase.
		* @return True if header exists, false otherwise.
		*/
		bool HasHeader ( const string& headerName ) const throw();

		/**
		 *     Does message have a non-zero body
		 * @return True if body exists, false otherwise
		 */
		bool HasMessageBody ( ) const throw();

		/**
		 *     Sets the message body, and the Content-Type and Content-Length fields
		 */
		void SetMessageBody ( const string& body, const string& rtpMap ) throw( SipMessageException );

		/**
		 *     Returns an reference to a header so it's values may be modified. If header doesn't exist, it is added.
		 * @param headerName The header to modify
		 * @return A reference to the vector<SipHeaderValue> indicated by the header.
		 */
		vector<SipHeaderValue>& ModifyHeader( const string& headerName);

		/**
		 * 	Adds or replaces a header referenced with the values given
		 * @param headerName The name of the header to add/replace
		 * @param values The value(s) to go along with it
		 */
		void SetHeader( const string& headerName, const vector<SipHeaderValue>& values ) throw();
		void SetHeader( const string& headerName, const string& value ) throw();
		void SetHeader( const string& headerName, const SipHeaderValue& value ) throw();
		/**
		*     Utility function for transforming a string of key & value tags into a map of tags
		* @param rawTags A string representations of one or more tags in the format (;key=value)*
		* @param tagMap The map to contain the tags
		*/
		static void FillTags ( const string& rawTags, map<string, string>& tagMap );

		string ToString() const;
		const string& GetOriginalRawMessage() const;

		MESSAGE_TYPE Type;

	protected:

		/**
		*       Adds one or more values, given a header name, to  SipMessage::m_headers
		* @param headerName The name of the header
		* @param rawString The raw string containing values in the following format: value(tags)*,value(tags)*,...
		* @note SipMessage::FillTags is used internal after the comma seperated values are split up
		* @warning If commas are used for anything but seperating values, the logic will be broken.
		*/
		void	ProcessSipHeaderValues ( const string& headerName, string& rawString ) throw ( SipMessageException );

		/**
		 *     Populates m_headers and messageBody (Content-Length header indicates last header line, RFC 3261 7.5)
		 * @param start The character folloing the end of the start-line and it's CR/LF
		 * @param end  The end of the entire message
		 */
		void ProcessSipMessage( string::const_iterator start, string::const_iterator end ) throw( SipMessageException );

		/**
		 * 	transforms any message header name into it's lower-case, long variant
		 * @param headerName The header name to be cleaned up
		 * @return The massaged, or 'cleaned' header name
		 */
		string MassageHeaderKey ( string headerName ) const throw();

		string messageBody, rawMessage;
		string m_recvAddress;
		bool m_hasBody, m_hasRecvAddress;
		vector<SipHeader> m_headers;

	private:
		SipMessage() {}

};

/**
* \class SipRequest
* \brief Holder for a SIP request.
* \sa SipServer::Accept(), SipMessage
*/
class SipRequest : public SipMessage
{
	friend class SipUtility;

	public:

		enum REQUEST_METHOD
		{
			REQUEST_METHOD_REGISTER,
			REQUEST_METHOD_INVITE,
			REQUEST_METHOD_SUBSCRIBE,
			REQUEST_METHOD_PUBLISH,
			REQUEST_METHOD_ACK,
			REQUEST_METHOD_PRACK,
			REQUEST_METHOD_CANCEL,
			REQUEST_METHOD_BYE,
			REQUEST_METHOD_OPTIONS,
			REQUEST_METHOD_MESSAGE,
			REQUEST_METHOD_REFER,
			REQUEST_METHOD_NOTIFY,
			REQUEST_METHOD_INFO,
			REQUEST_METHOD_FEATURE, //This might be 3Com specific...
			REQUEST_METHOD_UPDATE
		};

		/**
		 *     Creates an empty SipRequest
		 */
		SipRequest() throw();

		/**
		*     Creates a SipRequest for a specific request method.
		* @param rm The request type
		* @sa SipRequest::REQUEST_METHOD
		*/
		SipRequest ( SipRequest::REQUEST_METHOD rm );

		/**
		 * 	Create a request from a request. Since you would only be doing this to pass on a request, it only copies the transactionally important stuff.
		 * @param rhs The request to copy construct from
		 */
		SipRequest( const SipRequest& rhs );
		/**
		 *     Create a sip request from raw data received from a UDP packet
		 * @param data
		 */
		SipRequest ( const string& data ) throw ( SipMessageException, SipRequestException );

		/**8496
		*     Provides request method for this SipRequest
		* @return The request method
		*/
		SipRequest::REQUEST_METHOD RequestMethod() const throw();

		/**
		 *     Returns the request URI of the SIP request
		 * @return The request URI
		 */
		const URI& RequestURI() const throw();

		/**
		 *     Allows changing the request URI
		 * @param uri The new URI to use
		 */
		void SetRequestURI( const URI& uri ) throw();

		/**
		 *     Changes request method. If set to CANCEL, updates the CSeq method as well.
		 * @param rm The new request method
		 */
		void SetRequestMethod( const SipRequest::REQUEST_METHOD rm ) throw();

		string ToString() const;

//		ostream &operator<< ( ostream& stream ) const;


	private:

		URI m_requestURI;
		REQUEST_METHOD requestMethod;
};

/**
* \class CSeq
* \brief Creates a usable CSeq object from a generic SipHeaderValue
* \sa SipHeaderValue
*/
class CSeq
{
	public:
		/**
		 *     Parse a CSeq
		 * @param srhv The value to parse into a CSeq
		 */
		CSeq ( const SipHeaderValue& srhv ) throw ( CSeqException );

		CSeq ( int sequence, SipRequest::REQUEST_METHOD rm ) throw( CSeqException );
		CSeq ();

		string								ToString() const throw();

		int 									Sequence() const throw();
		SipRequest::REQUEST_METHOD 	RequestMethod() const throw();
		string								RequestMethodAsString() const throw();

		CSeq&									Increment() throw();
	protected:
		void ParseCSeq ( const string& rawValue ) throw ( CSeqException );

		SipRequest::REQUEST_METHOD	m_requestMethod;
		int								m_sequence;
		string							m_requestMethodString;
};

/**
* \class SipResponse
* \brief Holder for a SIP response. When creating, you must specify a response code.
* \sa SipMessage
*/
class SipResponse : public SipMessage
{
	friend class SipUtility;

	public:

		/**
		 *     Generates a response for the request you have.
		 * @param statusCode The status code (in start-line)
		 * @param reasonPhrase The reason phrase (in start-line)
		 * @param request The request you are responding to.
		 */
		SipResponse( const int statusCode, const string& reasonPhrase, const SipRequest& request ) throw( SipResponseException );

		/**
		 *     Generates a (mostly) blank SipResponse. Should only be needed for special occasions.
		 * @param statusCode The status code (in start-line)
		 * @param reasonPhrase The reason phrase (in start-line)
		 */
		SipResponse( const int statusCode = 500, const string& reasonPhrase = "Internal Server Error" );

		SipResponse( const string& rawResponseData ) throw ( SipResponseException );

		int StatusCode( ) const throw();
		const string& ReasonPhrase() const throw();

		void SetStatusCode( int newStatusCode );
		void SetReasonPhrase ( const string& newReasonPhrase );

		string ToString() const;
		//friend ostream &operator<< ( ostream &stream );
	protected:
		int m_statusCode;
		string m_reasonPhrase;
};

/**
* \class SipServer
* \brief Creates a class capable of translating raw SIP requests, UDP @ port 5060, to easily parseable data types. Also allows communication outwards to SIP UAS/UAC's
*/
class SipServer
{
	public:
		SipServer( int UDPPort = DEFAULT_UDP_LISTENPORT ) throw ( SipServerException );

		/**
		 *     Waits for a SIP datagram.
		 * @return The address the SipMessage was received from
		 * @param sipMessage An auto_ptr to a sip message. This will be re-assigned to the SipMessage received
		 * @todo Forward requests not meant for this-host
		 */
		string Accept( auto_ptr<SipMessage>& sipMessage );

		/**
		 * \brief Sends a sip response. Will decrement max-forwards field.
		 * \param response The response to send
		 * \throw SipServerException if errors processing message, or max-forwards is 0
		 */
		void Respond( SipResponse& response ) const;

		/**
		 * \brief Sends a sip request. Will decrement max-forwards field.
		 * \param request The response to send
		 * \throw SipServerException if errors processing message, or max-forwards is 0
		 */
		void Request( SipRequest& request ) const;

		int UDPPort() const throw();

	private:
		static void CSVSeperate( const string& rawString, queue<string>& elements );

		int 		m_sock;
		int		m_udpPort;
		sockaddr_in	m_serverAddr;

};


class SipUtility
{
	public:
		static string GetMessage( auto_ptr<SipMessage>& sipMessage, int socket );
		
		/**
		 * \brief Sends a sip response. Will decrement max-forwards field.
		 * \param response The response to send
		 * \throw SipServerException if errors processing message, or max-forwards is 0
		 */
		static void Respond( SipResponse& response );
		
		/**
		 * \brief Sends a sip request. Will decrement max-forwards field.
		 * \param request The response to send
		 * \throw SipServerException if errors processing message, or max-forwards is 0
		 */
		static void Request( SipRequest& request );
	private:
		static void SendSipMessage( const string& rawMessage, const URI& recipient );
		
		/**
		 * \brief Decrements the max-forwards header.
		 * \param mesg The SipMessage to operate on.
		 * \throw If max-forwards <= 0
		 */
		static void DecrementForwards( SipMessage& mesg );
};


//
// Lookup Tables
//

/**
* \class RequestTypesDef
* \brief Definition for a lookup table that converts SIP request methods as a string to the appropriate SipRequest::REQUEST_METHOD
*/
class RequestTypesDef : public LookupTable<string, SipRequest::REQUEST_METHOD>
{
	public:
		RequestTypesDef()
		{ LoadValues(); }
	private:
		void LoadValues()
		{
			table["REGISTER"] = SipRequest::REQUEST_METHOD_REGISTER;
			table["INVITE"] = SipRequest::REQUEST_METHOD_INVITE;
			table["SUBSCRIBE"] = SipRequest::REQUEST_METHOD_SUBSCRIBE;
			table["PUBLISH"] = SipRequest::REQUEST_METHOD_PUBLISH;
			table["ACK"] = SipRequest::REQUEST_METHOD_ACK;
			table["PRACK"] = SipRequest::REQUEST_METHOD_PRACK;
			table["CANCEL"] = SipRequest::REQUEST_METHOD_CANCEL;
			table["BYE"] = SipRequest::REQUEST_METHOD_BYE;
			table["OPTIONS"] = SipRequest::REQUEST_METHOD_OPTIONS;
			table["MESSAGE"] = SipRequest::REQUEST_METHOD_MESSAGE;
			table["REFER"] = SipRequest::REQUEST_METHOD_REFER;
			table["NOTIFY"] = SipRequest::REQUEST_METHOD_NOTIFY;
			table["INFO"] = SipRequest::REQUEST_METHOD_INFO;
			table["FEATURE"] = SipRequest::REQUEST_METHOD_FEATURE;
			table["UPDATE"] = SipRequest::REQUEST_METHOD_UPDATE;
		}
};

/**
* \class HeaderConversionsDef
* \brief Definition for a lookup table that converts short SIP request headers to their long equivalents
*/
class HeaderConversionsDef : public LookupTable<string, string>
{
	public:
		HeaderConversionsDef ()
		{ LoadValues(); }
	private:
		void LoadValues()
		{
			table["f"] = "from";
			table["t"] = "to";
			table["m"] = "contact";
			table["i"] = "call-id";
			table["v"] = "via";
			table["e"] = "content-encoding";
			table["l"] = "content-length";
			table["c"] = "content-type";
			table["s"] = "subject";
			table["r"] = "refer-to";
		}
};

class TransportProtocolTypesDef : public LookupTable<string, TRANSPORT_PROTOCOL>
{
	public:
		TransportProtocolTypesDef()
		{ LoadValues(); }
	private:
		void LoadValues()
		{
			table["udp"] = TRANSPORT_PROTOCOL_UDP;
			table["tcp"] = TRANSPORT_PROTOCOL_TCP;
			table["tls"] = TRANSPORT_PROTOCOL_TLS;
			table["sctp"] = TRANSPORT_PROTOCOL_SCTP;

		}
};

static RequestTypesDef RequestTypes;
static HeaderConversionsDef HeaderConversions;
static TransportProtocolTypesDef TransportProtocolTypes;
} //namespace Sip
#endif
