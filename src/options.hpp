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

#ifndef VMIME_OPTIONS_HPP_INCLUDED
#define VMIME_OPTIONS_HPP_INCLUDED


#include "base.hpp"
#include "utility/singleton.hpp"


namespace vmime
{


/** A class to set global options for VMime.
  */

class options : public utility::singleton <options>
{
	friend class utility::singleton <options>;

protected:

	/** Message-related options.
	  */
	class messageOptions
	{
	protected:

		friend class options;

		messageOptions()
			: m_maxLineLength(lineLengthLimits::convenient)
		{
		}

		string::size_type m_maxLineLength;

	public:

		const string::size_type& maxLineLength() const { return (m_maxLineLength); }
		string::size_type& maxLineLength() { return (m_maxLineLength); }
	};

	/** Multipart-related options.
	  */
	class multipartOptions
	{
	private:

		friend class options;

		multipartOptions();

		string m_prologText;
		string m_epilogText;

	public:

		const string& getPrologText() const;
		void setPrologText(const string& prologText);

		const string& getEpilogText() const;
		void setEpilogText(const string& epilogText);
	};

public:

	multipartOptions multipart;
	messageOptions message;
};


} // vmime


#endif // VMIME_OPTIONS_HPP_INCLUDED
