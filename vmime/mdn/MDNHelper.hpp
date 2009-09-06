//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2009 Vincent Richard <vincent@vincent-richard.net>
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

#ifndef VMIME_MDN_MDNHELPER_HPP_INCLUDED
#define VMIME_MDN_MDNHELPER_HPP_INCLUDED


#include "vmime/mdn/receivedMDNInfos.hpp"
#include "vmime/mdn/sendableMDNInfos.hpp"

#include "vmime/mailboxList.hpp"


namespace vmime {
namespace mdn {


/** Helper for creating or extracting Message Disposition
  * Notifications (MDN), as defined in RFC-3798.
  */

class MDNHelper
{
public:

	/** Attach a MDN request to the specified message.
	  *
	  * @param msg message in which to add a MDN request
	  * @param mailboxes list of mailboxes to which the MDN will be sent
	  */
	static void attachMDNRequest(ref <message> msg, const mailboxList& mailboxes);

	/** Attach a MDN request to the specified message.
	  *
	  * @param msg message in which to add a MDN request
	  * @param mbox mailbox to which the MDN will be sent
	  */
	static void attachMDNRequest(ref <message> msg, const mailbox& mbox);

	/** Return a list of possible MDNs that can be generated
	  * for the specified message.
	  *
	  * @param msg message for which to send a MDN
	  * @return list of possible MDNs
	  */
	static const std::vector <sendableMDNInfos> getPossibleMDNs(const ref <const message> msg);

	/** Test whether the specified message is a MDN.
	  *
	  * @param msg message
	  * @return true if the message is a MDN, false otherwise
	  */
	static bool isMDN(const ref <const message> msg);

	/** If the specified message is a MDN, return information
	  * about it.
	  *
	  * @param msg message
	  * @throw exceptions::invalid_argument if the message is not a MDN
	  * @return information about the MDN
	  */
	static receivedMDNInfos getReceivedMDN(const ref <const message> msg);

	/** Check whether we need user confirmation for sending a MDN even
	  * if he/she explicitely allowed automatic send of MDNs. This can
	  * happen in some situations, described in RFC-3798.
	  *
	  * @param msg message for which to send a MDN
	  * @return true if user confirmation should be asked, false otherwise
	  */
	static bool needConfirmation(const ref <const message> msg);

	/** Build a new MDN for the message. The resulting MDN can then be
	  * sent over SMTP transport service.
	  *
	  * @param mdnInfos information about the MDN to construct
	  * @param text human readable message. The purpose of this message is
	  *             to provide an easily-understood description of the
	  *             condition(s) that caused the report to be generated.
	  * @param ch charset of the text
	  * @param expeditor expeditor of the MDN
	  * @param dispo disposition information
	  * @param reportingUA name of reporting user-agent (optional)
	  * @param reportingUAProducts list of products in the reporting user-agent (optional)
	  * @return a new message object containing the MDN
	  */
	static ref <message> buildMDN(const sendableMDNInfos& mdnInfos,
	                              const string& text,
	                              const charset& ch,
	                              const mailbox& expeditor,
	                              const disposition& dispo,
	                              const string& reportingUA = NULL_STRING,
	                              const std::vector <string>& reportingUAProducts
	                                   = std::vector <string>());

private:

	static ref <bodyPart> createFirstMDNPart(const sendableMDNInfos& mdnInfos,
	                                         const string& text, const charset& ch);

	static ref <bodyPart> createSecondMDNPart(const sendableMDNInfos& mdnInfos,
	                                          const disposition& dispo,
	                                          const string& reportingUA,
	                                          const std::vector <string>& reportingUAProducts);

	static ref <bodyPart> createThirdMDNPart(const sendableMDNInfos& mdnInfos);
};


} // mdn
} // vmime


#endif // VMIME_MDN_MDNHELPER_HPP_INCLUDED
