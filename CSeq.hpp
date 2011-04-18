#ifndef CSEQ_HPP
#define CSEQ_HPP
#include <stdexcept>
#include "SipHeaderValue.hpp"
#include "SipRequest.hpp"

using std::runtime_error;

namespace Sip {
/**
 * * \class CSeqException
 * * \brief standard exception class for CSeq
 * */
class CSeqException : public runtime_error
{
	public:
			CSeqException ( std::string what ) throw() : runtime_error ( what ) {};
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
}; //class CSeq
}; //namespace Sip
#endif //CSEQ_HPP
