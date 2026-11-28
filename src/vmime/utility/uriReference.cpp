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

#include "vmime/utility/uriReference.hpp"
#include "vmime/parserHelpers.hpp"


namespace vmime {
namespace utility {


uriReference::uriReference()
	: m_hasScheme(false),
	  m_hasAuthority(false),
	  m_hasQuery(false),
	  m_hasFragment(false) {

}


uriReference::uriReference(const string& ref)
	: m_hasScheme(false),
	  m_hasAuthority(false),
	  m_hasQuery(false),
	  m_hasFragment(false) {

	size_t pos = 0;

	// scheme = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." ), followed by ':'
	if (!ref.empty() && parserHelpers::isAlpha(ref[0])) {

		size_t end = 1;

		while (end < ref.length() &&
		       (parserHelpers::isAlpha(ref[end]) || parserHelpers::isDigit(ref[end]) ||
		        ref[end] == '+' || ref[end] == '-' || ref[end] == '.')) {

			++end;
		}

		if (end < ref.length() && ref[end] == ':') {

			m_hasScheme = true;
			m_scheme = ref.substr(0, end);
			pos = end + 1;
		}
	}

	if (ref.compare(pos, 2, "//") == 0) {

		const size_t end = std::min(ref.find_first_of("/?#", pos + 2), ref.length());

		m_hasAuthority = true;
		m_authority = ref.substr(pos + 2, end - pos - 2);
		pos = end;
	}

	const size_t pathEnd = std::min(ref.find_first_of("?#", pos), ref.length());

	m_path = ref.substr(pos, pathEnd - pos);
	pos = pathEnd;

	if (pos < ref.length() && ref[pos] == '?') {

		const size_t end = std::min(ref.find('#', pos), ref.length());

		m_hasQuery = true;
		m_query = ref.substr(pos + 1, end - pos - 1);
		pos = end;
	}

	if (pos < ref.length()) {  // '#'

		m_hasFragment = true;
		m_fragment = ref.substr(pos + 1);
	}
}


bool uriReference::hasScheme() const {

	return m_hasScheme;
}


const string& uriReference::getScheme() const {

	return m_scheme;
}


bool uriReference::hasAuthority() const {

	return m_hasAuthority;
}


const string& uriReference::getAuthority() const {

	return m_authority;
}


const string& uriReference::getPath() const {

	return m_path;
}


bool uriReference::hasQuery() const {

	return m_hasQuery;
}


const string& uriReference::getQuery() const {

	return m_query;
}


bool uriReference::hasFragment() const {

	return m_hasFragment;
}


const string& uriReference::getFragment() const {

	return m_fragment;
}


// RFC 3986, section 5.2.4
static string removeDotSegments(const string& path) {

	string input = path;
	string output;

	while (!input.empty()) {

		// A. Remove a "../" or "./" prefix
		if (input.compare(0, 3, "../") == 0) {

			input.erase(0, 3);

		} else if (input.compare(0, 2, "./") == 0) {

			input.erase(0, 2);

		// B. Replace a "/./" or "/." prefix with "/"
		} else if (input.compare(0, 3, "/./") == 0 || input == "/.") {

			input.replace(0, input == "/." ? 2 : 3, "/");

		// C. Replace a "/../" or "/.." prefix with "/", and remove the
		// last segment of the output
		} else if (input.compare(0, 4, "/../") == 0 || input == "/..") {

			input.replace(0, input == "/.." ? 3 : 4, "/");

			const size_t lastSlash = output.rfind('/');
			output.erase(lastSlash == string::npos ? 0 : lastSlash);

		// D. Remove a "." or ".." path
		} else if (input == "." || input == "..") {

			input.clear();

		// E. Move the first segment, and the "/" before it, to the output
		} else {

			const size_t end = std::min(input.find('/', 1), input.length());

			output += input.substr(0, end);
			input.erase(0, end);
		}
	}

	return output;
}


// RFC 3986, section 5.2.3
static string mergePaths(const uriReference& base, const string& path) {

	if (base.hasAuthority() && base.getPath().empty()) {
		return "/" + path;
	}

	const size_t lastSlash = base.getPath().rfind('/');

	if (lastSlash == string::npos) {
		return path;
	}

	return base.getPath().substr(0, lastSlash + 1) + path;
}


uriReference uriReference::resolve(const uriReference& base) const {

	uriReference target;

	if (m_hasScheme || m_hasAuthority) {

		target = *this;
		target.m_path = removeDotSegments(m_path);

	} else {

		if (m_path.empty()) {

			target.m_path = base.m_path;
			target.m_hasQuery = m_hasQuery || base.m_hasQuery;
			target.m_query = m_hasQuery ? m_query : base.m_query;

		} else {

			target.m_path = removeDotSegments(m_path[0] == '/' ? m_path : mergePaths(base, m_path));
			target.m_hasQuery = m_hasQuery;
			target.m_query = m_query;
		}

		target.m_hasAuthority = base.m_hasAuthority;
		target.m_authority = base.m_authority;
	}

	if (!m_hasScheme) {

		target.m_hasScheme = base.m_hasScheme;
		target.m_scheme = base.m_scheme;
	}

	target.m_hasFragment = m_hasFragment;
	target.m_fragment = m_fragment;

	return target;
}


string uriReference::toString() const {

	string result;

	if (m_hasScheme) {
		result += m_scheme + ":";
	}

	if (m_hasAuthority) {
		result += "//" + m_authority;
	}

	result += m_path;

	if (m_hasQuery) {
		result += "?" + m_query;
	}

	if (m_hasFragment) {
		result += "#" + m_fragment;
	}

	return result;
}


} // utility
} // vmime
