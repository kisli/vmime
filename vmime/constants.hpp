//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2009 Vincent Richard <vincent@vincent-richard.net>
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
		extern const string::value_type* const TEXT;
		extern const string::value_type* const MULTIPART;
		extern const string::value_type* const MESSAGE;
		extern const string::value_type* const APPLICATION;
		extern const string::value_type* const IMAGE;
		extern const string::value_type* const AUDIO;
		extern const string::value_type* const VIDEO;

		// Sub-types
		extern const string::value_type* const TEXT_PLAIN;
		extern const string::value_type* const TEXT_HTML;
		extern const string::value_type* const TEXT_RICHTEXT;
		extern const string::value_type* const TEXT_ENRICHED;
		extern const string::value_type* const TEXT_RFC822_HEADERS;  // RFC-1892
		extern const string::value_type* const TEXT_DIRECTORY;  // RFC-2426

		extern const string::value_type* const MULTIPART_MIXED;
		extern const string::value_type* const MULTIPART_RELATED;
		extern const string::value_type* const MULTIPART_ALTERNATIVE;
		extern const string::value_type* const MULTIPART_PARALLEL;
		extern const string::value_type* const MULTIPART_DIGEST;
		extern const string::value_type* const MULTIPART_REPORT;  // RFC-1892

		extern const string::value_type* const MESSAGE_RFC822;
		extern const string::value_type* const MESSAGE_PARTIAL;
		extern const string::value_type* const MESSAGE_EXTERNAL_BODY;
		extern const string::value_type* const MESSAGE_DISPOSITION_NOTIFICATION;

		extern const string::value_type* const APPLICATION_OCTET_STREAM;

		extern const string::value_type* const IMAGE_JPEG;
		extern const string::value_type* const IMAGE_GIF;

		extern const string::value_type* const AUDIO_BASIC;

		extern const string::value_type* const VIDEO_MPEG;
	}


	/** Constants for encoding types. */
	namespace encodingTypes
	{
		extern const string::value_type* const SEVEN_BIT;
		extern const string::value_type* const EIGHT_BIT;
		extern const string::value_type* const BASE64;
		extern const string::value_type* const QUOTED_PRINTABLE;
		extern const string::value_type* const BINARY;
		extern const string::value_type* const UUENCODE;
	}


	/** Constants for content disposition types (RFC-2183). */
	namespace contentDispositionTypes
	{
		extern const string::value_type* const INLINE;
		extern const string::value_type* const ATTACHMENT;
	}


	/** Constants for charsets. */
	namespace charsets
	{
		extern const string::value_type* const ISO8859_1;
		extern const string::value_type* const ISO8859_2;
		extern const string::value_type* const ISO8859_3;
		extern const string::value_type* const ISO8859_4;
		extern const string::value_type* const ISO8859_5;
		extern const string::value_type* const ISO8859_6;
		extern const string::value_type* const ISO8859_7;
		extern const string::value_type* const ISO8859_8;
		extern const string::value_type* const ISO8859_9;
		extern const string::value_type* const ISO8859_10;
		extern const string::value_type* const ISO8859_13;
		extern const string::value_type* const ISO8859_14;
		extern const string::value_type* const ISO8859_15;
		extern const string::value_type* const ISO8859_16;

		extern const string::value_type* const CP_437;
		extern const string::value_type* const CP_737;
		extern const string::value_type* const CP_775;
		extern const string::value_type* const CP_850;
		extern const string::value_type* const CP_852;
		extern const string::value_type* const CP_853;
		extern const string::value_type* const CP_855;
		extern const string::value_type* const CP_857;
		extern const string::value_type* const CP_858;
		extern const string::value_type* const CP_860;
		extern const string::value_type* const CP_861;
		extern const string::value_type* const CP_862;
		extern const string::value_type* const CP_863;
		extern const string::value_type* const CP_864;
		extern const string::value_type* const CP_865;
		extern const string::value_type* const CP_866;
		extern const string::value_type* const CP_869;
		extern const string::value_type* const CP_874;
		extern const string::value_type* const CP_1125;
		extern const string::value_type* const CP_1250;
		extern const string::value_type* const CP_1251;
		extern const string::value_type* const CP_1252;
		extern const string::value_type* const CP_1253;
		extern const string::value_type* const CP_1254;
		extern const string::value_type* const CP_1255;
		extern const string::value_type* const CP_1256;
		extern const string::value_type* const CP_1257;

		extern const string::value_type* const US_ASCII;

		extern const string::value_type* const UTF_7;
		extern const string::value_type* const UTF_8;
		extern const string::value_type* const UTF_16;
		extern const string::value_type* const UTF_32;

		extern const string::value_type* const WINDOWS_1250;
		extern const string::value_type* const WINDOWS_1251;
		extern const string::value_type* const WINDOWS_1252;
		extern const string::value_type* const WINDOWS_1253;
		extern const string::value_type* const WINDOWS_1254;
		extern const string::value_type* const WINDOWS_1255;
		extern const string::value_type* const WINDOWS_1256;
		extern const string::value_type* const WINDOWS_1257;
		extern const string::value_type* const WINDOWS_1258;
	}

	/** Constants for standard field names. */
	namespace fields
	{
		extern const string::value_type* const RECEIVED;
		extern const string::value_type* const FROM;
		extern const string::value_type* const SENDER;
		extern const string::value_type* const REPLY_TO;
		extern const string::value_type* const TO;
		extern const string::value_type* const CC;
		extern const string::value_type* const BCC;
		extern const string::value_type* const DATE;
		extern const string::value_type* const SUBJECT;
		extern const string::value_type* const ORGANIZATION;
		extern const string::value_type* const USER_AGENT;
		extern const string::value_type* const DELIVERED_TO;
		extern const string::value_type* const RETURN_PATH;
		extern const string::value_type* const MIME_VERSION;
		extern const string::value_type* const MESSAGE_ID;
		extern const string::value_type* const CONTENT_TYPE;
		extern const string::value_type* const CONTENT_TRANSFER_ENCODING;
		extern const string::value_type* const CONTENT_DESCRIPTION;
		extern const string::value_type* const CONTENT_DISPOSITION;
		extern const string::value_type* const CONTENT_ID;
		extern const string::value_type* const CONTENT_LOCATION;
		extern const string::value_type* const IN_REPLY_TO;
		extern const string::value_type* const REFERENCES;

		extern const string::value_type* const X_MAILER;
		extern const string::value_type* const X_PRIORITY;

		// RFC-3798: Message Disposition Notification
		extern const string::value_type* const ORIGINAL_MESSAGE_ID;
		extern const string::value_type* const DISPOSITION_NOTIFICATION_TO;
		extern const string::value_type* const DISPOSITION_NOTIFICATION_OPTIONS;
		extern const string::value_type* const DISPOSITION;
		extern const string::value_type* const FAILURE;
		extern const string::value_type* const ERROR;
		extern const string::value_type* const WARNING;
		extern const string::value_type* const ORIGINAL_RECIPIENT;
		extern const string::value_type* const FINAL_RECIPIENT;
		extern const string::value_type* const REPORTING_UA;
		extern const string::value_type* const MDN_GATEWAY;
	}

	/** Constants for disposition action modes (RFC-3978). */
	namespace dispositionActionModes
	{
		/** User implicitely displayed or deleted the message (filter or
		  * any other automatic action). */
		extern const string::value_type* const MANUAL;

		/** User explicitely displayed or deleted the message (manual action). */
		extern const string::value_type* const AUTOMATIC;
	}

	/** Constants for disposition sending modes (RFC-3798). */
	namespace dispositionSendingModes
	{
		/** The MDN was sent because the MUA had previously been configured
		  * to do so automatically. */
		extern const string::value_type* const SENT_MANUALLY;

		/** User explicitly gave permission for this particular MDN to be sent. */
		extern const string::value_type* const SENT_AUTOMATICALLY;
	}

	/** Constants for disposition types (RFC-3798). */
	namespace dispositionTypes
	{
		/** Message has been displayed to the user. */
		extern const string::value_type* const DISPLAYED;
		/** Message has been deleted without being displayed. */
		extern const string::value_type* const DELETED;
		/** Message has been denied. */
		extern const string::value_type* const DENIED;
	}

	/** Constants for disposition modifiers (RFC-3798). */
	namespace dispositionModifiers
	{
		extern const string::value_type* const ERROR;
	}
}


#endif // VMIME_CONSTANTS_HPP_INCLUDED
