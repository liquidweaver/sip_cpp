#ifndef SIPHEADERVALUE_HPP
#define SIPHEADERVALUE_HPP
#include <map>
#include <string>
#include <stdexcept>
using std::string;
using std::map;
namespace Sip {
/**
* \class SipHeaderValueException
* \brief standard exception class for SipHeaderValue
*/
class SipHeaderValueException : public std::runtime_error
{
	public:
		SipHeaderValueException ( std::string what ) throw() : std::runtime_error ( what ) {};
};

/**
* \class SipHeaderValue
* \brief A container for a single value from a SIP header.
*/
class SipHeaderValue
{
	
	public:
		SipHeaderValue ( const string& value,  map<string, string> tags ) throw();
		SipHeaderValue ( const string& value ) throw();
		/**
		 *     Allows you to access this value tags, if applicable
		 * @return A const reference to the tags
		 * @throw SipHeaderValueException if not tags available
		 * @sa SipHeaderValue::HasTags()
		 */
		const map<string, string>& Tags() const throw ( SipHeaderValueException );

		/**
		 *     Retrieves the value of a specific tag.
		 * @param tagName The name of the tag
		 * @return The value of the tag
		 * @throw SipHeaderValueException if tag doesn't exist
		 */
		const string& GetTagValue( const string& tagName ) const throw( SipHeaderValueException );

		/**
		 *     Allows you to access the value, not including any tags.
		 * @return A const reference to the value.
		 */
		const string& Value() const throw();

		/**
		 * \brief Allows you to change the value.
		 * \param newValue The new value to set it to.
		 */
		void SetValue( const string& newValue ) throw();

		/**
		 *     Represents value and any tags
		 * @return
		 */
		string ToString() const throw();

		/**
		 *     Indicates whether this value had any tags. You should call this before calling SipHeaderValue::Tags()
		 * @return True if tags available, false otherwise
		 * @sa SipHeaderValue::Tags()
		 */
		bool HasTags() const throw();

		/**
		 *     Indicates whethor a specific tag is present. If there aren't any tags, always returns false.
		 * @param tagName The name of the tag in question
		 * @return True if tag exists, false otherwise
		 */
		bool HasTag( const string& tagName ) const throw();

		/**
		 *     Adds a tag to the value
		 * @param key The key
		 * @param value The value
		 */
		void AddTag( const string& key, const string& value ) throw();

	protected:
		bool m_hasTags;
		string m_value;
		map<string, string> m_tags;
};
}; //namespace Sip;
#endif //SIPHEADERVALUE_HPP
