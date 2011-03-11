//-------------------------------------------------------------------------------------------------
#if 0

fix8 is released under the New BSD License.

Copyright (c) 2010-11, David L. Dight <fix@fix8.org>
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of
	 	conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list
	 	of conditions and the following disclaimer in the documentation and/or other
		materials provided with the distribution.
    * Neither the name of the author nor the names of its contributors may be used to
	 	endorse or promote products derived from this software without specific prior
		written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
OR  IMPLIED  WARRANTIES,  INCLUDING,  BUT  NOT  LIMITED  TO ,  THE  IMPLIED  WARRANTIES  OF
MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE ARE  DISCLAIMED. IN  NO EVENT  SHALL
THE  COPYRIGHT  OWNER OR  CONTRIBUTORS BE  LIABLE  FOR  ANY DIRECT,  INDIRECT,  INCIDENTAL,
SPECIAL,  EXEMPLARY, OR CONSEQUENTIAL  DAMAGES (INCLUDING,  BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF USE, DATA,  OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED  AND ON ANY THEORY OF LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

---------------------------------------------------------------------------------------------------
$Id$
$Date$
$URL$

#endif
//-------------------------------------------------------------------------------------------------
#ifndef _F8_TYPES_HPP_
#define _F8_TYPES_HPP_

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

typedef std::string f8String;
typedef std::ostringstream f8ostrstream;

#if defined POOLALLOC
typedef std::basic_string<char, std::char_traits<char>, f8Allocator<char> > pf8String;
//typedef std::basic_ostringstream<char, std::char_traits<char>, f8Allocator<char> > f8ostrstream;
#else
typedef std::string pf8String;
#endif

const unsigned char default_field_separator(0x1);

//-------------------------------------------------------------------------------------------------
template<typename Key, typename Val>
class GeneratedTable
{
	struct Pair
	{
		Key _key;
		Val _value;

		struct Less
		{
			bool operator()(const Pair &p1, const Pair &p2) const { return p1._key < p2._key; }
		};
	};

	static const Pair _pairs[];
	static const size_t _pairsz;

	typedef Val NotFoundType;
	static const NotFoundType _noval;

public:
	static const Val& find_ref(const Key& key)
	{
		static const std::string error_str("Invalid metadata or entry not found");
		const Pair what = { key };
		std::pair<const Pair *, const Pair *> res(std::equal_range (_pairs, _pairs + _pairsz, what, typename Pair::Less()));
		if (res.first != res.second)
			return res.first->_value;
		throw InvalidMetadata(error_str);
	}
	static const Val find_val(const Key& key)
	{
		const Pair what = { key };
		std::pair<const Pair *, const Pair *> res(std::equal_range (_pairs, _pairs + _pairsz, what, typename Pair::Less()));
		return res.first != res.second ? res.first->_value : _noval;
	}
	static const Val *find_ptr(const Key& key)
	{
		const Pair what = { key };
		std::pair<const Pair *, const Pair *> res(std::equal_range (_pairs, _pairs + _pairsz, what, typename Pair::Less()));
		return res.first != res.second ? &res.first->_value : 0;
	}

	GeneratedTable() {}
};

//-------------------------------------------------------------------------------------------------
template<typename Key, typename Val, typename Compare=std::less<Key> >
struct StaticTable
{
	typedef typename std::map<Key, Val, Compare> TypeMap;
	typedef typename TypeMap::value_type TypePair;
	typedef Val NotFoundType;

	static const TypePair _valueTable[];
	static const TypeMap _valuemap;
	static const NotFoundType _noval;

	StaticTable() {}

	static const Val find_value(const Key& key)
	{
		typename TypeMap::const_iterator itr(_valuemap.find(key));
		return itr != _valuemap.end() ? itr->second : _noval;
	}
	static const Val& find_value_ref(const Key& key)
	{
		typename TypeMap::const_iterator itr(_valuemap.find(key));
		return itr != _valuemap.end() ? itr->second : _noval;
	}
	static const Val *find_ptr(const Key& key)
	{
		typename TypeMap::const_iterator itr(_valuemap.find(key));
		return itr != _valuemap.end() ? &itr->second : 0;
	}

	static const size_t get_count() { return _valuemap.size(); }
	static const TypePair *get_table_end() { return _valueTable + sizeof(_valueTable)/sizeof(TypePair); }
};

} // FIX8

#endif // _F8_TYPES_HPP_
