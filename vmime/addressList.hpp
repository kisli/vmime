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

#ifndef VMIME_ADDRESSLIST_HPP_INCLUDED
#define VMIME_ADDRESSLIST_HPP_INCLUDED


#include "vmime/base.hpp"
#include "vmime/headerFieldValue.hpp"

#include "vmime/address.hpp"


namespace vmime
{


class mailboxList;


/** A list of addresses.
  */

class addressList : public headerFieldValue
{
public:

	addressList();
	addressList(const addressList& addrList);

	~addressList();


	ref <component> clone() const;
	void copyFrom(const component& other);
	addressList& operator=(const addressList& other);
	addressList& operator=(const mailboxList& other);

	const std::vector <ref <component> > getChildComponents();


	/** Add a address at the end of the list.
	  *
	  * @param addr address to append
	  */
	void appendAddress(ref <address> addr);

	/** Insert a new address before the specified address.
	  *
	  * @param beforeAddress address before which the new address will be inserted
	  * @param addr address to insert
	  * @throw exceptions::no_such_address if the address is not in the list
	  */
	void insertAddressBefore(ref <address> beforeAddress, ref <address> addr);

	/** Insert a new address before the specified position.
	  *
	  * @param pos position at which to insert the new address (0 to insert at
	  * the beginning of the list)
	  * @param addr address to insert
	  */
	void insertAddressBefore(const int pos, ref <address> addr);

	/** Insert a new address after the specified address.
	  *
	  * @param afterAddress address after which the new address will be inserted
	  * @param addr address to insert
	  * @throw exceptions::no_such_address if the address is not in the list
	  */
	void insertAddressAfter(ref <address> afterAddress, ref <address> addr);

	/** Insert a new address after the specified position.
	  *
	  * @param pos position of the address before the new address
	  * @param addr address to insert
	  */
	void insertAddressAfter(const int pos, ref <address> addr);

	/** Remove the specified address from the list.
	  *
	  * @param addr address to remove
	  * @throw exceptions::no_such_address if the address is not in the list
	  */
	void removeAddress(ref <address> addr);

	/** Remove the address at the specified position.
	  *
	  * @param pos position of the address to remove
	  */
	void removeAddress(const int pos);

	/** Remove all addresses from the list.
	  */
	void removeAllAddresses();

	/** Return the number of addresses in the list.
	  *
	  * @return number of addresses
	  */
	int getAddressCount() const;

	/** Tests whether the list of addresses is empty.
	  *
	  * @return true if there is no address, false otherwise
	  */
	bool isEmpty() const;

	/** Return the address at the specified position.
	  *
	  * @param pos position
	  * @return address at position 'pos'
	  */
	ref <address> getAddressAt(const int pos);

	/** Return the address at the specified position.
	  *
	  * @param pos position
	  * @return address at position 'pos'
	  */
	const ref <const address> getAddressAt(const int pos) const;

	/** Return the address list.
	  *
	  * @return list of addresses
	  */
	const std::vector <ref <const address> > getAddressList() const;

	/** Return the address list.
	  *
	  * @return list of addresses
	  */
	const std::vector <ref <address> > getAddressList();

	/** Return a list of mailboxes.
	  * If some addresses are actually groups, mailboxes are recursively
	  * extracted from these groups.
	  *
	  * @return list of mailboxes
	  */
	ref <mailboxList> toMailboxList() const;

private:

	std::vector <ref <address> > m_list;

protected:

	// Component parsing & assembling
	void parseImpl
		(const string& buffer,
		 const string::size_type position,
		 const string::size_type end,
		 string::size_type* newPosition = NULL);

	void generateImpl
		(utility::outputStream& os,
		 const string::size_type maxLineLength = lineLengthLimits::infinite,
		 const string::size_type curLinePos = 0,
		 string::size_type* newLinePos = NULL) const;
};


} // vmime


#endif // VMIME_ADDRESSLIST_HPP_INCLUDED
