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
const string GETARGLIST("hvo:p:");
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
int processMessageFields(const std::string& where, XmlEntity *xt, FieldTraits& fts,
	const FieldToNumMap& ftonSpec, const FieldSpecMap& fspec);
int loadmessages(XmlEntity& xf, MessageSpecMap& mspec, const ComponentSpecMap& cspec,
	const FieldToNumMap& ftonSpec, const FieldSpecMap& fspec);
int loadcomponents(XmlEntity& xf, ComponentSpecMap& mspec, const FieldToNumMap& ftonSpec, const FieldSpecMap& fspec);
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
		prefix = "Myfix";

	//cout << "f8c " << _csMap.Find_Value_Ref(cs_copyright_short) << endl;

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
	if (loadFixVersion (*cfr, ctxt) < 0)
		return 1;
	for (unsigned ii(0); ii < Ctxt::count; ++ii)
	{
		ctxt._out[ii].first = prefix + ctxt._exts[ii];
		if (!ctxt._out[ii].second.reset(openofile(odir, ctxt._out[ii].first)))
			return 1;
	}

	cout << "Compiling fix version " << ctxt._version <<  " (" << ctxt._fixns << ")." << endl;
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
	fix->GetAttr("ns", ctxt._fixns);

	// fix version: <Major:1><Minor:1><Revision:2> eg. 4.2r10 is 4210
	ctxt._version = GetValue<int>(major) * 1000 + GetValue<int>(minor) * 100 + GetValue<int>(revision);
	if (ctxt._version < 4000 || ctxt._version > 6000)
	{
		cerr << "Invalid FIX version " << ctxt._version << " from fix header in " << inputFile << endl;
		return -1;
	}

	ostringstream ostr;
	ostr << "FIX" << ctxt._version;
	ctxt._systemns = ostr.str();
	if (ctxt._fixns.empty())
		ctxt._fixns = ctxt._systemns;
	ctxt._clname = prefix;
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
int processMessageFields(const std::string& where, XmlEntity *xt, FieldTraits& fts, const FieldToNumMap& ftonSpec,
	const FieldSpecMap& fspec)
{
	unsigned processed(0);
	XmlList flist;
	if (xt->find(where, flist))
	{
		bool hasMandatory(false);

		for(XmlList::const_iterator fitr(flist.begin()); fitr != flist.end(); ++fitr)
		{
			string fname, required;
			if ((*fitr)->GetAttr("name", fname) && (*fitr)->GetAttr("required", required))
			{
				FieldToNumMap::const_iterator ftonItr(ftonSpec.find(fname));
				FieldSpecMap::const_iterator fs_itr;
				if (ftonItr == ftonSpec.end() || (fs_itr = fspec.find(ftonItr->second)) == fspec.end())
				{
					cerr << "Could not locate Field " << fname << " from known field types in " << inputFile << endl;
					continue;
				}

				// add FieldTrait
				if (required == "Y")
					hasMandatory = true;
				if (!fts.add(FieldTrait(fs_itr->first, fs_itr->second._ftype, (*fitr)->GetSubIdx(), required == "Y", false, false)))
					cerr << "Could not add trait object " << fname << endl;
				else
					++processed;
			}
			else
				cerr << "Field element missing required attributes at "
					<< inputFile << '(' << (*fitr)->GetLine() << ')' << endl;
		}

		if (hasMandatory)
			fts.setHasMandatory();
	}

	return processed;
}

//-----------------------------------------------------------------------------------------
int loadcomponents(XmlEntity& xf, ComponentSpecMap& mspec, const FieldToNumMap& ftonSpec, const FieldSpecMap& fspec)
{
	int msgssLoaded(0);

	XmlList mlist;
	if (!xf.find("fix/components/component", mlist))
	{
		cerr << "No components found in " << inputFile << endl;
		return 0;
	}

	for(XmlList::const_iterator itr(mlist.begin()); itr != mlist.end(); ++itr)
	{
		string name;
		if ((*itr)->GetAttr("name", name))
		{
			pair<MessageSpecMap::iterator, bool> result(
				mspec.insert(ComponentSpecMap::value_type(name, ComponentSpec(name))));
			if (!result.second)
			{
				cerr << "Could not add component " << name << " at "
					<< inputFile << '(' << (*itr)->GetLine() << ')' << endl;
				continue;
			}

			XmlList grplist;
			if ((*itr)->find("component/group", grplist))
			{
				for(XmlList::const_iterator gitr(grplist.begin()); gitr != grplist.end(); ++gitr)
				{
					string gname, required;
					if ((*gitr)->GetAttr("name", gname) && (*gitr)->GetAttr("required", required))
					{
						// add group FieldTrait
						FieldToNumMap::const_iterator ftonItr(ftonSpec.find(gname));
						FieldSpecMap::const_iterator fs_itr;
						if (ftonItr != ftonSpec.end() && (fs_itr = fspec.find(ftonItr->second)) != fspec.end())
						{
							if (!result.first->second._fields.add(FieldTrait(fs_itr->first, FieldTrait::ft_int, (*gitr)->GetSubIdx(),
								required == "Y", true, false)))
									cerr << "Could not add group trait object " << gname << endl;
							else
							{
								pair<GroupMap::iterator, bool> gresult(
									result.first->second._groups.insert(GroupMap::value_type(fs_itr->first, FieldTraits())));
								if (!processMessageFields("group/field", *gitr, gresult.first->second, ftonSpec, fspec))
									cerr << "No fields found in component group " << gname << " at "
										<< inputFile << '(' << (*gitr)->GetLine() << ')' << endl;
								else
									gresult.first->second.setHasGroup();
							}
						}
						else
						{
							cerr << "Could not locate group Field " << gname << " from known field types in " << inputFile << endl;
							continue;
						}
					}
					else
						cerr << "Group element missing required attributes at "
							<< inputFile << '(' << (*gitr)->GetLine() << ')' << endl;
				}
			}
			(*itr)->GetAttr("comment", result.first->second._comment);

			processMessageFields("component/field", *itr, result.first->second._fields, ftonSpec, fspec);

			++msgssLoaded;
		}
		else
			cerr << "Component element missing required attributes at "
				<< inputFile << '(' << (*itr)->GetLine() << ')' << endl;
	}

	return msgssLoaded;
}

//-----------------------------------------------------------------------------------------
int loadmessages(XmlEntity& xf, MessageSpecMap& mspec, const ComponentSpecMap& cspec,
	const FieldToNumMap& ftonSpec, const FieldSpecMap& fspec)
{
	int msgssLoaded(0);

	XmlList mlist;
	if (!xf.find("fix/messages/message", mlist))
	{
		cerr << "No messages found in " << inputFile << endl;
		return 0;
	}

	// lookup msgtype domain - all messages must have corresponding entry here
	FieldSpecMap::const_iterator fsitr(fspec.find(35));	// always 35
	if (fsitr == fspec.end())
	{
		cerr << "Could not locate MsgType domain defintions in " << inputFile << endl;
		return 0;
	}

	for(XmlList::const_iterator itr(mlist.begin()); itr != mlist.end(); ++itr)
	{
		string msgcat, name, msgtype;
		if ((*itr)->GetAttr("msgtype", msgtype) && (*itr)->GetAttr("name", name) && (*itr)->GetAttr("msgcat", msgcat))
		{
			StringDomain sdom(msgtype, false);
			DomainMap::const_iterator ditr(fsitr->second._dvals->find(&sdom));
			if (ditr == fsitr->second._dvals->end())
			{
				cerr << "Message " << name << " does not have corrresponding entry in MsgType field"
					" domain at " << inputFile << '(' << (*itr)->GetLine() << ')' << endl;
				continue;
			}
			pair<MessageSpecMap::iterator, bool> result(
				mspec.insert(MessageSpecMap::value_type(msgtype, MessageSpec(name))));
			if (!result.second)
			{
				cerr << "Could not add message " << name << " (" << msgtype << ") at "
					<< inputFile << '(' << (*itr)->GetLine() << ')' << endl;
				continue;
			}

			XmlList grplist;
			if ((*itr)->find("message/group", grplist))
			{
				for(XmlList::const_iterator gitr(grplist.begin()); gitr != grplist.end(); ++gitr)
				{
					string gname, required;
					if ((*gitr)->GetAttr("name", gname) && (*gitr)->GetAttr("required", required))
					{
						// add group FieldTrait
						FieldToNumMap::const_iterator ftonItr(ftonSpec.find(gname));
						FieldSpecMap::const_iterator fs_itr;
						if (ftonItr != ftonSpec.end() && (fs_itr = fspec.find(ftonItr->second)) != fspec.end())
						{
							if (!result.first->second._fields.add(FieldTrait(fs_itr->first, FieldTrait::ft_int, (*gitr)->GetSubIdx(),
								required == "Y", true, false)))
									cerr << "Could not add group trait object " << gname << endl;
							else
							{
								pair<GroupMap::iterator, bool> gresult(
									result.first->second._groups.insert(GroupMap::value_type(fs_itr->first, FieldTraits())));
								if (!processMessageFields("group/field", *gitr, gresult.first->second, ftonSpec, fspec))
									cerr << "No fields found in group message " << gname << " at "
										<< inputFile << '(' << (*gitr)->GetLine() << ')' << endl;
								else
									gresult.first->second.setHasGroup();
							}
						}
						else
						{
							cerr << "Could not locate group Field " << gname << " from known field types in " << inputFile << endl;
							continue;
						}
					}
					else
						cerr << "Group element missing required attributes at "
							<< inputFile << '(' << (*gitr)->GetLine() << ')' << endl;
				}
			}

			(*itr)->GetAttr("comment", result.first->second._comment);

			if (!processMessageFields("message/field", *itr, result.first->second._fields, ftonSpec, fspec))
			{
				cerr << "No fields found in message " << name << " at "
					<< inputFile << '(' << (*itr)->GetLine() << ')' << endl;
				continue;
			}

			XmlList comlist;
			if ((*itr)->find("message/component", comlist))
			{
				for(XmlList::const_iterator citr(comlist.begin()); citr != comlist.end(); ++citr)
				{
					string cname, required;
					if (!(*citr)->GetAttr("name", cname) || !(*citr)->GetAttr("required", required))
						continue;
					ComponentSpecMap::const_iterator csitr(cspec.find(cname));
					if (csitr == cspec.end())
					{
						cerr << "Component not found " << name << " at "
							<< inputFile << '(' << (*citr)->GetLine() << ')' << endl;
						continue;
					}

					result.first->second._fields.add(FieldTrait(30000 + (*citr)->GetSubIdx(), FieldTrait::ft_untyped,
						(*citr)->GetSubIdx(), required == "Y", false, true));
				}
			}

			++msgssLoaded;
		}
		else
			cerr << "Message element missing required attributes at "
				<< inputFile << '(' << (*itr)->GetLine() << ')' << endl;
	}

	return msgssLoaded;
}

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

// ================================= Field processing =====================================
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

// ================================= Message processing ===================================

	FieldToNumMap ftonSpec;
	for (FieldSpecMap::const_iterator fs_itr(fspec.begin()); fs_itr != fspec.end(); ++fs_itr)
		ftonSpec.insert(FieldToNumMap::value_type(fs_itr->second._name, fs_itr->first));

	ComponentSpecMap cspec;
	loadcomponents(xf, cspec, ftonSpec, fspec);

	MessageSpecMap mspec;
	if (!loadmessages(xf, mspec, cspec, ftonSpec, fspec))
		return result;

	// output file preambles
	osc_hpp << _csMap.Find_Value_Ref(cs_do_not_edit) << endl;
	osc_hpp << _csMap.Find_Value_Ref(cs_divider) << endl;
	osc_hpp << _csMap.Find_Value_Ref(cs_copyright) << endl;
	osc_hpp << _csMap.Find_Value_Ref(cs_divider) << endl;
	osc_hpp << "#ifndef _" << flname(ctxt._out[Ctxt::classes_hpp].first) << '_' << endl;
	osc_hpp << "#define _" << flname(ctxt._out[Ctxt::classes_hpp].first) << '_' << endl << endl;
	osc_hpp << _csMap.Find_Value_Ref(cs_start_namespace) << endl;
	osc_hpp << "namespace " << ctxt._fixns << " {" << endl;

	osc_hpp << endl << _csMap.Find_Value_Ref(cs_divider) << endl;
	osc_cpp << _csMap.Find_Value_Ref(cs_do_not_edit) << endl;
	osc_cpp << _csMap.Find_Value_Ref(cs_divider) << endl;
	osc_cpp << _csMap.Find_Value_Ref(cs_copyright) << endl;
	osc_cpp << _csMap.Find_Value_Ref(cs_divider) << endl;
	osc_cpp << _csMap.Find_Value_Ref(cs_generated_includes) << endl;
	osc_cpp << "#include <" << ctxt._out[Ctxt::classes_hpp].first << '>' << endl;
	osc_cpp << _csMap.Find_Value_Ref(cs_divider) << endl;
	osc_cpp << _csMap.Find_Value_Ref(cs_start_namespace) << endl;
	osc_cpp << "namespace " << ctxt._fixns << " {" << endl << endl;

	FieldSpecMap::const_iterator fsitr(fspec.find(35));	// always 35
	for (MessageSpecMap::const_iterator mitr(mspec.begin()); mitr != mspec.end(); ++mitr)
	{
		osc_hpp << "class " << mitr->second._name << " : public Message" << endl << '{' << endl;

		if (mitr->second._fields.get_presence().size())
		{
			osc_cpp << _csMap.Find_Value_Ref(cs_divider) << endl;
			osc_cpp << "const FieldTrait::TraitBase " << mitr->second._name << "::_traits[] ="
				<< endl << '{' << endl;
			for (Presence::const_iterator flitr(mitr->second._fields.get_presence().begin());
				flitr != mitr->second._fields.get_presence().end(); ++flitr)
			{
				if (flitr != mitr->second._fields.get_presence().begin())
					osc_cpp << ',' << endl;
				osc_cpp << spacer << "{ " << flitr->_fnum << ", static_cast<FieldTrait::FieldType>("
					<< flitr->_ftype << "), " << flitr->_pos << ", "
					<< (flitr->_field_traits.has(FieldTrait::mandatory) ? '1' : '0') << ", "
					<< (flitr->_field_traits.has(FieldTrait::group) ? '1' : '0') << ", "
					<< (flitr->_field_traits.has(FieldTrait::component) ? '1' : '0') << " }";
			}
			osc_cpp << endl << "};" << endl;
			osc_cpp << "const size_t " << mitr->second._name << "::_fldcnt("
				<< mitr->second._fields.get_presence().size() << ");" << endl;
			osc_hpp << spacer << "static const FieldTrait::TraitBase _traits[];" << endl;
			osc_hpp << spacer << "static const size_t _fldcnt;" << endl << endl;
		}

		osc_hpp << "public:" << endl;
		osc_hpp << spacer << mitr->second._name << "();" << endl;
		osc_hpp << spacer << "virtual ~" << mitr->second._name << "() {} " << endl;

		osc_hpp << endl << spacer << "static const " << fsitr->second._name << " get_msgtype() { return " << fsitr->second._name
			<< "(\"" << mitr->first << "\"); }" << endl;
		osc_hpp << "};" << endl << endl;
		osc_hpp << _csMap.Find_Value_Ref(cs_divider) << endl;
	}

	// terminate files
	osc_hpp << endl << "} // namespace " << ctxt._fixns << endl;
	osc_hpp << _csMap.Find_Value_Ref(cs_end_namespace) << endl;
	osc_hpp << "#endif // _" << flname(ctxt._out[Ctxt::classes_hpp].first) << '_' << endl;
	osc_cpp << endl << _csMap.Find_Value_Ref(cs_end_namespace) << endl;
	osc_cpp << "} // namespace " << ctxt._fixns << endl;

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
