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
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// Linking this library statically or dynamically with other modules is making
// a combined work based on this library.  Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
//

#ifndef VMIME_UTILITY_STREAM_HPP_INCLUDED
#define VMIME_UTILITY_STREAM_HPP_INCLUDED


#include <istream>
#include <ostream>
#include <sstream>

#include "vmime/config.hpp"
#include "vmime/types.hpp"

#include "vmime/utility/progressionListener.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES
	namespace vmime {
	namespace net {
		class socket;  // forward reference
	} // net
	} // vmime
#endif

#if defined(_MSC_VER) && (_MSC_VER <= 1200)  // VC++6
#   include <cstring>
#endif


namespace vmime {
namespace utility {


class stringProxy;


/** Base class for input/output stream.
  */

class stream : public object
{
public:

	virtual ~stream() { }

	/** Type used to read/write one byte in the stream.
	  */
	typedef string::value_type value_type;

	/** Type used for lengths in streams.
	  */
	typedef string::size_type size_type;
};



/** Simple output stream.
  */

class outputStream : public stream
{
public:

	/** Write data to the stream.
	  *
	  * @param data buffer containing data to write
	  * @param count number of bytes to write
	  */
	virtual void write(const value_type* const data, const size_type count) = 0;
};



/** Simple input stream.
  */

class inputStream : public stream
{
public:

	/** Test for end of stream (no more data to read).
	  *
	  * @return true if we have reached the end of stream, false otherwise
	  */
	virtual const bool eof() const = 0;

	/** Set the read pointer to the beginning of the stream.
	  *
	  * @warning WARNING: this may not work for all stream types.
	  */
	virtual void reset() = 0;

	/** Read data from the stream.
	  *
	  * @param data will receive the data read
	  * @param count maximum number of bytes to read
	  * @return number of bytes read
	  */
	virtual const size_type read(value_type* const data, const size_type count) = 0;

	/** Skip a number of bytes.
	  *
	  * @param count maximum number of bytes to ignore
	  * @return number of bytes skipped
	  */
	virtual const size_type skip(const size_type count) = 0;
};



// Helpers functions

outputStream& operator<<(outputStream& os, const string& str);
outputStream& operator<<(outputStream& os, const stream::value_type c);


#if defined(_MSC_VER) && (_MSC_VER <= 1200)  // Internal compiler error with VC++6

inline outputStream& operator<<(outputStream& os, const char* str)
{
	os.write(str, ::strlen(str));
	return (os);
}

#else

template <int N>
outputStream& operator<<(outputStream& os, const char (&str)[N])
{
	os.write(str, N - 1);
	return (os);
}

#endif // defined(_MSC_VER) && (_MSC_VER <= 1200)


template <typename T>
outputStream& operator<<(outputStream& os, const T& t)
{
	std::ostringstream oss;
	oss << t;

	os << oss.str();

	return (os);
}


/** Copy data from one stream into another stream using a buffered method.
  *
  * @param is input stream (source data)
  * @param os output stream (destination for data)
  * @return number of bytes copied
  */

const stream::size_type bufferedStreamCopy(inputStream& is, outputStream& os);

/** Copy data from one stream into another stream using a buffered method
  * and notify progression of the operation.
  *
  * @param is input stream (source data)
  * @param os output stream (destination for data)
  * @param length predicted number of bytes to copy
  * @param progress listener to notify
  * @return number of bytes copied
  */

const stream::size_type bufferedStreamCopy(inputStream& is, outputStream& os,
	const stream::size_type length, progressionListener* progress);


// Adapters


/** An adapter class for C++ standard output streams.
  */

class outputStreamAdapter : public outputStream
{
public:

	/** @param os output stream to wrap
	  */
	outputStreamAdapter(std::ostream& os);

	void write(const value_type* const data, const size_type count);

private:

	std::ostream& m_stream;
};


/** An adapter class for string output.
  */

class outputStreamStringAdapter : public outputStream
{
public:

	outputStreamStringAdapter(string& buffer);

	void write(const value_type* const data, const size_type count);

private:

	string& m_buffer;
};


/** An adapter class for C++ standard input streams.
  */

class inputStreamAdapter : public inputStream
{
public:

	/** @param is input stream to wrap
	  */
	inputStreamAdapter(std::istream& is);

	const bool eof() const;
	void reset();
	const size_type read(value_type* const data, const size_type count);
	const size_type skip(const size_type count);

private:

	std::istream& m_stream;
};


/** An adapter class for string input.
  */

class inputStreamStringAdapter : public inputStream
{
public:

	inputStreamStringAdapter(const string& buffer);
	inputStreamStringAdapter(const string& buffer, const string::size_type begin, const string::size_type end);

	const bool eof() const;
	void reset();
	const size_type read(value_type* const data, const size_type count);
	const size_type skip(const size_type count);

private:

	inputStreamStringAdapter(const inputStreamStringAdapter&);

	const string m_buffer;  // do _NOT_ keep a reference...
	const string::size_type m_begin;
	const string::size_type m_end;
	string::size_type m_pos;
};


/** An adapter class for stringProxy input.
  */

class inputStreamStringProxyAdapter : public inputStream
{
public:

	/** @param buffer stringProxy object to wrap
	  */
	inputStreamStringProxyAdapter(const stringProxy& buffer);

	const bool eof() const;
	void reset();
	const size_type read(value_type* const data, const size_type count);
	const size_type skip(const size_type count);

private:

	inputStreamStringProxyAdapter(const inputStreamStringProxyAdapter&);

	const stringProxy& m_buffer;
	string::size_type m_pos;
};


/** An adapter class for pointer to C++ standard input stream.
  */

class inputStreamPointerAdapter : public inputStream
{
public:

	/** @param is input stream to wrap
	  * @param own if set to 'true', the pointer will be deleted when
	  * this object is destroyed
	  */
	inputStreamPointerAdapter(std::istream* is, const bool own = true);
	~inputStreamPointerAdapter();

	const bool eof() const;
	void reset();
	const size_type read(value_type* const data, const size_type count);
	const size_type skip(const size_type count);

private:

	inputStreamPointerAdapter(const inputStreamPointerAdapter&);

	std::istream* m_stream;
	const bool m_own;
};


/** An adapter class for reading from an array of bytes.
  */

class inputStreamByteBufferAdapter : public inputStream
{
public:

	inputStreamByteBufferAdapter(const byte* buffer, size_type length);

	const bool eof() const;
	void reset();
	const size_type read(value_type* const data, const size_type count);
	const size_type skip(const size_type count);

private:

	const byte* m_buffer;
	const size_type m_length;

	size_type m_pos;
};


#if VMIME_HAVE_MESSAGING_FEATURES


/** An output stream that is connected to a socket.
  */

class outputStreamSocketAdapter : public outputStream
{
public:

	outputStreamSocketAdapter(net::socket& sok);

	void write(const value_type* const data, const size_type count);

private:

	outputStreamSocketAdapter(const outputStreamSocketAdapter&);

	net::socket& m_socket;
};


/** An input stream that is connected to a socket.
  */

class inputStreamSocketAdapter : public inputStream
{
public:

	inputStreamSocketAdapter(net::socket& sok);

	const bool eof() const;
	void reset();
	const size_type read(value_type* const data, const size_type count);
	const size_type skip(const size_type count);

private:

	inputStreamSocketAdapter(const inputStreamSocketAdapter&);

	net::socket& m_socket;
};


#endif // VMIME_HAVE_MESSAGING_FEATURES


} // utility
} // vmime


#endif // VMIME_UTILITY_STREAM_HPP_INCLUDED
