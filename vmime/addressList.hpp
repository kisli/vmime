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

class VMIME_EXPORT addressList : public headerFieldValue
{
public:

	addressList();
	addressList(const addressList& addrList);

	~addressList();


	shared_ptr <component> clone() const;
	void copyFrom(const component& other);
	addressList& operator=(const addressList& other);
	addressList& operator=(const mailboxList& other);

	const std::vector <shared_ptr <component> > getChildComponents();


	/** Add a address at the end of the list.
	  *
	  * @param addr address to append
	  */
	void appendAddress(shared_ptr <address> addr);

	/** Insert a new address before the specified address.
	  *
	  * @param beforeAddress address before which the new address will be inserted
	  * @param addr address to insert
	  * @throw std::out_of_range if the address is not in the list
	  */
	void insertAddressBefore(shared_ptr <address> beforeAddress, shared_ptr <address> addr);

	/** Insert a new address before the specified position.
	  *
	  * @param pos position at which to insert the new address (0 to insert at
	  * the beginning of the list)
	  * @param addr address to insert
	  * @throw std::out_of_range if the position is out of range
	  */
	void insertAddressBefore(const size_t pos, shared_ptr <address> addr);

	/** Insert a new address after the specified address.
	  *
	  * @param afterAddress address after which the new address will be inserted
	  * @param addr address to insert
	  * @throw std::out_of_range if the address is not in the list
	  */
	void insertAddressAfter(shared_ptr <address> afterAddress, shared_ptr <address> addr);

	/** Insert a new address after the specified position.
	  *
	  * @param pos position of the address before the new address
	  * @param addr address to insert
	  * @throw std::out_of_range if the position is out of range
	  */
	void insertAddressAfter(const size_t pos, shared_ptr <address> addr);

	/** Remove the specified address from the list.
	  *
	  * @param addr address to remove
	  * @throw std::out_of_range if the address is not in the list
	  */
	void removeAddress(shared_ptr <address> addr);

	/** Remove the address at the specified position.
	  *
	  * @param pos position of the address to remove
	  * @throw std::out_of_range if the position is out of range
	  */
	void removeAddress(const size_t pos);

	/** Remove all addresses from the list.
	  */
	void removeAllAddresses();

	/** Return the number of addresses in the list.
	  *
	  * @return number of addresses
	  */
	size_t getAddressCount() const;

	/** Tests whether the list of addresses is empty.
	  *
	  * @return true if there is no address, false otherwise
	  */
	bool isEmpty() const;

	/** Return the address at the specified position.
	  *
	  * @param pos position
	  * @return address at position 'pos'
	  * @throw std::out_of_range if the position is out of range
	  */
	shared_ptr <address> getAddressAt(const size_t pos);

	/** Return the address at the specified position.
	  *
	  * @param pos position
	  * @return address at position 'pos'
	  * @throw std::out_of_range if the position is out of range
	  */
	const shared_ptr <const address> getAddressAt(const size_t pos) const;

	/** Return the address list.
	  *
	  * @return list of addresses
	  */
	const std::vector <shared_ptr <const address> > getAddressList() const;

	/** Return the address list.
	  *
	  * @return list of addresses
	  */
	const std::vector <shared_ptr <address> > getAddressList();

	/** Return a list of mailboxes.
	  * If some addresses are actually groups, mailboxes are recursively
	  * extracted from these groups.
	  *
	  * @return list of mailboxes
	  */
	shared_ptr <mailboxList> toMailboxList() const;

private:

	std::vector <shared_ptr <address> > m_list;

protected:

	// Component parsing & assembling
	void parseImpl
		(const parsingContext& ctx,
		 const string& buffer,
		 const size_t position,
		 const size_t end,
		 size_t* newPosition = NULL);

	void generateImpl
		(const generationContext& ctx,
		 utility::outputStream& os,
		 const size_t curLinePos = 0,
		 size_t* newLinePos = NULL) const;
};


} // vmime


#endif // VMIME_ADDRESSLIST_HPP_INCLUDED
