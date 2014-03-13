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

#ifndef VMIME_UTILITY_PROGRESSLISTENER_HPP_INCLUDED
#define VMIME_UTILITY_PROGRESSLISTENER_HPP_INCLUDED


#include "vmime/config.hpp"
#include "vmime/types.hpp"


namespace vmime {
namespace utility {


/** An interface to implement if you want to be notified
  * of a state of progress by some objects.
  */

class VMIME_EXPORT progressListener
{
protected:

	virtual ~progressListener() { }

public:

	/** Called at the beginning of the operation.
	  *
	  * @param predictedTotal predicted amount of units (this has
	  * no concrete meaning: these are not bytes, nor percentage...)
	  */
	virtual void start(const size_t predictedTotal) = 0;

	/** Called during the operation (can be called several times).
	  *
	  * @param current current position
	  * @param currentTotal adjusted total amount of units
	  */
	virtual void progress(const size_t current, const size_t currentTotal) = 0;

	/** Called at the end of the operation.
	  *
	  * @param total final total amount of units
	  */
	virtual void stop(const size_t total) = 0;
};



/** A progress listener used when total size is known by the
  * receiver, but not by the notifier.
  */

class VMIME_EXPORT progressListenerSizeAdapter : public progressListener
{
public:

	/** Construct a new progressListenerSizeAdapter object.
	  *
	  * @param list wrapped progress listener (can be NULL)
	  * @param total predicted total
	  */
	progressListenerSizeAdapter(progressListener* list, const size_t total);

	void start(const size_t predictedTotal);
	void progress(const size_t current, const size_t currentTotal);
	void stop(const size_t total);

private:

	progressListener* m_wrapped;
	size_t m_total;
};


} // utility
} // vmime


#endif // VMIME_UTILITY_PROGRESSLISTENER_HPP_INCLUDED

