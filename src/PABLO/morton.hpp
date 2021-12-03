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

#ifndef __BITPIT_PABLO_MORTON_HPP__
#define __BITPIT_PABLO_MORTON_HPP__

#include <cstdint>
#include <limits>

namespace bitpit {

namespace PABLO {

const uint64_t INVALID_MORTON = std::numeric_limits<uint64_t>::max();

/**
* Seperate bits from a given integer 3 positions apart.
*
* The function comes from libmorton (see https://github.com/Forceflow/libmorton).
*
* \param a is an integer position
* \result Separated bits.
*/
inline uint64_t splitBy3(uint32_t a)
{
    uint64_t x = a & 0x1fffff; // we only look at the first 21 bits
    x = (x | x << 32) & 0x001F00000000FFFF; // shift left 32 bits, OR with self, and 0000000000011111000000000000000000000000000000001111111111111111
    x = (x | x << 16) & 0x001F0000FF0000FF; // shift left 16 bits, OR with self, and 0000000000011111000000000000000011111111000000000000000011111111
    x = (x | x <<  8) & 0x100F00F00F00F00F; // shift left  8 bits, OR with self, and 0001000000001111000000001111000000001111000000001111000000000000
    x = (x | x <<  4) & 0x10C30C30C30C30C3; // shift left  4 bits, OR with self, and 0001000011000011000011000011000011000011000011000011000100000000
    x = (x | x <<  2) & 0x1249249249249249; // shift left  2 bits, OR with self, and 0001001001001001001001001001001001001001001001001001001001001001

    return x;
}

/**
* Seperate bits from a given integer 2 positions apart.
*
* The function comes from libmorton (see https://github.com/Forceflow/libmorton).
*
* \param a is an integer position
* \result Separated bits.
*/
inline uint64_t splitBy2(uint32_t a)
{
    uint64_t x = a;
    x = (x | x << 16) & 0x0000FFFF0000FFFF; // shift left 16 bits, OR with self, and 0000000000000000111111111111111100000000000000001111111111111111
    x = (x | x <<  8) & 0x00FF00FF00FF00FF; // shift left  8 bits, OR with self, and 0000000011111111000000001111111100000000111111110000000011111111
    x = (x | x <<  4) & 0x0F0F0F0F0F0F0F0F; // shift left  4 bits, OR with self, and 0000111100001111000011110000111100001111000011110000111100001111
    x = (x | x <<  2) & 0x3333333333333333; // shift left  2 bits, OR with self, and 0011001100110011001100110011001100110011001100110011001100110011
    x = (x | x <<  1) & 0x5555555555555555; // shift left  1 bits, OR with self, and 0101010101010101010101010101010101010101010101010101010101010101

    return x;
}

/**
* Get the third bits of the given Morton number.
*
* The function comes from libmorton (see https://github.com/Forceflow/libmorton).
*
* \param morton is the morton number
* \result The third bit of the given Morton number.
*/
inline uint32_t getThirdBits(uint64_t morton)
{
    uint64_t x = morton & 0x1249249249249249;
    x = (x ^ (x >>  2)) & 0x10C30C30C30C30C3;
    x = (x ^ (x >>  4)) & 0x100F00F00F00F00F;
    x = (x ^ (x >>  8)) & 0x001F0000FF0000FF;
    x = (x ^ (x >> 16)) & 0x001F00000000FFFF;
    x = (x ^ (x >> 32)) & 0x00000000001FFFFF;

    return static_cast<uint32_t>(x);
}

/**
* Get the second bits of the given Morton number.
*
* The function comes from libmorton (see https://github.com/Forceflow/libmorton).
*
* \param morton is the morton number
* \result The third bit of the given Morton number.
*/
inline uint32_t getSecondBits(uint64_t morton)
{
    uint64_t x = morton & 0x5555555555555555;
    x = (x ^ (x >>  2)) & 0x3333333333333333;
    x = (x ^ (x >>  4)) & 0x0F0F0F0F0F0F0F0F;
    x = (x ^ (x >>  8)) & 0x00FF00FF00FF00FF;
    x = (x ^ (x >> 16)) & 0x0000FFFF0000FFFF;

    return static_cast<uint32_t>(x);
}

/**
* Compute the Morton number of the given set of coordinates.
*
* The function uses the "magic bits" algorithm of the libmorton library
* (see https://github.com/Forceflow/libmorton).
*
* \param x is the integer x position
* \param y is the integer y position
* \param z is the integer z position
* \result The Morton number.
*/
inline uint64_t computeMorton(uint32_t x, uint32_t y, uint32_t z)
{
    uint64_t morton = splitBy3(x) | (splitBy3(y) << 1) | (splitBy3(z) << 2);

    return morton;
}

/**
* Compute the Morton number of the given set of coordinates.
*
* The function uses the "magic bits" algorithm of the libmorton library
* (see https://github.com/Forceflow/libmorton).
*
* \param x is the integer x position
* \param y is the integer y position
* \result The Morton number.
*/
inline uint64_t computeMorton(uint32_t x, uint32_t y)
{
    uint64_t morton = splitBy2(x) | (splitBy2(y) << 1);

    return morton;
}

/**
* Compute the specified coordinate value from the given Morton number.
*
* The function uses the "magic bits" algorithm of the libmorton library
* (see https://github.com/Forceflow/libmorton).
*
* \param morton is the morton number
* \param coord is the coordinate that will be computed
* \result The coordinate value.
*/
inline uint32_t computeCoordinate3D(uint64_t morton, int coord)
{
    return getThirdBits(morton >> coord);
}

/**
* Compute the specified coordinate value from the given Morton number.
*
* The function uses the "magic bits" algorithm of the libmorton library
* (see https://github.com/Forceflow/libmorton).
*
* \param morton is the morton number
* \param coord is the coordinate that will be computed
* \result The coordinate value.
*/
inline uint32_t computeCoordinate2D(uint64_t morton, int coord)
{
    return getSecondBits(morton >> coord);
}

/**
* Compute the XYZ key of the given set of coordinates.
*
* The XYZ key combines three 32bit coordinates and generates a unique 64bit
* value (this is possible because with a 64bit-wide Morton number not all
* 32 bits of the coordinates are used).
*
* \param x is the integer x position
* \param y is the integer y position
* \param z is the integer z position
* \result The unique XYZ key of the coordinates.
*/
inline uint64_t computeXYZKey(uint32_t x, uint32_t y, uint32_t z)
{
    static const int SHIFT = (8 * sizeof(uint64_t)) / 3;

    uint64_t key = x | (static_cast<uint_fast64_t>(y) << SHIFT) | (static_cast<uint_fast64_t>(z) << (2 * SHIFT));

    return key;
}

/**
* Compute the XYZ key number of the given set of coordinates.
*
* The XYZ key combines two 32bit coordinates and generates a unique 64bit
* value.
*
* \param x is the integer x position
* \param y is the integer y position
* \result The unique XYZ key of the coordinates.
*/
inline uint64_t computeXYZKey(uint32_t x, uint32_t y)
{
    static const int SHIFT = (8 * sizeof(uint64_t)) / 2;

    uint64_t key = x | (static_cast<uint_fast64_t>(y) << SHIFT);

    return key;
}

}

}

#endif
