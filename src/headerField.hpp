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

#ifndef VMIME_HEADERFIELD_HPP_INCLUDED
#define VMIME_HEADERFIELD_HPP_INCLUDED


#include "base.hpp"
#include "component.hpp"


namespace vmime
{


/** Base class for header fields.
  */

class headerField : public component
{
	friend class headerFieldFactory;

protected:

	headerField();
	headerField(const string& fieldName);

public:

	~headerField();

public:

	// Header field types (in the order in which they will appear
	// in the message header)
	enum Types
	{
		Received,                // Relay
		From,                    // Expeditor
		Sender,                  // Sender
		ReplyTo,                 // Reply-To
		To,                      // Recipient(s)
		Cc,                      // Carbon copy recipient(s)
		Bcc,                     // Blind carbon-copy recipient(s)
		Date,                    // Date sent
		Subject,                 // Subject
		Organization,            // Organization
		UserAgent,               // User agent
		DeliveredTo,             // Delivered-To
		ReturnPath,              // Return-Path
		MimeVersion,             // Mime-Version
		MessageId,               // Message-Id
		ContentType,             // Content-Type
		ContentTransferEncoding, // Content-Transfer-Encoding
		ContentDescription,      // Content-Description
		ContentDisposition,      // Content-Disposition
		ContentId,               // Content-Id
		ContentLocation,         // Content-Location

		Custom,                  // Unknown or custom field (eg. X-Priority, X-Mailer, etc.)

		Last
	};

protected:

	Types m_type;
	string m_name;	// In case of custom field

public:

	const bool operator<(const headerField& field) const;

	const Types type() const;
	const string name() const;

	const bool isCustom() const;

	virtual void copyFrom(const headerField& field);
	headerField& operator=(const headerField& field);
	headerField* clone() const;

	static const Types nameToType(const string& name);
	static const string typeToName(const Types type);


	// Component assembling
	using component::generate;

	void generate(utility::outputStream& os, const string::size_type maxLineLength = lineLengthLimits::infinite, const string::size_type curLinePos = 0, string::size_type* newLinePos = NULL) const;
};


} // vmime


#endif // VMIME_HEADERFIELD_HPP_INCLUDED
