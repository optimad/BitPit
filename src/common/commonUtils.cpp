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

#define __BITPIT_COMMON_UTILS_SRC__

#include <iostream>
#include <cmath>

#include "commonUtils.hpp"

namespace bitpit {

namespace utils {

template bool addToOrderedVector<>(const long&, std::vector<long>&, std::less<long>);
template bool addToOrderedVector<>(const unsigned long&, std::vector<unsigned long>&, std::less<unsigned long>);

template std::vector<long>::const_iterator findInOrderedVector<>(const long&, const std::vector<long>&, std::less<long>);
template std::vector<unsigned long>::const_iterator findInOrderedVector<>(const unsigned long&, const std::vector<unsigned long>&, std::less<unsigned long>);

/*!
* \ingroup common_misc
*
* Swap two values stored in the specified std::vector<bool> container.
*
* \param v is a reference to the container
* \param i is the index of the first element that will be swapped
* \param j is the index of the second element that will be swapped
*/
template<>
void swapValue(std::vector<bool> &v, std::size_t i, std::size_t j)
{
    v.swap(v[i], v[j]);
}

/*!
* \ingroup common_misc
*
* Extract n integers in the interval [0,m] without replacement.
* if n = m+1, returns a random permutation of {0, 1, 2, ..., m}
*
* \param[in] n is the number of extraction
* \param[in] m is the upper bound of extraction interval
* \param[in,out] list is a vector with size n, storing extracted values
*/
void extractWithoutReplacement(int n, int m, std::vector<int> &list)
{
    // Initialize variables
    if (n > m+1) {
        std::cout << "error" << std::endl;
        return;
    }

    // Resize input variables
    list.resize(n);

    // Initialize extraction set
    int N = m;
    std::vector<int> set(m+1, -1);
    for (int i = 0; i < m+1; i++) {
        set[i] = i;
    }

    // Extract integers without replacement
    for (int i = 0; i < n; i++) {
        int index = (int) round(((double) N) * ((double) rand())/((double) RAND_MAX));
        list[i] = set[index];
        set[index] = set[N];
        N--;
    }
}


/*!
 * \ingroup common_misc
 *
 * Count the number of digits of the given number.
 *
 * \param[in] n is number whose digits should be counted
 * \result The number of digits of the specified number.
 */
std::size_t countDigits(int n)
{
    if (n== 0) {
        return 1;
    }

    std::size_t nDigits = 0;
    while (n != 0) {
        n /= 10;
        ++nDigits;
    }

    return nDigits;
}

/*!
 * \ingroup common_misc
 *
 * Compute the factorial of the specified number.
 *
 * \param[in] n is the argument for which the factorial has to be evaluated
 * \result The factorial of the specified number.
 */
unsigned long factorial(unsigned long n)
{
    unsigned long factorial = 1;
    for (unsigned long i = 1; i <= n; ++i) {
        factorial *= i;
    }

    return factorial;
}

}

}
