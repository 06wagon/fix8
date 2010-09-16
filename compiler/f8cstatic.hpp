//-------------------------------------------------------------------------------------------------
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

---------------------------------------------------------------------------------------------------
$Id$
$Date$
$URL$

#endif
//-------------------------------------------------------------------------------------------------
template<>
const BaseTypeMap::TypePair BaseTypeMap::_valueTable[] =
{
	BaseTypeMap::TypePair("INT", FieldTrait::ft_int),
	BaseTypeMap::TypePair("LENGTH", FieldTrait::ft_Length),
	BaseTypeMap::TypePair("TAGNUM", FieldTrait::ft_TagNum),
	BaseTypeMap::TypePair("SEQNUM", FieldTrait::ft_SeqNum),
	BaseTypeMap::TypePair("NUMINGROUP", FieldTrait::ft_NumInGroup),
	BaseTypeMap::TypePair("DAYOFMONTH", FieldTrait::ft_DayOfMonth),
	BaseTypeMap::TypePair("FLOAT", FieldTrait::ft_float),
	BaseTypeMap::TypePair("QTY", FieldTrait::ft_Qty),
	BaseTypeMap::TypePair("PRICE", FieldTrait::ft_Price),
	BaseTypeMap::TypePair("PRICEOFFSET", FieldTrait::ft_PriceOffset),
	BaseTypeMap::TypePair("AMT", FieldTrait::ft_Amt),
	BaseTypeMap::TypePair("PERCENTAGE", FieldTrait::ft_Percentage),
	BaseTypeMap::TypePair("CHAR", FieldTrait::ft_char),
	BaseTypeMap::TypePair("BOOLEAN", FieldTrait::ft_Boolean),
	BaseTypeMap::TypePair("STRING", FieldTrait::ft_string),
	BaseTypeMap::TypePair("MULTIPLECHARVALUE", FieldTrait::ft_MultipleCharValue),
	BaseTypeMap::TypePair("MULTIPLESTRINGVALUE", FieldTrait::ft_MultipleStringValue),
	BaseTypeMap::TypePair("COUNTRY", FieldTrait::ft_Country),
	BaseTypeMap::TypePair("CURRENCY", FieldTrait::ft_Currency),
	BaseTypeMap::TypePair("EXCHANGE", FieldTrait::ft_Exchange),
	BaseTypeMap::TypePair("MONTHYEAR", FieldTrait::ft_MonthYear),
	BaseTypeMap::TypePair("UTCTIMESTAMP", FieldTrait::ft_UTCTimestamp),
	BaseTypeMap::TypePair("UTCTIMEONLY", FieldTrait::ft_UTCTimeOnly),
	BaseTypeMap::TypePair("UTCDATEONLY", FieldTrait::ft_UTCDateOnly),
	BaseTypeMap::TypePair("LOCALMKTDATE", FieldTrait::ft_LocalMktDate),
	BaseTypeMap::TypePair("TZTIMEONLY", FieldTrait::ft_TZTimeOnly),
	BaseTypeMap::TypePair("TZTIMESTAMP", FieldTrait::ft_TZTimestamp),
	BaseTypeMap::TypePair("XMLDATA", FieldTrait::ft_XMLData),
	BaseTypeMap::TypePair("DATA", FieldTrait::ft_data),
	BaseTypeMap::TypePair("PATTERN", FieldTrait::ft_pattern),
	BaseTypeMap::TypePair("TENOR", FieldTrait::ft_Tenor),
	BaseTypeMap::TypePair("RESERVED100PLUS", FieldTrait::ft_Reserved100Plus),
	BaseTypeMap::TypePair("RESERVED1000PLUS", FieldTrait::ft_Reserved1000Plus),
	BaseTypeMap::TypePair("RESERVED4000PLUS", FieldTrait::ft_Reserved4000Plus)
};
template<>
const BaseTypeMap::TypeMap BaseTypeMap::_valuemap(BaseTypeMap::_valueTable, BaseTypeMap::Get_Table_End());
template<>
const BaseTypeMap::NoValType BaseTypeMap::_noval(FieldTrait::ft_untyped);

//-------------------------------------------------------------------------------------------------
template<>
const TypeToCPP::TypePair TypeToCPP::_valueTable[] =
{
	TypeToCPP::TypePair(FieldTrait::ft_int, "int"),
	TypeToCPP::TypePair(FieldTrait::ft_Length, "Length"),
	TypeToCPP::TypePair(FieldTrait::ft_TagNum, "Tagnum"),
	TypeToCPP::TypePair(FieldTrait::ft_SeqNum, "Seqnum"),
	TypeToCPP::TypePair(FieldTrait::ft_NumInGroup, "NumInGroup"),
	TypeToCPP::TypePair(FieldTrait::ft_DayOfMonth, "DayOfMonth"),
	TypeToCPP::TypePair(FieldTrait::ft_float, "double"),
	TypeToCPP::TypePair(FieldTrait::ft_Qty, "Qty"),
	TypeToCPP::TypePair(FieldTrait::ft_Price, "Price"),
	TypeToCPP::TypePair(FieldTrait::ft_PriceOffset, "PriceOffset"),
	TypeToCPP::TypePair(FieldTrait::ft_Amt, "Amt"),
	TypeToCPP::TypePair(FieldTrait::ft_Percentage, "Percentage"),
	TypeToCPP::TypePair(FieldTrait::ft_char, "char"),
	TypeToCPP::TypePair(FieldTrait::ft_Boolean, "Boolean"),
	TypeToCPP::TypePair(FieldTrait::ft_string, "std::string"),
	TypeToCPP::TypePair(FieldTrait::ft_MultipleCharValue, "MultipleCharValue"),
	TypeToCPP::TypePair(FieldTrait::ft_MultipleStringValue, "MultipleStringValue"),
	TypeToCPP::TypePair(FieldTrait::ft_Country, "Country"),
	TypeToCPP::TypePair(FieldTrait::ft_Currency, "Currency"),
	TypeToCPP::TypePair(FieldTrait::ft_Exchange, "Exchange"),
	TypeToCPP::TypePair(FieldTrait::ft_MonthYear, "MonthYear"),
	TypeToCPP::TypePair(FieldTrait::ft_UTCTimestamp, "UTCTimestamp"),
	TypeToCPP::TypePair(FieldTrait::ft_UTCTimeOnly, "UTCTimeOnly"),
	TypeToCPP::TypePair(FieldTrait::ft_UTCDateOnly, "UTCDateOnly"),
	TypeToCPP::TypePair(FieldTrait::ft_LocalMktDate, "LocalMktDate"),
	TypeToCPP::TypePair(FieldTrait::ft_TZTimeOnly, "TZTimeOnly"),
	TypeToCPP::TypePair(FieldTrait::ft_TZTimestamp, "TZTimestamp"),
	TypeToCPP::TypePair(FieldTrait::ft_XMLData, "XMLData"),
	TypeToCPP::TypePair(FieldTrait::ft_data, "data"),
	TypeToCPP::TypePair(FieldTrait::ft_pattern, "pattern"),
	TypeToCPP::TypePair(FieldTrait::ft_Tenor, "Tenor"),
	TypeToCPP::TypePair(FieldTrait::ft_Reserved100Plus, "std::string"),
	TypeToCPP::TypePair(FieldTrait::ft_Reserved1000Plus, "std::string"),
	TypeToCPP::TypePair(FieldTrait::ft_Reserved4000Plus, "std::string"),
	TypeToCPP::TypePair(FieldTrait::ft_Language, "Language")
};
template<>
const TypeToCPP::TypeMap TypeToCPP::_valuemap(TypeToCPP::_valueTable, TypeToCPP::Get_Table_End());
template<>
const TypeToCPP::NoValType TypeToCPP::_noval("Unknown");

//-------------------------------------------------------------------------------------------------
template<>
const CSMap::TypePair CSMap::_valueTable[] =
{
	CSMap::TypePair(cs_do_not_edit, "// *** f8c generated file: Do Not Edit"),
	CSMap::TypePair(cs_start_namespace, "namespace FIX8 {"),
	CSMap::TypePair(cs_end_namespace, "} // namespace FIX8"),
	CSMap::TypePair(cs_divider,
"//-------------------------------------------------------------------------------------------------"),

	CSMap::TypePair(cs_copyright,
"#if 0\n"
"\n"
"Fix8 is released under the New BSD License.\n"
"\n"
"Copyright (c) 2010, David L. Dight <www@orbweb.org>\n"
"All rights reserved.\n"
"\n"
"Redistribution and use in source and binary forms, with or without modification, are\n"
"permitted provided that the following conditions are met:\n"
"\n"
"    * Redistributions of source code must retain the above copyright notice, this list of\n"
"	 	conditions and the following disclaimer.\n"
"    * Redistributions in binary form must reproduce the above copyright notice, this list\n"
"	 	of conditions and the following disclaimer in the documentation and/or other\n"
"		materials provided with the distribution.\n"
"    * Neither the name of the author nor the names of its contributors may be used to\n"
"	 	endorse or promote products derived from this software without specific prior\n"
"		written permission.\n"
"\n"
"THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND ANY EXPRESS\n"
"OR  IMPLIED  WARRANTIES,  INCLUDING,  BUT  NOT  LIMITED  TO ,  THE  IMPLIED  WARRANTIES  OF\n"
"MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE ARE  DISCLAIMED. IN  NO EVENT  SHALL\n"
"THE  COPYRIGHT  OWNER OR  CONTRIBUTORS BE  LIABLE  FOR  ANY DIRECT,  INDIRECT,  INCIDENTAL,\n"
"SPECIAL,  EXEMPLARY, OR CONSEQUENTIAL  DAMAGES (INCLUDING,  BUT NOT LIMITED TO, PROCUREMENT\n"
"OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF USE, DATA,  OR PROFITS; OR BUSINESS INTERRUPTION)\n"
"HOWEVER CAUSED  AND ON ANY THEORY OF LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR\n"
"TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE\n"
"EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n"
"\n"
"#endif\n"),
	CSMap::TypePair(cs_copyright_short,
	"Copyright (c) 2010, David L. Dight <www@orbweb.org>, All rights reserved."),

	CSMap::TypePair(cs_fcreate_entry_hpp,
		"typedef GeneratedTable<unsigned, BaseField *(*)(const std::string&)> FCreateEntry;"),
	CSMap::TypePair(cs_fcreate_entry_table,
		"template<>\nconst FCreateEntry::Pair FCreateEntry::_pairs[] =\n{"),
	CSMap::TypePair(cs_fcreate_entry_cpp,
		"template<>\nconst size_t FCreateEntry::_pairsz(sizeof(_pairs)/sizeof(FCreateEntry));\n"
		"template<>\nconst FCreateEntry::NoValType FCreateEntry::_noval(0);"),
};
template<>
const CSMap::TypeMap CSMap::_valuemap(CSMap::_valueTable, CSMap::Get_Table_End());
template<>
const CSMap::NoValType CSMap::_noval("not found");

//-------------------------------------------------------------------------------------------------
