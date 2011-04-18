#ifndef SIPUTILITY_H
#define SIPUTILITY_H
#include <string>
#include <map>
using std::string;
using std::map;
namespace Sip { namespace Utility {
/**
 *     Utility function for transforming a string of key & value tags into a map of tags
 * @param rawTags A string representations of one or more tags in the format (;key=value)*
 * @param tagMap The map to contain the tags
 */
static void FillTags ( const string& rawTags, map<string, string>& tagMap );
}; }; //namespace Sip::Utility
#endif //SIPUTILITY_H
