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

#ifndef VMIME_UTILITY_PROGRESSIONLISTENER_HPP_INCLUDED
#define VMIME_UTILITY_PROGRESSIONLISTENER_HPP_INCLUDED


namespace vmime {
namespace utility {


/** An interface to implement if you want to be notified
  * of a progression status by some objects.
  */

class progressionListener
{
protected:

	virtual ~progressionListener() { }

public:

	/** Allow the caller object to cancel the current operation.
	  *
	  * @warning WARNING: this is implementation-dependant: cancelling
	  * may not be supported by the notifier object.
	  *
	  * @return true to cancel the operation, false otherwise
	  */
	virtual const bool cancel() const = 0;

	/** Called at the beginning of the operation.
	  *
	  * @param predictedTotal predicted amount of units (this has
	  * no concrete meaning: these are not bytes, nor percentage...)
	  */
	virtual void start(const int predictedTotal) = 0;

	/** Called during the operation (can be called several times).
	  *
	  * @param current current position
	  * @param currentTotal adjusted total amount of units
	  */
	virtual void progress(const int current, const int currentTotal) = 0;

	/** Called at the end of the operation.
	  *
	  * @param total final total amount of units
	  */
	virtual void stop(const int total) = 0;
};



/** A progression listener used when total size is known by the
  * receiver, but not by the notifier.
  */

class progressionListenerSizeAdapter : public progressionListener
{
public:

	/** Construct a new progressionListenerSizeAdapter object.
	  *
	  * @param list wrapped progression listener (can be NULL)
	  * @param total predicted total
	  */
	progressionListenerSizeAdapter(progressionListener* list, const int total);

	const bool cancel() const;

	void start(const int predictedTotal);
	void progress(const int current, const int currentTotal);
	void stop(const int total);

private:

	progressionListener* m_wrapped;
	int m_total;
};


} // utility
} // vmime


#endif // VMIME_UTILITY_PROGRESSIONLISTENER_HPP_INCLUDED
