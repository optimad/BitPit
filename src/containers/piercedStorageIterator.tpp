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

#ifndef __BITPIT_PIERCED_STORAGE_ITERATOR_TPP__
#define __BITPIT_PIERCED_STORAGE_ITERATOR_TPP__

namespace bitpit {

/**
* Creates a new uninitialized iterator
*/
template<typename value_t, typename id_t, typename value_no_cv_t>
PiercedStorageIterator<value_t, id_t, value_no_cv_t>::PiercedStorageIterator()
    : PiercedKernelIterator<id_t>(), m_storage(nullptr)
{
}

/*!
    Creates a new constant iterator pointing to the same position of the
    specified iterator.

    \param other is the iterator that will be copied
*/
template<typename value_t, typename id_t, typename value_no_cv_t>
template<typename other_value_t, typename std::enable_if<std::is_const<value_t>::value && !std::is_const<other_value_t>::value && std::is_same<other_value_t, typename std::remove_cv<value_t>::type>::value, int>::type>
PiercedStorageIterator<value_t, id_t, value_no_cv_t>::PiercedStorageIterator(const PiercedStorageIterator<other_value_t, id_t, value_no_cv_t> &other)
    : PiercedStorageIterator(other.m_storage, other.getRawIndex())
{
}

/**
* Creates a new iterator and initializes it with the position of the const
* base iterator recevied in input.
*/
template<typename value_t, typename id_t, typename value_no_cv_t>
PiercedStorageIterator<value_t, id_t, value_no_cv_t>::PiercedStorageIterator(storage_t *storage, std::size_t pos)
    : PiercedKernelIterator<id_t>(storage->getKernel(), pos), m_storage(storage)
{
}

/**
* Creates a new iterator and initializes it with the position of the const
* base iterator recevied in input.
*/
template<typename value_t, typename id_t, typename value_no_cv_t>
PiercedStorageIterator<value_t, id_t, value_no_cv_t>::PiercedStorageIterator(storage_t *storage, const bitpit::PiercedKernelIterator<id_t> &iterator)
    : PiercedKernelIterator<id_t>(iterator), m_storage(storage)
{
    assert(&(iterator.getKernel()) == &(storage->getKernel()));
}

/**
* Exchanges the values of the current iterator and the iterator recevied as
* argument.
*
* \param other the iterator to exchange values with
*/
template<typename value_t, typename id_t, typename value_no_cv_t>
void PiercedStorageIterator<value_t, id_t, value_no_cv_t>::swap(PiercedStorageIterator& other) noexcept
{
    PiercedKernelIterator<id_t>::swap();

    std::swap(m_storage, other.m_storage);
}

/*!
* Get a constant reference of the storage associated with the iterator.
*
* \result A constant reference of the storage associated with the iterator.
*/
template<typename value_t, typename id_t, typename value_no_cv_t>
typename PiercedStorageIterator<value_t, id_t, value_no_cv_t>::storage_type & PiercedStorageIterator<value_t, id_t, value_no_cv_t>::getStorage() const
{
    return *m_storage;
}

/*!
* Get a constant reference to the kernel iterator.
*
* \result A constant reference to the kernel iterator.
*/
template<typename value_t, typename id_t, typename value_no_cv_t>
const PiercedKernelIterator<id_t> & PiercedStorageIterator<value_t, id_t, value_no_cv_t>::getKernelIterator() const
{
    return static_cast<const PiercedKernelIterator<id_t> &>(*this);
}

/**
* Gets the values of the current element.
*
* \param k is the index of the requested field
* \return The values of the current element.
*/
template<typename value_t, typename id_t, typename value_no_cv_t>
__PSI_REFERENCE__ PiercedStorageIterator<value_t, id_t, value_no_cv_t>::getValue(std::size_t k) const
{
    if (*this == m_storage->getKernel().end()) {
        throw std::out_of_range("Iterator points to an invalid position.");
    }

    std::size_t rawIndex = getRawIndex();

    return m_storage->rawAt(rawIndex, k);
}

/**
* Pre-increment operator.
*
* Increment operator does not check whether it crosses the end() of the
* container. Thus, calling this function if the iterator is already at
* the end of the container results in undefined behavior.
*/
template<typename value_t, typename id_t, typename value_no_cv_t>
PiercedStorageIterator<value_t, id_t, value_no_cv_t> & PiercedStorageIterator<value_t, id_t, value_no_cv_t>::operator++()
{
    PiercedKernelIterator<id_t>::operator++();

    return *this;
}

/**
* Post-increment operator.
*
* Increment operator does not check whether it crosses the end() of the
* container. Thus, calling this function if the iterator is already at
* the end of the container results in undefined behavior.
*/
template<typename value_t, typename id_t, typename value_no_cv_t>
PiercedStorageIterator<value_t, id_t, value_no_cv_t> PiercedStorageIterator<value_t, id_t, value_no_cv_t>::operator++(int)
{
    std::size_t rawIndex = getRawIndex();
    PiercedStorageIterator tmp(m_storage, rawIndex);

    ++(*this);

    return tmp;
}

/**
* Pre-decrement operator.
*
* Decrement operator does not check whether it crosses the begin() of the
* container. Thus, calling this function if the iterator is already at
* the begin of the container results in undefined behavior.
*/
template<typename value_t, typename id_t, typename value_no_cv_t>
PiercedStorageIterator<value_t, id_t, value_no_cv_t> & PiercedStorageIterator<value_t, id_t, value_no_cv_t>::operator--()
{
    PiercedKernelIterator<id_t>::operator--();

    return *this;
}

/**
* Post-decrement operator.
*
* Decrement operator does not check whether it crosses the begin() of the
* container. Thus, calling this function if the iterator is already at
* the begin of the container results in undefined behavior.
*/
template<typename value_t, typename id_t, typename value_no_cv_t>
PiercedStorageIterator<value_t, id_t, value_no_cv_t> PiercedStorageIterator<value_t, id_t, value_no_cv_t>::operator--(int)
{
    std::size_t rawIndex = getRawIndex();
    PiercedStorageIterator tmp(m_storage, rawIndex);

    --(*this);

    return tmp;
}

/**
* Deference operator.
*
* \result A reference to the element currently pointed to by the iterator.
*/
template<typename value_t, typename id_t, typename value_no_cv_t>
__PSI_REFERENCE__ PiercedStorageIterator<value_t, id_t, value_no_cv_t>::operator*() const
{
    std::size_t rawIndex = getRawIndex();

    return m_storage->rawAt(rawIndex, 0);
}

/**
* Deference operator.
*
* \result A reference to the element currently pointed to by the iterator.
*/
template<typename value_t, typename id_t, typename value_no_cv_t>
__PSI_POINTER__ PiercedStorageIterator<value_t, id_t, value_no_cv_t>::operator->() const
{
    std::size_t rawIndex = getRawIndex();

    return m_storage->rawData(rawIndex);
}

/*!
* Copy assignment operator to create a constant iterator from a non-constant
* one.
*
* \param other is the iterator that will be copied
*/
template<typename value_t, typename id_t, typename value_no_cv_t>
template<typename other_value_t, typename std::enable_if<std::is_const<value_t>::value && !std::is_const<other_value_t>::value && std::is_same<other_value_t, typename std::remove_cv<value_t>::type>::value, int>::type>
PiercedStorageIterator<value_t, id_t, value_no_cv_t> & PiercedStorageIterator<value_t, id_t, value_no_cv_t>::operator=(const PiercedStorageIterator<other_value_t, id_t, value_no_cv_t> &other)
{
    PiercedKernelIterator<id_t>::operator=(other);
    m_storage = other.m_storage;

    return *this;
}

}

#endif
