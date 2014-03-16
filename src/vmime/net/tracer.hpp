//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2014 Vincent Richard <vincent@vmime.org>
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

#ifndef VMIME_NET_TRACER_HPP_INCLUDED
#define VMIME_NET_TRACER_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES


#include "vmime/base.hpp"


namespace vmime {
namespace net {


class service;


/** Base class for an object used to trace network communication
  * between the client and the server.
  */

class VMIME_EXPORT tracer : public object
{
public:

	virtual ~tracer() { }

	/** Trace raw bytes which have been received.
	  *
	  * @param count number of bytes
	  * @param state protocol state (eg. "SASL exchange"), or empty
	  */
	virtual void traceReceiveBytes(const size_t count, const string& state = "");

	/** Trace raw bytes which have been sent.
	  *
	  * @param count number of bytes
	  * @param state protocol state (eg. "SASL exchange"), or empty
	  */
	virtual void traceSendBytes(const size_t count, const string& state = "");

	/** Trace a command line which has been sent.
	  *
	  * @param line command line
	  */
	virtual void traceSend(const string& line) = 0;

	/** Trace a response line which has been received.
	  *
	  * @param line response line
	  */
	virtual void traceReceive(const string& line) = 0;
};



/** A class to create 'tracer' objects.
  */

class VMIME_EXPORT tracerFactory : public object
{
public:

	virtual ~tracerFactory() { }

	/** Creates a tracer for the specified service.
	  *
	  * @param serv messaging service
	  * @param connectionId an identifier for the connection to distinguate between
	  * different connections used by a service
	  * @return a new tracer
	  */
	virtual shared_ptr <tracer> create(shared_ptr <service> serv, const int connectionId) = 0;
};


} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES

#endif // VMIME_NET_TRACER_HPP_INCLUDED
