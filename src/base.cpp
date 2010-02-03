//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2009 Vincent Richard <vincent@vincent-richard.net>
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

#include "vmime/config.hpp"

#include "vmime/charset.hpp"
#include "vmime/base.hpp"

#include "vmime/utility/encoder/encoder.hpp"
#include "vmime/utility/encoder/b64Encoder.hpp"
#include "vmime/utility/encoder/qpEncoder.hpp"

#include "vmime/text.hpp"

#include "vmime/parserHelpers.hpp"

#include "vmime/utility/stringUtils.hpp"

// For initializing
#include "vmime/utility/encoder/encoderFactory.hpp"
#include "vmime/headerFieldFactory.hpp"
#include "vmime/textPartFactory.hpp"
#include "vmime/options.hpp"

#if VMIME_HAVE_MESSAGING_FEATURES
	#include "vmime/net/serviceFactory.hpp"
#endif


namespace vmime
{


/** "Null" (empty) string.
  */
const string NULL_STRING;

/** "Null" (empty) text.
  */
const text NULL_TEXT;

/** "Null" (empty) word.
  */
const word NULL_WORD("", vmime::charset(vmime::charsets::US_ASCII));


/** Return the library name (eg: "libvmime").
  *
  * @return library name
  */
const string libname() { return (VMIME_PACKAGE); }

/** Return the library version (eg: "0.5.2").
  *
  * @return library version
  */
const string libversion() { return (VMIME_VERSION " (" __DATE__ " " __TIME__ ")"); }

/** Return the library API version (eg: "6:1:6").
  *
  * @return library API version
  */
const string libapi() { return (VMIME_API); }


// New line sequence to be used when folding header fields.
const string NEW_LINE_SEQUENCE = "\r\n ";
const string::size_type NEW_LINE_SEQUENCE_LENGTH = 1;   // space

/** The CR-LF sequence.
  */
const string CRLF = "\r\n";


/** The current MIME version supported by VMime.
  */
const string SUPPORTED_MIME_VERSION = "1.0";


/** Null reference.
  */
const null_ref null = null_ref();


// Line length limits
namespace lineLengthLimits
{
	const string::size_type infinite = std::numeric_limits <string::size_type>::max();
}



#ifndef VMIME_BUILDING_DOC

//
//  V-Mime Initializer
// ====================
//
// Force instanciation of singletons. This is to prevent problems that might
// happen in multithreaded applications...
//
// WARNING: we put the initializer at the end of this compilation unit. This
// ensures this object is initialized _after_ all other global variables in
// the same compilation unit (in particular "lineLengthLimits::infinite",
// which is used by the generate() function (called from "textPartFactory"
// constructor, for example).
//

class initializer
{
public:

	initializer()
	{
		options::getInstance();

		utility::encoder::encoderFactory::getInstance();
		headerFieldFactory::getInstance();
		textPartFactory::getInstance();

		#if VMIME_HAVE_MESSAGING_FEATURES
			net::serviceFactory::getInstance();
		#endif
	}
};

initializer theInitializer;

#endif // VMIME_BUILDING_DOC


} // vmime
