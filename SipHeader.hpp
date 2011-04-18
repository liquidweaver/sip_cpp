#ifndef SIPHEADER_HPP
#define SIPHEADER_HPP
#include <string>
#include <vector>
#include <strings.h> //strcasecmp
#include "SipHeaderValue.hpp"

using std::string;
using std::vector;

namespace Sip {
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
}; //namespace Sip

#endif //SIPHEADER_HPP
