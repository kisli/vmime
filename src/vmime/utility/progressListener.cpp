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

#include "vmime/utility/progressListener.hpp"


namespace vmime {
namespace utility {


// progressListenerSizeAdapter

progressListenerSizeAdapter::progressListenerSizeAdapter
	(progressListener* list, const size_t total)
	: m_wrapped(list), m_total(total)
{
}


void progressListenerSizeAdapter::start(const size_t predictedTotal)
{
	if (m_wrapped)
		m_wrapped->start(predictedTotal);
}


void progressListenerSizeAdapter::progress(const size_t current, const size_t currentTotal)
{
	if (m_wrapped)
	{
		if (currentTotal > m_total)
			m_total = currentTotal;

		m_wrapped->progress(current, m_total);
	}
}


void progressListenerSizeAdapter::stop(const size_t total)
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

