#ifndef REGISTRAR_HPP
#define REGISTRAR_HPP
#include "UAS.hpp"

namespace Sip {

/** 
* @brief Handles REGISTER requests, adds appropriate entries to redis db
* @details Sets registrar:ENDPOINTID (hash)
* 					 uri (field, string)
* 					 callid (field, string )
* 					 expires (field, num ) 
*/
class Registrar : public UAS {
	public:
		auto_ptr<SipResponse> HandleRequest( const SipRequest& request );
};
}; //namespace Sip
#endif //REGISTRAR_HPP
