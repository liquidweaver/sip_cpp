#ifndef UAS_HPP
#define UAS_HPP
#include <stdexcept>
#include <memory>
#include <string>
#include "SipResponse.hpp"

namespace Sip {

using std::runtime_error;
using std::string;
class UASException : public runtime_error {
	public:
		UASException( string what )
			: runtime_error( what ) { }
};

class SipRequest;
class UAS {
	virtual std::auto_ptr<SipResponse> HandleRequest( const SipRequest& request ) = 0;
};
}; //namespace Sip
#endif //UAS_HPP
