#ifndef URI_HPP
#define URI_HPP
#include <string>
#include <map>
#include <stdexcept>
#include <iosfwd>
#include "SipHeaderValue.hpp"
namespace Sip {
using std::runtime_error;
using std::ostream;
/**
 * * \class URIException
 * * \brief standard exception class for URI
 * */
class URIException : public runtime_error
{
	public:
		URIException ( std::string what ) throw() : runtime_error ( what ) {};
};

using std::string;
using std::map;
class Via;
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
}; //class URI
}; //namespace Sip
#endif //URI_HPP
