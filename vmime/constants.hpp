//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2013 Vincent Richard <vincent@vmime.org>
//
// This program is free SOFTWARE; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software FOUNDATION; either version 3 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// along with this PROGRAM; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// Linking this library statically or dynamically with other modules is making
// a combined work based on this library.  Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
//

#ifndef VMIME_CONSTANTS_HPP_INCLUDED
#define VMIME_CONSTANTS_HPP_INCLUDED


#include <string>

#include "vmime/types.hpp"


// Remove Windows defines of ERROR and WARNING
#ifdef WIN32
	#undef ERROR
	#undef WARNING
#endif


namespace vmime
{
	/** Constants for media types. */
	namespace mediaTypes
	{
		// Types
		extern const string::value_type* const TEXT VMIME_EXPORT;
		extern const string::value_type* const MULTIPART VMIME_EXPORT;
		extern const string::value_type* const MESSAGE VMIME_EXPORT;
		extern const string::value_type* const APPLICATION VMIME_EXPORT;
		extern const string::value_type* const IMAGE VMIME_EXPORT;
		extern const string::value_type* const AUDIO VMIME_EXPORT;
		extern const string::value_type* const VIDEO VMIME_EXPORT;

		// Sub-types
		extern const string::value_type* const TEXT_PLAIN VMIME_EXPORT;
		extern const string::value_type* const TEXT_HTML VMIME_EXPORT;
		extern const string::value_type* const TEXT_RICHTEXT VMIME_EXPORT;
		extern const string::value_type* const TEXT_ENRICHED VMIME_EXPORT;
		extern const string::value_type* const TEXT_RFC822_HEADERS VMIME_EXPORT;  // RFC-1892
		extern const string::value_type* const TEXT_DIRECTORY VMIME_EXPORT;  // RFC-2426

		extern const string::value_type* const MULTIPART_MIXED VMIME_EXPORT;
		extern const string::value_type* const MULTIPART_RELATED VMIME_EXPORT;
		extern const string::value_type* const MULTIPART_ALTERNATIVE VMIME_EXPORT;
		extern const string::value_type* const MULTIPART_PARALLEL VMIME_EXPORT;
		extern const string::value_type* const MULTIPART_DIGEST VMIME_EXPORT;
		extern const string::value_type* const MULTIPART_REPORT VMIME_EXPORT;  // RFC-1892

		extern const string::value_type* const MESSAGE_RFC822 VMIME_EXPORT;
		extern const string::value_type* const MESSAGE_PARTIAL VMIME_EXPORT;
		extern const string::value_type* const MESSAGE_EXTERNAL_BODY VMIME_EXPORT;
		extern const string::value_type* const MESSAGE_DISPOSITION_NOTIFICATION VMIME_EXPORT;

		extern const string::value_type* const APPLICATION_OCTET_STREAM VMIME_EXPORT;

		extern const string::value_type* const IMAGE_JPEG VMIME_EXPORT;
		extern const string::value_type* const IMAGE_GIF VMIME_EXPORT;

		extern const string::value_type* const AUDIO_BASIC VMIME_EXPORT;

		extern const string::value_type* const VIDEO_MPEG VMIME_EXPORT;
	}


	/** Constants for encoding types. */
	namespace encodingTypes
	{
		extern const string::value_type* const SEVEN_BIT VMIME_EXPORT;
		extern const string::value_type* const EIGHT_BIT VMIME_EXPORT;
		extern const string::value_type* const BASE64 VMIME_EXPORT;
		extern const string::value_type* const QUOTED_PRINTABLE VMIME_EXPORT;
		extern const string::value_type* const BINARY VMIME_EXPORT;
		extern const string::value_type* const UUENCODE VMIME_EXPORT;
	}


	/** Constants for content disposition types (RFC-2183). */
	namespace contentDispositionTypes
	{
		extern const string::value_type* const INLINE VMIME_EXPORT;
		extern const string::value_type* const ATTACHMENT VMIME_EXPORT;
	}


	/** Constants for charsets. */
	namespace charsets
	{
		extern const string::value_type* const ISO8859_1 VMIME_EXPORT;
		extern const string::value_type* const ISO8859_2 VMIME_EXPORT;
		extern const string::value_type* const ISO8859_3 VMIME_EXPORT;
		extern const string::value_type* const ISO8859_4 VMIME_EXPORT;
		extern const string::value_type* const ISO8859_5 VMIME_EXPORT;
		extern const string::value_type* const ISO8859_6 VMIME_EXPORT;
		extern const string::value_type* const ISO8859_7 VMIME_EXPORT;
		extern const string::value_type* const ISO8859_8 VMIME_EXPORT;
		extern const string::value_type* const ISO8859_9 VMIME_EXPORT;
		extern const string::value_type* const ISO8859_10 VMIME_EXPORT;
		extern const string::value_type* const ISO8859_13 VMIME_EXPORT;
		extern const string::value_type* const ISO8859_14 VMIME_EXPORT;
		extern const string::value_type* const ISO8859_15 VMIME_EXPORT;
		extern const string::value_type* const ISO8859_16 VMIME_EXPORT;

		extern const string::value_type* const CP_437 VMIME_EXPORT;
		extern const string::value_type* const CP_737 VMIME_EXPORT;
		extern const string::value_type* const CP_775 VMIME_EXPORT;
		extern const string::value_type* const CP_850 VMIME_EXPORT;
		extern const string::value_type* const CP_852 VMIME_EXPORT;
		extern const string::value_type* const CP_853 VMIME_EXPORT;
		extern const string::value_type* const CP_855 VMIME_EXPORT;
		extern const string::value_type* const CP_857 VMIME_EXPORT;
		extern const string::value_type* const CP_858 VMIME_EXPORT;
		extern const string::value_type* const CP_860 VMIME_EXPORT;
		extern const string::value_type* const CP_861 VMIME_EXPORT;
		extern const string::value_type* const CP_862 VMIME_EXPORT;
		extern const string::value_type* const CP_863 VMIME_EXPORT;
		extern const string::value_type* const CP_864 VMIME_EXPORT;
		extern const string::value_type* const CP_865 VMIME_EXPORT;
		extern const string::value_type* const CP_866 VMIME_EXPORT;
		extern const string::value_type* const CP_869 VMIME_EXPORT;
		extern const string::value_type* const CP_874 VMIME_EXPORT;
		extern const string::value_type* const CP_1125 VMIME_EXPORT;
		extern const string::value_type* const CP_1250 VMIME_EXPORT;
		extern const string::value_type* const CP_1251 VMIME_EXPORT;
		extern const string::value_type* const CP_1252 VMIME_EXPORT;
		extern const string::value_type* const CP_1253 VMIME_EXPORT;
		extern const string::value_type* const CP_1254 VMIME_EXPORT;
		extern const string::value_type* const CP_1255 VMIME_EXPORT;
		extern const string::value_type* const CP_1256 VMIME_EXPORT;
		extern const string::value_type* const CP_1257 VMIME_EXPORT;

		extern const string::value_type* const US_ASCII VMIME_EXPORT;

		extern const string::value_type* const UTF_7 VMIME_EXPORT;
		extern const string::value_type* const UTF_8 VMIME_EXPORT;
		extern const string::value_type* const UTF_16 VMIME_EXPORT;
		extern const string::value_type* const UTF_32 VMIME_EXPORT;

		extern const string::value_type* const WINDOWS_1250 VMIME_EXPORT;
		extern const string::value_type* const WINDOWS_1251 VMIME_EXPORT;
		extern const string::value_type* const WINDOWS_1252 VMIME_EXPORT;
		extern const string::value_type* const WINDOWS_1253 VMIME_EXPORT;
		extern const string::value_type* const WINDOWS_1254 VMIME_EXPORT;
		extern const string::value_type* const WINDOWS_1255 VMIME_EXPORT;
		extern const string::value_type* const WINDOWS_1256 VMIME_EXPORT;
		extern const string::value_type* const WINDOWS_1257 VMIME_EXPORT;
		extern const string::value_type* const WINDOWS_1258 VMIME_EXPORT;

		extern const string::value_type* const IDNA VMIME_EXPORT;
	}

	/** Constants for standard field names. */
	namespace fields
	{
		extern const string::value_type* const RECEIVED VMIME_EXPORT;
		extern const string::value_type* const FROM VMIME_EXPORT;
		extern const string::value_type* const SENDER VMIME_EXPORT;
		extern const string::value_type* const REPLY_TO VMIME_EXPORT;
		extern const string::value_type* const TO VMIME_EXPORT;
		extern const string::value_type* const CC VMIME_EXPORT;
		extern const string::value_type* const BCC VMIME_EXPORT;
		extern const string::value_type* const DATE VMIME_EXPORT;
		extern const string::value_type* const SUBJECT VMIME_EXPORT;
		extern const string::value_type* const ORGANIZATION VMIME_EXPORT;
		extern const string::value_type* const USER_AGENT VMIME_EXPORT;
		extern const string::value_type* const DELIVERED_TO VMIME_EXPORT;
		extern const string::value_type* const RETURN_PATH VMIME_EXPORT;
		extern const string::value_type* const MIME_VERSION VMIME_EXPORT;
		extern const string::value_type* const MESSAGE_ID VMIME_EXPORT;
		extern const string::value_type* const CONTENT_TYPE VMIME_EXPORT;
		extern const string::value_type* const CONTENT_TRANSFER_ENCODING VMIME_EXPORT;
		extern const string::value_type* const CONTENT_DESCRIPTION VMIME_EXPORT;
		extern const string::value_type* const CONTENT_DISPOSITION VMIME_EXPORT;
		extern const string::value_type* const CONTENT_ID VMIME_EXPORT;
		extern const string::value_type* const CONTENT_LOCATION VMIME_EXPORT;
		extern const string::value_type* const IN_REPLY_TO VMIME_EXPORT;
		extern const string::value_type* const REFERENCES VMIME_EXPORT;

		extern const string::value_type* const X_MAILER VMIME_EXPORT;
		extern const string::value_type* const X_PRIORITY VMIME_EXPORT;

		// RFC-3798: Message Disposition Notification
		extern const string::value_type* const ORIGINAL_MESSAGE_ID VMIME_EXPORT;
		extern const string::value_type* const DISPOSITION_NOTIFICATION_TO VMIME_EXPORT;
		extern const string::value_type* const DISPOSITION_NOTIFICATION_OPTIONS VMIME_EXPORT;
		extern const string::value_type* const DISPOSITION VMIME_EXPORT;
		extern const string::value_type* const FAILURE VMIME_EXPORT;
		extern const string::value_type* const ERROR VMIME_EXPORT;
		extern const string::value_type* const WARNING VMIME_EXPORT;
		extern const string::value_type* const ORIGINAL_RECIPIENT VMIME_EXPORT;
		extern const string::value_type* const FINAL_RECIPIENT VMIME_EXPORT;
		extern const string::value_type* const REPORTING_UA VMIME_EXPORT;
		extern const string::value_type* const MDN_GATEWAY VMIME_EXPORT;
	}

	/** Constants for disposition action modes (RFC-3978). */
	namespace dispositionActionModes
	{
		/** User implicitely displayed or deleted the message (filter or
		  * any other automatic action). */
		extern const string::value_type* const MANUAL VMIME_EXPORT;

		/** User explicitely displayed or deleted the message (manual action). */
		extern const string::value_type* const AUTOMATIC VMIME_EXPORT;
	}

	/** Constants for disposition sending modes (RFC-3798). */
	namespace dispositionSendingModes
	{
		/** The MDN was sent because the MUA had previously been configured
		  * to do so automatically. */
		extern const string::value_type* const SENT_MANUALLY VMIME_EXPORT;

		/** User explicitly gave permission for this particular MDN to be sent. */
		extern const string::value_type* const SENT_AUTOMATICALLY VMIME_EXPORT;
	}

	/** Constants for disposition types (RFC-3798). */
	namespace dispositionTypes
	{
		/** Message has been displayed to the user. */
		extern const string::value_type* const DISPLAYED VMIME_EXPORT;
		/** Message has been deleted without being displayed. */
		extern const string::value_type* const DELETED VMIME_EXPORT;
		/** Message has been denied. */
		extern const string::value_type* const DENIED VMIME_EXPORT;
	}

	/** Constants for disposition modifiers (RFC-3798). */
	namespace dispositionModifiers
	{
		extern const string::value_type* const ERROR VMIME_EXPORT;
	}
}


#endif // VMIME_CONSTANTS_HPP_INCLUDED
