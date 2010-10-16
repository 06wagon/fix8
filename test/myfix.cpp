//-----------------------------------------------------------------------------------------
#include <iostream>
#include <memory>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <iterator>
#include <algorithm>
#include <bitset>

#include <regex.h>
#include <errno.h>
#include <string.h>

// f8 headers
#include <f8exception.hpp>
#include <f8utils.hpp>
#include <traits.hpp>
#include <field.hpp>
#include <f8types.hpp>
#include <message.hpp>
#include "Myfix_types.hpp"
#include "Myfix_classes.hpp"

//-----------------------------------------------------------------------------------------
using namespace std;
using namespace FIX8;

//-----------------------------------------------------------------------------------------
static const std::string rcsid("$Id: f8c.cpp 515 2010-09-16 01:13:48Z davidd $");

//-----------------------------------------------------------------------------------------
int main(int argc, char **argv)
{
#if 0
	try
	{
		const BaseEntry& be(TEX::Myfix_BaseEntry::find_ref(35));
		//const BaseEntry& be(TEX::Myfix::find_ref(101));
		string logon("A");
		auto_ptr<BaseField> fld(be._create(logon, &be));
		if (be._comment)
			cout << be._comment << ": ";
		cout << *fld << endl;
		cout << "is valid: " << boolalpha << fld->is_valid() << endl << endl;
	}
	catch (InvalidMetadata& ex)
	{
		cerr << ex.what() << endl;
	}

	FieldTrait::TraitBase a = { 1, FieldTrait::ft_int, 0, true, false, false };

	const BaseEntry& be1(TEX::Myfix_BaseEntry::find_ref(98));
	string encryptmethod("101");
	auto_ptr<BaseField> fld1(be1._create(encryptmethod, &be1));
	if (be1._comment)
		cout << be1._comment << ": ";
	cout << *fld1 << endl;
	cout << "is valid: " << boolalpha << fld1->is_valid() << endl;
	//TEX::EncryptMethod& em(dynamic_cast<TEX::EncryptMethod&>(*fld1));
	//cout << em.get() << endl;
#endif

	string from("35=1005=hello114=Y87=STOP47=10.239=14");
	RegExp elmnt("([0-9]+)=([^\x01]+)\x01");
	RegMatch match;
	unsigned s_offset(0);
	while (s_offset < from.size() && elmnt.SearchString(match, from, 3, s_offset) == 3)
	{
		string tag, val;
		elmnt.SubExpr(match, from, tag, s_offset, 1);
		elmnt.SubExpr(match, from, val, s_offset, 2);
		cout << tag << " => " << val << endl;
		s_offset += match.SubSize();
	}
	cout << "ol=" << from.size() << " cl=" << s_offset << endl;

	cout << TEX::ctx.version() << endl;
	return 0;
}

