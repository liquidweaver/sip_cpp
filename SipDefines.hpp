#ifndef SIPDEFINES_HPP
#define SIPDEFINES_HPP
#include "lookuptable.hpp"
namespace Sip {

//In milliseconds, the basic T1 retry interval and other intervals ( RFC 3261, 17.1.1.1 )
const int SIP_T1 = 500;
const int SIP_T2 = SIP_T1 * 4;
const int SIP_T4 = 4000;

const int SIP_TB = SIP_T1 * 64;
const int SIP_TH = SIP_T1 * 64;
const int SIP_TD = SIP_T1 * 64;
const int SIP_TF = SIP_T1 * 64;
const int SIP_TK = SIP_T4;
const int SIP_TI = SIP_T4;
const int SIP_TJ = SIP_T1 * 64;


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
// Lookup Tables
//


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

static HeaderConversionsDef HeaderConversions;
static TransportProtocolTypesDef TransportProtocolTypes;

}; //namespace Sip
#endif //SIPDEFINES_HPP
