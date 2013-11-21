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


// Helper function to obtain an encoder given its name
static vmime::shared_ptr <vmime::utility::encoder::encoder> getEncoder(const vmime::string& name,
	 int maxLineLength = 0, const vmime::propertySet props = vmime::propertySet())
{
	vmime::shared_ptr <vmime::utility::encoder::encoder> enc =
		vmime::utility::encoder::encoderFactory::getInstance()->create(name);

	enc->getProperties() = props;

	if (maxLineLength != 0)
		enc->getProperties()["maxlinelength"] = maxLineLength;

	return enc;
}


// Encoding helper function
static const vmime::string encode(const vmime::string& name, const vmime::string& in,
	int maxLineLength = 0, const vmime::propertySet props = vmime::propertySet())
{
	vmime::shared_ptr <vmime::utility::encoder::encoder> enc = getEncoder(name, maxLineLength, props);

	vmime::utility::inputStreamStringAdapter vin(in);

	std::ostringstream out;
	vmime::utility::outputStreamAdapter vout(out);

	enc->encode(vin, vout);

	return (out.str());
}


// Decoding helper function
static const vmime::string decode(const vmime::string& name, const vmime::string& in, int maxLineLength = 0)
{
	vmime::shared_ptr <vmime::utility::encoder::encoder> enc = getEncoder(name, maxLineLength);

	vmime::utility::inputStreamStringAdapter vin(in);

	std::ostringstream out;
	vmime::utility::outputStreamAdapter vout(out);

	enc->decode(vin, vout);

	return (out.str());
}
