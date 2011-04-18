#include "SipUtility.hpp"
#include <boost/regex.hpp>
namespace Sip { namespace Utility {
void FillTags( const string& rawTags, map<string, string>& tagMap)
{
			boost::regex tagSearch( ";([^;=]+)(?:=([^;]+))?" );
			boost::match_results<std::string::const_iterator> tags;
   		string::const_iterator start, end;
   		start = rawTags.begin();
   		end = rawTags.end();

			while ( boost::regex_search( start, end, tags, tagSearch, boost::match_default ) )
			{
				if ( tags[2].matched )
					tagMap[ string( tags[1].first, tags[1].second ) ] = string( tags[2].first, tags[2].second );
				else
					tagMap[ string( tags[1].first, tags[1].second ) ] = "";
				start = tags[0].second;
			}
}
}; };//namespace Sip::Utility
