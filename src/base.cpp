//
// VMime library (http://vmime.sourceforge.net)
// Copyright (C) 2002-2004 Vincent Richard <vincent@vincent-richard.net>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#include "config.hpp"

#include "charset.hpp"
#include "base.hpp"

#include "encoder.hpp"
#include "encoderB64.hpp"
#include "encoderQP.hpp"

#include "text.hpp"

#include "parserHelpers.hpp"

#include "utility/stringUtils.hpp"

// For initializing
#include "encoderFactory.hpp"
#include "headerFieldFactory.hpp"
#include "parameterFactory.hpp"
#include "textPartFactory.hpp"
#include "options.hpp"

#if VMIME_HAVE_MESSAGING_FEATURES
	#include "messaging/serviceFactory.hpp"
#endif


namespace vmime
{


/** "Null" (empty) string.
  */
const string NULL_STRING;

#if VMIME_WIDE_CHAR_SUPPORT
	/** "Null" (empty) wide-char string.
	  */
	const wstring NULL_WSTRING;
#endif

/** "Null" (empty) text.
  */
const text NULL_TEXT;


/** Return the library name (eg. "libvmime").
  *
  * @return library name
  */
const string libname() { return (VMIME_PACKAGE); }

/** Return the library version (eg. "0.5.2").
  *
  * @return library version
  */
const string libversion() { return (VMIME_VERSION " (" __DATE__ " " __TIME__ ")"); }


// New line sequence to be used when folding header fields.
const string NEW_LINE_SEQUENCE = "\r\n ";
const string::size_type NEW_LINE_SEQUENCE_LENGTH = 1;   // space

/** The CR-LF sequence.
  */
const string CRLF = "\r\n";


/** The current MIME version supported by VMime.
  */
const string MIME_VERSION = "1.0";


// Line length limits
namespace lineLengthLimits
{
	const string::size_type infinite = std::numeric_limits <string::size_type>::max();
}



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

		encoderFactory::getInstance();
		headerFieldFactory::getInstance();
		parameterFactory::getInstance();
		textPartFactory::getInstance();

		#if VMIME_HAVE_MESSAGING_FEATURES
			messaging::serviceFactory::getInstance();
		#endif
	}
};

initializer theInitializer;


} // vmime
