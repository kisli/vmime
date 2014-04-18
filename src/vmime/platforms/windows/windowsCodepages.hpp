//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2013 Vincent Richard <vincent@vmime.org>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// Linking this library statically or dynamically with other modules is making
// a combined work based on this library.  Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
//

#ifndef VMIME_PLATFORMS_WINDOWS_CODEPAGES_HPP_INCLUDED
#define VMIME_PLATFORMS_WINDOWS_CODEPAGES_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_PLATFORM_IS_WINDOWS


#include <string.h>


namespace vmime {
namespace platforms {
namespace windows {


class windowsCodepages
{
public:

	static int getByName(const char* s8_Name)
	{
		if (stricmp(s8_Name, "ASMO-708")          == 0) return 708;
		if (stricmp(s8_Name, "big5")              == 0) return 950;
		if (stricmp(s8_Name, "cp1025")            == 0) return 21025;
		if (stricmp(s8_Name, "cp866")             == 0) return 866;
		if (stricmp(s8_Name, "cp875")             == 0) return 875;
		if (stricmp(s8_Name, "DOS-720")           == 0) return 720;
		if (stricmp(s8_Name, "DOS-862")           == 0) return 862;
		if (stricmp(s8_Name, "EUC-CN")            == 0) return 51936;
		if (stricmp(s8_Name, "euc-jp")            == 0) return 51932;
		if (stricmp(s8_Name, "EUC-JP")            == 0) return 20932;
		if (stricmp(s8_Name, "euc-kr")            == 0) return 51949;
		if (stricmp(s8_Name, "GB18030")           == 0) return 54936;
		if (stricmp(s8_Name, "GBK")	          == 0) return 54936;
		if (stricmp(s8_Name, "gb2312")            == 0) return 936;
		if (stricmp(s8_Name, "hz-gb-2312")        == 0) return 52936;
		if (stricmp(s8_Name, "IBM00858")          == 0) return 858;
		if (stricmp(s8_Name, "IBM00924")          == 0) return 20924;
		if (stricmp(s8_Name, "IBM01047")          == 0) return 1047;
		if (stricmp(s8_Name, "IBM01140")          == 0) return 1140;
		if (stricmp(s8_Name, "IBM01141")          == 0) return 1141;
		if (stricmp(s8_Name, "IBM01142")          == 0) return 1142;
		if (stricmp(s8_Name, "IBM01143")          == 0) return 1143;
		if (stricmp(s8_Name, "IBM01144")          == 0) return 1144;
		if (stricmp(s8_Name, "IBM01145")          == 0) return 1145;
		if (stricmp(s8_Name, "IBM01146")          == 0) return 1146;
		if (stricmp(s8_Name, "IBM01147")          == 0) return 1147;
		if (stricmp(s8_Name, "IBM01148")          == 0) return 1148;
		if (stricmp(s8_Name, "IBM01149")          == 0) return 1149;
		if (stricmp(s8_Name, "IBM037")            == 0) return 37;
		if (stricmp(s8_Name, "IBM1026")           == 0) return 1026;
		if (stricmp(s8_Name, "IBM273")            == 0) return 20273;
		if (stricmp(s8_Name, "IBM277")            == 0) return 20277;
		if (stricmp(s8_Name, "IBM278")            == 0) return 20278;
		if (stricmp(s8_Name, "IBM280")            == 0) return 20280;
		if (stricmp(s8_Name, "IBM284")            == 0) return 20284;
		if (stricmp(s8_Name, "IBM285")            == 0) return 20285;
		if (stricmp(s8_Name, "IBM290")            == 0) return 20290;
		if (stricmp(s8_Name, "IBM297")            == 0) return 20297;
		if (stricmp(s8_Name, "IBM420")            == 0) return 20420;
		if (stricmp(s8_Name, "IBM423")            == 0) return 20423;
		if (stricmp(s8_Name, "IBM424")            == 0) return 20424;
		if (stricmp(s8_Name, "IBM437")            == 0) return 437;
		if (stricmp(s8_Name, "IBM500")            == 0) return 500;
		if (stricmp(s8_Name, "ibm737")            == 0) return 737;
		if (stricmp(s8_Name, "ibm775")            == 0) return 775;
		if (stricmp(s8_Name, "ibm850")            == 0) return 850;
		if (stricmp(s8_Name, "ibm852")            == 0) return 852;
		if (stricmp(s8_Name, "IBM855")            == 0) return 855;
		if (stricmp(s8_Name, "ibm857")            == 0) return 857;
		if (stricmp(s8_Name, "IBM860")            == 0) return 860;
		if (stricmp(s8_Name, "ibm861")            == 0) return 861;
		if (stricmp(s8_Name, "IBM863")            == 0) return 863;
		if (stricmp(s8_Name, "IBM864")            == 0) return 864;
		if (stricmp(s8_Name, "IBM865")            == 0) return 865;
		if (stricmp(s8_Name, "ibm869")            == 0) return 869;
		if (stricmp(s8_Name, "IBM870")            == 0) return 870;
		if (stricmp(s8_Name, "IBM871")            == 0) return 20871;
		if (stricmp(s8_Name, "IBM880")            == 0) return 20880;
		if (stricmp(s8_Name, "IBM905")            == 0) return 20905;
		if (stricmp(s8_Name, "IBM-Thai")          == 0) return 20838;
		if (stricmp(s8_Name, "iso-2022-jp")       == 0) return 50222;
		if (stricmp(s8_Name, "iso-2022-kr")       == 0) return 50225;
		if (stricmp(s8_Name, "iso-8859-1")        == 0) return 28591;
		if (stricmp(s8_Name, "iso-8859-13")       == 0) return 28603;
		if (stricmp(s8_Name, "iso-8859-15")       == 0) return 28605;
		if (stricmp(s8_Name, "iso-8859-2")        == 0) return 28592;
		if (stricmp(s8_Name, "iso-8859-3")        == 0) return 28593;
		if (stricmp(s8_Name, "iso-8859-4")        == 0) return 28594;
		if (stricmp(s8_Name, "iso-8859-5")        == 0) return 28595;
		if (stricmp(s8_Name, "iso-8859-6")        == 0) return 28596;
		if (stricmp(s8_Name, "iso-8859-7")        == 0) return 28597;
		if (stricmp(s8_Name, "iso-8859-8")        == 0) return 28598;
		if (stricmp(s8_Name, "iso-8859-8-i")      == 0) return 38598;
		if (stricmp(s8_Name, "iso-8859-9")        == 0) return 28599;
		if (stricmp(s8_Name, "Johab")             == 0) return 1361;
		if (stricmp(s8_Name, "koi8-r")            == 0) return 20866;
		if (stricmp(s8_Name, "koi8-u")            == 0) return 21866;
		if (stricmp(s8_Name, "ks_c_5601-1987")    == 0) return 949;
		if (stricmp(s8_Name, "macintosh")         == 0) return 10000;
		if (stricmp(s8_Name, "unicodeFFFE")       == 0) return 1201;
		if (stricmp(s8_Name, "us-ascii")          == 0) return 20127;
		if (stricmp(s8_Name, "utf-16")            == 0) return 1200;
		if (stricmp(s8_Name, "utf-32")            == 0) return 12000;
		if (stricmp(s8_Name, "utf-32BE")          == 0) return 12001;
		if (stricmp(s8_Name, "utf-7")             == 0) return 65000;
		if (stricmp(s8_Name, "utf-8")             == 0) return 65001;
		if (stricmp(s8_Name, "windows-1250")      == 0) return 1250;
		if (stricmp(s8_Name, "windows-1251")      == 0) return 1251;
		if (stricmp(s8_Name, "Windows-1252")      == 0) return 1252;
		if (stricmp(s8_Name, "windows-1253")      == 0) return 1253;
		if (stricmp(s8_Name, "windows-1254")      == 0) return 1254;
		if (stricmp(s8_Name, "windows-1255")      == 0) return 1255;
		if (stricmp(s8_Name, "windows-1256")      == 0) return 1256;
		if (stricmp(s8_Name, "windows-1257")      == 0) return 1257;
		if (stricmp(s8_Name, "windows-1258")      == 0) return 1258;
		if (stricmp(s8_Name, "windows-874")       == 0) return 874;
		if (stricmp(s8_Name, "x-Chinese-CNS")     == 0) return 20000;
		if (stricmp(s8_Name, "x-Chinese-Eten")    == 0) return 20002;
		if (stricmp(s8_Name, "x-cp20001")         == 0) return 20001;
		if (stricmp(s8_Name, "x-cp20003")         == 0) return 20003;
		if (stricmp(s8_Name, "x-cp20004")         == 0) return 20004;
		if (stricmp(s8_Name, "x-cp20005")         == 0) return 20005;
		if (stricmp(s8_Name, "x-cp20261")         == 0) return 20261;
		if (stricmp(s8_Name, "x-cp20269")         == 0) return 20269;
		if (stricmp(s8_Name, "x-cp20936")         == 0) return 20936;
		if (stricmp(s8_Name, "x-cp20949")         == 0) return 20949;
		if (stricmp(s8_Name, "x-cp50227")         == 0) return 50227;
		if (stricmp(s8_Name, "x-EBCDIC-KoreanExtended") == 0) return 20833;
		if (stricmp(s8_Name, "x-Europa")          == 0) return 29001;
		if (stricmp(s8_Name, "x-IA5")             == 0) return 20105;
		if (stricmp(s8_Name, "x-IA5-German")      == 0) return 20106;
		if (stricmp(s8_Name, "x-IA5-Norwegian")   == 0) return 20108;
		if (stricmp(s8_Name, "x-IA5-Swedish")     == 0) return 20107;
		if (stricmp(s8_Name, "x-iscii-as")        == 0) return 57006;
		if (stricmp(s8_Name, "x-iscii-be")        == 0) return 57003;
		if (stricmp(s8_Name, "x-iscii-de")        == 0) return 57002;
		if (stricmp(s8_Name, "x-iscii-gu")        == 0) return 57010;
		if (stricmp(s8_Name, "x-iscii-ka")        == 0) return 57008;
		if (stricmp(s8_Name, "x-iscii-ma")        == 0) return 57009;
		if (stricmp(s8_Name, "x-iscii-or")        == 0) return 57007;
		if (stricmp(s8_Name, "x-iscii-pa")        == 0) return 57011;
		if (stricmp(s8_Name, "x-iscii-ta")        == 0) return 57004;
		if (stricmp(s8_Name, "x-iscii-te")        == 0) return 57005;
		if (stricmp(s8_Name, "x-mac-arabic")      == 0) return 10004;
		if (stricmp(s8_Name, "x-mac-ce")          == 0) return 10029;
		if (stricmp(s8_Name, "x-mac-chinesesimp") == 0) return 10008;
		if (stricmp(s8_Name, "x-mac-chinesetrad") == 0) return 10002;
		if (stricmp(s8_Name, "x-mac-croatian")    == 0) return 10082;
		if (stricmp(s8_Name, "x-mac-cyrillic")    == 0) return 10007;
		if (stricmp(s8_Name, "x-mac-greek")       == 0) return 10006;
		if (stricmp(s8_Name, "x-mac-hebrew")      == 0) return 10005;
		if (stricmp(s8_Name, "x-mac-icelandic")   == 0) return 10079;
		if (stricmp(s8_Name, "x-mac-japanese")    == 0) return 10001;
		if (stricmp(s8_Name, "x-mac-korean")      == 0) return 10003;
		if (stricmp(s8_Name, "x-mac-romanian")    == 0) return 10010;
		if (stricmp(s8_Name, "x-mac-thai")        == 0) return 10021;
		if (stricmp(s8_Name, "x-mac-turkish")     == 0) return 10081;
		if (stricmp(s8_Name, "x-mac-ukrainian")   == 0) return 10017;

		throw exception(std::string("Unknown charset: ") + s8_Name);
	}
};


} // windows
} // platforms
} // vmime


#endif // VMIME_PLATFORM_IS_WINDOWS

#endif // VMIME_PLATFORMS_WINDOWS_CODEPAGES_HPP_INCLUDED
