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

#ifndef __BITPIT_PABLO_OCTANT_HPP__
#define __BITPIT_PABLO_OCTANT_HPP__

// INCLUDES                                                                            //
#include "tree_constants.hpp"

#include <vector>
#include <bitset>
#include <array>

#include "bitpit_containers.hpp"

namespace bitpit {

// =================================================================================== //
// TYPEDEFS
// =================================================================================== //
typedef std::vector<uint8_t>                 u8vector;
typedef std::vector<uint32_t>                u32vector;
typedef std::vector<uint64_t>                u64vector;
typedef std::vector<double>                  dvector;
typedef std::array<double, 3>                darray3;
typedef std::array<int8_t, 3>                i8array3;
typedef std::array<uint32_t, 3>              u32array3;
typedef std::vector<std::vector<uint32_t>>   u32vector2D;
typedef std::vector<std::vector<uint64_t>>   u64vector2D;
typedef std::vector<u32array3>               u32arr3vector;

// =================================================================================== //
// STREAM OPERATORS
// =================================================================================== //
class Octant;

IBinaryStream & operator>>(IBinaryStream &buffer, Octant& octant);
OBinaryStream & operator<<(OBinaryStream &buffer, const Octant& octant);

// =================================================================================== //
// CLASS DEFINITION                                                                    //
// =================================================================================== //
/*!
 *    \ingroup        PABLO
 *    \date            10/dec/2015
 *    \authors        Edoardo Lombardi
 *    \authors        Marco Cisternino
 *    \copyright        Copyright (C) 2015-2021 OPTIMAD engineering srl. All rights reserved.
 *
 *    \brief Octant class definition
 *
 *    Octants are the grid elements of PABLO. In the logical domain octants are
 *    squares or cubes, depending on dimension, with size function of their level.
 *    Each octant has nodes and faces ordered with Z-order.
 *    \image html PabloOctant.png
 *    \image html PabloOctant3D.png
 *    The main feature of each octant are:
 *    - x,y,z        : coordinates of the node 0 of the octant;
 *    - Morton index : classical Morton index defined anly by the coordinates
 *    (info about level used additionally for equality operator);
 *    - marker       : refinement marker can assume negative, positive or zero values, wich mean
 *    a coarsening, refinement and none adaptation respectively;
 *    - level        : octant level in the octree, zero for the first upper level.
 *    - balance      : flag to fix if the octant has to 2:1 balanced with respect
 *    to its face neighbours.
 *
 */
class Octant{

    // =================================================================================== //
    // FRIENDSHIPS
    // =================================================================================== //

    friend class LocalTree;
    friend class ParaTree;
    friend class Global;

    friend OBinaryStream& (operator<<) (OBinaryStream& buf, const Octant& octant);
    friend IBinaryStream& (operator>>) (IBinaryStream& buf, Octant& octant);


    // =================================================================================== //
    // STATIC MEMBERS
    // =================================================================================== //

    static const TreeConstants::Instances sm_treeConstants; /**< Static member for internal use. */

    // =================================================================================== //
    // MEMBERS
    // =================================================================================== //
    enum OctantInfo {
        INFO_BOUNDFACE0     = 0,  /**< Identifier to access face bound information in bitset info for face 0 */
        INFO_BOUNDFACE1     = 1,  /**< Identifier to access face bound information in bitset info for face 1 */
        INFO_BOUNDFACE2     = 2,  /**< Identifier to access face bound information in bitset info for face 2 */
        INFO_BOUNDFACE3     = 3,  /**< Identifier to access face bound information in bitset info for face 3 */
        INFO_BOUNDFACE4     = 4,  /**< Identifier to access face bound information in bitset info for face 4 */
        INFO_BOUNDFACE5     = 5,  /**< Identifier to access face bound information in bitset info for face 5 */
        INFO_PBOUNDFACE0    = 6,  /**< Identifier to access face process bound information in bitset info for face 0 */
        INFO_PBOUNDFACE1    = 7,  /**< Identifier to access face process bound information in bitset info for face 1 */
        INFO_PBOUNDFACE2    = 8,  /**< Identifier to access face process bound information in bitset info for face 2 */
        INFO_PBOUNDFACE3    = 9,  /**< Identifier to access face process bound information in bitset info for face 3 */
        INFO_PBOUNDFACE4    = 10, /**< Identifier to access face process bound information in bitset info for face 4 */
        INFO_PBOUNDFACE5    = 11, /**< Identifier to access face process bound information in bitset info for face 5 */
        INFO_NEW4REFINEMENT = 12, /**< Identifier to access the bit storing if the octant is new for refinement */
        INFO_NEW4COARSENING = 13, /**< Identifier to access the bit storing if the octant is new for coarsening */
        INFO_BALANCED       = 14, /**< Identifier to access the bit storing if the octant has to be kept unbalanced */
        INFO_AUX            = 15, /**< Identifier to access an auxiliary bit */

        INFO_ITEM_COUNT     = 16  /**< Number of items contained in the enum */
    };

private:
    uint64_t                        m_morton;       /**< Morton number */
    uint8_t                         m_level;        /**< Refinement level (0=root) */
    int8_t                          m_marker;       /**< Set for Refinement(m>0) or Coarsening(m<0) |m|-times */
    std::bitset<INFO_ITEM_COUNT>    m_info;         /**< -Info[0..5]: true if 0..5 face is a boundary face [bound] \n
                                                         -Info[6..11]: true if 0..6 face is a process boundary face [pbound] \n
                                                         -Info[12/13]: true if octant is new after refinement/coarsening \n
                                                         -Info[14]   : true if balancing is required for this octant \n
                                                         -Info[15]   : Aux */
    uint8_t                         m_dim;          /**< Dimension of octant (2D/3D) */
    int                             m_ghost;        /**< Ghost specifier:\n
                                                         -1 : internal, \n
                                                          0 : ghost in the 0-th layer of the halo, \n
                                                          1 : ghost in the 1-st layer of the halo, \n
                                                          ... \n
                                                          n : ghost in the n-th layer of the halo. */

    //TODO add bitset for edge & node

    // =================================================================================== //
    // CONSTRUCTORS AND OPERATORS
    // =================================================================================== //
public:
    Octant();
private:
    Octant(uint8_t dim);
    Octant(uint8_t dim, uint8_t level, uint64_t morton);
    Octant(uint8_t dim, uint8_t level, int32_t x, int32_t y, int32_t z = 0);
    Octant(bool bound, uint8_t dim, uint8_t level, int32_t x, int32_t y, int32_t z = 0);
    bool operator ==(const Octant & oct2);

    // =================================================================================== //
    // STATIC METHODS
    // =================================================================================== //
public:
    static unsigned int getBinarySize();

private:
    // =================================================================================== //
    // PRIVATE METHODS
    // =================================================================================== //
    void initialize();
    void initialize(uint8_t dim, uint8_t level, bool bound);

public:
    // =================================================================================== //
    // PUBLIC METHODS
    // =================================================================================== //
    uint64_t        computeLastDescMorton() const;
    u32array3       computeLastDescCoordinates() const;
    uint64_t        computeFatherMorton() const;
    u32array3       computeFatherCoordinates() const;

    // =================================================================================== //
    // BASIC GET/SET METHODS
    // =================================================================================== //
    uint32_t    getDim() const;
    u32array3   getLogicalCoordinates() const;
    uint32_t    getLogicalCoordinates(int coord) const;
    uint8_t     getLevel() const;
    int8_t      getMarker() const;
    bool        getBound(uint8_t face) const;
    bool        getEdgeBound(uint8_t edge) const;
    bool        getNodeBound(uint8_t node) const;
    bool        getBound() const;
    bool        getPbound(uint8_t face) const;
    bool        getPbound() const;
    bool        getIsNewR() const;
    bool        getIsNewC() const;
    bool        getIsGhost() const;
    int         getGhostLayer() const;
    bool        getBalance() const;
protected:
    void        setBound(uint8_t face);
    void        setMarker(int8_t marker);
    void        setBalance(bool balance);
    void        setLevel(uint8_t level);
    void        setPbound(uint8_t face, bool flag);
    void        setGhostLayer(int ghostLayer);

    // =================================================================================== //
    // OTHER GET/SET METHODS
    // =================================================================================== //
public:
    uint32_t        getLogicalSize() const;
    uint64_t        getLogicalArea() const;
    uint64_t        getLogicalVolume() const;
    darray3         getLogicalCenter() const;
    darray3         getLogicalFaceCenter(uint8_t iface) const;
    darray3         getLogicalEdgeCenter(uint8_t iedge) const;
    void            getLogicalNodes(u32arr3vector & nodes) const;
    u32arr3vector   getLogicalNodes() const;
    void            getLogicalNode(u32array3 & node, uint8_t inode) const;
    u32array3       getLogicalNode(uint8_t inode) const;
    void            getNormal(uint8_t iface, i8array3 & normal, const int8_t (&normals)[6][3]) const;
    uint64_t        getMorton() const;
    uint64_t        computeNodePersistentKey(uint8_t inode) const;
    uint64_t        computeNodePersistentKey(const u32array3 &node) const;

    // =================================================================================== //
    // OTHER METHODS                                                                   //
    // =================================================================================== //
    Octant                 buildLastDesc() const;
    Octant                 buildFather() const;
    uint8_t                countChildren() const;
    std::vector<Octant>    buildChildren() const;
    void                   buildChildren(Octant *children) const;
protected:
    uint8_t getFamilySplittingNode() const;
};

}

#endif /* __BITPIT_PABLO_OCTANT_HPP__ */
