#ifndef SIPMESSAGE_HPP
#define SIPMESSAGE_HPP
#include <string>
#include <queue>
#include "SipHeader.hpp"
#include "SipHeaderValue.hpp"
namespace Sip {

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

using std::string;
using std::queue;
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
		string& ModifyMessageBody() throw ( SipMessageException );

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
		 * 	Replaces or sets a header referenced with the values given
		 * @param headerName The name of the header to add/replace
		 * @param values The value(s) to go along with it
		 */
		void SetHeader( const string& headerName, const vector<SipHeaderValue>& values ) throw();
		void SetHeader( const string& headerName, const string& value ) throw();
		void SetHeader( const string& headerName, const SipHeaderValue& value ) throw();
		/** 
		* @brief Adds to or sets a header with the given vector<SipHeaderValue>
		* 
		* @param headerName
		* @param 
		*/
		void PushHeader( const string& headerName, const vector<SipHeaderValue>& values ) throw();
		void PushHeader( const string& headerName, const string& value ) throw();
		void PushHeader( const string& headerName, const SipHeaderValue& value ) throw();

		/** 
		* @brief Deletes a header
		* 
		* @headerName A string representing the header name. Case insensitive matching is performed. 
		*/
		void DeleteHeader( const string& headerName ) throw();

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
		static void CSVSeperate( const string& rawString, queue<string>& elements );
		SipMessage() {}

};
}; //namespace Sip
#endif //SIPMESSAGE_HPP
