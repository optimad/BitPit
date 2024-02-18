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

#ifndef __BITPIT_PABLO_TREE_CONSTANTS_HPP__
#define __BITPIT_PABLO_TREE_CONSTANTS_HPP__

// =================================================================================== //
// INCLUDES                                                                            //
// =================================================================================== //
#include "bitpit_common.hpp"

#include <stdint.h>

#include <array>
#include <vector>

namespace bitpit {

/*!
 *	\ingroup		PABLO
 *	\date			23/apr/2014
 *	\authors		Edoardo Lombardi
 *	\authors		Marco Cisternino
 *	\copyright		Copyright (C) 2014-2021 OPTIMAD engineering srl. All rights reserved.
 *
 *	\brief Global constants associated to a PABLO tree
 *
 *	Global constants are used in PABLO everywhere and they are public, i.e.
 *	each global variable can be used by any external code.
 *
 */
struct TreeConstants {

public:
	// =================================================================================== //
	// TYPEDEFS
	// =================================================================================== //
	typedef std::array<TreeConstants, 4> Instances;

	// =================================================================================== //
	// STATIC MEMBERS
	// =================================================================================== //
	BITPIT_PUBLIC_API static const TreeConstants & instance(uint8_t dim);
	BITPIT_PUBLIC_API static const Instances & instances();

	// =================================================================================== //
	// MEMBERS
	// =================================================================================== //
	int8_t   maxLevel;              /**< Maximum allowed refinement level of octree */

	uint8_t  nChildren;             /**< Number of children of an octant */
	uint8_t  nFaces;                /**< Number of faces of an octant */
	uint8_t  nEdges;                /**< Number of edges of an octant */
	uint8_t  nNodes;                /**< Number of nodes of an octant */
	uint8_t  nNodesPerFace;         /**< Number of nodes per face of an octant */

	uint8_t  oppositeFace[6];       /**< oppositeFace[i]  = Index of the face of an octant neighbour through the i-th face of the current octant */
	uint8_t  nodeFace[8][3];        /**< nodeface[i][0:1] = Local indices of faces sharing the i-th node of an octant */
	uint8_t  nodeEdge[8][3];        /**< nodeedge[i][0:1] = Local indices of edges sharing the i-th node of an octant */
	uint8_t  faceNode[6][4];        /**< facenode[i][0:1] = Local indices of nodes of the i-th face of an octant */
	uint8_t  edgeFace[12][2];       /**< edgeface[i][0:1] = Local indices of faces sharing the i-th edge of an octant */
	uint8_t  edgeNode[12][2];       /**< edgeNode[i][0:1] = Local indices of nodes of the i-th edge of an octant */
	int8_t   normals[6][3];         /**< Components (x,y,z) of the normals per face (z=0 in 2D) */
	int8_t   edgeCoeffs[12][3];     /**< Components (x,y,z) of the "normals" per edge */
	int8_t   nodeCoeffs[8][3];      /**< Components (x,y,z) of the "normals" per node */
	uint8_t  parallelEdges[12][3];  /**< Parallel edges per edge */

	uint8_t  nodeCoordinates[8][3];    /**< nodeCoordinates[i][0:2] = Local coordinates of the i-th node */
	uint8_t  edgeDisplacements[12][3]; /**< edgeDisplacements[i][0:2] = Twice the local displacement of the i-th edge centroid */
	uint8_t  faceDisplacements[6][3];  /**< faceDisplacements[i][0:2] = Twice the local displacement of the i-th face centroid */

	uint8_t  nodeFromCoordinates[2][2][2];    /**< nodeFromCoordinates[0:1][0:1][0:1] = Local node index from Local coordinates [x][y][z] */

	std::vector<uint32_t> lengths;   /**< Lengths associated to the levels */
	std::vector<uint64_t> areas;     /**< Areas associated to the levels */
	std::vector<uint64_t> volumes;   /**< Volumes associated to the levels */

private:
	// =================================================================================== //
	// METHODS
	// =================================================================================== //
	TreeConstants(uint8_t dim);

	void initialize(uint8_t dim);

};

}

#endif /* __BITPIT_PABLO_TREE_CONSTANTS_HPP__ */
