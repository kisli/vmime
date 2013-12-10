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

#include "vmime/constants.hpp"


namespace vmime
{


// Media Types
namespace mediaTypes
{
	// Types
	const char* const TEXT = "text";
	const char* const MULTIPART = "multipart";
	const char* const MESSAGE = "message";
	const char* const APPLICATION = "application";
	const char* const IMAGE = "image";
	const char* const AUDIO = "audio";
	const char* const VIDEO = "video";

	// Sub-types
	const char* const TEXT_PLAIN = "plain";
	const char* const TEXT_HTML = "html";
	const char* const TEXT_RICHTEXT = "richtext";
	const char* const TEXT_ENRICHED = "enriched";
	const char* const TEXT_RFC822_HEADERS = "rfc822-headers";  // RFC-1892
	const char* const TEXT_DIRECTORY = "directory";  // RFC-2426

	const char* const MULTIPART_MIXED = "mixed";
	const char* const MULTIPART_RELATED = "related";
	const char* const MULTIPART_ALTERNATIVE = "alternative";
	const char* const MULTIPART_PARALLEL = "parallel";
	const char* const MULTIPART_DIGEST = "digest";
	const char* const MULTIPART_REPORT = "report";  // RFC-1892

	const char* const MESSAGE_RFC822 = "rfc822";
	const char* const MESSAGE_PARTIAL = "partial";
	const char* const MESSAGE_EXTERNAL_BODY = "external-body";
	const char* const MESSAGE_DISPOSITION_NOTIFICATION = "disposition-notification";

	const char* const APPLICATION_OCTET_STREAM = "octet-stream";

	const char* const IMAGE_JPEG = "jpeg";
	const char* const IMAGE_GIF = "gif";

	const char* const AUDIO_BASIC = "basic";

	const char* const VIDEO_MPEG = "mpeg";
}


// Encoding types
namespace encodingTypes
{
	const char* const SEVEN_BIT = "7bit";
	const char* const EIGHT_BIT = "8bit";
	const char* const BASE64 = "base64";
	const char* const QUOTED_PRINTABLE = "quoted-printable";
	const char* const BINARY = "binary";
	const char* const UUENCODE = "uuencode";
}


// Content disposition types
namespace contentDispositionTypes
{
	const char* const INLINE = "inline";
	const char* const ATTACHMENT = "attachment";
}


// Charsets
namespace charsets
{
	const char* const ISO8859_1 = "iso-8859-1";
	const char* const ISO8859_2 = "iso-8859-2";
	const char* const ISO8859_3 = "iso-8859-3";
	const char* const ISO8859_4 = "iso-8859-4";
	const char* const ISO8859_5 = "iso-8859-5";
	const char* const ISO8859_6 = "iso-8859-6";
	const char* const ISO8859_7 = "iso-8859-7";
	const char* const ISO8859_8 = "iso-8859-8";
	const char* const ISO8859_9 = "iso-8859-9";
	const char* const ISO8859_10 = "iso-8859-10";
	const char* const ISO8859_13 = "iso-8859-13";
	const char* const ISO8859_14 = "iso-8859-14";
	const char* const ISO8859_15 = "iso-8859-15";
	const char* const ISO8859_16 = "iso-8859-16";

	const char* const CP_437 = "cp437";
	const char* const CP_737 = "cp737";
	const char* const CP_775 = "cp775";
	const char* const CP_850 = "cp850";
	const char* const CP_852 = "cp852";
	const char* const CP_853 = "cp853";
	const char* const CP_855 = "cp855";
	const char* const CP_857 = "cp857";
	const char* const CP_858 = "cp858";
	const char* const CP_860 = "cp860";
	const char* const CP_861 = "cp861";
	const char* const CP_862 = "cp862";
	const char* const CP_863 = "cp863";
	const char* const CP_864 = "cp864";
	const char* const CP_865 = "cp865";
	const char* const CP_866 = "cp866";
	const char* const CP_869 = "cp869";
	const char* const CP_874 = "cp874";
	const char* const CP_1125 = "cp1125";
	const char* const CP_1250 = "cp1250";
	const char* const CP_1251 = "cp1251";
	const char* const CP_1252 = "cp1252";
	const char* const CP_1253 = "cp1253";
	const char* const CP_1254 = "cp1254";
	const char* const CP_1255 = "cp1255";
	const char* const CP_1256 = "cp1256";
	const char* const CP_1257 = "cp1257";

	const char* const US_ASCII = "us-ascii";

	const char* const UTF_7 = "utf-7";
	const char* const UTF_8 = "utf-8";
	const char* const UTF_16 = "utf-16";
	const char* const UTF_32 = "utf-32";

	const char* const WINDOWS_1250 = "windows-1250";
	const char* const WINDOWS_1251 = "windows-1251";
	const char* const WINDOWS_1252 = "windows-1252";
	const char* const WINDOWS_1253 = "windows-1253";
	const char* const WINDOWS_1254 = "windows-1254";
	const char* const WINDOWS_1255 = "windows-1255";
	const char* const WINDOWS_1256 = "windows-1256";
	const char* const WINDOWS_1257 = "windows-1257";
	const char* const WINDOWS_1258 = "windows-1258";

	const char* const IDNA = "idna";
}


// Fields
namespace fields
{
	const char* const RECEIVED = "Received";
	const char* const FROM = "From";
	const char* const SENDER = "Sender";
	const char* const REPLY_TO = "Reply-To";
	const char* const TO = "To";
	const char* const CC = "Cc";
	const char* const BCC = "Bcc";
	const char* const DATE = "Date";
	const char* const SUBJECT = "Subject";
	const char* const ORGANIZATION = "Organization";
	const char* const USER_AGENT = "User-Agent";
	const char* const DELIVERED_TO = "Delivered-To";
	const char* const RETURN_PATH = "Return-Path";
	const char* const MIME_VERSION = "Mime-Version";
	const char* const MESSAGE_ID = "Message-Id";
	const char* const CONTENT_TYPE = "Content-Type";
	const char* const CONTENT_TRANSFER_ENCODING = "Content-Transfer-Encoding";
	const char* const CONTENT_DESCRIPTION = "Content-Description";
	const char* const CONTENT_DISPOSITION = "Content-Disposition";
	const char* const CONTENT_ID = "Content-Id";
	const char* const CONTENT_LOCATION = "Content-Location";
	const char* const IN_REPLY_TO = "In-Reply-To";
	const char* const REFERENCES = "References";

	const char* const X_MAILER = "X-Mailer";
	const char* const X_PRIORITY = "X-Priority";

	// RFC-3798: Message Disposition
	const char* const ORIGINAL_MESSAGE_ID = "Original-Message-ID";
	const char* const DISPOSITION_NOTIFICATION_TO = "Disposition-Notification-To";
	const char* const DISPOSITION_NOTIFICATION_OPTIONS = "Disposition-Notification-Options";
	const char* const DISPOSITION = "Disposition";
	const char* const FAILURE = "Failure";
	const char* const ERROR = "Error";
	const char* const WARNING = "Warning";
	const char* const ORIGINAL_RECIPIENT = "Original-Recipient";
	const char* const FINAL_RECIPIENT = "Final-Recipient";
	const char* const REPORTING_UA = "Reporting-UA";
	const char* const MDN_GATEWAY = "MDN-Gateway";
}


// Constants for disposition action modes (RFC-3978).
namespace dispositionActionModes
{
	const char* const MANUAL = "manual";
	const char* const AUTOMATIC = "automatic";
}


// Constants for disposition sending modes (RFC-3798).
namespace dispositionSendingModes
{
	const char* const SENT_MANUALLY = "MDN-sent-manually";
	const char* const SENT_AUTOMATICALLY ="MDN-sent-automatically";
}


// Constants for disposition types (RFC-3798).
namespace dispositionTypes
{
	const char* const DISPLAYED = "displayed";
	const char* const DELETED = "deleted";
}


// Constants for disposition modifiers (RFC-3798).
namespace dispositionModifiers
{
	const char* const ERROR = "error";
}


} // vmime
