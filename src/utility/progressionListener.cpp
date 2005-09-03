//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2005 Vincent Richard <vincent@vincent-richard.net>
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

#include "vmime/utility/progressionListener.hpp"


namespace vmime {
namespace utility {


// progressionListenerSizeAdapter

progressionListenerSizeAdapter::progressionListenerSizeAdapter
	(progressionListener* list, const int total)
	: m_wrapped(list), m_total(total)
{
}


const bool progressionListenerSizeAdapter::cancel() const
{
	return (m_wrapped ? m_wrapped->cancel() : false);
}


void progressionListenerSizeAdapter::start(const int predictedTotal)
{
	if (m_wrapped)
		m_wrapped->start(predictedTotal);
}


void progressionListenerSizeAdapter::progress(const int current, const int currentTotal)
{
	if (m_wrapped)
	{
		if (currentTotal > m_total)
			m_total = currentTotal;

		m_wrapped->progress(current, m_total);
	}
}


void progressionListenerSizeAdapter::stop(const int total)
{
	if (m_wrapped)
	{
		if (total > m_total)
			m_total = total;

		m_wrapped->stop(m_total);
	}
}


} // utility
} // vmime

