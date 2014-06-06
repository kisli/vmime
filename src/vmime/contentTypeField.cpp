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

#include "vmime/contentTypeField.hpp"
#include "vmime/exception.hpp"


namespace vmime
{


contentTypeField::contentTypeField()
{
}


contentTypeField::contentTypeField(contentTypeField&)
	: parameterizedHeaderField()
{
}


bool contentTypeField::hasBoundary() const
{
	return hasParameter("boundary");
}


const string contentTypeField::getBoundary() const
{
	shared_ptr <parameter> param = findParameter("boundary");

	if (param)
		return param->getValue().getBuffer();
	else
		return "";
}


void contentTypeField::setBoundary(const string& boundary)
{
	getParameter("boundary")->setValue(word(boundary, vmime::charsets::US_ASCII));
}


bool contentTypeField::hasCharset() const
{
	return hasParameter("charset");
}


const charset contentTypeField::getCharset() const
{
	shared_ptr <parameter> param = findParameter("charset");

	if (param)
		return param->getValueAs <charset>();
	else
		return charset();
}


void contentTypeField::setCharset(const charset& ch)
{
	getParameter("charset")->setValue(ch);
}


bool contentTypeField::hasReportType() const
{
	return hasParameter("report-type");
}


const string contentTypeField::getReportType() const
{
	shared_ptr <parameter> param = findParameter("report-type");

	if (param)
		return param->getValue().getBuffer();
	else
		return "";
}


void contentTypeField::setReportType(const string& reportType)
{
	getParameter("report-type")->setValue(word(reportType, vmime::charsets::US_ASCII));
}


} // vmime

