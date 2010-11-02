//
// C++ Interface: lookuptable
//
// Description: 
//
//
// Author: Joshua Weaver <josh@metropark.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef LOOKUPTABLE_H
#define LOOKUPTABLE_H
#include <map>

using namespace std;


/**
 * \class LookupTableException
 * \brief standard exception class for LookupTable
 */
class LookupTableException : public std::exception
{
	public:
		LookupTableException ( std::string what ) throw() : m_s ( what ) {};
		~LookupTableException () throw() {}

		std::string what() throw() { return m_s; }

	private:

		std::string m_s;

};


/**
 * \class LookupTable
 * \brief Provides a class that is meant to be inherited from to create a lookup-table using a bultin map.
 * \code
//Example usage:
class NumbersLookupDef : public LookupTable<string, int>
{
	public:
		NumbersLookupDef()
{ LoadValues(); }
	private:
		void LoadValues()
{
			table["fifteen"] = 15;
			table["ninety-six"] = 96;
}
};

//Now create a static instance
static NumbersLookupDef NumbersLookup;
\endcode
 */
template <class K, class V>
		class LookupTable
{
	protected:
		map<K, V> table;
		virtual void LoadValues() = 0;

	public:

		/**
	 *     Returns a value given a key
	 * @param key The key corresponding the value desired
	 * @return The value
		 */
		const V& Get ( const K& key ) const throw ( LookupTableException )
		{
			typename map<K, V>::const_iterator iter;
			iter = table.find ( key );
			if ( iter == table.end() )
				throw LookupTableException ( "Key not found." );
			else
				return ( *iter ).second;
		}
		
		/**
		 *     Returns a _key_ given a _value_
		 * @warning This isn't terribly efficient, it has to iterate the entire map, so on avg O(N/2)
		 * @param value The value to search for
		 * @return The key
		 */
		const K& ReverseGet ( const V& value ) const throw ( LookupTableException )
		{
			typename map<K, V>::const_iterator iter;
			for ( iter = table.begin(); iter != table.end(); ++iter )
			{
				if ( ( *iter ).second == value )
					return ( *iter ).first;
			}

			throw LookupTableException ( "Value not found." );
		}
		
		bool HasKey ( const K& key ) const throw()
		{
			return table.find( key ) != table.end();
		}
};

#endif
