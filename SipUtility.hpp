#ifndef SIPUTILITY_H
#define SIPUTILITY_H
#include <string>
#include <map>
#include <memory>
#include "SipMessage.hpp"
using std::string;
using std::map;
using std::auto_ptr;
namespace Sip { namespace Utility {
	/**
	 *     Utility function for transforming a string of key & value tags into a map of tags
	 * @param rawTags A string representations of one or more tags in the format (;key=value)*
	 * @param tagMap The map to contain the tags
	 */
	static void FillTags ( const string& rawTags, map<string, string>& tagMap );

	/** 
	 * @brief Parses a raw string into a SipMessage.
	 * 
	 * @param message an auto_ptr<SipMessage>
	 * @param data The raw string to parse
	 */
	static void ParseMessage( auto_ptr<SipMessage>& sipMessage, const string& data );

}; }; //namespace Sip::Utility
#endif //SIPUTILITY_H
