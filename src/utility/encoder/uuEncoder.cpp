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

#include "vmime/utility/encoder/uuEncoder.hpp"
#include "vmime/parserHelpers.hpp"


namespace vmime {
namespace utility {
namespace encoder {


uuEncoder::uuEncoder()
{
	getProperties()["mode"] = 644;
	getProperties()["filename"] = "no_name";
	getProperties()["maxlinelength"] = 46;
}


const std::vector <string> uuEncoder::getAvailableProperties() const
{
	std::vector <string> list(encoder::getAvailableProperties());

	list.push_back("maxlinelength");

	list.push_back("mode");
	list.push_back("filename");

	return (list);
}


// This is the character encoding function to make a character printable
static inline byte_t UUENCODE(const unsigned int c)
{
	return static_cast <byte_t>((c & 077) + ' ');
}

// Single character decoding
static inline unsigned int UUDECODE(const unsigned int c)
{
	return (c - ' ') & 077;
}


size_t uuEncoder::encode(utility::inputStream& in,
	utility::outputStream& out, utility::progressListener* progress)
{
	in.reset();  // may not work...

	const string propFilename = getProperties().getProperty <string>("filename", "");
	const string propMode = getProperties().getProperty <string>("mode", "644");

	const size_t maxLineLength =
		std::min(getProperties().getProperty <size_t>("maxlinelength", 46), static_cast <size_t>(46));

	size_t total = 0;
	size_t inTotal = 0;

	// Output the prelude text ("begin [mode] [filename]")
	out << "begin";

	if (!propFilename.empty())
	{
		out << " " << propMode << " " << propFilename;
		total += 2 + propMode.length() + propFilename.length();
	}

	out << "\r\n";
	total += 7;

	// Process the data
	byte_t inBuffer[64];
	byte_t outBuffer[64];

	if (progress)
		progress->start(0);

	while (!in.eof())
	{
		// Process up to 45 characters per line
		std::fill(inBuffer, inBuffer + sizeof(inBuffer), 0);

		const size_t inLength = in.read(inBuffer, maxLineLength - 1);

		outBuffer[0] = UUENCODE(static_cast <unsigned int>(inLength)); // Line length

		size_t j = 1;

		for (size_t i = 0 ; i < inLength ; i += 3, j += 4)
		{
			const byte_t c1 = inBuffer[i];
			const byte_t c2 = inBuffer[i + 1];
			const byte_t c3 = inBuffer[i + 2];

			outBuffer[j]     = UUENCODE(c1 >> 2);
			outBuffer[j + 1] = UUENCODE(((c1 << 4) & 060) | ((c2 >> 4) & 017));
			outBuffer[j + 2] = UUENCODE(((c2 << 2) & 074) | ((c3 >> 6) & 03));
			outBuffer[j + 3] = UUENCODE(c3 & 077);
		}

		outBuffer[j] = '\r';
		outBuffer[j + 1] = '\n';

		out.write(outBuffer, j + 2);

		total += j + 2;
		inTotal += inLength;

		if (progress)
			progress->progress(inTotal, inTotal);
	}

	out << "end\r\n";
	total += 5;

	if (progress)
		progress->stop(inTotal);

	return (total);
}


size_t uuEncoder::decode(utility::inputStream& in,
	utility::outputStream& out, utility::progressListener* progress)
{
	in.reset();  // may not work...

	// Process the data
	byte_t inBuffer[64];
	byte_t outBuffer[64];

	size_t total = 0;
	size_t inTotal = 0;

	bool stop = false;

	std::fill(inBuffer, inBuffer + sizeof(inBuffer), 0);

	if (progress)
		progress->start(0);

	while (!stop && !in.eof())
	{
		// Get the line length
		byte_t lengthChar;

		if (in.read(&lengthChar, 1) == 0)
			break;

		const size_t outLength = UUDECODE(lengthChar);
		const size_t inLength = std::min((outLength * 4) / 3, static_cast <size_t>(64));
		size_t inPos = 0;

		switch (lengthChar)
		{
		case ' ':
		case '\t':
		case '\r':
		case '\n':
		{
			// Ignore
			continue;
		}
		case 'b':
		{
			// Read 5 characters more to check for begin ("begin ...\r\n" or "begin ...\n")
			inPos = in.read(inBuffer, 5);

			if (inPos == 5 &&
			    inBuffer[0] == 'e' &&
			    inBuffer[1] == 'g' &&
			    inBuffer[2] == 'i' &&
			    inBuffer[3] == 'n' &&
			    parserHelpers::isSpace(inBuffer[4]))
			{
				inTotal += 5;

				byte_t c = 0;

				size_t count = 0;
				byte_t buffer[512];

				while (count < sizeof(buffer) - 1 && in.read(&c, 1) == 1)
				{
					if (c == '\n')
						break;

					buffer[count++] = c;
				}

				inTotal += count;

				if (c != '\n')
				{
					// OOPS! Weird line. Don't try to decode more...

					if (progress)
						progress->stop(inTotal);

					return (total);
				}

				// Parse filename and mode
				if (count > 0)
				{
					buffer[count] = '\0';

					byte_t* p = buffer;

					while (*p && parserHelpers::isSpace(*p)) ++p;

					byte_t* modeStart = buffer;

					while (*p && !parserHelpers::isSpace(*p)) ++p;

					getResults()["mode"] = string(modeStart, p);

					while (*p && parserHelpers::isSpace(*p)) ++p;

					byte_t* filenameStart = buffer;

					while (*p && !(*p == '\r' || *p == '\n')) ++p;

					getResults()["filename"] = string(filenameStart, p);
				}
				// No filename or mode specified
				else
				{
					getResults()["filename"] = "untitled";
					getResults()["mode"] = 644;
				}

				continue;
			}

			break;
		}
		case 'e':
		{
			// Read 3 characters more to check for end ("end\r\n" or "end\n")
			inPos = in.read(inBuffer, 3);

			if (inPos == 3 &&
			    inBuffer[0] == 'n' &&
			    inBuffer[1] == 'd' &&
			    (inBuffer[2] == '\r' || inBuffer[2] == '\n'))
			{
				stop = true;
				inTotal += 3;
				continue;
			}

			break;
		}

		}

		// Read encoded data
		if (in.read(inBuffer + inPos, inLength - inPos) != inLength - inPos)
		{
			// Premature end of data
			break;
		}

		inTotal += (inLength - inPos);

		// Decode data
		for (size_t i = 0, j = 0 ; i < inLength ; i += 4, j += 3)
		{
			const byte_t c1 = inBuffer[i];
			const byte_t c2 = inBuffer[i + 1];
			const byte_t c3 = inBuffer[i + 2];
			const byte_t c4 = inBuffer[i + 3];

			const size_t n = std::min(inLength - i, static_cast <size_t>(3));

			switch (n)
			{
			default:
			case 3: outBuffer[j + 2] = static_cast <byte_t>(UUDECODE(c3) << 6 | UUDECODE(c4));
			case 2: outBuffer[j + 1] = static_cast <byte_t>(UUDECODE(c2) << 4 | UUDECODE(c3) >> 2);
			case 1: outBuffer[j]     = static_cast <byte_t>(UUDECODE(c1) << 2 | UUDECODE(c2) >> 4);
			case 0: break;
			}

			total += n;
		}

		out.write(outBuffer, outLength);

		std::fill(inBuffer, inBuffer + sizeof(inBuffer), 0);

		if (progress)
			progress->progress(inTotal, inTotal);
	}

	if (progress)
		progress->stop(inTotal);

	return (total);
}


size_t uuEncoder::getEncodedSize(const size_t n) const
{
	// 3 bytes of input provide 4 bytes of output.
	// Count CRLF (2 bytes) for each line of 45 characters.
	// Also reserve some space for header and footer.
	return 200 + n * 3 + (n / 45) * 2;
}


size_t uuEncoder::getDecodedSize(const size_t n) const
{
	// 4 bytes of input provide 3 bytes of output
	return (n * 3) / 4;
}


} // encoder
} // utility
} // vmime
