//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002 Vincent Richard <vincent@vmime.org>
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

#include "vmime/mailbox.hpp"
#include "vmime/parserHelpers.hpp"
#include "vmime/utility/outputStreamStringAdapter.hpp"


namespace vmime {


mailbox::mailbox() {

}


mailbox::mailbox(const mailbox& mbox)
	: address(),
	  m_name(mbox.m_name),
	  m_email(mbox.m_email) {

}


mailbox::mailbox(const emailAddress& email)
	: m_email(email) {

}


mailbox::mailbox(const text& name, const emailAddress& email)
	: m_name(name),
	  m_email(email) {

}


/*

 RFC #2822:
 3.4.  ADDRESS SPECIFICATION

mailbox         =       name-addr / addr-spec

name-addr       =       [display-name] angle-addr

angle-addr      =       [CFWS] "<" addr-spec ">" [CFWS] / obs-angle-addr

*/

void mailbox::parseImpl(
	parsingContext& ctx,
	const string& buffer,
	const size_t position,
	const size_t end,
	size_t* newPosition
) {

	const char* const pend = buffer.data() + end;
	const char* const pstart = buffer.data() + position;
	const char* p = pstart;

	// Current state for parsing machine
	enum States {
		State_None,
		State_String,
		State_Name,
		State_Bracket,
		State_Address_Bracketed,
		State_Address_Unbracketed,
		State_NameAt,
	};

	States state = State_None;

	// Temporary buffers for extracted name and address
	string name, address;
	unsigned int hold = 0;
	bool hadBrackets = false;

	while (p < pend) {
		if (state == State_None) {
			while (p < pend && parserHelpers::isSpace(*p))
				++p;
			if (*p == ',' || *p == ';')
				/* Completely new mailbox -- stop parsing this one. */
				break;
			if (*p == '<') {
				state = State_Bracket;
				continue;
			}
			if (*p == '"') {  // Quoted string
				state = State_String;
				++p;
				continue;
			}
			state = State_Name;
		} else if (state == State_String) {
			bool escaped = false;

			for (; p < pend; ++p) {
				if (escaped) {
					name += *p;
					escaped = false;
					continue;
				} else if (*p == '\\') {
					escaped = true;
					continue;
				}
				if (*p == '"') {
					++p;
					state = State_None;
					break;
				}
				name += *p;
			}
		} else if (state == State_Address_Bracketed) {
			bool escaped = false;
			int comment = 0;

			for (; p < pend; ++p) {
				if (escaped) {
					if (!comment) {
						address += *p;
						name += *p;
						++hold;
					}
					escaped = false;
				} else if (comment) {
					if (*p == '\\') {
						escaped = true;
					} else if (*p == '(') {
						++comment;
					} else if (*p == ')') {
						--comment;
					}
				} else if (*p == '(') {
					++comment;
				} else if (*p == '\\') {
					escaped = true;
				} else if (*p == '<') {
					state = State_Bracket;
					break;
				} else if (*p == '>') {
					hadBrackets = true;
					name += *p++;
					++hold;
					state = State_Name;
					break;
				} else if (parserHelpers::isSpace(*p)) {
					name += *p;
					++hold;
				} else {
					address += *p;
					name += *p;
					++hold;
				}
			}
		} else if (state == State_Address_Unbracketed) {
			bool escaped = false;
			int comment = 0;

			for (; p < pend; ++p) {
				if (escaped) {
					if (!comment) {
						address += *p;
						name += *p;
						++hold;
					}
					escaped = false;
				} else if (comment) {
					if (*p == '\\')
						escaped = true;
					else if (*p == '(')
						++comment;
					else if (*p == ')')
						--comment;
				} else if (*p == '(') {
					++comment;
				} else if (*p == '\\') {
					escaped = true;
				} else if (*p == '<') {
					state = State_Bracket;
					break;
				} else if (*p == '>') {
					name += *p++;
					++hold;
					state = State_None;
					break;
				} else if (*p == ',' || *p == ';') {
					state = State_None;
					break;
				} else if (parserHelpers::isSpace(*p)) {
					state = State_Name;
					break;
				} else {
					address += *p;
					name += *p;
					++hold;
				}
			}
		} else if (state == State_Name) {
			bool escaped = false;
			unsigned int comment = 0, at_hold = 0;

			for (; p < pend; ++p) {
				if (escaped) {
					if (!comment) {
						name += *p;
						++at_hold;
					}
					escaped = false;
				} else if (comment) {
					if (*p == '\\')
						escaped = true;
					else if (*p == '(')
						++comment;
					else if (*p == ')')
						--comment;
				} else if (*p == '\\') {
					escaped = true;
				} else if (*p == '(') {
					++comment;
				} else if (*p == '<') {
					state = State_Bracket;
					break;
				} else if (*p == '@') {
					hold = at_hold;
					state = State_NameAt;
					break;
				} else if (parserHelpers::isSpace(*p)) {
					name += *p;
					++hold;
					at_hold = 1;
				} else {
					name += *p;
					hold = 0;
					++at_hold;
				}
			}
		} else if (state == State_Bracket) {
			string::const_iterator q = name.cend();
			unsigned int nh = 0;
			while (nh < hold && q != name.cbegin() && parserHelpers::isSpace(*(q - 1))) {
				--q;
				++nh;
			}
			hold = nh;
			name += *p++;
			++hold;
			if (!address.empty())
				// If we found a '<' here, it means that the address
				// starts _only_ here...and the stuff we have parsed
				// before belongs actually to the display name!
				address.clear();
			state = State_Address_Bracketed;
		} else if (state == State_NameAt) {
			name += *p++;
			++hold;
			// (*) Actually, we were parsing the local-part of an address
			// and not a display name...
			// Back up to the last whitespace and treat as address
			string::const_iterator q = name.cend();
			unsigned int nh = 0;
			while (nh < hold && q != name.cbegin() && !parserHelpers::isSpace(*(q - 1))) {
				--q;
				++nh;
			}
			address.assign(q, name.cend());
			while (nh < hold && q != name.cbegin() && parserHelpers::isSpace(*(q - 1))) {
				--q;
				++nh;
			}
			hold = nh;
			state = State_Address_Unbracketed;
		}
	}

	if (hold <= name.size())
		name.erase(name.size() - hold);

	// Swap name and address when no address was found
	// (email address is mandatory, whereas name is optional).
	if (address.empty() && !name.empty() && !hadBrackets) {

		m_name.removeAllWords();
		m_email.parse(ctx, name);

	} else {

		text::decodeAndUnfold(ctx, name, &m_name);
		m_email.parse(ctx, address);
	}

	setParsedBounds(position, position + (p - pstart));

	if (newPosition) {
		*newPosition = position + (p - pstart);
	}
}


void mailbox::generateImpl(
	const generationContext& ctx,
	utility::outputStream& os,
	const size_t curLinePos,
	size_t* newLinePos
) const {

	string generatedEmail;
	utility::outputStreamStringAdapter generatedEmailStream(generatedEmail);
	m_email.generate(ctx, generatedEmailStream, 0, NULL);

	if (m_name.isEmpty()) {

		size_t pos = curLinePos;

		// No display name is specified, only email address.
		if (curLinePos + generatedEmail.length() > ctx.getMaxLineLength()) {

			os << NEW_LINE_SEQUENCE;
			pos = NEW_LINE_SEQUENCE.length();
		}

		os << generatedEmail;
		pos += generatedEmail.length();

		if (newLinePos) {
			*newLinePos = pos;
		}

	} else {

		// We have to encode the name:
		//   - if it contains characters in a charset different from "US-ASCII",
		//   - and/or if it contains one or more of these special chars:
		//        CR  LF  TAB  "  ;  ,  <  >  (  )  @  /  ?  .  =  :
		//     these special chars only require quoting, not full encoding
		// Check whether there are words that are not "US-ASCII"
		// and/or contain the special chars.
		bool forceEncode = false;

		for (size_t w = 0 ; !forceEncode && w != m_name.getWordCount() ; ++w) {

			if (m_name.getWordAt(w)->getCharset() != charset(charsets::US_ASCII)) {
				forceEncode = true;
			}
		}

		size_t pos = curLinePos;

		m_name.encodeAndFold(
			ctx, os, pos, &pos,
			text::QUOTE_IF_POSSIBLE | (forceEncode ? text::FORCE_ENCODING : 0)
		);

		if (pos + generatedEmail.length() + 3 > ctx.getMaxLineLength()) {

			os << NEW_LINE_SEQUENCE;
			pos = NEW_LINE_SEQUENCE.length();
		}

		os << " <" << generatedEmail << ">";
		pos += 2 + generatedEmail.length() + 1;

		if (newLinePos) {
			*newLinePos = pos;
		}
	}
}


bool mailbox::operator==(const class mailbox& mailbox) const {

	return m_name == mailbox.m_name && m_email == mailbox.m_email;
}


bool mailbox::operator!=(const class mailbox& mailbox) const {

	return !(*this == mailbox);
}


void mailbox::copyFrom(const component& other) {

	const mailbox& source = dynamic_cast <const mailbox&>(other);

	m_name = source.m_name;
	m_email = source.m_email;
}


mailbox& mailbox::operator=(const mailbox& other) {

	copyFrom(other);
	return *this;
}


shared_ptr <component>mailbox::clone() const {

	return make_shared <mailbox>(*this);
}


bool mailbox::isEmpty() const {

	return m_email.isEmpty();
}


void mailbox::clear() {

	m_name.removeAllWords();
	m_email = emailAddress();
}


bool mailbox::isGroup() const {

	return false;
}


const text& mailbox::getName() const {

	return m_name;
}


void mailbox::setName(const text& name) {

	m_name = name;
}


const emailAddress& mailbox::getEmail() const {

	return m_email;
}


void mailbox::setEmail(const emailAddress& email) {

	m_email = email;
}


const std::vector <shared_ptr <component> > mailbox::getChildComponents() {

	return std::vector <shared_ptr <component> >();
}


} // vmime
