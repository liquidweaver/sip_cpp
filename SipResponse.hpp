#ifndef SIPRESPONSE_HPP
#define SIPRESPONSE_HPP
#include "SipMessage.hpp"
#include "SipRequest.hpp"
#include <string>
namespace Sip {

/**
* \class SipResponseException
* \brief standard exception class for SipResponse
*/
class SipResponseException: public SipMessageException
{
	public:
		SipResponseException ( std::string what ) throw() : SipMessageException ( what ) {};
};

using std::string;
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
}; //class SipResponse
}; //namespace SIP
#endif //SIPRESPONSE_HPP
