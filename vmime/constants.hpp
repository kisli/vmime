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
#ifdef _WIN32
	#undef ERROR
	#undef WARNING
#endif


namespace vmime
{
	/** Constants for media types. */
	namespace mediaTypes
	{
		// Types
		extern VMIME_EXPORT const char* const TEXT;
		extern VMIME_EXPORT const char* const MULTIPART;
		extern VMIME_EXPORT const char* const MESSAGE;
		extern VMIME_EXPORT const char* const APPLICATION;
		extern VMIME_EXPORT const char* const IMAGE;
		extern VMIME_EXPORT const char* const AUDIO;
		extern VMIME_EXPORT const char* const VIDEO;

		// Sub-types
		extern VMIME_EXPORT const char* const TEXT_PLAIN;
		extern VMIME_EXPORT const char* const TEXT_HTML;
		extern VMIME_EXPORT const char* const TEXT_RICHTEXT;
		extern VMIME_EXPORT const char* const TEXT_ENRICHED;
		extern VMIME_EXPORT const char* const TEXT_RFC822_HEADERS;  // RFC-1892
		extern VMIME_EXPORT const char* const TEXT_DIRECTORY;  // RFC-2426

		extern VMIME_EXPORT const char* const MULTIPART_MIXED;
		extern VMIME_EXPORT const char* const MULTIPART_RELATED;
		extern VMIME_EXPORT const char* const MULTIPART_ALTERNATIVE;
		extern VMIME_EXPORT const char* const MULTIPART_PARALLEL;
		extern VMIME_EXPORT const char* const MULTIPART_DIGEST;
		extern VMIME_EXPORT const char* const MULTIPART_REPORT;  // RFC-1892

		extern VMIME_EXPORT const char* const MESSAGE_RFC822;
		extern VMIME_EXPORT const char* const MESSAGE_PARTIAL;
		extern VMIME_EXPORT const char* const MESSAGE_EXTERNAL_BODY;
		extern VMIME_EXPORT const char* const MESSAGE_DISPOSITION_NOTIFICATION;

		extern VMIME_EXPORT const char* const APPLICATION_OCTET_STREAM;

		extern VMIME_EXPORT const char* const IMAGE_JPEG;
		extern VMIME_EXPORT const char* const IMAGE_GIF;

		extern VMIME_EXPORT const char* const AUDIO_BASIC;

		extern VMIME_EXPORT const char* const VIDEO_MPEG;
	}


	/** Constants for encoding types. */
	namespace encodingTypes
	{
		extern VMIME_EXPORT const char* const SEVEN_BIT;
		extern VMIME_EXPORT const char* const EIGHT_BIT;
		extern VMIME_EXPORT const char* const BASE64;
		extern VMIME_EXPORT const char* const QUOTED_PRINTABLE;
		extern VMIME_EXPORT const char* const BINARY;
		extern VMIME_EXPORT const char* const UUENCODE;
	}


	/** Constants for content disposition types (RFC-2183). */
	namespace contentDispositionTypes
	{
		extern VMIME_EXPORT const char* const INLINE;
		extern VMIME_EXPORT const char* const ATTACHMENT;
	}


	/** Constants for charsets. */
	namespace charsets
	{
		extern VMIME_EXPORT const char* const ISO8859_1;
		extern VMIME_EXPORT const char* const ISO8859_2;
		extern VMIME_EXPORT const char* const ISO8859_3;
		extern VMIME_EXPORT const char* const ISO8859_4;
		extern VMIME_EXPORT const char* const ISO8859_5;
		extern VMIME_EXPORT const char* const ISO8859_6;
		extern VMIME_EXPORT const char* const ISO8859_7;
		extern VMIME_EXPORT const char* const ISO8859_8;
		extern VMIME_EXPORT const char* const ISO8859_9;
		extern VMIME_EXPORT const char* const ISO8859_10;
		extern VMIME_EXPORT const char* const ISO8859_13;
		extern VMIME_EXPORT const char* const ISO8859_14;
		extern VMIME_EXPORT const char* const ISO8859_15;
		extern VMIME_EXPORT const char* const ISO8859_16;

		extern VMIME_EXPORT const char* const CP_437;
		extern VMIME_EXPORT const char* const CP_737;
		extern VMIME_EXPORT const char* const CP_775;
		extern VMIME_EXPORT const char* const CP_850;
		extern VMIME_EXPORT const char* const CP_852;
		extern VMIME_EXPORT const char* const CP_853;
		extern VMIME_EXPORT const char* const CP_855;
		extern VMIME_EXPORT const char* const CP_857;
		extern VMIME_EXPORT const char* const CP_858;
		extern VMIME_EXPORT const char* const CP_860;
		extern VMIME_EXPORT const char* const CP_861;
		extern VMIME_EXPORT const char* const CP_862;
		extern VMIME_EXPORT const char* const CP_863;
		extern VMIME_EXPORT const char* const CP_864;
		extern VMIME_EXPORT const char* const CP_865;
		extern VMIME_EXPORT const char* const CP_866;
		extern VMIME_EXPORT const char* const CP_869;
		extern VMIME_EXPORT const char* const CP_874;
		extern VMIME_EXPORT const char* const CP_1125;
		extern VMIME_EXPORT const char* const CP_1250;
		extern VMIME_EXPORT const char* const CP_1251;
		extern VMIME_EXPORT const char* const CP_1252;
		extern VMIME_EXPORT const char* const CP_1253;
		extern VMIME_EXPORT const char* const CP_1254;
		extern VMIME_EXPORT const char* const CP_1255;
		extern VMIME_EXPORT const char* const CP_1256;
		extern VMIME_EXPORT const char* const CP_1257;

		extern VMIME_EXPORT const char* const US_ASCII;

		extern VMIME_EXPORT const char* const UTF_7;
		extern VMIME_EXPORT const char* const UTF_8;
		extern VMIME_EXPORT const char* const UTF_16;
		extern VMIME_EXPORT const char* const UTF_32;

		extern VMIME_EXPORT const char* const WINDOWS_1250;
		extern VMIME_EXPORT const char* const WINDOWS_1251;
		extern VMIME_EXPORT const char* const WINDOWS_1252;
		extern VMIME_EXPORT const char* const WINDOWS_1253;
		extern VMIME_EXPORT const char* const WINDOWS_1254;
		extern VMIME_EXPORT const char* const WINDOWS_1255;
		extern VMIME_EXPORT const char* const WINDOWS_1256;
		extern VMIME_EXPORT const char* const WINDOWS_1257;
		extern VMIME_EXPORT const char* const WINDOWS_1258;

		extern VMIME_EXPORT const char* const IDNA;
	}

	/** Constants for standard field names. */
	namespace fields
	{
		extern VMIME_EXPORT const char* const RECEIVED;
		extern VMIME_EXPORT const char* const FROM;
		extern VMIME_EXPORT const char* const SENDER;
		extern VMIME_EXPORT const char* const REPLY_TO;
		extern VMIME_EXPORT const char* const TO;
		extern VMIME_EXPORT const char* const CC;
		extern VMIME_EXPORT const char* const BCC;
		extern VMIME_EXPORT const char* const DATE;
		extern VMIME_EXPORT const char* const SUBJECT;
		extern VMIME_EXPORT const char* const ORGANIZATION;
		extern VMIME_EXPORT const char* const USER_AGENT;
		extern VMIME_EXPORT const char* const DELIVERED_TO;
		extern VMIME_EXPORT const char* const RETURN_PATH;
		extern VMIME_EXPORT const char* const MIME_VERSION;
		extern VMIME_EXPORT const char* const MESSAGE_ID;
		extern VMIME_EXPORT const char* const CONTENT_TYPE;
		extern VMIME_EXPORT const char* const CONTENT_TRANSFER_ENCODING;
		extern VMIME_EXPORT const char* const CONTENT_DESCRIPTION;
		extern VMIME_EXPORT const char* const CONTENT_DISPOSITION;
		extern VMIME_EXPORT const char* const CONTENT_ID;
		extern VMIME_EXPORT const char* const CONTENT_LOCATION;
		extern VMIME_EXPORT const char* const IN_REPLY_TO;
		extern VMIME_EXPORT const char* const REFERENCES;

		extern VMIME_EXPORT const char* const X_MAILER;
		extern VMIME_EXPORT const char* const X_PRIORITY;

		// RFC-3798: Message Disposition Notification
		extern VMIME_EXPORT const char* const ORIGINAL_MESSAGE_ID;
		extern VMIME_EXPORT const char* const DISPOSITION_NOTIFICATION_TO;
		extern VMIME_EXPORT const char* const DISPOSITION_NOTIFICATION_OPTIONS;
		extern VMIME_EXPORT const char* const DISPOSITION;
		extern VMIME_EXPORT const char* const FAILURE;
		extern VMIME_EXPORT const char* const ERROR;
		extern VMIME_EXPORT const char* const WARNING;
		extern VMIME_EXPORT const char* const ORIGINAL_RECIPIENT;
		extern VMIME_EXPORT const char* const FINAL_RECIPIENT;
		extern VMIME_EXPORT const char* const REPORTING_UA;
		extern VMIME_EXPORT const char* const MDN_GATEWAY;
	}

	/** Constants for disposition action modes (RFC-3978). */
	namespace dispositionActionModes
	{
		/** User implicitely displayed or deleted the message (filter or
		  * any other automatic action). */
		extern VMIME_EXPORT const char* const AUTOMATIC;

		/** User explicitely displayed or deleted the message (manual action). */
		extern VMIME_EXPORT const char* const MANUAL;
	}

	/** Constants for disposition sending modes (RFC-3798). */
	namespace dispositionSendingModes
	{
		/** The MDN was sent because the MUA had previously been configured
		  * to do so automatically. */
		extern VMIME_EXPORT const char* const SENT_AUTOMATICALLY;

		/** User explicitly gave permission for this particular MDN to be sent. */
		extern VMIME_EXPORT const char* const SENT_MANUALLY;
	}

	/** Constants for disposition types (RFC-3798). */
	namespace dispositionTypes
	{
		/** Message has been displayed to the user. */
		extern VMIME_EXPORT const char* const DISPLAYED;
		/** Message has been deleted without being displayed. */
		extern VMIME_EXPORT const char* const DELETED;
		/** Message has been denied. */
		extern VMIME_EXPORT const char* const DENIED;
	}

	/** Constants for disposition modifiers (RFC-3798). */
	namespace dispositionModifiers
	{
		extern VMIME_EXPORT const char* const ERROR;
	}
}


#endif // VMIME_CONSTANTS_HPP_INCLUDED
