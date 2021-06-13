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

#ifndef __BITPIT_PROXY_VECTOR_TPP__
#define __BITPIT_PROXY_VECTOR_TPP__

namespace bitpit {

/*!
    Constructor
*/
template<typename value_t, typename value_no_cv_t>
ProxyVectorIterator<value_t, value_no_cv_t>::ProxyVectorIterator()
    : m_position(nullptr)
{
}

/*!
    Constructor

    This constructor allows to generate a constant iterator from a non
    constnat iterator.

    \param other is the iterator that will be copied
*/
template<typename value_t, typename value_no_cv_t>
template<typename other_value_t, typename std::enable_if<std::is_const<value_t>::value && !std::is_const<other_value_t>::value && std::is_same<other_value_t, typename std::remove_cv<value_t>::type>::value, int>::type>
ProxyVectorIterator<value_t, value_no_cv_t>::ProxyVectorIterator(const ProxyVectorIterator<other_value_t> &other)
    : m_position(other.m_position)
{
}

/*!
    Exchanges the values of the current iterator and
    the iterator recevied as argument.

    \param other is the iterator to exchange values with
*/
template<typename value_t, typename value_no_cv_t>
void ProxyVectorIterator<value_t, value_no_cv_t>::swap(ProxyVectorIterator& other) noexcept
{
    std::swap(m_position, other.m_position);
}

/*!
    Pre-increment operator.
*/
template<typename value_t, typename value_no_cv_t>
ProxyVectorIterator<value_t, value_no_cv_t> & ProxyVectorIterator<value_t, value_no_cv_t>::operator++()
{
    m_position++;

    return *this;
}

/*!
    Post-increment operator.
*/
template<typename value_t, typename value_no_cv_t>
ProxyVectorIterator<value_t, value_no_cv_t> ProxyVectorIterator<value_t, value_no_cv_t>::operator++(int)
{
    ProxyVectorIterator tmp(m_position);

    ++(*this);

    return tmp;
}

/*!
    Pre-decrement operator.
*/
template<typename value_t, typename value_no_cv_t>
ProxyVectorIterator<value_t, value_no_cv_t> & ProxyVectorIterator<value_t, value_no_cv_t>::operator--()
{
    m_position--;

    return *this;
}

/*!
    Post-decrement operator.
*/
template<typename value_t, typename value_no_cv_t>
ProxyVectorIterator<value_t, value_no_cv_t> ProxyVectorIterator<value_t, value_no_cv_t>::operator--(int)
{
    ProxyVectorIterator tmp(m_position);

    --(*this);

    return tmp;
}

/*!
    Compound assigment operator.

    \param increment is the increment
*/
template<typename value_t, typename value_no_cv_t>
ProxyVectorIterator<value_t, value_no_cv_t>& ProxyVectorIterator<value_t, value_no_cv_t>::operator+=(int increment)
{
    m_position += increment;

    return *this;
}

/*!
    Deference operator.

    \result A reference to the element currently pointed to by the iterator.
*/
template<typename value_t, typename value_no_cv_t>
__PXI_REFERENCE__ ProxyVectorIterator<value_t, value_no_cv_t>::operator*() const
{
    return *m_position;
}

/*!
    Deference operator.

    \result A reference to the element currently pointed to by the iterator.
*/
template<typename value_t, typename value_no_cv_t>
__PXI_POINTER__ ProxyVectorIterator<value_t, value_no_cv_t>::operator->() const
{
    return m_position;
}

/*!
* Copy assignment operator to create a constant iterator from a non-constant
* one.
*
* \param other is the iterator that will be copied
*/
template<typename value_t, typename value_no_cv_t>
template<typename other_value_t, typename std::enable_if<std::is_const<value_t>::value && !std::is_const<other_value_t>::value && std::is_same<other_value_t, typename std::remove_cv<value_t>::type>::value, int>::type>
ProxyVectorIterator<value_t, value_no_cv_t> & ProxyVectorIterator<value_t, value_no_cv_t>::operator=(const ProxyVectorIterator<other_value_t> &other)
{
    m_position = other.m_position;

    return *this;
}

/*!
    Creates a new iterator and initializes it with the position of
    the const base iterator recevied in input.
*/
template<typename value_t, typename value_no_cv_t>
ProxyVectorIterator<value_t, value_no_cv_t>::ProxyVectorIterator(__PXI_POINTER__ position)
    : m_position(position)
{
}

/*!
    Distance operator.

    \param other is the iterator from which the distance will be evaluated
    \result The distance between the specified iterator.
*/
template<typename value_t, typename value_no_cv_t>
std::size_t ProxyVectorIterator<value_t, value_no_cv_t>::operator-(const ProxyVectorIterator &other)
{
    return (m_position - other.m_position);
}

/*!
    Constructor.

    \param size is the size of the storage expressed in number of elements.
*/
template<typename value_t, typename pointer_t, typename const_pointer_t>
ProxyVectorDummyStorage<value_t, pointer_t, const_pointer_t>::ProxyVectorDummyStorage(std::size_t size)
{
    BITPIT_UNUSED(size);
}

/*!
    Swaps the contents.

    \param other is another storage of the same type
*/
template<typename value_t, typename pointer_t, typename const_pointer_t>
void ProxyVectorDummyStorage<value_t, pointer_t, const_pointer_t>::swap(ProxyVectorDummyStorage &other) noexcept
{
    BITPIT_UNUSED(other);
}

/*!
    Return a pointer to the data.

    \result A pointer to the data.
*/
template<typename value_t, typename pointer_t, typename const_pointer_t>
typename ProxyVectorDummyStorage<value_t, pointer_t, const_pointer_t>::pointer ProxyVectorDummyStorage<value_t, pointer_t, const_pointer_t>::data()
{
    return nullptr;
}

/*!
    Return a constant pointer to the data.

    \result A constant pointer to the data.
*/
template<typename value_t, typename pointer_t, typename const_pointer_t>
typename ProxyVectorDummyStorage<value_t, pointer_t, const_pointer_t>::const_pointer ProxyVectorDummyStorage<value_t, pointer_t, const_pointer_t>::data() const
{
    return nullptr;
}
/*!
    Check if the storage is empty.

    \result Returns true if the storage is empty, false otherwise.
*/
template<typename value_t, typename pointer_t, typename const_pointer_t>
bool ProxyVectorDummyStorage<value_t, pointer_t, const_pointer_t>::empty() const
{
    return true;
}

/*!
    Get the size of the storage expressed in number of elements.

    \result The size of the storage, expressed in number of elements.
*/
template<typename value_t, typename pointer_t, typename const_pointer_t>
std::size_t ProxyVectorDummyStorage<value_t, pointer_t, const_pointer_t>::size() const
{
    return 0;
}

/*!
    Resize the storage.

    \result The size of the storage expressed in number of elements.
*/
template<typename value_t, typename pointer_t, typename const_pointer_t>
void ProxyVectorDummyStorage<value_t, pointer_t, const_pointer_t>::resize(std::size_t size)
{
    BITPIT_UNUSED(size);
}

/*!
    Memory pool
*/
template<typename value_t, typename pointer_t, typename const_pointer_t>
std::vector<std::unique_ptr<std::vector<value_t>>> ProxyVectorStorage<value_t, pointer_t, const_pointer_t>::m_storagePool = std::vector<std::unique_ptr<std::vector<value_t>>>();

/*!
    Create a storage.

    \param size is the size of the storage, expressed in number of elements.
*/
template<typename value_t, typename pointer_t, typename const_pointer_t>
std::unique_ptr<std::vector<value_t>> ProxyVectorStorage<value_t, pointer_t, const_pointer_t>::createStorage(std::size_t size)
{
    if (size == 0) {
        return std::unique_ptr<std::vector<value_t>>(nullptr);
    }

    if (!m_storagePool.empty()) {
        std::unique_ptr<std::vector<value_t>> storage = std::move(m_storagePool.back());
        if (storage->size() < size) {
            storage->resize(size);
        }

        m_storagePool.resize(m_storagePool.size() - 1);

        return storage;
    } else {
        return std::unique_ptr<std::vector<value_t>>(new std::vector<value_t>(size));
    }
}

/*!
    Create a storage.

    \param source is the storage whose content will be copied in newly created
    storage
*/
template<typename value_t, typename pointer_t, typename const_pointer_t>
std::unique_ptr<std::vector<value_t>> ProxyVectorStorage<value_t, pointer_t, const_pointer_t>::createStorage(const std::unique_ptr<std::vector<value_t>> &source)
{
    if (!source || source->empty()) {
        return std::unique_ptr<std::vector<value_t>>(nullptr);
    }

    if (!m_storagePool.empty()) {
        std::unique_ptr<std::vector<value_t>> storage = createStorage(source->size());
        std::copy_n(source->data(), source->size(), storage->data());

        return storage;
    } else {
        return std::unique_ptr<std::vector<value_t>>(new std::vector<value_t>(*source));
    }
}

/*!
    Delete a storage.
*/
template<typename value_t, typename pointer_t, typename const_pointer_t>
void ProxyVectorStorage<value_t, pointer_t, const_pointer_t>::destroyStorage(std::unique_ptr<std::vector<value_t>> *storage)
{
    if (!(*storage)) {
        return;
    }

    if (m_storagePool.size() < MEMORY_POOL_VECTOR_COUNT) {
        if ((*storage)->size() > MEMORY_POOL_MAX_CAPACITY) {
            (*storage)->resize(MEMORY_POOL_MAX_CAPACITY);
        }

        m_storagePool.emplace_back(std::move(*storage));
        storage->reset();
    }
}

/*!
    Constructor.

    \param size is the size of the storage expressed in number of elements.
*/
template<typename value_t, typename pointer_t, typename const_pointer_t>
ProxyVectorStorage<value_t, pointer_t, const_pointer_t>::ProxyVectorStorage(std::size_t size)
    : m_storage(createStorage(size))
{
}

/*!
    Copy constructor.

    \param x is another storage of the same type (i.e., instantiated with
    the same template parameters) whose content is copied in this container.
*/
template<typename value_t, typename pointer_t, typename const_pointer_t>
ProxyVectorStorage<value_t, pointer_t, const_pointer_t>::ProxyVectorStorage(const ProxyVectorStorage<value_t, pointer_t, const_pointer_t> &other)
    : m_storage(createStorage(other.m_storage))
{
}

/*!
    Destructor.
*/
template<typename value_t, typename pointer_t, typename const_pointer_t>
ProxyVectorStorage<value_t, pointer_t, const_pointer_t>::~ProxyVectorStorage()
{
    destroyStorage(&m_storage);
}

/*!
    Swaps the contents.

    \param other is another storage of the same type
*/
template<typename value_t, typename pointer_t, typename const_pointer_t>
void ProxyVectorStorage<value_t, pointer_t, const_pointer_t>::swap(ProxyVectorStorage &other) noexcept
{
    m_storage.swap(other.m_storage);
}

/*!
    Return a pointer to the data.

    \result A pointer to the data.
*/
template<typename value_t, typename pointer_t, typename const_pointer_t>
typename ProxyVectorStorage<value_t, pointer_t, const_pointer_t>::pointer ProxyVectorStorage<value_t, pointer_t, const_pointer_t>::data()
{
    if (empty()) {
        return nullptr;
    }

    return m_storage->data();
}

/*!
    Return a constant pointer to the data.

    \result A constant pointer to the data.
*/
template<typename value_t, typename pointer_t, typename const_pointer_t>
typename ProxyVectorStorage<value_t, pointer_t, const_pointer_t>::const_pointer ProxyVectorStorage<value_t, pointer_t, const_pointer_t>::data() const
{
    if (empty()) {
        return nullptr;
    }

    return m_storage->data();
}

/*!
    Check if the storage is empty.

    \result Returns true if the storage is empty, false otherwise.
*/
template<typename value_t, typename pointer_t, typename const_pointer_t>
bool ProxyVectorStorage<value_t, pointer_t, const_pointer_t>::empty() const
{
    if (!m_storage) {
        return true;
    }

    return m_storage->empty();
}

/*!
    Get the size of the storage expressed in number of elements.

    \result The size of the storage, expressed in number of elements.
*/
template<typename value_t, typename pointer_t, typename const_pointer_t>
std::size_t ProxyVectorStorage<value_t, pointer_t, const_pointer_t>::size() const
{
    if (!m_storage) {
        return 0;
    }

    return m_storage->size();
}

/*!
    Resize the storage.

    \result The size of the storage expressed in number of elements.
*/
template<typename value_t, typename pointer_t, typename const_pointer_t>
void ProxyVectorStorage<value_t, pointer_t, const_pointer_t>::resize(std::size_t size)
{
    if (size == 0) {
        destroyStorage(&m_storage);
        return;

    }

    if (m_storage) {
        m_storage->resize(size);
    } else {
        m_storage = createStorage(size);
    }
}

/*!
    Constructor
*/
template<typename value_t>
ProxyVector<value_t>::ProxyVector()
    : m_size(0), m_data(nullptr)
{
}

/*!
    Constructor

    The container will create an internal storage that later can be filled
    with data. This is allowed because the container points to constant data,
    i.e., the container is not allowed to change the data it points to. Having
    the data stored internallt or pointing to external data doesn't change the
    behaviour of the container: in both cases it acts as a proxy to some
    constant data.

    \param size is the number elements contained in the data
*/
template<typename value_t>
template<typename other_value_t, typename std::enable_if<std::is_const<other_value_t>::value, int>::type>
ProxyVector<value_t>::ProxyVector(std::size_t size)
    : ProxyVector<value_t>(INTERNAL_STORAGE, size, size)
{
}

/*!
    Constructor

    The container will create an internal storage that later can be filled
    with data. This is allowed because the container points to constant data,
    i.e., the container is not allowed to change the data it points to. Having
    the data stored internallt or pointing to external data doesn't change the
    behaviour of the container: in both cases it acts as a proxy to some
    constant data.

    \param size is the number elements contained in the data
    \param capacity is the size of the internal storage space expressed in
    number of elements, the capacity of the container cannot be smaller than
    the size of the data, if a smaller capacity is specified the storage will
    be resized using data size
*/
template<typename value_t>
template<typename other_value_t, typename std::enable_if<std::is_const<other_value_t>::value, int>::type>
ProxyVector<value_t>::ProxyVector(std::size_t size, std::size_t capacity)
    : ProxyVector<value_t>::ProxyVector(INTERNAL_STORAGE, size, capacity)
{
}

/*!
    Constructor

    The container will create an internal storage that later can be filled
    with data. This is allowed because the container points to constant data,
    i.e., the container is not allowed to change the data it points to. Having
    the data stored internallt or pointing to external data doesn't change the
    behaviour of the container: in both cases it acts as a proxy to some
    constant data.

    \param data a pointer to the data
    \param size is the number elements contained in the data
*/
template<typename value_t>
template<typename other_value_t, typename std::enable_if<std::is_const<other_value_t>::value, int>::type>
ProxyVector<value_t>::ProxyVector(__PXV_POINTER__ data, std::size_t size)
    : ProxyVector<value_t>::ProxyVector(data, size, (data != INTERNAL_STORAGE) ? 0 : size)
{
}

/*!
    Constructor

    If data is set to INTERNAL_STORAGE, the container will create an internal
    storage that later can be filled with data. This is allowed because the
    container points to constant data, i.e., the container is not allowed to
    change the data it points to. Having the data stored internallt or pointing
    to external data doesn't change the behaviour of the container: in both
    cases it acts as a proxy to some constant data.

    \param data a pointer to the data
    \param size is the number elements contained in the data
    \param capacity is the size of the internal storage space expressed in
    number of elements, the capacity of the container cannot be smaller than
    the size of the data, if a smaller capacity is specified the storage will
    be resized using data size
*/
template<typename value_t>
template<typename other_value_t, typename std::enable_if<std::is_const<other_value_t>::value, int>::type>
ProxyVector<value_t>::ProxyVector(__PXV_POINTER__ data, std::size_t size, std::size_t capacity)
    : m_storage(capacity), m_size(size), m_data((data != INTERNAL_STORAGE) ? data : m_storage.data())
{
}

/*!
    Constructor

    Containers that point to non-constant data cannot use the internal storage.
    This guarantees that all the pointers returned by the container are always
    pointing to the original data (i.e., the container acts as a proxy to the
    original data).

    \param data a pointer to the data
    \param size is the number elements contained in the data
*/
template<typename value_t>
template<typename other_value_t, typename std::enable_if<!std::is_const<other_value_t>::value, int>::type>
ProxyVector<value_t>::ProxyVector(__PXV_POINTER__ data, std::size_t size)
    : m_storage(0), m_size(size), m_data(data)
{
    assert(data != INTERNAL_STORAGE);
}

/*!
    Copy constructor.
*/
template<typename value_t>
ProxyVector<value_t>::ProxyVector(const ProxyVector &other)
    : m_storage(other.m_storage), m_size(other.m_size), m_data(other.storedData() ? m_storage.data() : other.m_data)
{
}

/*!
    Copy assignment operator.

    Assigns new contents to the container, replacing its current contents,
    and modifying its size accordingly.
*/
template<typename value_t>
ProxyVector<value_t> & ProxyVector<value_t>::operator=(const ProxyVector &other)
{
    if (this != &other) {
        ProxyVector temporary(other);
        temporary.swap(*this);
    }

    return *this;
}

/*!
    Sets the content of the container.

    If data is set to INTERNAL_STORAGE, the container will create an internal
    storage that later can be filled with data. This is allowed because the
    container points to constant data, i.e., the container is not allowed to
    change the data it points to. Having the data stored internallt or pointing
    to external data doesn't change the behaviour of the container: in both
    cases it acts as a proxy to some constant data.

    \param data a pointer to the data, if the value INTERNAL_STORAGE is
    specified, the proxy will point to the data contained in the internal
    storage
    \param size is the number elements contained in the data
*/
template<typename value_t>
template<typename other_value_t, typename std::enable_if<std::is_const<other_value_t>::value, int>::type>
void ProxyVector<value_t>::set(__PXV_POINTER__ data, std::size_t size)
{
    std::size_t capacity;
    if (data != INTERNAL_STORAGE) {
        capacity = 0;
    } else {
        capacity = size;
    }
    set(data, size, capacity);
}

/*!
    Sets the content of the container.

    If data is set to INTERNAL_STORAGE, the container will create an internal
    storage that later can be filled with data. This is allowed because the
    container points to constant data, i.e., the container is not allowed to
    change the data it points to. Having the data stored internallt or pointing
    to external data doesn't change the behaviour of the container: in both
    cases it acts as a proxy to some constant data.

    \param data a pointer to the data, if the value INTERNAL_STORAGE is
    specified, the proxy will point to the data contained in the internal
    storage
    \param size is the number elements contained in the data
    \param capacity is the number elements the internal contained should be
    able to contain, the capacity of the container cannot be smaller thatn
    the size of the data, if a smaller capacity is specified the storage
    will be resize using data size
*/
template<typename value_t>
template<typename other_value_t, typename std::enable_if<std::is_const<other_value_t>::value, int>::type>
void ProxyVector<value_t>::set(__PXV_POINTER__ data, std::size_t size, std::size_t capacity)
{
    m_storage.resize(std::max(size, capacity));

    m_size = size;
    if (data == INTERNAL_STORAGE) {
        if (m_size == 0) {
            m_data = nullptr;
        } else {
            m_data = m_storage.data();
        }
    } else {
        m_data = data;
    }
}

/*!
    Sets the content of the container.

    Containers that point to non-constant data cannot use the internal storage.
    This guarantees that all the pointers returned by the container are always
    pointing to the original data (i.e., the container acts as a proxy to the
    original data).

    \param data a pointer to the data, containers that point to non-constant
    data cannot use the internal storage, hence the value INTERNAL_STORAGE in
    not allowed
    \param size is the number elements contained in the data
*/
template<typename value_t>
template<typename other_value_t, typename std::enable_if<!std::is_const<other_value_t>::value, int>::type>
void ProxyVector<value_t>::set(__PXV_POINTER__ data, std::size_t size)
{
    assert(data != INTERNAL_STORAGE);

    m_storage.resize(0);

    m_size = size;
    m_data = data;
}

/*!
    Returns a direct pointer to the memory of the internal storage.

    If the container is not using the internal storage, a null pointer is
    returned.

    \result A a direct pointer to the memory of the internal storage.
*/
template<typename value_t>
__PXV_STORAGE_POINTER__ ProxyVector<value_t>::storedData() noexcept
{
    __PXV_STORAGE_POINTER__ internalData = m_storage.data();
    if (!internalData) {
        return nullptr;
    } else if (internalData != m_data) {
        return nullptr;
    }

    return internalData;
}

/*!
    Returns a constant direct pointer to the memory of the internal storage.

    If the container is not using the internal storage, a null pointer is
    returned.

    \result A a direct pointer to the memory of the internal storage.
*/
template<typename value_t>
__PXV_STORAGE_CONST_POINTER__ ProxyVector<value_t>::storedData() const noexcept
{
    __PXV_STORAGE_CONST_POINTER__ internalData = m_storage.data();
    if (!internalData) {
        return nullptr;
    } else if (internalData != m_data) {
        return nullptr;
    }

    return internalData;
}

/*!
    Swaps the content.

    \param other is another container of the same type
*/
template<typename value_t>
void ProxyVector<value_t>::swap(ProxyVector &other)
{
    std::swap(m_size, other.m_size);
    std::swap(m_data, other.m_data);

    m_storage.swap(other.m_storage);
}

/*!
    Tests whether two containers are equal.

    \result true if the containers are equal, false otherwise.
*/
template<typename value_t>
bool ProxyVector<value_t>::operator==(const ProxyVector& other) const
{
    if (m_size != other.m_size) {
        return false;
    }

    if (!storedData() && !other.storedData()) {
        if (m_data != other.m_data) {
            return false;
        }
    }

    for (std::size_t i = 0; i < m_size; ++i) {
        if (m_data[i] != other.m_data[i]) {
            return false;
        }
    }

    return true;
}

/*!
    Tests whether the container is empty.

    \result true if the container size is 0, false otherwise.
*/
template<typename value_t>
bool ProxyVector<value_t>::empty() const
{
    return size() == 0;
}

/*!
    Returns the number of elements in the container

    \result The number of elements in the container.
*/
template<typename value_t>
std::size_t ProxyVector<value_t>::size() const
{
    return m_size;
}

/*!
    Returns a direct pointer to the memory where the elments are stored.

    \result A direct pointer to the memory where the elments are stored.
*/
template<typename value_t>
template<typename other_value_t, typename std::enable_if<!std::is_const<other_value_t>::value, int>::type>
__PXV_POINTER__ ProxyVector<value_t>::data() noexcept
{
    return m_data;
}

/*!
    Returns a direct constant pointer to the memory where the elments are
    stored.

    \result A direct constant pointer to the memory where the elments are
    stored.
*/
template<typename value_t>
__PXV_CONST_POINTER__ ProxyVector<value_t>::data() const noexcept
{
    return m_data;
}

/*!
    Returns a reference to the specified element.

    \param n is the position of the requested element
    \result A reference to the specified element.
*/
template<typename value_t>
template<typename other_value_t, typename std::enable_if<!std::is_const<other_value_t>::value, int>::type>
__PXV_REFERENCE__ ProxyVector<value_t>::operator[](std::size_t n)
{
    return m_data[n];
}

/*!
    Returns a constant reference to the specified element.

    \param n is the position of the requested element
    \result A constant reference to the specified element.
*/
template<typename value_t>
__PXV_CONST_REFERENCE__ ProxyVector<value_t>::operator[](std::size_t n) const
{
    return m_data[n];
}

/*!
    Returns a reference to the specified element.

    \param n is the position of the requested element
    \result A reference to the specified element.
*/
template<typename value_t>
template<typename other_value_t, typename std::enable_if<!std::is_const<other_value_t>::value, int>::type>
__PXV_REFERENCE__ ProxyVector<value_t>::at(std::size_t n)
{
    return m_data[n];
}

/*!
    Returns a constant reference to the specified element.

    \param n is the position of the requested element
    \result A constant reference to the specified element.
*/
template<typename value_t>
__PXV_CONST_REFERENCE__ ProxyVector<value_t>::at(std::size_t n) const
{
    return m_data[n];
}

/*!
    Gets a reference to the first element in the container.

    \result A reference to the first element in the container.
*/
template<typename value_t>
template<typename other_value_t, typename std::enable_if<!std::is_const<other_value_t>::value, int>::type>
__PXV_REFERENCE__ ProxyVector<value_t>::front()
{
    return m_data[0];
}

/*!
    Gets a constant reference to the first element in the container.

    \result A constant reference to the first element in the container.
*/
template<typename value_t>
__PXV_CONST_REFERENCE__ ProxyVector<value_t>::front() const
{
    return m_data[0];
}

/*!
    Gets a reference to the last element in the container.

    \result A reference to the last element in the container.
*/
template<typename value_t>
template<typename other_value_t, typename std::enable_if<!std::is_const<other_value_t>::value, int>::type>
__PXV_REFERENCE__ ProxyVector<value_t>::back()
{
    return m_data[m_size - 1];
}

/*!
    Gets a constant reference to the last element in the container.

    \result A constant reference to the last element in the container.
*/
template<typename value_t>
__PXV_CONST_REFERENCE__ ProxyVector<value_t>::back() const
{
    return m_data[m_size - 1];
}

/*!
    Returns an iterator pointing to the first element in the container.

    \result An iterator pointing to the first element in the container.
*/
template<typename value_t>
template<typename other_value_t, typename std::enable_if<!std::is_const<other_value_t>::value, int>::type>
__PXV_ITERATOR__ ProxyVector<value_t>::begin()
{
    return iterator(m_data);
}

/*!
    Returns a constant iterator pointing to the first element in the container.

    \result A constant iterator pointing to the first element in the container.
*/
template<typename value_t>
__PXV_CONST_ITERATOR__ ProxyVector<value_t>::begin() const
{
    return const_iterator(m_data);
}

/*!
    Returns an iterator referring to the past-the-end element in the container.

    \result An iterator referring to the past-the-end element in the container.
*/
template<typename value_t>
template<typename other_value_t, typename std::enable_if<!std::is_const<other_value_t>::value, int>::type>
__PXV_ITERATOR__ ProxyVector<value_t>::end()
{
    return iterator(m_data + m_size);
}

/*!
    Returns a constant iterator referring to the past-the-end element in the
    container.

    \result A constant iterator referring to the past-the-end element in the
    container.
*/
template<typename value_t>
__PXV_CONST_ITERATOR__ ProxyVector<value_t>::end() const
{
    return const_iterator(m_data + m_size);
}

/*!
    Returns a constant iterator pointing to the first element in the container.

    \result A constant iterator pointing to the first element in the container.
*/
template<typename value_t>
__PXV_CONST_ITERATOR__ ProxyVector<value_t>::cbegin()
{
    return const_iterator(m_data);
}

/*!
    Returns a constant iterator referring to the past-the-end element in the
    container.

    \result A constant iterator referring to the past-the-end element in the
    container.
*/
template<typename value_t>
__PXV_CONST_ITERATOR__ ProxyVector<value_t>::cend()
{
    return const_iterator(m_data + m_size);
}

}

#endif
