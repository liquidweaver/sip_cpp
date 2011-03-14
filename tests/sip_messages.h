const char* sip_messages[] = {

"INVITE sip:2100@172.20.3.28;user=phone SIP/2.0\r\n"
"Via: SIP/2.0/UDP 172.20.3.46;branch=z9hG4bKce49e348C486E67\r\n"
"From: \"2278\" <sip:2278@172.20.3.28>;tag=F29AA123-A06C7B62\r\n"
"To: <sip:2100@172.20.3.28;user=phone>\r\n"
"CSeq: 1 INVITE\r\n"
"Call-ID: f3c99e26-20e78e85-6ecadb84@172.20.3.46\r\n"
"Contact: <sip:2278@172.20.3.46>\r\n"
"Allow: INVITE, ACK, BYE, CANCEL, OPTIONS, INFO, MESSAGE, SUBSCRIBE, NOTIFY, PRACK, UPDATE, REFER\r\n"
"User-Agent: PolycomSoundPointIP-SPIP_650-UA/3.1.3.0439\r\n"
"Accept-Language: en\r\n"
"Supported: 100rel,replaces\r\n"
"Allow-Events: talk,hold,conference\r\n"
"Max-Forwards: 70\r\n"
"Content-Type: application/sdp\r\n"
"Content-Length: 292\r\n"
"\r\n"
"v=0\r\n"
"o=- 1251311173 1251311173 IN IP4 172.20.3.46\r\n"
"s=Polycom IP Phone\r\n"
"c=IN IP4 172.20.3.46\r\n"
"t=0 0\r\n"
"a=sendrecv\r\n"
"m=audio 2262 RTP/AVP 9 0 8 18 101\r\n"
"a=rtpmap:9 G722/8000\r\n"
"a=rtpmap:0 PCMU/8000\r\n"
"a=rtpmap:8 PCMA/8000\r\n"
"a=rtpmap:18 G729/8000\r\n"
"a=fmtp:18 annexb=no\r\n"
"a=rtpmap:101 telephone-event/8000\r\n",

NULL
};
