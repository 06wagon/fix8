//-----------------------------------------------------------------------------------------
#if 0

Fix8 is released under the New BSD License.

Copyright (c) 2010, David L. Dight <www@orbweb.org>
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

-------------------------------------------------------------------------------------------
$Id$
$Date$
$URL$

#endif
//-----------------------------------------------------------------------------------------
#include <iostream>
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
#include <cctype>
//#ifdef HAVE_GETOPT_H
#include <getopt.h>
//#endif

// f8 headers
#include <f8exception.hpp>
#include <f8utils.hpp>
#include <traits.hpp>
#include <field.hpp>
#include <f8types.hpp>
#include <message.hpp>
#include <usage.hpp>
#include <xml.hpp>
#include <f8c.hpp>

//#include <config.h>

//-----------------------------------------------------------------------------------------
using namespace std;
using namespace FIX8;

//-----------------------------------------------------------------------------------------
static const std::string rcsid("$Id$");

//-----------------------------------------------------------------------------------------
const string Ctxt::_exts[count] = { "_types.cpp", "_types.hpp", "_classes.cpp", "_classes.hpp" };

//-----------------------------------------------------------------------------------------
namespace {

string inputFile, odir("./"), prefix;
const string GETARGLIST("hvo:p:n:");
const string spacer(3, ' ');

}

//-----------------------------------------------------------------------------------------
// static data
#include <f8cstatic.hpp>

//-----------------------------------------------------------------------------------------
void print_usage();
int process(XmlEntity& xf, Ctxt& ctxt);
int loadFixVersion (XmlEntity& xf, Ctxt& ctxt);
int loadfields(XmlEntity& xf, FieldSpecMap& fspec);
ostream *openofile(const string& odir, const string& fname);
const string flname(const string& from);
void processValueEnums(FieldSpecMap::const_iterator itr, ostream& ost_hpp, ostream& ost_cpp);

//-----------------------------------------------------------------------------------------
class filestdout
{
	std::ostream *os_;
	bool del_;

public:
	filestdout(std::ostream *os, bool del=false) : os_(os), del_(del) {}
	~filestdout() { if (del_) delete os_; }

	std::ostream& operator()() { return *os_; }
};

//-----------------------------------------------------------------------------------------
int main(int argc, char **argv)
{
	int val;

//#ifdef HAVE_GETOPT_LONG
	option long_options[] =
	{
		{ "help",			0,	0,	'h' },
		{ "version",		0,	0,	'v' },
		{ "odir",			0,	0,	'o' },
		{ "prefix",			0,	0,	'p' },
		{ 0 },
	};

	while ((val = getopt_long (argc, argv, GETARGLIST.c_str(), long_options, 0)) != -1)
//#else
//	while ((val = getopt (argc, argv, GETARGLIST.c_str())) != -1)
//#endif
	{
      switch (val)
		{
		case 'v':
			//cout << "f8c for "PACKAGE" version "VERSION << endl;
			cout << rcsid << endl;
			return 0;
		case 'h': print_usage(); return 0;
		case ':': case '?': return 1;
		case 'o': odir = optarg; break;
		case 'p': prefix = optarg; break;
		default: break;
		}
	}

	if (optind < argc)
		inputFile = argv[optind];
	else
	{
		cerr << "no input xml file specified" << endl;
		print_usage();
		return 1;
	}

	if (prefix.empty())
		prefix = "myfix";

	cout << "f8c " << _csMap.Find_Value_Ref(cs_copyright_short) << endl;

	scoped_ptr<XmlEntity> cfr(XmlEntity::Factory(inputFile));
	if (!cfr.get())
	{
		cerr << "Error reading file \'" << inputFile << '\'';
		if	(errno)
			cerr << " (" << strerror(errno) << ')';
		cerr << endl;
		return 1;
	}

	if (cfr->GetErrorCnt())
	{
		cerr << cfr->GetErrorCnt() << " error"
			<< (cfr->GetErrorCnt() == 1 ? " " : "s ") << "found in \'" << inputFile << '\'' << endl;
		return 1;
	}

	//cout << *cfr;

	Ctxt ctxt;
	for (unsigned ii(0); ii < Ctxt::count; ++ii)
	{
		ctxt._out[ii].first = prefix + ctxt._exts[ii];
		if (!ctxt._out[ii].second.reset(openofile(odir, ctxt._out[ii].first)))
			return 1;
	}

	if (loadFixVersion (*cfr, ctxt) < 0)
		return 1;
	cout << "compiling fix version " << ctxt._version << endl;
	return process(*cfr, ctxt);
}

//-----------------------------------------------------------------------------------------
ostream *openofile(const string& odir, const string& fname)
{
	ostringstream ofs;
	string odirect(odir);
	ofs << CheckAddTrailingSlash(odirect) << fname;
	ofstream *os(new ofstream(ofs.str().c_str()));
	if (!*os)
	{
		cerr << "Error opening file \'" << ofs.str() << '\'';
		if	(errno)
			cerr << " (" << strerror(errno) << ')';
		cerr << endl;
		return 0;
	}

	return os;
}

//-----------------------------------------------------------------------------------------
int loadFixVersion (XmlEntity& xf, Ctxt& ctxt)
{
	XmlEntity *fix(xf.find("fix"));
	if (!fix)
	{
		cerr << "No fix header found in " << inputFile << endl;
		return -1;
	}

	string major, minor, revision("0");

	if (!fix->GetAttr("major", major) || !fix->GetAttr("minor", minor))
	{
		cerr << "Missing required attributes (major/minor) from fix header in " << inputFile << endl;
		return -1;
	}

	fix->GetAttr("revision", revision);

	// fix version: <Major><Minor><Revision> eg. 4.2r1 is 421
	unsigned vers(GetValue<int>(major) * 100 + GetValue<int>(minor) * 10 + GetValue<int>(revision));
	if (vers < 400 || vers > 600)
	{
		cerr << "Invalid FIX version " << vers << " from fix header in " << inputFile << endl;
		return -1;
	}

	ctxt._version = vers;
	ostringstream ostr;
	ostr << "FIX" << (ctxt._version = vers);
	ctxt._fixns = ostr.str();
	ctxt._clname = prefix;
	ctxt._clname[0] = toupper(ctxt._clname[0]);
	return 0;
}

//-----------------------------------------------------------------------------------------
int loadfields(XmlEntity& xf, FieldSpecMap& fspec)
{
	int fieldsLoaded(0);

	XmlList flist;
	if (!xf.find("fix/fields/field", flist))
	{
		cerr << "No fields found in " << inputFile << endl;
		return 0;
	}

	for(XmlList::const_iterator itr(flist.begin()); itr != flist.end(); ++itr)
	{
		string number, name, type;
		if ((*itr)->GetAttr("number", number) && (*itr)->GetAttr("name", name) && (*itr)->GetAttr("type", type))
		{
			InPlaceStrToUpper(type);
			FieldTrait::FieldType ft(FieldSpec::_baseTypeMap.Find_Value(type));
			pair<FieldSpecMap::iterator, bool> result;
			if (ft != FieldTrait::ft_untyped)
				result = fspec.insert(FieldSpecMap::value_type(GetValue<unsigned>(number), FieldSpec(name, ft)));
			else
			{
				cerr << "Unknown field type: " << type << " in " << name << " at "
					<< inputFile << '(' << (*itr)->GetLine() << ')' << endl;
				continue;
			}

			(*itr)->GetAttr("description", result.first->second._description);
			(*itr)->GetAttr("comment", result.first->second._comment);

			++fieldsLoaded;

			XmlList domlist;
			if ((*itr)->find("field/value", domlist))
			{
				for(XmlList::const_iterator ditr(domlist.begin()); ditr != domlist.end(); ++ditr)
				{
					string enum_str, description;
					if ((*ditr)->GetAttr("enum", enum_str) && (*ditr)->GetAttr("description", description))
					{
						if (!result.first->second._dvals)
							result.first->second._dvals = new DomainMap;
						string lower, upper;
						bool isRange((*ditr)->GetAttr("range", lower) && (lower == "lower" || upper == "upper"));
						DomainObject *domval(DomainObject::create(enum_str, ft, isRange));
						if (isRange)
							result.first->second._dtype = DomainBase::dt_range;
						if (domval)
							result.first->second._dvals->insert(DomainMap::value_type(domval, description));
					}
					else
						cerr << "Value element missing required attributes at "
							<< inputFile << '(' << (*itr)->GetLine() << ')' << endl;
				}
			}
		}
		else
			cerr << "Field element missing required attributes at "
				<< inputFile << '(' << (*itr)->GetLine() << ')' << endl;
	}

	return fieldsLoaded;
}

//-----------------------------------------------------------------------------------------
#if 0
int loadmessages(XmlEntity& xf, MessageSpecMap& mspec)
{
	int msgssLoaded(0);

	XmlList mlist;
	if (!xf.find("fix/messages/message", mlist))
	{
		cerr << "No messages found in " << inputFile << endl;
		return 0;
	}

	for(XmlList::const_iterator itr(mlist.begin()); itr != mlist.end(); ++itr)
	{
		string msgcat, name, msgtype;
		if ((*itr)->GetAttr("msgtype", msgtype) && (*itr)->GetAttr("name", name) && (*itr)->GetAttr("msgcat", msgcat))
		{
			FieldTrait::FieldType ft(FieldSpec::_baseTypeMap.Find_Value(type));
			pair<FieldSpecMap::iterator, bool> result;
			if (ft != FieldTrait::ft_untyped)
				result = fspec.insert(FieldSpecMap::value_type(GetValue<unsigned>(number), FieldSpec(name, ft)));
			else
			{
				cerr << "Unknown field type: " << type << " in " << name << " at "
					<< inputFile << '(' << (*itr)->GetLine() << ')' << endl;
				continue;
			}

			(*itr)->GetAttr("description", result.first->second._description);
			(*itr)->GetAttr("domain", result.first->second._domain);

			++msgssLoaded;

			XmlList domlist;
			if ((*itr)->find("field/value", domlist))
			{
				for(XmlList::const_iterator ditr(domlist.begin()); ditr != domlist.end(); ++ditr)
				{
					string enum_str, description;
					if ((*ditr)->GetAttr("enum", enum_str) && (*ditr)->GetAttr("description", description))
					{
						if (!result.first->second._dvals)
							result.first->second._dvals = new DomainMap;
						result.first->second._dvals->insert(DomainMap::value_type(enum_str, description));
					}
					else
						cerr << "Value element missing required attributes at "
							<< inputFile << '(' << (*itr)->GetLine() << ')' << endl;
				}
			}
		}
		else
			cerr << "Message element missing required attributes at "
				<< inputFile << '(' << (*itr)->GetLine() << ')' << endl;
	}

	return msgssLoaded;
}
#endif

//-----------------------------------------------------------------------------------------
void processValueEnums(FieldSpecMap::const_iterator itr, ostream& ost_hpp, ostream& ost_cpp)
{
	string typestr("const ");
	if (FieldTrait::is_int(itr->second._ftype))
		typestr += "int ";
	else if (FieldTrait::is_char(itr->second._ftype))
		typestr += "char ";
	else if (FieldTrait::is_float(itr->second._ftype))
		typestr += "double ";
	else if (FieldTrait::is_string(itr->second._ftype))
		typestr += "std::string ";
	else
		return;

	ost_cpp << typestr << itr->second._name << "_domain[] = " << endl << spacer << "{ ";
	unsigned cnt(0);
	for (DomainMap::const_iterator ditr(itr->second._dvals->begin()); ditr != itr->second._dvals->end(); ++ditr)
	{
		if (cnt)
			ost_cpp << ", ";
		ost_cpp << *ditr->first;
		ost_hpp << typestr << itr->second._name << '_' << ditr->second;
		if (ditr->first->is_range())
		{
			if (cnt == 0)
				ost_hpp << "_lower";
			else if (cnt == 1)
				ost_hpp << "_upper";
		}
		ost_hpp << '(' << *ditr->first << ");" << endl;
		++cnt;
	}
	ost_hpp << "const size_t " << itr->second._name << "_dom_els(" << itr->second._dvals->size() << ");" << endl;
	ost_cpp << " };" << endl;
}

//-----------------------------------------------------------------------------------------
const string flname(const string& from)
{
	return from.substr(0, from.find_first_of('.'));
}

//-----------------------------------------------------------------------------------------
int process(XmlEntity& xf, Ctxt& ctxt)
{
	ostream& ost_cpp(*ctxt._out[Ctxt::types_cpp].second.get());
	ostream& ost_hpp(*ctxt._out[Ctxt::types_hpp].second.get());
	ostream& osc_cpp(*ctxt._out[Ctxt::classes_cpp].second.get());
	ostream& osc_hpp(*ctxt._out[Ctxt::classes_hpp].second.get());
	int result(0);

	// parse fields
	FieldSpecMap fspec;
	if (!loadfields(xf, fspec))
		return result;

	// output file preambles
	ost_hpp << _csMap.Find_Value_Ref(cs_do_not_edit) << endl;
	ost_hpp << _csMap.Find_Value_Ref(cs_divider) << endl;
	ost_hpp << _csMap.Find_Value_Ref(cs_copyright) << endl;
	ost_hpp << _csMap.Find_Value_Ref(cs_divider) << endl;
	ost_hpp << "#ifndef _" << flname(ctxt._out[Ctxt::types_hpp].first) << '_' << endl;
	ost_hpp << "#define _" << flname(ctxt._out[Ctxt::types_hpp].first) << '_' << endl << endl;
	ost_hpp << _csMap.Find_Value_Ref(cs_start_namespace) << endl;
	ost_hpp << "namespace " << ctxt._fixns << " {" << endl;

	ost_hpp << endl << _csMap.Find_Value_Ref(cs_divider) << endl;
	ost_cpp << _csMap.Find_Value_Ref(cs_do_not_edit) << endl;
	ost_cpp << _csMap.Find_Value_Ref(cs_divider) << endl;
	ost_cpp << _csMap.Find_Value_Ref(cs_copyright) << endl;
	ost_cpp << _csMap.Find_Value_Ref(cs_divider) << endl;
	ost_cpp << _csMap.Find_Value_Ref(cs_generated_includes) << endl;
	ost_cpp << "#include <" << ctxt._out[Ctxt::types_hpp].first << '>' << endl;
	ost_cpp << _csMap.Find_Value_Ref(cs_divider) << endl;
	ost_cpp << _csMap.Find_Value_Ref(cs_start_namespace) << endl;
	ost_cpp << "namespace " << ctxt._fixns << " {" << endl << endl;

	ost_cpp << _csMap.Find_Value_Ref(cs_start_anon_namespace) << endl;
	ost_cpp << endl << _csMap.Find_Value_Ref(cs_divider) << endl;
	// generate field types
	for (FieldSpecMap::const_iterator fitr(fspec.begin()); fitr != fspec.end(); ++fitr)
	{
		ost_hpp << "typedef Field<" << FieldSpec::_typeToCPP.Find_Value_Ref(fitr->second._ftype)
			<< ", " << fitr->first << "> " << fitr->second._name << ';' << endl;
		if (fitr->second._dvals)
			processValueEnums(fitr, ost_hpp, ost_cpp);
		ost_hpp << _csMap.Find_Value_Ref(cs_divider) << endl;
	}

	// generate dombase objs
	ost_cpp << endl << _csMap.Find_Value_Ref(cs_divider) << endl;
	ost_cpp << "const DomainBase dombases[] =" << endl << '{' << endl;
	unsigned dcnt(0);
	for (FieldSpecMap::iterator fitr(fspec.begin()); fitr != fspec.end(); ++fitr)
	{
		if (!fitr->second._dvals)
			continue;
		ost_cpp << spacer << "{ reinterpret_cast<const void *>(" << fitr->second._name << "_domain), "
			<< "static_cast<DomainBase::DomType>(" << fitr->second._dtype << "), " << endl << spacer << spacer
			<< "static_cast<FieldTrait::FieldType>(" << fitr->second._ftype << "), "
			<< fitr->second._dvals->size() << " }," << endl;
		fitr->second._doffset = dcnt++;
	}
	ost_cpp << "};" << endl;

	// generate field instantiators
	ost_cpp << endl << _csMap.Find_Value_Ref(cs_divider) << endl;
	for (FieldSpecMap::const_iterator fitr(fspec.begin()); fitr != fspec.end(); ++fitr)
	{
		ost_cpp << "BaseField *Create_" << fitr->second._name << "(const std::string& from, const BaseEntry *be)";
		ost_cpp << endl << spacer << "{ return new " << fitr->second._name << "(from, be->_dom); }" << endl;
	}

	ost_cpp << endl << _csMap.Find_Value_Ref(cs_end_anon_namespace) << endl;
	ost_cpp << "} // namespace " << ctxt._fixns << endl;

	// generate field instantiator lookup
	ost_hpp << "typedef GeneratedTable<unsigned, BaseEntry, " << ctxt._version << "> " << ctxt._clname << ';' << endl;

	ost_cpp << endl << _csMap.Find_Value_Ref(cs_divider) << endl;
	ost_cpp << "template<>" << endl << "const " << ctxt._fixns << "::" << ctxt._clname << "::Pair "
		<< ctxt._fixns << "::" << ctxt._clname << "::_pairs[] =" << endl << '{' << endl;
	for (FieldSpecMap::const_iterator fitr(fspec.begin()); fitr != fspec.end(); ++fitr)
	{
		if (fitr != fspec.begin())
			ost_cpp << ',' << endl;
		ost_cpp << spacer << "{ " << fitr->first << ", { &" << ctxt._fixns << "::Create_" << fitr->second._name << ", ";
		if (fitr->second._dvals)
			ost_cpp << "&" << ctxt._fixns << "::dombases[" << fitr->second._doffset << ']';
		else
			ost_cpp << '0';
		if (!fitr->second._comment.empty())
			ost_cpp << ',' << endl << spacer << spacer << '"' << fitr->second._comment << '"';
		else
			ost_cpp << ", 0";
		ost_cpp << " } }";
	}
	ost_cpp << endl << "};" << endl;
	ost_cpp << "template<>" << endl << "const size_t " << ctxt._fixns << "::" << ctxt._clname << "::_pairsz(sizeof(_pairs)/sizeof("
		<< ctxt._fixns << "::" << ctxt._clname << "));" << endl;
	ost_cpp << "template<>" << endl << "const " << ctxt._fixns << "::" << ctxt._clname << "::NoValType "
		<< ctxt._fixns << "::" << ctxt._clname << "::_noval = {0, 0};" << endl;

	// terminate files
	ost_hpp << endl << "} // namespace " << ctxt._fixns << endl;
	ost_hpp << _csMap.Find_Value_Ref(cs_end_namespace) << endl;
	ost_hpp << "#endif // _" << flname(ctxt._out[Ctxt::types_hpp].first) << '_' << endl;
	ost_cpp << endl << _csMap.Find_Value_Ref(cs_end_namespace) << endl;
	return result;
}

//-----------------------------------------------------------------------------------------
void print_usage()
{
	UsageMan um("f8c", GETARGLIST, "<input xml schema>");
	um.setdesc("f8c -- compile xml schema into fix8 application classes");
	um.add('o', "odir <file>", "output target directory");
	um.add('p', "prefix <file>", "output filename prefix");
	um.add('h', "help", "help, this screen");
	um.add('v', "version", "print version, exit");
	um.add("e.g.");
	um.add("@f8c -p myfix myfix.xml");
	um.print(cerr);
}

//-------------------------------------------------------------------------------------------------
DomainObject *DomainObject::create(const string& from, FieldTrait::FieldType ftype, bool isRange)
{
	if (FieldTrait::is_int(ftype))
		return new TypedDomain<int>(GetValue<int>(from), isRange);
	if (FieldTrait::is_char(ftype))
		return new CharDomain(from[0], isRange);
	if (FieldTrait::is_float(ftype))
		return new TypedDomain<double>(GetValue<double>(from), isRange);
	if (FieldTrait::is_string(ftype))
		return new StringDomain(from, isRange);
	return 0;
}

//-------------------------------------------------------------------------------------------------
