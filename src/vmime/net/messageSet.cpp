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

#include "vmime/net/messageSet.hpp"

#include <iterator>
#include <algorithm>
#include <typeinfo>


namespace vmime {
namespace net {


// messageRange

messageRange::messageRange()
{
}


messageRange::~messageRange()
{
}


// numberMessageRange

numberMessageRange::numberMessageRange(const int number)
	: m_first(number), m_last(number)
{
	if (number < 1)
		throw std::invalid_argument("number");
}


numberMessageRange::numberMessageRange(const int first, const int last)
	: m_first(first), m_last(last)
{
	if (first < 1)
		throw std::invalid_argument("first");
	else if (last != -1 && last < first)
		throw std::invalid_argument("last");
}


numberMessageRange::numberMessageRange(const numberMessageRange& other)
	: messageRange(), m_first(other.m_first), m_last(other.m_last)
{
}


int numberMessageRange::getFirst() const
{
	return m_first;
}


int numberMessageRange::getLast() const
{
	return m_last;
}


void numberMessageRange::enumerate(messageSetEnumerator& en) const
{
	en.enumerateNumberMessageRange(*this);
}


messageRange* numberMessageRange::clone() const
{
	return new numberMessageRange(*this);
}


// UIDMessageRange

UIDMessageRange::UIDMessageRange(const message::uid& uid)
	: m_first(uid), m_last(uid)
{
}


UIDMessageRange::UIDMessageRange(const message::uid& first, const message::uid& last)
	: m_first(first), m_last(last)
{
}


UIDMessageRange::UIDMessageRange(const UIDMessageRange& other)
	: messageRange(), m_first(other.m_first), m_last(other.m_last)
{
}


const message::uid UIDMessageRange::getFirst() const
{
	return m_first;
}


const message::uid UIDMessageRange::getLast() const
{
	return m_last;
}


void UIDMessageRange::enumerate(messageSetEnumerator& en) const
{
	en.enumerateUIDMessageRange(*this);
}


messageRange* UIDMessageRange::clone() const
{
	return new UIDMessageRange(*this);
}


// messageSet


messageSet::messageSet()
{
}


messageSet::messageSet(const messageSet& other)
	: object()
{
	m_ranges.resize(other.m_ranges.size());

	for (size_t i = 0, n = other.m_ranges.size() ; i < n ; ++i)
		m_ranges[i] = other.m_ranges[i]->clone();
}


messageSet::~messageSet()
{
	for (size_t i = 0, n = m_ranges.size() ; i < n ; ++i)
		delete m_ranges[i];
}


// static
messageSet messageSet::empty()
{
	return messageSet();
}


// static
messageSet messageSet::byNumber(const int number)
{
	messageSet set;
	set.m_ranges.push_back(new numberMessageRange(number));

	return set;
}


// static
messageSet messageSet::byNumber(const int first, const int last)
{
	messageSet set;
	set.m_ranges.push_back(new numberMessageRange(first, last));

	return set;
}


// static
messageSet messageSet::byNumber(const std::vector <int>& numbers)
{
	// Sort a copy of the list
	std::vector <int> sortedNumbers;

	sortedNumbers.resize(numbers.size());

	std::copy(numbers.begin(), numbers.end(), sortedNumbers.begin());
	std::sort(sortedNumbers.begin(), sortedNumbers.end());

	// Build the set by detecting ranges of continuous numbers
	int previous = -1, rangeStart = -1;
	messageSet set;

	for (std::vector <int>::const_iterator it = sortedNumbers.begin() ;
	     it != sortedNumbers.end() ; ++it)
	{
		const int current = *it;

		if (current == previous)
			continue;  // skip duplicates

		if (previous == -1)
		{
			previous = current;
			rangeStart = current;
		}
		else
		{
			if (current == previous + 1)
			{
				previous = current;
			}
			else
			{
				set.m_ranges.push_back(new numberMessageRange(rangeStart, previous));

				previous = current;
				rangeStart = current;
			}
		}
	}

	set.m_ranges.push_back(new numberMessageRange(rangeStart, previous));

	return set;
}


// static
messageSet messageSet::byUID(const message::uid& uid)
{
	messageSet set;
	set.m_ranges.push_back(new UIDMessageRange(uid));

	return set;
}


messageSet messageSet::byUID(const message::uid& first, const message::uid& last)
{
	messageSet set;
	set.m_ranges.push_back(new UIDMessageRange(first, last));

	return set;
}


messageSet messageSet::byUID(const std::vector <message::uid>& uids)
{
	std::vector <vmime_uint32> numericUIDs;

	for (size_t i = 0, n = uids.size() ; i < n ; ++i)
	{
		const string uid = uids[i];
		int numericUID = 0;

		const char* p = uid.c_str();

		for ( ; *p >= '0' && *p <= '9' ; ++p)
			 numericUID = (numericUID * 10) + (*p - '0');

		if (*p != '\0')
		{
			messageSet set;

			// Non-numeric UID, fall back to plain UID list (single-UID ranges)
			for (size_t i = 0, n = uids.size() ; i < n ; ++i)
				set.m_ranges.push_back(new UIDMessageRange(uids[i]));

			return set;
		}

		numericUIDs.push_back(numericUID);
	}

	// Sort a copy of the list
	std::vector <vmime_uint32> sortedUIDs;

	sortedUIDs.resize(numericUIDs.size());

	std::copy(numericUIDs.begin(), numericUIDs.end(), sortedUIDs.begin());
	std::sort(sortedUIDs.begin(), sortedUIDs.end());

	// Build the set by detecting ranges of continuous numbers
	vmime_uint32 previous = -1U, rangeStart = -1U;
	messageSet set;

	for (std::vector <vmime_uint32>::const_iterator it = sortedUIDs.begin() ;
	     it != sortedUIDs.end() ; ++it)
	{
		const vmime_uint32 current = *it;

		if (current == previous)
			continue;  // skip duplicates

		if (previous == -1U)
		{
			previous = current;
			rangeStart = current;
		}
		else
		{
			if (current == previous + 1)
			{
				previous = current;
			}
			else
			{
				set.m_ranges.push_back(new UIDMessageRange
					(utility::stringUtils::toString(rangeStart),
					 utility::stringUtils::toString(previous)));

				previous = current;
				rangeStart = current;
			}
		}
	}

	set.m_ranges.push_back(new UIDMessageRange
		(utility::stringUtils::toString(rangeStart),
	     utility::stringUtils::toString(previous)));

	return set;
}


void messageSet::addRange(const messageRange& range)
{
	if (!m_ranges.empty() && typeid(*m_ranges[0]) != typeid(range))
		throw std::invalid_argument("range");

	m_ranges.push_back(range.clone());
}


void messageSet::enumerate(messageSetEnumerator& en) const
{
	for (size_t i = 0, n = m_ranges.size() ; i < n ; ++i)
		m_ranges[i]->enumerate(en);
}


bool messageSet::isEmpty() const
{
	return m_ranges.empty();
}


bool messageSet::isNumberSet() const
{
	return !isEmpty() && dynamic_cast <numberMessageRange*>(m_ranges[0]) != NULL;
}


bool messageSet::isUIDSet() const
{
	return !isEmpty() && dynamic_cast <UIDMessageRange*>(m_ranges[0]) != NULL;
}


size_t messageSet::getRangeCount() const
{
	return m_ranges.size();
}


const messageRange& messageSet::getRangeAt(const size_t i) const
{
	return *m_ranges[i];
}


} // net
} // vmime
