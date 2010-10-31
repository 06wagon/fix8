#include <config.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <iterator>
#include <memory>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <bitset>

#ifdef HAS_TR1_UNORDERED_MAP
#include <tr1/unordered_map>
#endif

#include <strings.h>
#include <regex.h>

#include <f8exception.hpp>
#include <f8types.hpp>
#include <f8utils.hpp>
#include <traits.hpp>
#include <field.hpp>
#include <message.hpp>

//-------------------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std;

//-------------------------------------------------------------------------------------------------
RegExp MessageBase::_elmnt("([0-9]+)=([^\x01]+)\x01");
RegExp Message::_hdr("8=([^\x01]+)\x01{1}9=([^\x01]+)\x01{1}(35=)([^\x01]+)\x01");
RegExp Message::_tlr("(10=)([^\x01]+)\x01");

//-------------------------------------------------------------------------------------------------
namespace {
	const string spacer(3, ' ');
}

//-------------------------------------------------------------------------------------------------
unsigned MessageBase::decode(const f8String& from, const unsigned offset)
{
	RegMatch match;
	unsigned s_offset(offset);

	for (unsigned pos(0); s_offset < from.size() && _elmnt.SearchString(match, from, 3, s_offset) == 3; )
	{
		f8String tag, val;
		_elmnt.SubExpr(match, from, tag, s_offset, 1);
		_elmnt.SubExpr(match, from, val, s_offset, 2);
		const unsigned tv(GetValue<unsigned>(tag));
		const BaseEntry *be(_ctx._be.find_ptr(tv));
		if (!be)
			throw InvalidField(tv);
		if (!_fp.has(tv))
			break;
		s_offset += match.SubSize();
		if (_fp.get(tv))
		{
			if (!_fp.get(tv, FieldTrait::automatic))
				throw DuplicateField(tv);
		}
		else
		{
			add_field(tv, ++pos, be->_create(val, be->_rlm));
			if (_fp.is_group(tv))
				s_offset = decode_group(tv, from, s_offset);
		}
	}

	//const unsigned short missing(_fp.find_missing());
	//if (missing)
	//	throw MissingMandatoryField(missing);

	return s_offset;
}

//-------------------------------------------------------------------------------------------------
unsigned MessageBase::decode_group(const unsigned short fnum, const f8String& from, const unsigned offset)
{
	unsigned s_offset(offset);
	GroupBase *grpbase(find_group(fnum));
	if (!grpbase)
		throw InvalidRepeatingGroup(fnum);

	bool ok(true);
	for (; ok && s_offset < from.size(); )
	{
		RegMatch match;
		auto_ptr<MessageBase> grp(grpbase->create_group());

		for (unsigned pos(0); s_offset < from.size() && _elmnt.SearchString(match, from, 3, s_offset) == 3; )
		{
			f8String tag, val;
			_elmnt.SubExpr(match, from, tag, s_offset, 1);
			_elmnt.SubExpr(match, from, val, s_offset, 2);
			const unsigned tv(GetValue<unsigned>(tag));
			if (grp->_fp.get(tv))	// already present; next group?
				break;
			if (pos == 0 && grp->_fp.getPos(tv) != 1)	// first field in group is mandatory
				throw MissingRepeatingGroupField(tv);
			const BaseEntry *be(_ctx._be.find_ptr(tv));
			if (!be)
				throw InvalidField(tv);
			if (!grp->_fp.has(tv))	// field not found in sub-group - end of repeats?
			{
				ok = false;
				break;
			}
			s_offset += match.SubSize();
			grp->add_field(tv, ++pos, be->_create(val, be->_rlm));
			grp->_fp.set(tv);	// is present
		}

		*grpbase += grp.release();

		//const unsigned short missing(grp->_fp.find_missing());
		//if (missing)
		//	throw MissingMandatoryField(missing);
	}

	return s_offset;
}

//-------------------------------------------------------------------------------------------------
unsigned MessageBase::check_positions()
{
	return 0;
}

//-------------------------------------------------------------------------------------------------
unsigned Message::decode(const f8String& from)
{
	unsigned offset(_header->decode(from, 0));
	offset = MessageBase::decode(from, offset);
	return _trailer->decode(from, offset);
}

//-------------------------------------------------------------------------------------------------
Message *Message::factory(const F8MetaCntx& ctx, const f8String& from)
{
	RegMatch match;
	Message *msg(0);
	if (_hdr.SearchString(match, from, 5, 0) == 5)
	{
		f8String ver, len, mtype;
		_hdr.SubExpr(match, from, ver, 0, 1);
		_hdr.SubExpr(match, from, len, 0, 2);
		_hdr.SubExpr(match, from, mtype, 0, 4);
		const int mtpos(match.SubPos(3));
		const unsigned mlen(GetValue<unsigned>(len));

		msg = ctx._bme.find_ptr(mtype)->_create();
		msg->decode(from);

		Fields::const_iterator fitr(msg->_header->_fields.find(Common_BodyLength));
		static_cast<Field<Length, Common_BodyLength> *>(fitr->second)->set(mlen);
		fitr = msg->_header->_fields.find(Common_MsgType);
		static_cast<Field<f8String, Common_MsgType> *>(fitr->second)->set(mtype);
		msg->check_set_rlm(fitr->second);

		if (_tlr.SearchString(match, from, 3, 0) == 3)
		{
			f8String chksum;
			_hdr.SubExpr(match, from, chksum, 0, 2);
			const unsigned chkpos(match.SubPos(1));

			Fields::const_iterator fitr(msg->_trailer->_fields.find(Common_CheckSum));
			static_cast<Field<f8String, Common_CheckSum> *>(fitr->second)->set(chksum);
			unsigned chkval(GetValue<unsigned>(chksum));
			if (chkval != calc_chksum(from, mtpos, chkpos - mtpos))
				throw BadCheckSum(mtype);
		}

	}

	return msg;
}

//-------------------------------------------------------------------------------------------------
void MessageBase::check_set_rlm(BaseField *where)
{
	if (!where->_rlm)
	{
		const BaseEntry *tbe(_ctx._be.find_ptr(where->_fnum));
		if (tbe && tbe->_rlm)
			where->_rlm = tbe->_rlm;	// populate realm;
	}
}

//-------------------------------------------------------------------------------------------------
unsigned MessageBase::encode(ostream& to)
{
	const std::ios::pos_type where(to.tellp());
	for (Positions::const_iterator itr(_pos.begin()); itr != _pos.end(); ++itr)
	{
		check_set_rlm(itr->second);
		if (!_fp.get(itr->second->_fnum, FieldTrait::suppress))	// some fields are not encoded until unsuppressed (eg. checksum)
		{
			itr->second->encode(to);
			if (_fp.get(itr->second->_fnum, FieldTrait::group))
				encode_group(itr->second->_fnum, to);
		}
	}

	return to.tellp() - where;
}

//-------------------------------------------------------------------------------------------------
unsigned MessageBase::encode_group(const unsigned short fnum, std::ostream& to)
{
	const std::ios::pos_type where(to.tellp());
	GroupBase *grpbase(find_group(fnum));
	if (!grpbase)
		throw InvalidRepeatingGroup(fnum);
	for (GroupElement::iterator itr(grpbase->_msgs.begin()); itr != grpbase->_msgs.end(); ++itr)
		(*itr)->encode(to);
	return to.tellp() - where;
}

//-------------------------------------------------------------------------------------------------
unsigned Message::encode(f8String& to)
{
	ostringstream msg;
	if (!_header)
		throw MissingMessageComponent("header");
	Fields::const_iterator fitr(_header->_fields.find(Common_MsgType));
	static_cast<Field<f8String, Common_MsgType> *>(fitr->second)->set(_msgType);
	_header->encode(msg);
	MessageBase::encode(msg);
	if (!_trailer)
		throw MissingMessageComponent("trailer");
	_trailer->encode(msg);
	const unsigned msgLen(msg.str().size());	// checksummable msglength

	if ((fitr = _trailer->_fields.find(Common_CheckSum)) == _trailer->_fields.end())
		throw MissingMandatoryField(Common_CheckSum);
	static_cast<Field<f8String, Common_CheckSum> *>(fitr->second)->set(fmt_chksum(calc_chksum(msg.str())));
	_trailer->_fp.clear(Common_CheckSum, FieldTrait::suppress);
	fitr->second->encode(msg);

	ostringstream hmsg;
	if ((fitr = _header->_fields.find(Common_BeginString)) == _header->_fields.end())
		throw MissingMandatoryField(Common_BeginString);
	_header->_fp.clear(Common_BeginString, FieldTrait::suppress);
	fitr->second->encode(hmsg);

	if ((fitr = _header->_fields.find(Common_BodyLength)) == _header->_fields.end())
		throw MissingMandatoryField(Common_BodyLength);
	_header->_fp.clear(Common_BodyLength, FieldTrait::suppress);
	static_cast<Field<Length, Common_BodyLength> *>(fitr->second)->set(msgLen);
	fitr->second->encode(hmsg);

	hmsg << msg.str();
	to = hmsg.str();
	return to.size();
}

//-------------------------------------------------------------------------------------------------
void MessageBase::print(ostream& os) const
{
	const BaseMsgEntry *tbme(_ctx._bme.find_ptr(_msgType));
	if (tbme)
		os << tbme->_name << " (\"" << _msgType << "\")" << endl;
	for (Positions::const_iterator itr(_pos.begin()); itr != _pos.end(); ++itr)
	{
		const BaseEntry& tbe(_ctx._be.find_ref(itr->second->_fnum));
		os << spacer << tbe._name << " (" << itr->second->_fnum << "): ";
		int idx;
		if (itr->second->_rlm && (idx = (itr->second->get_rlm_idx())) >= 0)
			os << itr->second->_rlm->_descriptions[idx] << " (" << *itr->second << ')' << endl;
		else
			os << *itr->second << endl;
		if (_fp.is_group(itr->second->_fnum))
			print_group(itr->second->_fnum, os);
	}
}
//-------------------------------------------------------------------------------------------------
const f8String Message::fmt_chksum(const unsigned val)
{
	std::ostringstream ostr;
	ostr << std::setfill('0') << std::setw(3) << val;
	return ostr.str();
}

//-------------------------------------------------------------------------------------------------
void MessageBase::print_group(const unsigned short fnum, ostream& os) const
{
	const GroupBase *grpbase(find_group(fnum));
	if (!grpbase)
		throw InvalidRepeatingGroup(fnum);
	for (GroupElement::const_iterator itr(grpbase->_msgs.begin()); itr != grpbase->_msgs.end(); ++itr)
	{
		os << spacer << spacer << "Repeating group (\"" << (*itr)->_msgType << "\")" << endl << spacer << spacer;
		(*itr)->print(os);
	}
}

//-------------------------------------------------------------------------------------------------
void Message::print(ostream& os) const
{
	if (_header)
		_header->print(os);
	else
		os << "Null Header" << endl;
	MessageBase::print(os);
	if (_trailer)
		_trailer->print(os);
	else
		os << "Null Trailer" << endl;
}

