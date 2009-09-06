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

#include "vmime/constants.hpp"


namespace vmime
{


// Media Types
namespace mediaTypes
{
	// Types
	const string::value_type* const TEXT = "text";
	const string::value_type* const MULTIPART = "multipart";
	const string::value_type* const MESSAGE = "message";
	const string::value_type* const APPLICATION = "application";
	const string::value_type* const IMAGE = "image";
	const string::value_type* const AUDIO = "audio";
	const string::value_type* const VIDEO = "video";

	// Sub-types
	const string::value_type* const TEXT_PLAIN = "plain";
	const string::value_type* const TEXT_HTML = "html";
	const string::value_type* const TEXT_RICHTEXT = "richtext";
	const string::value_type* const TEXT_ENRICHED = "enriched";
	const string::value_type* const TEXT_RFC822_HEADERS = "rfc822-headers";  // RFC-1892
	const string::value_type* const TEXT_DIRECTORY = "directory";  // RFC-2426

	const string::value_type* const MULTIPART_MIXED = "mixed";
	const string::value_type* const MULTIPART_RELATED = "related";
	const string::value_type* const MULTIPART_ALTERNATIVE = "alternative";
	const string::value_type* const MULTIPART_PARALLEL = "parallel";
	const string::value_type* const MULTIPART_DIGEST = "digest";
	const string::value_type* const MULTIPART_REPORT = "report";  // RFC-1892

	const string::value_type* const MESSAGE_RFC822 = "rfc822";
	const string::value_type* const MESSAGE_PARTIAL = "partial";
	const string::value_type* const MESSAGE_EXTERNAL_BODY = "external-body";
	const string::value_type* const MESSAGE_DISPOSITION_NOTIFICATION = "disposition-notification";

	const string::value_type* const APPLICATION_OCTET_STREAM = "octet-stream";

	const string::value_type* const IMAGE_JPEG = "jpeg";
	const string::value_type* const IMAGE_GIF = "gif";

	const string::value_type* const AUDIO_BASIC = "basic";

	const string::value_type* const VIDEO_MPEG = "mpeg";
}


// Encoding types
namespace encodingTypes
{
	const string::value_type* const SEVEN_BIT = "7bit";
	const string::value_type* const EIGHT_BIT = "8bit";
	const string::value_type* const BASE64 = "base64";
	const string::value_type* const QUOTED_PRINTABLE = "quoted-printable";
	const string::value_type* const BINARY = "binary";
	const string::value_type* const UUENCODE = "uuencode";
}


// Content disposition types
namespace contentDispositionTypes
{
	const string::value_type* const INLINE = "inline";
	const string::value_type* const ATTACHMENT = "attachment";
}


// Charsets
namespace charsets
{
	const string::value_type* const ISO8859_1 = "iso-8859-1";
	const string::value_type* const ISO8859_2 = "iso-8859-2";
	const string::value_type* const ISO8859_3 = "iso-8859-3";
	const string::value_type* const ISO8859_4 = "iso-8859-4";
	const string::value_type* const ISO8859_5 = "iso-8859-5";
	const string::value_type* const ISO8859_6 = "iso-8859-6";
	const string::value_type* const ISO8859_7 = "iso-8859-7";
	const string::value_type* const ISO8859_8 = "iso-8859-8";
	const string::value_type* const ISO8859_9 = "iso-8859-9";
	const string::value_type* const ISO8859_10 = "iso-8859-10";
	const string::value_type* const ISO8859_13 = "iso-8859-13";
	const string::value_type* const ISO8859_14 = "iso-8859-14";
	const string::value_type* const ISO8859_15 = "iso-8859-15";
	const string::value_type* const ISO8859_16 = "iso-8859-16";

	const string::value_type* const CP_437 = "cp437";
	const string::value_type* const CP_737 = "cp737";
	const string::value_type* const CP_775 = "cp775";
	const string::value_type* const CP_850 = "cp850";
	const string::value_type* const CP_852 = "cp852";
	const string::value_type* const CP_853 = "cp853";
	const string::value_type* const CP_855 = "cp855";
	const string::value_type* const CP_857 = "cp857";
	const string::value_type* const CP_858 = "cp858";
	const string::value_type* const CP_860 = "cp860";
	const string::value_type* const CP_861 = "cp861";
	const string::value_type* const CP_862 = "cp862";
	const string::value_type* const CP_863 = "cp863";
	const string::value_type* const CP_864 = "cp864";
	const string::value_type* const CP_865 = "cp865";
	const string::value_type* const CP_866 = "cp866";
	const string::value_type* const CP_869 = "cp869";
	const string::value_type* const CP_874 = "cp874";
	const string::value_type* const CP_1125 = "cp1125";
	const string::value_type* const CP_1250 = "cp1250";
	const string::value_type* const CP_1251 = "cp1251";
	const string::value_type* const CP_1252 = "cp1252";
	const string::value_type* const CP_1253 = "cp1253";
	const string::value_type* const CP_1254 = "cp1254";
	const string::value_type* const CP_1255 = "cp1255";
	const string::value_type* const CP_1256 = "cp1256";
	const string::value_type* const CP_1257 = "cp1257";

	const string::value_type* const US_ASCII = "us-ascii";

	const string::value_type* const UTF_7 = "utf-7";
	const string::value_type* const UTF_8 = "utf-8";
	const string::value_type* const UTF_16 = "utf-16";
	const string::value_type* const UTF_32 = "utf-32";

	const string::value_type* const WINDOWS_1250 = "windows-1250";
	const string::value_type* const WINDOWS_1251 = "windows-1251";
	const string::value_type* const WINDOWS_1252 = "windows-1252";
	const string::value_type* const WINDOWS_1253 = "windows-1253";
	const string::value_type* const WINDOWS_1254 = "windows-1254";
	const string::value_type* const WINDOWS_1255 = "windows-1255";
	const string::value_type* const WINDOWS_1256 = "windows-1256";
	const string::value_type* const WINDOWS_1257 = "windows-1257";
	const string::value_type* const WINDOWS_1258 = "windows-1258";
}


// Fields
namespace fields
{
	const string::value_type* const RECEIVED = "Received";
	const string::value_type* const FROM = "From";
	const string::value_type* const SENDER = "Sender";
	const string::value_type* const REPLY_TO = "Reply-To";
	const string::value_type* const TO = "To";
	const string::value_type* const CC = "Cc";
	const string::value_type* const BCC = "Bcc";
	const string::value_type* const DATE = "Date";
	const string::value_type* const SUBJECT = "Subject";
	const string::value_type* const ORGANIZATION = "Organization";
	const string::value_type* const USER_AGENT = "User-Agent";
	const string::value_type* const DELIVERED_TO = "Delivered-To";
	const string::value_type* const RETURN_PATH = "Return-Path";
	const string::value_type* const MIME_VERSION = "Mime-Version";
	const string::value_type* const MESSAGE_ID = "Message-Id";
	const string::value_type* const CONTENT_TYPE = "Content-Type";
	const string::value_type* const CONTENT_TRANSFER_ENCODING = "Content-Transfer-Encoding";
	const string::value_type* const CONTENT_DESCRIPTION = "Content-Description";
	const string::value_type* const CONTENT_DISPOSITION = "Content-Disposition";
	const string::value_type* const CONTENT_ID = "Content-Id";
	const string::value_type* const CONTENT_LOCATION = "Content-Location";
	const string::value_type* const IN_REPLY_TO = "In-Reply-To";
	const string::value_type* const REFERENCES = "References";

	const string::value_type* const X_MAILER = "X-Mailer";
	const string::value_type* const X_PRIORITY = "X-Priority";

	// RFC-3798: Message Disposition
	const string::value_type* const ORIGINAL_MESSAGE_ID = "Original-Message-ID";
	const string::value_type* const DISPOSITION_NOTIFICATION_TO = "Disposition-Notification-To";
	const string::value_type* const DISPOSITION_NOTIFICATION_OPTIONS = "Disposition-Notification-Options";
	const string::value_type* const DISPOSITION = "Disposition";
	const string::value_type* const FAILURE = "Failure";
	const string::value_type* const ERROR = "Error";
	const string::value_type* const WARNING = "Warning";
	const string::value_type* const ORIGINAL_RECIPIENT = "Original-Recipient";
	const string::value_type* const FINAL_RECIPIENT = "Final-Recipient";
	const string::value_type* const REPORTING_UA = "Reporting-UA";
	const string::value_type* const MDN_GATEWAY = "MDN-Gateway";
}


// Constants for disposition action modes (RFC-3978).
namespace dispositionActionModes
{
	const string::value_type* const MANUAL = "manual";
	const string::value_type* const AUTOMATIC = "automatic";
}


// Constants for disposition sending modes (RFC-3798).
namespace dispositionSendingModes
{
	const string::value_type* const SENT_MANUALLY = "MDN-sent-manually";
	const string::value_type* const SENT_AUTOMATICALLY ="MDN-sent-automatically";
}


// Constants for disposition types (RFC-3798).
namespace dispositionTypes
{
	const string::value_type* const DISPLAYED = "displayed";
	const string::value_type* const DELETED = "deleted";
}


// Constants for disposition modifiers (RFC-3798).
namespace dispositionModifiers
{
	const string::value_type* const ERROR = "error";
}


} // vmime
