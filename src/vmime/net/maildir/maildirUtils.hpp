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

#ifndef VMIME_NET_MAILDIR_MAILDIRUTILS_HPP_INCLUDED
#define VMIME_NET_MAILDIR_MAILDIRUTILS_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_MAILDIR


#include "vmime/utility/file.hpp"
#include "vmime/utility/path.hpp"

#include "vmime/net/messageSet.hpp"


namespace vmime {
namespace net {
namespace maildir {


class maildirStore;


/** Miscellaneous helpers functions for maildir messaging system.
  */

class VMIME_EXPORT maildirUtils
{
public:

	/** Comparator for message filenames, based only on the
	  * unique identifier part of the filename.
	  */
	class messageIdComparator
	{
	public:

		messageIdComparator(const utility::file::path::component& comp);

		bool operator()(const utility::file::path::component& other) const;

	private:

		const utility::file::path::component m_comp;
	};

	/** Test whether the specified file-system object is a message.
	  *
	  * @param file reference to a file-system object
	  * @return true if the specified object is a message file,
	  * false otherwise
	  */
	static bool isMessageFile(const utility::file& file);

	/** Extract the unique identifier part of the message filename.
	  * Eg: for the filename "1071577232.28549.m03s:2,RS", it will
	  * return "1071577232.28549.m03s".
	  *
	  * @param filename filename part
	  * @return part of the filename that corresponds to the unique
	  * identifier of the message
	  */
	static const utility::file::path::component extractId(const utility::file::path::component& filename);

	/** Extract message flags from the specified message filename.
	  * Eg: for the filename "1071577232.28549.m03s:2,RS", it will
	  * return (message::FLAG_SEEN | message::FLAG_REPLIED).
	  *
	  * @param comp filename part
	  * @return message flags extracted from the specified filename
	  */
	static int extractFlags(const utility::file::path::component& comp);

	/** Return a string representing the specified message flags.
	  * Eg: for (message::FLAG_SEEN | message::FLAG_REPLIED), it will
	  * return "RS".
	  *
	  * @param flags set of flags
	  * @return message flags in a string representation
	  */
	static const utility::file::path::component buildFlags(const int flags);

	/** Build a filename with the specified id and flags.
	  *
	  * @param id id part of the filename
	  * @param flags flags part of the filename
	  * @return message filename
	  */
	static const utility::file::path::component buildFilename(const utility::file::path::component& id, const utility::file::path::component& flags);

	/** Build a filename with the specified id and flags.
	  *
	  * @param id id part of the filename
	  * @param flags set of flags
	  * @return message filename
	  */
	static const utility::file::path::component buildFilename(const utility::file::path::component& id, const int flags);

	/** Generate a new unique message identifier.
	  *
	  * @return unique message id
	  */
	static const utility::file::path::component generateId();

	/** Recursively delete a directory on the file system.
	  *
	  * @param dir directory to delete
	  */
	static void recursiveFSDelete(shared_ptr <utility::file> dir);

	/** Returns a list of message numbers given a message set.
	  *
	  * @param msgs message set
	  * @return list of message numbers
	  */
	static const std::vector <int> messageSetToNumberList(const messageSet& msgs);
};


} // maildir
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_MAILDIR

#endif // VMIME_NET_MAILDIR_MAILDIRUTILS_HPP_INCLUDED
