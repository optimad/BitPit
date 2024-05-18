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

#ifndef __BITPIT_STENCIL_TPP__
#define __BITPIT_STENCIL_TPP__

namespace bitpit {

/*!
* The weight manager.
*/
template<typename weight_t, typename value_t>
const typename DiscreteStencil<weight_t, value_t>::weight_manager_type DiscreteStencil<weight_t, value_t>::m_weightManager = typename DiscreteStencil<weight_t>::weight_manager_type();

/*!
* Get a constant reference to the weight manager.
*
* \result A constant reference to the weight manager.
*/
template<typename weight_t, typename value_t>
const typename DiscreteStencil<weight_t, value_t>::weight_manager_type & DiscreteStencil<weight_t, value_t>::getWeightManager()
{
    return m_weightManager;
}

/*!
* Output stream operator from class DiscreteStencil to communication buffer.
*
* \param[in] buffer is the output memory stream
* \param[in] stencil is the stencil to be streamed
* \result Returns the same output stream received in input.
*/
template<typename weight_t, typename value_t>
OBinaryStream& operator<<(OBinaryStream &buffer, const DiscreteStencil<weight_t, value_t> &stencil)
{
    buffer << stencil.m_zero;

    std::size_t nItems = stencil.size();
    buffer << nItems;

    const long *patternData = stencil.patternData();
    const weight_t *weightData = stencil.weightData();
    for (std::size_t n = 0; n < nItems; ++n) {
        buffer << patternData[n];
        buffer << weightData[n];
    }

    buffer << stencil.m_constant;

    return buffer;
}

/*!
* Input stream operator from class DiscreteStencil to communication buffer.
*
* \param[in] buffer is the input memory stream
* \param[in] stencil is the stencil to be streamed
* \result Returns the same input stream received in input.
*/
template<typename weight_t, typename value_t>
IBinaryStream& operator>>(IBinaryStream &buffer, DiscreteStencil<weight_t, value_t> &stencil)
{
    buffer >> stencil.m_zero;

    std::size_t nItems;
    buffer >> nItems;

    stencil.resize(nItems);
    long *patternData = stencil.patternData();
    weight_t *weightData = stencil.weightData();
    for (std::size_t n = 0; n < nItems; ++n) {
        buffer >> patternData[n];
        buffer >> weightData[n];
    }

    buffer >> stencil.m_constant;

    return buffer;
}

/*!
* Constructor
*
* \param zero is the value to be used as zero
*/
template<typename weight_t, typename value_t>
DiscreteStencil<weight_t, value_t>::DiscreteStencil(const weight_t &zero)
    : DiscreteStencil(0, zero)
{
}

/*!
* Constructor
*
* \param size is the stencil size, expressed in number of elements
* \param zero is the value to be used as zero
*/
template<typename weight_t, typename value_t>
DiscreteStencil<weight_t, value_t>::DiscreteStencil(std::size_t size, const weight_t &zero)
    : m_zero(zero),
      m_pattern(size, -1), m_weights(size, m_zero),
      m_constant(m_zero)
{
}

/*!
* Constructor
*
* \param size is the stencil size, expressed in number of elements
* \param pattern is the patterns of the stencil
* \param zero is the value to be used as zero
*/
template<typename weight_t, typename value_t>
DiscreteStencil<weight_t, value_t>::DiscreteStencil(std::size_t size, const long *pattern, const weight_t &zero)
    : m_zero(zero),
      m_pattern(pattern, pattern + size), m_weights(size, m_zero),
      m_constant(m_zero)
{
}

/*!
* Constructor
*
* \param size is the stencil size, expressed in number of elements
* \param pattern is the patterns of the stencil
* \param weights are the weights of the stencil
* \param zero is the value to be used as zero
*/
template<typename weight_t, typename value_t>
DiscreteStencil<weight_t, value_t>::DiscreteStencil(std::size_t size, const long *pattern, const weight_t *weights, const weight_t &zero)
    : m_zero(zero),
      m_pattern(pattern, pattern + size), m_weights(weights, weights + size),
      m_constant(m_zero)
{
}

/*!
* Initialize the stencil.
*
* \param size is the stencil size, expressed in number of elements
* \param zero is the value to be used as zero
*/
template<typename weight_t, typename value_t>
void DiscreteStencil<weight_t, value_t>::initialize(std::size_t size, const weight_t &zero)
{
    m_weightManager.copy(zero, &m_zero);

    std::size_t previousSize = this->size();
    std::size_t commonSize   = std::min(previousSize, size);
    for (std::size_t n = 0; n < commonSize; ++n) {
        m_pattern[n] = -1;
        m_weightManager.copy(m_zero, m_weights.data() + n);
    }
    if (previousSize != size) {
        resize(size);
    }

    zeroConstant();
}

/*!
* Initialize the stencil.
*
* \param size is the stencil size, expressed in number of elements
* \param pattern is the patterns of the stencil
* \param zero is the value to be used as zero
*/
template<typename weight_t, typename value_t>
void DiscreteStencil<weight_t, value_t>::initialize(std::size_t size, const long *pattern, const weight_t &zero)
{
    m_weightManager.copy(zero, &m_zero);

    std::size_t previousSize = this->size();
    std::size_t commonSize   = std::min(previousSize, size);
    for (std::size_t n = 0; n < commonSize; ++n) {
        m_pattern[n] = pattern[n];
        m_weightManager.copy(m_zero, m_weights.data() + n);
    }
    if (previousSize != size) {
        resize(size);
        for (std::size_t n = previousSize; n < size; ++n) {
            m_pattern[n] = pattern[n];
        }
    }

    zeroConstant();
}

/*!
* Initialize the stencil.
*
* \param size is the stencil size, expressed in number of elements
* \param pattern is the patterns of the stencil
* \param weights are the weights of the stencil
* \param zero is the value to be used as zero
*/
template<typename weight_t, typename value_t>
void DiscreteStencil<weight_t, value_t>::initialize(std::size_t size, const long *pattern, const weight_t *weights, const weight_t &zero)
{
    m_weightManager.copy(zero, &m_zero);

    std::size_t previousSize = this->size();
    std::size_t commonSize   = std::min(previousSize, size);
    for (std::size_t n = 0; n < commonSize; ++n) {
        m_pattern[n] = pattern[n];
        m_weightManager.copy(weights[n], m_weights.data() + n);
    }
    if (size > previousSize) {
        reserve(size);
        for (std::size_t n = previousSize; n < size; ++n) {
            appendItem(pattern[n], weights[n]);
        }
    } else if (size < previousSize) {
        resize(size);
    }

    zeroConstant();
}

/*!
* Initialize the stencil.
*
* \param other is another stencil of the same time, whose items will be used
* to initialize this stencil
*/
template<typename weight_t, typename value_t>
void DiscreteStencil<weight_t, value_t>::initialize(const DiscreteStencil<weight_t, value_t> &other)
{
    initialize(other.size(), other.m_pattern.data(), other.m_weights.data(), other.m_zero);
}

/*!
* Get the total size of the stencil, expressed in number of items.
*
* \result The total size of the stencil, expressed in number of items.
*/
template<typename weight_t, typename value_t>
std::size_t DiscreteStencil<weight_t, value_t>::size() const
{
    return m_pattern.size();
}

/*!
* Resizes the container so that it contains the specified number of items.
*
* \param size is the new stencil size, expressed in number of elements
*/
template<typename weight_t, typename value_t>
void DiscreteStencil<weight_t, value_t>::resize(std::size_t size)
{
    m_pattern.resize(size, -1);
    m_weights.resize(size, m_zero);
}

/*!
* Requests a change in capacity.
*
* Requests that the stencil capacity be at least the specified value.
*
* \param capacity is the minimum number of items that the stencil should
* be able to contain
*/
template<typename weight_t, typename value_t>
void DiscreteStencil<weight_t, value_t>::reserve(std::size_t capacity)
{
    m_pattern.reserve(capacity);
    m_weights.reserve(capacity);
}

/*!
* Get a reference to the specified element of the pattern.
*
* \param pos is the position of the pattern element
* \result A reference to the specified element of the pattern.
*/
template<typename weight_t, typename value_t>
long & DiscreteStencil<weight_t, value_t>::getPattern(std::size_t pos)
{
    return m_pattern[pos];
}

/*!
* Get a constant reference to the specified element of the pattern.
*
* \param pos is the position of the pattern element
* \result A constant reference to the specified element of the pattern.
*/
template<typename weight_t, typename value_t>
const long & DiscreteStencil<weight_t, value_t>::getPattern(std::size_t pos) const
{
    return m_pattern[pos];
}

/*!
* Get a pointer to the underlying array serving as pattern storage.
*
* \result A pointer to the underlying array serving as pattern storage.
*/
template<typename weight_t, typename value_t>
long * DiscreteStencil<weight_t, value_t>::patternData()
{
    return m_pattern.data();
}

/*!
* Get a constant pointer to the underlying array serving as pattern storage.
*
* \result A constant pointer to the underlying array serving as pattern
* storage.
*/
template<typename weight_t, typename value_t>
const long * DiscreteStencil<weight_t, value_t>::patternData() const
{
    return m_pattern.data();
}

/*!
* Set the index of the specified element of the pattern.
*
* \param pos is the position of the pattern element
* \param id is the index that will be set
*/
template<typename weight_t, typename value_t>
void DiscreteStencil<weight_t, value_t>::setPattern(std::size_t pos, long id)
{
    m_pattern[pos] = id;
}

/*!
* Get a reference to the specified weight of the stencil.
*
* \param pos is the position of the weight
* \result A reference to the specified weight of the stencil.
*/
template<typename weight_t, typename value_t>
weight_t & DiscreteStencil<weight_t, value_t>::getWeight(std::size_t pos)
{
    return m_weights[pos];
}

/*!
* Get a constant reference to the specified weight of the stencil.
*
* \param pos is the position of the weight
* \result A constant reference to the specified weight of the stencil.
*/
template<typename weight_t, typename value_t>
const weight_t & DiscreteStencil<weight_t, value_t>::getWeight(std::size_t pos) const
{
    return m_weights[pos];
}

/*!
* Get a pointer to the underlying array serving as weight storage.
*
* \result A pointer to the underlying array serving as weight storage.
*/
template<typename weight_t, typename value_t>
weight_t * DiscreteStencil<weight_t, value_t>::weightData()
{
    return m_weights.data();
}

/*!
* Get a constant pointer to the underlying array serving as weight storage.
*
* \result A constant pointer to the underlying array serving as weight storage.
*/
template<typename weight_t, typename value_t>
const weight_t * DiscreteStencil<weight_t, value_t>::weightData() const
{
    return m_weights.data();
}

/*!
* Set the value of the specified weight of the stencil.
*
* \param pos is the position of the weight
* \param weight is the value that will be set
*/
template<typename weight_t, typename value_t>
void DiscreteStencil<weight_t, value_t>::setWeight(std::size_t pos, const weight_t &weight)
{
    m_weights[pos] = weight;
}

/*!
* Set the value of the specified weight of the stencil.
*
* \param pos is the position of the weight
* \param weight is the value that will be set
*/
template<typename weight_t, typename value_t>
void DiscreteStencil<weight_t, value_t>::setWeight(std::size_t pos, weight_t &&weight)
{
    m_weights[pos] = std::move(weight);
}

/*!
* Sum the given weight to the one at the specified position of the stencil.
*
* \param pos is the position of the weight
* \param weight is the value that will be summed
* \param factor is the factor by which the weight will be multiplied
*/
template<typename weight_t, typename value_t>
void DiscreteStencil<weight_t, value_t>::sumWeight(std::size_t pos, const weight_t &weight, double factor)
{
    m_weightManager.sum(weight, factor, m_weights.data() + pos);
}

/*!
* Zeros the weight at the specified position of the stencil.
*
* \param pos is the position of the weight
*/
template<typename weight_t, typename value_t>
void DiscreteStencil<weight_t, value_t>::zeroWeight(std::size_t pos)
{
    m_weightManager.copy(m_zero, m_weights.data() + pos);
}

/*!
* Set the specified item of the stencil.
*
* \param pos is the position of the weight
* \param id is the index that will be set
* \param weight is the weight that will be set
*/
template<typename weight_t, typename value_t>
void DiscreteStencil<weight_t, value_t>::setItem(std::size_t pos, long id, const weight_t &weight)
{
    setPattern(pos, id);
    setWeight(pos, weight);
}

/*!
* Set the specified item of the stencil.
*
* \param pos is the position of the weight
* \param id is the index that will be set
* \param weight is the weight that will be set
*/
template<typename weight_t, typename value_t>
void DiscreteStencil<weight_t, value_t>::setItem(std::size_t pos, long id, weight_t &&weight)
{
    setPattern(pos, id);
    setWeight(pos, std::move(weight));
}

/*!
* Sum the given weight to the item of the stencil with the specified index.
*
* If an item with the same id already exists, the given weight will be summed
* to the weight of the existing item. Otherwise, a new item will be appended.
*
* \param id is the index of the item
* \param weight is the weight that will be summed
* \param factor is the factor by which the weight will be multiplied
*/
template<typename weight_t, typename value_t>
void DiscreteStencil<weight_t, value_t>::sumItem(long id, const weight_t &weight, double factor)
{
    weight_t *target = findWeight(id);
    if (target) {
        m_weightManager.sum(weight, factor, target);
    } else {
        appendItem(id, weight);
        if (factor != 1.) {
            m_weights.back() *= factor;
        }
    }
}

/*!
* Append an item the stencil.
*
* The item will be appended to the stencil also if the stencil already
* contains an item with the same id.
*
* \param id is the index that will be set
* \param weight is the weight that will be set
*/
template<typename weight_t, typename value_t>
void DiscreteStencil<weight_t, value_t>::appendItem(long id, const weight_t &weight)
{
    m_pattern.push_back(id);
    appendWeight(weight);
}

/*!
* Append an item the stencil.
*
* The item will be appended to the stencil also if the stencil already
* contains an item with the same id.
*
* \param id is the index that will be set
* \param weight is the weight that will be set
*/
template<typename weight_t, typename value_t>
void DiscreteStencil<weight_t, value_t>::appendItem(long id, weight_t &&weight)
{
    m_pattern.push_back(id);
    appendWeight(std::move(weight));
}

/*!
* Get a constant reference to the constant associated to the stencil.
*
* \result A constant reference to the constant associated to the stencil.
*/
template<typename weight_t, typename value_t>
const weight_t & DiscreteStencil<weight_t, value_t>::getConstant() const
{
    return m_constant;
}

/*!
* Get a reference to the constant associated to the stencil.
*
* \result A reference to the constant associated to the stencil.
*/
template<typename weight_t, typename value_t>
weight_t & DiscreteStencil<weight_t, value_t>::getConstant()
{
    return m_constant;
}

/*!
* Set the value of the constant associated to the stencil.
*
* \param constant is the constant that will be set
*/
template<typename weight_t, typename value_t>
void DiscreteStencil<weight_t, value_t>::setConstant(const weight_t &constant)
{
    m_weightManager.copy(constant, &m_constant);
}

/*!
* Set the value of the constant associated to the stencil.
*
* \param constant is the constant that will be set
*/
template<typename weight_t, typename value_t>
void DiscreteStencil<weight_t, value_t>::setConstant(weight_t &&constant)
{
    m_weightManager.move(std::move(constant), &m_constant);
}

/*!
* Sum the specified constant to the constant associated to the stencil.
*
* \param constant is the constant that will be summed
* \param factor is the factor by which the constant will be multiplied
*/
template<typename weight_t, typename value_t>
void DiscreteStencil<weight_t, value_t>::sumConstant(const weight_t &constant, double factor)
{
    m_weightManager.sum(constant, factor, &m_constant);
}

/*!
* Zero the constant associated to the stencil.
*/
template<typename weight_t, typename value_t>
void DiscreteStencil<weight_t, value_t>::zeroConstant()
{
    m_weightManager.copy(m_zero, &m_constant);
}

/*!
* Clears the items of the stencil.
*
* Removes all items from the stencil (which are destroyed), leaving it
* with a size of 0.
*
* \param release if it's true the memory hold by the stencil will be
* released, otherwise the stencil will be cleared but its memory will
* not be relased
*/
template<typename weight_t, typename value_t>
void DiscreteStencil<weight_t, value_t>::clear(bool release)
{
    m_pattern.clear();
    if (release) {
        m_pattern.shrink_to_fit();
    }

    clearWeights(release);

    zeroConstant();
}

/*!
* Sum the specified stencil.
*
* \param other is the stencil that will be summed
* \param factor is the factor by which the other stencil will be multiplied
*/
template<typename weight_t, typename value_t>
void DiscreteStencil<weight_t, value_t>::sum(const DiscreteStencil<weight_t, value_t> &other, double factor)
{
    const std::size_t other_nItems = other.size();
    for (std::size_t n = 0; n < other_nItems; ++n) {
        sumItem(other.m_pattern[n], other.m_weights[n], factor);
    }

    sumConstant(other.m_constant, factor);
}

/*!
* Optimize the stencil.
*
* The negligible elements will be removed from the stencil.
*/
template<typename weight_t, typename value_t>
void DiscreteStencil<weight_t, value_t>::optimize(double tolerance)
{
    std::size_t nItems = size();
    for (std::size_t n = 0; n < nItems; ++n) {
        if (m_weightManager.isNegligible(m_weights[n], m_zero, tolerance)) {
            m_pattern.erase(m_pattern.begin() + n);
            m_weights.erase(m_weights.begin() + n);
            --n;
            --nItems;
        }
    }
}

/*!
* Renumber the indexes of the stencil according to the specified map.
*
* \param map is the renumbering map
*/
template<typename weight_t, typename value_t>
void DiscreteStencil<weight_t, value_t>::renumber(const std::unordered_map<long, long> &map)
{
    renumber([&map] (long id) -> long { return map.at(id); });
}

/*!
* Renumber the indexes of the stencil according to the specified map.
*
* \param map is the functor that will perform the mapping, the functor should have an
* operator() that take as an argument the original id and return the renumbered id.
*/
template<typename weight_t, typename value_t>
template<typename Mapper>
void DiscreteStencil<weight_t, value_t>::renumber(const Mapper &map)
{
    for (long &id : m_pattern) {
        id = map(id);
    }
}

/*!
* Add a new item that complements the stencil to zero.
*
* \param id is the index associated to the new item
*/
template<typename weight_t, typename value_t>
void DiscreteStencil<weight_t, value_t>::addComplementToZero(long id)
{
    const std::size_t nItems = size();
    if (nItems == 0) {
        return;
    }

    weight_t complement = m_zero;
    for (const weight_t &weight : m_weights) {
        m_weightManager.sum(weight, -1., &complement);
    }

    appendItem(id, complement);
}

/*!
* Set weights and constant to zero.
*/
template<typename weight_t, typename value_t>
void DiscreteStencil<weight_t, value_t>::zero()
{
    for (weight_t &weight : m_weights) {
        m_weightManager.copy(m_zero, &weight);
    }

    setConstant(m_zero);
}

/*!
* Find the weight associated to the specified id.
*
* \param id is the index associated to the weight
* \result A pointer to the weight associated to the specified id. If the weight
* is not found, a null pointer is returned.
*/
template<typename weight_t, typename value_t>
weight_t * DiscreteStencil<weight_t, value_t>::findWeight(long id)
{
    auto patternBegin = m_pattern.cbegin();
    auto patternEnd   = m_pattern.cend();
    for (auto itr = patternBegin; itr != patternEnd; ++itr) {
        if (*itr == id) {
            return (m_weights.data() + std::distance(patternBegin, itr));
        }
    }

    return nullptr;
}

/*!
* Find the weight associated to the specified id.
*
* \param id is the index associated to the weight
* \result A constant pointer to the weight associated to the specified id.
* If the weight is not found, a null pointer is returned.
*/
template<typename weight_t, typename value_t>
const weight_t * DiscreteStencil<weight_t, value_t>::findWeight(long id) const
{
    auto patternBegin = m_pattern.cbegin();
    auto patternEnd   = m_pattern.cend();
    for (auto itr = patternBegin; itr != patternEnd; ++itr) {
        if (*itr == id) {
            return (m_weights.data() + std::distance(patternBegin, itr));
        }
    }

    return nullptr;
}

/*!
* Append a weight to the stencil.
*
* \param weight is the weight that will be appended
*/
template<typename weight_t, typename value_t>
void DiscreteStencil<weight_t, value_t>::appendWeight(weight_t &&weight)
{
    m_weights.push_back(std::move(weight));
}

/*!
* Append a weight to the stencil.
*
* \param weight is the weight that will be appended
*/
template<typename weight_t, typename value_t>
void DiscreteStencil<weight_t, value_t>::appendWeight(const weight_t &weight)
{
    m_weights.push_back(weight);
}

/*!
* Clears the weights of the stencil.
*
* Removes all weights from the stencil (which are destroyed), leaving the
* weight container with a size of 0.
*
* \param release if it's true the memory hold by the weight container will be
* released, otherwise the weight container will be cleared but its memory will
* not be relased
*/
template<typename weight_t, typename value_t>
void DiscreteStencil<weight_t, value_t>::clearWeights(bool release)
{
    m_weights.clear();
    if (release) {
        m_weights.shrink_to_fit();
    }
}

/*!
* Display the stencil.
*
* \param out is the stream that will be used for the output
* \param factor is an optional factor the weights will be muliplied for
*/
template<typename weight_t, typename value_t>
void DiscreteStencil<weight_t, value_t>::display(std::ostream &out, double factor) const
{
    const std::size_t nItems = size();

    weight_t sum = m_zero;
    for (std::size_t n = 0; n < nItems; ++n) {
        long id = m_pattern[n];
        weight_t weight = factor * m_weights[n];
        out << "   id: " << id << " weight: " << weight << std::endl;
        m_weightManager.sum(weight, 1., &sum);
    }

    out << " constant : " << (factor * m_constant) << std::endl;
    out << " sum      : " << sum << std::endl;
}

/*!
* Returns the buffer size (in bytes) required to store the stencil.
*
* \result The buffer size (in bytes) required to store the stencil.
*/
template<typename weight_t, typename value_t>
size_t DiscreteStencil<weight_t, value_t>::getBinarySize() const
{
    std::size_t nItems = size();

    return (sizeof(m_zero) + nItems * (sizeof(long) + sizeof(weight_t)) + sizeof(m_constant));
}

/*!
* Get a reference to a weight associated with an item with the specified index.
*
* If there are no items with the specified index, an exception will be thrown.
* If there are multiple items associated with the specified id, the first one
* will be returned.
*
* \param id is the index associated to the item
* \return A reference to the weight associated with the item with the specified
* index.
*/
template<typename weight_t, typename value_t>
weight_t & DiscreteStencil<weight_t, value_t>::at(long id)
{
    return const_cast<weight_t &>(static_cast<const DiscreteStencil<weight_t, value_t> &>(*this).at(id));
}

/*!
* Get a constant reference to a weight associated with an item with the specified index.
*
* If there are no items with the specified index, an exception will be thrown.
* If there are multiple items associated with the specified id, the first one
* will be returned.
*
* \param id is the index associated to the item
* \return A constant reference to the weight associated with the item with the specified
* index.
*/
template<typename weight_t, typename value_t>
const weight_t & DiscreteStencil<weight_t, value_t>::at(long id) const
{
    const weight_t *weight = findWeight(id);
    if (weight) {
        return *weight;
    }

    throw("The stencil doens't contain an item with the specified id");
}

/*!
* Get a reference to a weight associated with an item with the specified index.
*
* If there are no items with the specified id, a new empty item will be added to
* the stencil and its reference will be returned. If there are multiple items
* associated with the specified id, the first one will be returned.
*
* \param id is the index associated to the item
* \return A reference to the weight associated with the item with the specified
* index.
*/
template<typename weight_t, typename value_t>
weight_t & DiscreteStencil<weight_t, value_t>::operator[](long id)
{
    weight_t *weight = findWeight(id);
    if (weight) {
        return *weight;
    }

    appendItem(id, m_zero);

    return m_weights.back();
}

/*!
* The binary multiplication assignment operator.
*
* \param factor is the factor of the multiplication
* \result A reference to the stencil
*/
template<typename weight_t, typename value_t>
DiscreteStencil<weight_t, value_t> & DiscreteStencil<weight_t, value_t>::operator*=(double factor)
{
    for (weight_t &weight : m_weights) {
        weight *= factor;
    }
    m_constant *= factor;

    return *this;
}

/*!
* The binary division assignment operator.
*
* \param factor is the factor of the division
* \result A reference to the stencil
*/
template<typename weight_t, typename value_t>
DiscreteStencil<weight_t, value_t> & DiscreteStencil<weight_t, value_t>::operator/=(double factor)
{
    for (weight_t &weight : m_weights) {
        weight /= factor;
    }
    m_constant /= factor;

    return *this;
}

/*!
* The binary sum assignment operator.
*
* \param other is the stencil that will be summed
* \result A reference to the stencil
*/
template<typename weight_t, typename value_t>
DiscreteStencil<weight_t, value_t> & DiscreteStencil<weight_t, value_t>::operator+=(const DiscreteStencil<weight_t, value_t> &other)
{
    sum(other, 1.);

    return *this;
}

/*!
* The binary subtraction assignment operator.
*
* \param other is the stencil that will be subtracted
* \result A reference to the stencil
*/
template<typename weight_t, typename value_t>
DiscreteStencil<weight_t, value_t> & DiscreteStencil<weight_t, value_t>::operator-=(const DiscreteStencil<weight_t, value_t> &other)
{
    sum(other, - 1.);

    return *this;
}

/*!
* Constructor
*
* \param zero is the value to be used as zero
*/
template<typename weight_t, typename value_t>
MPDiscreteStencil<weight_t, value_t>::MPDiscreteStencil(const weight_t &zero)
    : DiscreteStencil<weight_t, value_t>(zero),
      m_weightPool(nullptr)
{
}

/*!
* Constructor
*
* \param size is the stencil size, expressed in number of elements
* \param zero is the value to be used as zero
*/
template<typename weight_t, typename value_t>
MPDiscreteStencil<weight_t, value_t>::MPDiscreteStencil(std::size_t size, const weight_t &zero)
    : DiscreteStencil<weight_t, value_t>(size, zero),
      m_weightPool(nullptr)
{
}

/*!
* Constructor
*
* \param size is the stencil size, expressed in number of elements
* \param pattern is the patterns of the stencil
* \param zero is the value to be used as zero
*/
template<typename weight_t, typename value_t>
MPDiscreteStencil<weight_t, value_t>::MPDiscreteStencil(std::size_t size, const long *pattern, const weight_t &zero)
    : DiscreteStencil<weight_t, value_t>(size, pattern, zero),
      m_weightPool(nullptr)
{
}

/*!
* Constructor
*
* \param size is the stencil size, expressed in number of elements
* \param pattern is the patterns of the stencil
* \param weights are the weights of the stencil
* \param zero is the value to be used as zero
*/
template<typename weight_t, typename value_t>
MPDiscreteStencil<weight_t, value_t>::MPDiscreteStencil(std::size_t size, const long *pattern, const weight_t *weights, const weight_t &zero)
    : DiscreteStencil<weight_t, value_t>(size, pattern, weights, zero),
      m_weightPool(nullptr)

{
}

/*!
* Set the weight pool.
*
* \param pool is the weight pool that will be used
*/
template<typename weight_t, typename value_t>
void MPDiscreteStencil<weight_t, value_t>::setWeightPool(MPDiscreteStencil<weight_t, value_t>::weight_pool_type *pool)
{
    m_weightPool = pool;
}

/*!
* Append a weight to the stencil.
*
* \param weight is the weight that will be appended
*/
template<typename weight_t, typename value_t>
void MPDiscreteStencil<weight_t, value_t>::appendWeight(const weight_t &weight)
{
    if (m_weightPool && m_weightPool->size() > 0) {
        this->m_weights.emplace_back(m_weightPool->retrieve());
        this->m_weights.back() = weight;
    } else {
        DiscreteStencil<weight_t, value_t>::appendWeight(weight);
    }
}

/*!
* Clears the weights of the stencil.
*
* Removes all weights from the stencil (which are destroyed), leaving the
* weight container with a size of 0.
*
* \param release if it's true the memory hold by the weight container will be
* released, otherwise the weight container will be cleared but its memory will
* not be relased
*/
template<typename weight_t, typename value_t>
void MPDiscreteStencil<weight_t, value_t>::clearWeights(bool release)
{
    if (m_weightPool) {
        m_weightPool->store(&(this->m_weights));
    }

    DiscreteStencil<weight_t, value_t>::clearWeights(release);
}

}

/*!
* The multiplication operator between a stencil and a scalar value.
*
* \param stencil is the stencil
* \param factor is the factor of the multiplication
* \result The result fo the multiplication.
*/
template<typename weight_t, typename value_t>
bitpit::DiscreteStencil<weight_t, value_t> operator*(const bitpit::DiscreteStencil<weight_t, value_t> &stencil, double factor)
{
    return (factor * stencil);
}

/*!
* The multiplication operator between scalar value and a stencil.
*
* \param factor is the factor of the multiplication
* \param stencil is the stencil
* \result The result fo the multiplication.
*/
template<typename weight_t, typename value_t>
bitpit::DiscreteStencil<weight_t, value_t> operator*(double factor, const bitpit::DiscreteStencil<weight_t, value_t> &stencil)
{
    bitpit::DiscreteStencil<weight_t, value_t> stencil_result(stencil);
    stencil_result *= factor;

    return stencil_result;
}

/*!
* The division operator between a stencil and a scalar value.
*
* \param stencil is the stencil
* \param factor is the factor of the division
* \result The result fo the division.
*/
template<typename weight_t, typename value_t>
bitpit::DiscreteStencil<weight_t, value_t> operator/(const bitpit::DiscreteStencil<weight_t, value_t> &stencil, double factor)
{
    bitpit::DiscreteStencil<weight_t, value_t> stencil_result(stencil);
    stencil_result /= factor;

    return stencil_result;
}

/*!
* The sum operator between two stencils.
*
* \param stencil_A is the first stencil
* \param stencil_B is the second stencil
* \result The result fo the sum.
*/
template<typename weight_t, typename value_t>
bitpit::DiscreteStencil<weight_t, value_t> operator+(const bitpit::DiscreteStencil<weight_t, value_t> &stencil_A, const bitpit::DiscreteStencil<weight_t, value_t> &stencil_B)
{
    bitpit::DiscreteStencil<weight_t, value_t> stencil_result(stencil_A);
    stencil_result += stencil_B;

    return stencil_result;
}

/*!
* The subtraction operator between two stencils.
*
* \param stencil_A is the first stencil
* \param stencil_B is the second stencil
* \result The result fo the subtraction.
*/
template<typename weight_t, typename value_t>
bitpit::DiscreteStencil<weight_t, value_t> operator-(const bitpit::DiscreteStencil<weight_t, value_t> &stencil_A, const bitpit::DiscreteStencil<weight_t, value_t> &stencil_B)
{
    bitpit::DiscreteStencil<weight_t, value_t> stencil_result(stencil_A);
    stencil_result -= stencil_B;

    return stencil_result;
}

/*!
* The multiplication operator between a scalar stencil and a vector.
*
* \param stencil is the stencil
* \param vector is the vector
* \result The result fo the multiplication.
*/
template <typename V>
typename bitpit::DiscreteStencil<std::array<V, 3>> operator*(const typename bitpit::DiscreteStencil<V> &stencil, const std::array<V, 3> &vector)
{
    return (vector * stencil);
}

/*!
* The multiplication operator between a vector and a scalar stencil.
*
* \param vector is the vector
* \param stencil is the stencil
* \result The result fo the multiplication.
*/
template <typename V>
typename bitpit::DiscreteStencil<std::array<V, 3>> operator*(const std::array<V, 3> &vector, const typename bitpit::DiscreteStencil<V> &stencil)
{
    const std::size_t nItems = stencil.size();
    bitpit::StencilVector stencil_B;
    stencil_B.resize(nItems);
    for (std::size_t n = 0; n < nItems; ++n) {
        stencil_B.setPattern(n, stencil.getPattern(n));
        stencil_B.setWeight(n, ::operator*(stencil.getWeight(n), vector));
    }
    stencil_B.setConstant(::operator*(stencil.getConstant(), vector));

    return stencil_B;
}

/*!
* The dot procduct operator betwee a vector stencil and a vector.
*
* \param stencil is the vector stencil
* \param vector is the vector
* \result The result fo the dot product.
*/
template <typename V>
typename bitpit::DiscreteStencil<V> dotProduct(const typename bitpit::DiscreteStencil<std::array<V, 3>> &stencil, const typename bitpit::DiscreteStencil<std::array<V, 3>>::weight_type &vector)
{
    typename bitpit::DiscreteStencil<V> stencil_dotProduct;
    dotProduct(stencil, vector, &stencil_dotProduct);

    return stencil_dotProduct;
}

/*!
* The dot procduct operator betwee a vector stencil and a vector.
*
* \param stencil is the vector stencil
* \param vector is the vector
* \param[out] stencil_dotProduct on output will contain the dot product
*/
template <typename V>
void dotProduct(const typename bitpit::DiscreteStencil<std::array<V, 3>> &stencil, const typename bitpit::DiscreteStencil<std::array<V, 3>>::weight_type &vector, typename bitpit::DiscreteStencil<V> *stencil_dotProduct)
{
    const std::size_t nItems = stencil.size();
    stencil_dotProduct->resize(nItems);

    const long *patternData = stencil.patternData();
    const bitpit::StencilVector::weight_type *weightData = stencil.weightData();
    long *patternData_dotProduct = stencil_dotProduct->patternData();
    typename bitpit::DiscreteStencil<V>::weight_type *weightData_dotProduct = stencil_dotProduct->weightData();
    for (std::size_t n = 0; n < nItems; ++n) {
        patternData_dotProduct[n] = patternData[n];
        weightData_dotProduct[n]  = ::dotProduct(weightData[n], vector);
    }

    stencil_dotProduct->setConstant(::dotProduct(stencil.getConstant(), vector));
}

/*!
* Project the stencil along the specified direction.
*
* \param stencil is the vector stencil
* \param direction is the direction
* \result The projection of the stencil along the specified direction.
*/
template <typename V>
typename bitpit::DiscreteStencil<std::array<V, 3>> project(const typename bitpit::DiscreteStencil<std::array<V, 3>> &stencil, const std::array<V, 3> &direction)
{
    typename bitpit::DiscreteStencil<std::array<V, 3>> stencil_projection;
    project(stencil, direction, &stencil_projection);

    return stencil_projection;
}

/*!
* Project the stencil along the specified direction.
*
* \param stencil is the vector stencil
* \param direction is the direction
* \param[out] stencil_projection on output will contain the projection
*/
template <typename V>
void project(const typename bitpit::DiscreteStencil<std::array<V, 3>> &stencil, const std::array<V, 3> &direction, typename bitpit::DiscreteStencil<std::array<V, 3>> *stencil_projection)
{
    stencil_projection->initialize(stencil);
    const std::size_t nItems = stencil_projection->size();

    typename bitpit::DiscreteStencil<std::array<V, 3>>::weight_type *weightData_projection = stencil_projection->weightData();
    for (std::size_t n = 0; n < nItems; ++n) {
        typename bitpit::DiscreteStencil<std::array<V, 3>>::weight_type &weight = weightData_projection[n];
        weight = ::dotProduct(weight, direction) * direction;
    }

    stencil_projection->setConstant(::dotProduct(stencil_projection->getConstant(), direction) * direction);
}

#endif
