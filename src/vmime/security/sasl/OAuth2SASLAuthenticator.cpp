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

#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES


#include "vmime/security/sasl/OAuth2SASLAuthenticator.hpp"

#include "vmime/security/sasl/SASLMechanism.hpp"


namespace vmime {
namespace security {
namespace sasl {


namespace {

// OAuth 2.0 mechanisms, in order of preference: OAUTHBEARER is the standard
// one (RFC 7628), and XOAUTH2 is supported by more servers
const char* const OAUTH2_MECHANISMS[] = {
	"OAUTHBEARER",
	"XOAUTH2"
};

const size_t OAUTH2_MECHANISM_COUNT = sizeof(OAUTH2_MECHANISMS) / sizeof(OAUTH2_MECHANISMS[0]);


bool isOAuth2Mechanism(const string& name) {

	for (size_t i = 0 ; i < OAUTH2_MECHANISM_COUNT ; ++i) {

		if (name == OAUTH2_MECHANISMS[i]) {
			return true;
		}
	}

	return false;
}

}


OAuth2SASLAuthenticator::OAuth2SASLAuthenticator(const Mode mode)
	: m_mode(mode) {

}


OAuth2SASLAuthenticator::~OAuth2SASLAuthenticator() {

}


const std::vector <shared_ptr <SASLMechanism> >
	OAuth2SASLAuthenticator::getAcceptableMechanisms(
		const std::vector <shared_ptr <SASLMechanism> >& available,
		const shared_ptr <SASLMechanism>& suggested
	) const {

	std::vector <shared_ptr <SASLMechanism> > mechs;

	// Only try the preferred OAuth 2.0 mechanism among those supported by the
	// server: they all send the same access token, so if the server rejects it
	// with one mechanism, it would also reject it with the others
	for (size_t i = 0 ; mechs.empty() && i < OAUTH2_MECHANISM_COUNT ; ++i) {

		for (size_t j = 0 ; j < available.size() ; ++j) {

			if (available[j]->getName() == OAUTH2_MECHANISMS[i]) {

				mechs.push_back(available[j]);
				break;
			}
		}
	}

	if (m_mode == MODE_SUGGEST) {

		// Then, try the other mechanisms (except OAuth 2.0 ones) in the default order
		const std::vector <shared_ptr <SASLMechanism> > others =
			defaultSASLAuthenticator::getAcceptableMechanisms(available, suggested);

		for (size_t i = 0 ; i < others.size() ; ++i) {

			if (!isOAuth2Mechanism(others[i]->getName())) {
				mechs.push_back(others[i]);
			}
		}
	}

	return mechs;
}


} // sasl
} // security
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES
