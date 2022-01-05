/*---------------------------------------------------------------------------*\
 *
 *  bitpit
 *
 *  Copyright (C) 2015-2021 OPTIMAD engineering Srl
 *
 *  -------------------------------------------------------------------------
 *  License
 *  This file is part of bitpit.
 *
 *  bitpit is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License v3 (LGPL)
 *  as published by the Free Software Foundation.
 *
 *  bitpit is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 *  License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with bitpit. If not, see <http://www.gnu.org/licenses/>.
 *
\*---------------------------------------------------------------------------*/

#ifndef __BITPIT_INTERFACE_HPP__
#define __BITPIT_INTERFACE_HPP__

#include <array>
#include <memory>

#include "bitpit_containers.hpp"

#include "element.hpp"

namespace bitpit {

class Interface : public Element {

public:
	Interface();
	Interface(long id, ElementType type = ElementType::UNDEFINED);
	Interface(long id, ElementType type, int connectSize = 0);
	Interface(long id, ElementType type, std::unique_ptr<long[]> &&connectStorage);

	void swap(Interface &other) noexcept;

	void initialize(long id, ElementType type);
	void initialize(long id, ElementType type, int connectSize);
	void initialize(long id, ElementType type, std::unique_ptr<long[]> &&connectStorage);

	bool isBorder() const;

	static std::array<std::array<double, 3>, 3> evalRotationFromCartesian(std::array<double, 3> &versor);
	static std::array<std::array<double, 3>, 3> evalRotationToCartesian(std::array<double, 3> &versor);
	static std::array<std::array<double, 3>, 3> evalRotationInverse(const std::array<std::array<double, 3>, 3> &R);
	static std::array<std::array<double, 3>, 3> evalRotationTranspose(const std::array<std::array<double, 3>, 3> &R);

	void setOwner(long owner, int onwerFace);
	void unsetOwner();
	long getOwner() const;
	int getOwnerFace() const;

	void setNeigh(long neigh, int neighFace);
	void unsetNeigh();
	long getNeigh() const;
	int getNeighFace() const;

	std::array<long, 2> getOwnerNeigh() const;

	void display(std::ostream &out, unsigned short int indent) const;

private:
	long m_owner;
	long m_neigh;

	int m_ownerFace;
	int m_neighFace;

	void _initialize(long owner, long ownerFace, long neigh, long neighFace);

};

template<typename QualifiedInterface>
class QualifiedInterfaceHalfEdge : public ElementHalfFace<QualifiedInterface> {

public:
	typedef typename ElementHalfFace<QualifiedInterface>::Winding Winding;

	QualifiedInterfaceHalfEdge(QualifiedInterface &interface, int edge, Winding winding = Winding::WINDING_NATURAL);

	QualifiedInterface & getInterface() const;

};

extern template class PiercedVector<Interface>;

extern template class QualifiedInterfaceHalfEdge<Interface>;
extern template class QualifiedInterfaceHalfEdge<const Interface>;

typedef QualifiedInterfaceHalfEdge<Interface> InterfaceHalfEdge;
typedef QualifiedInterfaceHalfEdge<const Interface> InterfaceConstHalfEdge;


}

// Include template implementations
#include "interface.tpp"

#endif
