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

#ifndef __BITPIT_PIERCED_STORAGE_ITERATOR_HPP__
#define __BITPIT_PIERCED_STORAGE_ITERATOR_HPP__

#include <cassert>
#include <iterator>
#include <limits>
#include <type_traits>

#define  __PSI_REFERENCE__ typename PiercedStorageIterator<value_t, id_t, value_no_cv_t>::reference
#define  __PSI_POINTER__   typename PiercedStorageIterator<value_t, id_t, value_no_cv_t>::pointer

namespace bitpit{

template<typename value_t, typename id_t>
class PiercedStorage;

/**
* \ingroup containers
*
* \brief Iterator for the class PiercedStorage
*
* \tparam value_t is the type of elements in the storage
* \tparam id_t is the type of ids associated to the elements
*/
template<typename value_t, typename id_t = long,
         typename value_no_cv_t = typename std::remove_cv<value_t>::type>
class PiercedStorageIterator
    : protected PiercedKernelIterator<id_t>
{

friend class PiercedStorageIterator<typename std::add_const<value_t>::type, id_t, value_no_cv_t>;

template<typename PS_value_t, typename PS_id_t>
friend class PiercedStorage;

private:
    /**
    * Storage.
    */
    template<typename PS_value_t, typename PS_id_t>
    using Storage = PiercedStorage<PS_value_t, PS_id_t>;

    /**
    * Storage type
    *
    * When building a const_iterator the pointer to the storage has to be
    * declared const.
    */
    typedef
        typename std::conditional<std::is_const<value_t>::value,
            const Storage<value_no_cv_t, id_t>,
            Storage<value_no_cv_t, id_t>
        >::type

        storage_t;

public:
    /**
    * Id type
    */
    typedef id_t id_type;

    /*!
    * Iterator category
    */
    typedef std::bidirectional_iterator_tag iterator_category;

    /*!
    * Value type
    */
    typedef value_t value_type;

    /*!
    * Difference type
    */
    typedef std::ptrdiff_t difference_type;

    /*!
    * Pointer type
    */
    typedef
        typename std::conditional<std::is_const<value_t>::value,
            typename Storage<value_no_cv_t, id_t>::const_pointer,
            typename Storage<value_no_cv_t, id_t>::pointer
        >::type

        pointer;

    /*!
    * Reference type
    */
    typedef
        typename std::conditional<std::is_const<value_t>::value,
            typename Storage<value_no_cv_t, id_t>::const_reference,
            typename Storage<value_no_cv_t, id_t>::reference
        >::type

        reference;

    /*!
    *Constant container
    */
    typedef storage_t storage_type;

    // Constructors
    PiercedStorageIterator();

    template<typename other_value_t, typename std::enable_if<std::is_const<value_t>::value && !std::is_const<other_value_t>::value && std::is_same<other_value_t, typename std::remove_cv<value_t>::type>::value, int>::type = 0>
    PiercedStorageIterator(const PiercedStorageIterator<other_value_t, id_t, value_no_cv_t> &other);

    // General methods
    void swap(PiercedStorageIterator& other) noexcept;

    storage_type & getStorage() const;

    const PiercedKernelIterator<id_t> & getKernelIterator() const;

    // Methos to extract information on the current element
    using PiercedKernelIterator<id_t>::getId;
    using PiercedKernelIterator<id_t>::getRawIndex;

    __PSI_REFERENCE__ getValue(std::size_t k = 0) const;

    // Operators
    PiercedStorageIterator& operator++();
    PiercedStorageIterator operator++(int);

    PiercedStorageIterator& operator--();
    PiercedStorageIterator operator--(int);

    __PSI_REFERENCE__ operator*() const;
    __PSI_POINTER__ operator->() const;

    template<typename other_value_t, typename std::enable_if<std::is_const<value_t>::value && !std::is_const<other_value_t>::value && std::is_same<other_value_t, typename std::remove_cv<value_t>::type>::value, int>::type = 0>
    PiercedStorageIterator & operator=(const PiercedStorageIterator<other_value_t, id_t, value_no_cv_t> &other);

    /**
    * Two-way comparison.
    */
    bool operator==(const PiercedStorageIterator &rhs) const
    {
        return (PiercedKernelIterator<id_t>::operator==(rhs) && m_storage == rhs.m_storage);
    }

    /**
    * Two-way comparison.
    */
    bool operator!=(const PiercedStorageIterator &rhs) const
    {
        return (PiercedKernelIterator<id_t>::operator!=(rhs) || m_storage != rhs.m_storage);
    }

private:
    /**
    * Internal pointer to the storage.
    */
    storage_t *m_storage;

    // Constructors
    explicit PiercedStorageIterator(storage_t *storage, std::size_t pos);
    explicit PiercedStorageIterator(storage_t *storage, const bitpit::PiercedKernelIterator<id_t> &iterator);

};

}

// Include the implementation
#include "piercedStorageIterator.tpp"

#endif
