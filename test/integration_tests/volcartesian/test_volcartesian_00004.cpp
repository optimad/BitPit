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

#include <array>
#if BITPIT_ENABLE_MPI==1
#include <mpi.h>
#endif

#include "bitpit_common.hpp"
#include "bitpit_volcartesian.hpp"

using namespace bitpit;

/*!
* Subtest 001
*
* Testing basic features of a 2D patch in light memory mode.
*/
int subtest_001()
{
	std::array<double, 3> minPoint;
	std::array<double, 3> maxPoint;

	std::vector<double> cellData;
	std::vector<double> vertexData;

	std::array<double, 3> origin = {{-10., -10., -10.}};
	double length = 20;
	double dh = 0.5;

	log::cout() << "  >> 2D Cartesian patch" << "\n";

	VolCartesian *patch_2D = new VolCartesian(2, origin, length, dh);
	patch_2D->switchMemoryMode(VolCartesian::MEMORY_LIGHT);

	log::cout() << std::endl;
	log::cout() << "\n  >> 2D centroids " << "\n";
	log::cout() << std::endl;

	for (long i = 0; i < patch_2D->getCellCount(); ++i) {
		std::array<double, 3> cellCentroid = patch_2D->evalCellCentroid(i);
		log::cout() << "  >> Cell : " << i << "  Centroid : " << cellCentroid << std::endl;
	}

	log::cout() << std::endl;
	log::cout() << "\n  >> 2D bounding box" << "\n";
	log::cout() << std::endl;

	patch_2D->getBoundingBox(minPoint, maxPoint);

	log::cout() << "  >> Minimum x : " << minPoint[0] << std::endl;
	log::cout() << "  >> Minimum y : " << minPoint[1] << std::endl;
	log::cout() << "  >> Minimum z : " << minPoint[2] << std::endl;

	log::cout() << "  >> Maximum x : " << maxPoint[0] << std::endl;
	log::cout() << "  >> Maximum y : " << maxPoint[1] << std::endl;
	log::cout() << "  >> Maximum z : " << maxPoint[2] << std::endl;

	log::cout() << std::endl;
	log::cout() << "\n  >> 2D neighbour test" << "\n";

	std::vector<long> neighs_2D;

	long cellId_2D = 7;
	log::cout() << std::endl;
	log::cout() << "Cell id: " << cellId_2D << std::endl << std::endl;

	log::cout() << "Face neighbours (complete list): " << std::endl;
	neighs_2D = patch_2D->findCellFaceNeighs(cellId_2D);
	for (unsigned int i = 0; i < neighs_2D.size(); ++i) {
		log::cout() << " - " << neighs_2D[i] << std::endl;
	}

	log::cout() << "Vertex neighbours (complete list): " << std::endl;
	neighs_2D = patch_2D->findCellVertexNeighs(cellId_2D, true);
	for (unsigned int i = 0; i < neighs_2D.size(); ++i) {
		log::cout() << " - " << neighs_2D[i] << std::endl;
	}

	log::cout() << "Vertex neighbours (excuding face neighbours): " << std::endl;
	neighs_2D = patch_2D->findCellVertexNeighs(cellId_2D, false);
	for (unsigned int i = 0; i < neighs_2D.size(); ++i) {
		log::cout() << " - " << neighs_2D[i] << std::endl;
	}

	log::cout() << std::endl;

	delete patch_2D;

    return 0;
}

/*!
* Subtest 002
*
* Testing basic features of a 3D patch in light memory mode.
*/
int subtest_002()
{
	std::array<double, 3> minPoint;
	std::array<double, 3> maxPoint;

	std::vector<double> cellData;
	std::vector<double> vertexData;

	std::array<double, 3> origin = {{-10., -10., -10.}};
	double length = 20;
	double dh = 0.5;

	log::cout() << "  >> 3D Cartesian patch" << "\n";

	VolCartesian *patch_3D = new VolCartesian(3, origin, length, dh);
	patch_3D->switchMemoryMode(VolCartesian::MEMORY_LIGHT);

	log::cout() << std::endl;
	log::cout() << "\n  >> 3D centroids " << "\n";
	log::cout() << std::endl;

	for (long i = 0; i < patch_3D->getCellCount(); ++i) {
		std::array<double, 3> cellCentroid = patch_3D->evalCellCentroid(i);
		log::cout() << "  >> Cell : " << i << "  Centroid : " << cellCentroid << std::endl;
	}

	log::cout() << std::endl;
	log::cout() << "\n  >> 3D bounding box" << "\n";
	log::cout() << std::endl;

	patch_3D->getBoundingBox(minPoint, maxPoint);

	log::cout() << "  >> Minimum x : " << minPoint[0] << std::endl;
	log::cout() << "  >> Minimum y : " << minPoint[1] << std::endl;
	log::cout() << "  >> Minimum z : " << minPoint[2] << std::endl;

	log::cout() << "  >> Maximum x : " << maxPoint[0] << std::endl;
	log::cout() << "  >> Maximum y : " << maxPoint[1] << std::endl;
	log::cout() << "  >> Maximum z : " << maxPoint[2] << std::endl;

	log::cout() << std::endl;
	log::cout() << "\n  >> 3D neighbour test" << "\n";

	std::vector<long> neighs_3D;

	long cellId_3D = 13;
	log::cout() << std::endl;
	log::cout() << "Cell id: " << cellId_3D << std::endl << std::endl;

	log::cout() << "Face neighbours (complete list): " << std::endl;
	neighs_3D = patch_3D->findCellFaceNeighs(cellId_3D);
	for (unsigned int i = 0; i < neighs_3D.size(); ++i) {
		log::cout() << " - " << neighs_3D[i] << std::endl;
	}

	log::cout() << "Edge neighbours (complete list): " << std::endl;
	neighs_3D = patch_3D->findCellEdgeNeighs(cellId_3D, true);
	for (unsigned int i = 0; i < neighs_3D.size(); ++i) {
		log::cout() << " - " << neighs_3D[i] << std::endl;
	}

	log::cout() << "Edge neighbours (excuding face neighbours): " << std::endl;
	neighs_3D = patch_3D->findCellEdgeNeighs(cellId_3D, false);
	for (unsigned int i = 0; i < neighs_3D.size(); ++i) {
		log::cout() << " - " << neighs_3D[i] << std::endl;
	}

	log::cout() << "Vertex neighbours (complete list): " << std::endl;
	neighs_3D = patch_3D->findCellVertexNeighs(cellId_3D, true);
	for (unsigned int i = 0; i < neighs_3D.size(); ++i) {
		log::cout() << " - " << neighs_3D[i] << std::endl;
	}

	log::cout() << "Vertex neighbours (excuding face and edge neighbours): " << std::endl;
	neighs_3D = patch_3D->findCellVertexNeighs(cellId_3D, false);
	for (unsigned int i = 0; i < neighs_3D.size(); ++i) {
		log::cout() << " - " << neighs_3D[i] << std::endl;
	}

	log::cout() << std::endl;

	delete patch_3D;

	return 0;
}

/*!
* Main program.
*/
int main(int argc, char *argv[])
{
#if BITPIT_ENABLE_MPI==1
	MPI_Init(&argc,&argv);
#else
	BITPIT_UNUSED(argc);
	BITPIT_UNUSED(argv);
#endif

	// Initialize the logger
	log::manager().initialize(log::MODE_COMBINE);

	// Run the subtests
	log::cout() << "Testing basic features of Cartesian patches" << std::endl;

	int status;
	try {
		status = subtest_001();
		if (status != 0) {
			return status;
		}

		status = subtest_002();
		if (status != 0) {
			return status;
		}
	} catch (const std::exception &exception) {
		log::cout() << exception.what();
		exit(1);
	}

#if BITPIT_ENABLE_MPI==1
	MPI_Finalize();
#endif
}
