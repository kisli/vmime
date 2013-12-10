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

#include "vmime/header.hpp"
#include "vmime/parserHelpers.hpp"

#include <algorithm>
#include <iterator>


namespace vmime
{


header::header()
{
}


header::~header()
{
	removeAllFields();
}


/*

 RFC #822:
 3.2.  HEADER FIELD DEFINITIONS

field       =  field-name ":" [ field-body ] CRLF

field-name  =  1*<any CHAR, excluding CTLs, SPACE, and ":">

field-body  =  field-body-contents
		[CRLF LWSP-char field-body]

field-body-contents =
		<the ASCII characters making up the field-body, as
		 defined in the following sections, and consisting
		 of combinations of atom, quoted-string, and
		 specials tokens, or else consisting of texts>
*/

void header::parseImpl
	(const parsingContext& ctx, const string& buffer, const size_t position,
	 const size_t end, size_t* newPosition)
{
	size_t pos = position;

	removeAllFields();

	while (pos < end)
	{
		shared_ptr <headerField> field = headerField::parseNext(ctx, buffer, pos, end, &pos);
		if (field == NULL) break;

		m_fields.push_back(field);
	}

	setParsedBounds(position, pos);

	if (newPosition)
		*newPosition = pos;
}


void header::generateImpl
	(const generationContext& ctx, utility::outputStream& os,
	 const size_t /* curLinePos */, size_t* newLinePos) const
{
	// Generate the fields
	for (std::vector <shared_ptr <headerField> >::const_iterator it = m_fields.begin() ;
	     it != m_fields.end() ; ++it)
	{
		(*it)->generate(ctx, os);
		os << CRLF;
	}

	if (newLinePos)
		*newLinePos = 0;
}


size_t header::getGeneratedSize(const generationContext& ctx)
{
	return component::getGeneratedSize(ctx) + 2 * m_fields.size() /* CRLF */;
}


shared_ptr <component> header::clone() const
{
	shared_ptr <header> hdr = make_shared <header>();

	hdr->m_fields.reserve(m_fields.size());

	for (std::vector <shared_ptr <headerField> >::const_iterator it = m_fields.begin() ;
	     it != m_fields.end() ; ++it)
	{
		hdr->m_fields.push_back(vmime::clone(*it));
	}

	return (hdr);
}


void header::copyFrom(const component& other)
{
	const header& h = dynamic_cast <const header&>(other);

	std::vector <shared_ptr <headerField> > fields;

	fields.reserve(h.m_fields.size());

	for (std::vector <shared_ptr <headerField> >::const_iterator it = h.m_fields.begin() ;
	     it != h.m_fields.end() ; ++it)
	{
		fields.push_back(vmime::clone(*it));
	}

	m_fields.clear();
	m_fields.resize(fields.size());

	std::copy(fields.begin(), fields.end(), m_fields.begin());
}


header& header::operator=(const header& other)
{
	copyFrom(other);
	return (*this);
}


bool header::hasField(const string& fieldName) const
{
	std::vector <shared_ptr <headerField> >::const_iterator pos =
		std::find_if(m_fields.begin(), m_fields.end(),
		             fieldHasName(utility::stringUtils::toLower(fieldName)));

	return (pos != m_fields.end());
}


shared_ptr <headerField> header::findField(const string& fieldName) const
{
	// Find the first field that matches the specified name
	std::vector <shared_ptr <headerField> >::const_iterator pos =
		std::find_if(m_fields.begin(), m_fields.end(),
		             fieldHasName(utility::stringUtils::toLower(fieldName)));

	// No field with this name can be found
	if (pos == m_fields.end())
		return null;

	// Else, return a reference to the existing field
	return (*pos);
}


std::vector <shared_ptr <headerField> > header::findAllFields(const string& fieldName)
{
	std::vector <shared_ptr <headerField> > result;
	std::back_insert_iterator <std::vector <shared_ptr <headerField> > > back(result);

	std::remove_copy_if(m_fields.begin(), m_fields.end(), back,
	                    fieldHasNotName(utility::stringUtils::toLower(fieldName)));

	return result;
}


shared_ptr <headerField> header::getField(const string& fieldName)
{
	const string name = utility::stringUtils::toLower(fieldName);

	// Find the first field that matches the specified name
	std::vector <shared_ptr <headerField> >::const_iterator pos = m_fields.begin();
	const std::vector <shared_ptr <headerField> >::const_iterator end = m_fields.end();

	while (pos != end && utility::stringUtils::toLower((*pos)->getName()) != name)
		++pos;

	// If no field with this name can be found, create a new one
	if (pos == end)
	{
		shared_ptr <headerField> field = headerFieldFactory::getInstance()->create(fieldName);

		appendField(field);

		// Return a reference to the new field
		return (field);
	}
	// Else, return a reference to the existing field
	else
	{
		return (*pos);
	}
}


void header::appendField(shared_ptr <headerField> field)
{
	m_fields.push_back(field);
}


void header::insertFieldBefore(shared_ptr <headerField> beforeField, shared_ptr <headerField> field)
{
	const std::vector <shared_ptr <headerField> >::iterator it = std::find
		(m_fields.begin(), m_fields.end(), beforeField);

	if (it == m_fields.end())
		throw exceptions::no_such_field();

	m_fields.insert(it, field);
}


void header::insertFieldBefore(const size_t pos, shared_ptr <headerField> field)
{
	m_fields.insert(m_fields.begin() + pos, field);
}


void header::insertFieldAfter(shared_ptr <headerField> afterField, shared_ptr <headerField> field)
{
	const std::vector <shared_ptr <headerField> >::iterator it = std::find
		(m_fields.begin(), m_fields.end(), afterField);

	if (it == m_fields.end())
		throw exceptions::no_such_field();

	m_fields.insert(it + 1, field);
}


void header::insertFieldAfter(const size_t pos, shared_ptr <headerField> field)
{
	m_fields.insert(m_fields.begin() + pos + 1, field);
}


void header::removeField(shared_ptr <headerField> field)
{
	const std::vector <shared_ptr <headerField> >::iterator it = std::find
		(m_fields.begin(), m_fields.end(), field);

	if (it == m_fields.end())
		throw exceptions::no_such_field();

	m_fields.erase(it);
}


void header::removeField(const size_t pos)
{
	const std::vector <shared_ptr <headerField> >::iterator it = m_fields.begin() + pos;

	m_fields.erase(it);
}


void header::replaceField(shared_ptr <headerField> field, shared_ptr <headerField> newField)
{
	insertFieldBefore(field, newField);
	removeField(field);
}


void header::removeAllFields()
{
	m_fields.clear();
}


void header::removeAllFields(const string& fieldName)
{
	std::vector <shared_ptr <headerField> > fields = findAllFields(fieldName);

	for (unsigned int i = 0 ; i < fields.size() ; ++i)
		removeField(fields[i]);
}


size_t header::getFieldCount() const
{
	return (m_fields.size());
}


bool header::isEmpty() const
{
	return (m_fields.empty());
}


const shared_ptr <headerField> header::getFieldAt(const size_t pos)
{
	return (m_fields[pos]);
}


const shared_ptr <const headerField> header::getFieldAt(const size_t pos) const
{
	return (m_fields[pos]);
}


const std::vector <shared_ptr <const headerField> > header::getFieldList() const
{
	std::vector <shared_ptr <const headerField> > list;

	list.reserve(m_fields.size());

	for (std::vector <shared_ptr <headerField> >::const_iterator it = m_fields.begin() ;
	     it != m_fields.end() ; ++it)
	{
		list.push_back(*it);
	}

	return (list);
}


const std::vector <shared_ptr <headerField> > header::getFieldList()
{
	return (m_fields);
}


const std::vector <shared_ptr <component> > header::getChildComponents()
{
	std::vector <shared_ptr <component> > list;

	copy_vector(m_fields, list);

	return (list);
}



// Field search


header::fieldHasName::fieldHasName(const string& name)
	: m_name(name)
{
}

bool header::fieldHasName::operator() (const shared_ptr <const headerField>& field)
{
	return utility::stringUtils::toLower(field->getName()) == m_name;
}


header::fieldHasNotName::fieldHasNotName(const string& name)
	: m_name(name)
{
}

bool header::fieldHasNotName::operator() (const shared_ptr <const headerField>& field)
{
	return utility::stringUtils::toLower(field->getName()) != m_name;
}


} // vmime
