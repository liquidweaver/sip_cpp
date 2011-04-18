#ifndef SIPREQUEST_HPP
#define SIPREQUEST_HPP
#include "SipMessage.hpp" //Superclass
#include "lookuptable.hpp"
#include "URI.hpp"
#include <string>

namespace Sip {

/**
* \class SipRequestException
* \brief standard exception class for SipRequest
*/
class SipRequestException : public SipMessageException
{
	public:
		SipRequestException ( std::string what ) throw() : SipMessageException ( what ) {};
};

using std::string;
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
			table["register"] = SipRequest::REQUEST_METHOD_REGISTER;
			table["invite"] = SipRequest::REQUEST_METHOD_INVITE;
			table["subscribe"] = SipRequest::REQUEST_METHOD_SUBSCRIBE;
			table["publish"] = SipRequest::REQUEST_METHOD_PUBLISH;
			table["ack"] = SipRequest::REQUEST_METHOD_ACK;
			table["prack"] = SipRequest::REQUEST_METHOD_PRACK;
			table["cancel"] = SipRequest::REQUEST_METHOD_CANCEL;
			table["bye"] = SipRequest::REQUEST_METHOD_BYE;
			table["options"] = SipRequest::REQUEST_METHOD_OPTIONS;
			table["message"] = SipRequest::REQUEST_METHOD_MESSAGE;
			table["refer"] = SipRequest::REQUEST_METHOD_REFER;
			table["notify"] = SipRequest::REQUEST_METHOD_NOTIFY;
			table["info"] = SipRequest::REQUEST_METHOD_INFO;
			table["feature"] = SipRequest::REQUEST_METHOD_FEATURE;
			table["update"] = SipRequest::REQUEST_METHOD_UPDATE;
		}
};
static RequestTypesDef RequestTypes;

}; //namespace Sip
#endif //SIPREQUEST_HPP
