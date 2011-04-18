#ifndef VIA_HPP
#define VIA_HPP
#include <string>
#include "SipDefines.hpp"
#include "SipHeaderValue.hpp"

namespace Sip {
/**
 * \class ViaException
 * \brief standard exception class for Via
 */
class ViaException : public SipHeaderValueException
{
	public:
		ViaException ( std::string what ) throw() : SipHeaderValueException ( what ) {};

};

/**
* \class Via
* \brief Creates a usable Via object from a generic SipHeaderValue
* \sa SipHeaderValue
*/
using std::string;
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
		SipHeaderValue ToSHV();

	protected:
		void ParseFromSHV( const SipHeaderValue& shv );
		int m_port;
		string m_host;
		string m_branch;
		TRANSPORT_PROTOCOL m_transportProtocol;
		bool has_port, has_host, has_transportProtocol, m_rfc3261compliant, has_branch;

};
}; //namespace Sip
#endif //VIA_HPP
