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

#ifndef VMIME_MESSAGING_MAILDIRMESSAGE_HPP_INCLUDED
#define VMIME_MESSAGING_MAILDIRMESSAGE_HPP_INCLUDED


#include "message.hpp"
#include "folder.hpp"


namespace vmime {
namespace messaging {


class maildirFolder;


/** maildir message implementation.
  */

class maildirMessage : public message
{
	friend class maildirFolder;

protected:

	maildirMessage(maildirFolder* folder, const int num);
	maildirMessage(const maildirMessage&) : message() { }

	~maildirMessage();

public:

	const int number() const;

	const uid uniqueId() const;

	const int size() const;

	const bool isExpunged() const;

	const class structure& structure() const;
	class structure& structure();

	const class header& header() const;

	const int flags() const;
	void setFlags(const int flags, const int mode = FLAG_MODE_SET);

	void extract(utility::outputStream& os, progressionListener* progress = NULL, const int start = 0, const int length = -1) const;
	void extractPart(const part& p, utility::outputStream& os, progressionListener* progress = NULL, const int start = 0, const int length = -1) const;

	void fetchPartHeader(part& p);
};


} // messaging
} // vmime


#endif // VMIME_MESSAGING_MAILDIRMESSAGE_HPP_INCLUDED
