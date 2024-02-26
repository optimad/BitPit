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

//Standard Template Library
# include <ctime>
# include <chrono>

#if BITPIT_ENABLE_MPI==1
# include <mpi.h>
#endif

// bitpit
# include "bitpit_IO.hpp"
# include "bitpit_surfunstructured.hpp"
# include "bitpit_volcartesian.hpp"
# include "bitpit_voloctree.hpp"
# include "bitpit_volunstructured.hpp"
# include "bitpit_levelset.hpp"

/*!
 * Generate segmentation.
 *
 * \result The generated segmentation.
 */
std::unique_ptr<bitpit::SurfUnstructured> generateSegmentation()
{
    // Input geometry
#if BITPIT_ENABLE_MPI
    std::unique_ptr<bitpit::SurfUnstructured> segmentation( new bitpit::SurfUnstructured(2, MPI_COMM_NULL) );
#else
    std::unique_ptr<bitpit::SurfUnstructured> segmentation( new bitpit::SurfUnstructured(2) );
#endif

    segmentation->importSTL("./data/cube.stl", true);

    segmentation->deleteCoincidentVertices() ;
    segmentation->initializeAdjacencies() ;

    segmentation->getVTK().setName("geometry_010") ;
    segmentation->write() ;

    return segmentation;
}

/*!
 * Generate the Cartesian mesh.
 *
 * \result The generated Cartesian mesh.
 */
std::unique_ptr<bitpit::VolCartesian> generateCartesianMesh(const bitpit::SurfUnstructured &segmentation)
{
    int dimensions = 3;

    std::array<double, 3> meshMin, meshMax;
    segmentation.getBoundingBox(meshMin, meshMax);

    std::array<double, 3> delta = meshMax - meshMin;
    meshMin -= 0.1 * delta;
    meshMax += 0.1 * delta;
    delta = meshMax - meshMin;

    std::array<int, 3> nc = {{64, 64, 64}};

    std::unique_ptr<bitpit::VolCartesian> mesh(new bitpit::VolCartesian(dimensions, meshMin, delta, nc));

    return mesh;
}

/*!
 * Generate the Octree mesh.
 *
 * \result The generated Octree mesh.
 */
std::unique_ptr<bitpit::VolOctree> generateOctreeMesh(const bitpit::SurfUnstructured &segmentation)
{
    int dimensions = 3;

    std::array<double, 3> segmentationMin;
    std::array<double, 3> segmentationMax;
    segmentation.getBoundingBox(segmentationMin, segmentationMax);

    std::array<double, 3> delta = segmentationMax - segmentationMin;
    segmentationMin -= 0.1 * delta;
    segmentationMax += 0.1 * delta;
    delta = segmentationMax - segmentationMin;

    std::array<double, 3> origin = segmentationMin;

    double length = 0.;
    for (int i = 0; i < 3; ++i) {
        length = std::max(length, segmentationMax[i] - segmentationMin[i]);
    };

    double dh = length / 64;

#if BITPIT_ENABLE_MPI
    std::unique_ptr<bitpit::VolOctree> mesh(new bitpit::VolOctree(dimensions, origin, length, dh, MPI_COMM_NULL));
#else
    std::unique_ptr<bitpit::VolOctree> mesh(new bitpit::VolOctree(dimensions, origin, length, dh));
#endif

    return mesh;
}

/*!
 * Generate the Unstructured mesh.
 *
 * \result The generated Unstructured mesh.
 */
std::unique_ptr<bitpit::VolUnstructured> generateUnstructuredMesh(const bitpit::SurfUnstructured &segmentation)
{
    int dimensions = 3;

    std::array<double, 3> segmentationMin;
    std::array<double, 3> segmentationMax;
    segmentation.getBoundingBox(segmentationMin, segmentationMax);

    std::array<double, 3> delta = segmentationMax - segmentationMin;
    segmentationMin -= 0.1 * delta;
    segmentationMax += 0.1 * delta;
    delta = segmentationMax - segmentationMin;

    std::array<int, 3> nCells    = {{64, 64, 64}};
    std::array<int, 3> nVertices = {{nCells[0] + 1, nCells[1] + 1, nCells[2] + 1}};

    // Create patch
#if BITPIT_ENABLE_MPI
    std::unique_ptr<bitpit::VolUnstructured> mesh(new bitpit::VolUnstructured(dimensions, MPI_COMM_NULL));
#else
    std::unique_ptr<bitpit::VolUnstructured> mesh(new bitpit::VolUnstructured(dimensions));
#endif
    mesh->setVertexAutoIndexing(false);

    // Create vertices
    for (int i = 0; i < nVertices[0]; ++i) {
        double x = segmentationMin[0] + delta[0] / nCells[0] * i;
        for (int j = 0; j < nVertices[1]; ++j) {
            double y = segmentationMin[1] + delta[1] / nCells[1] * j;
            for (int k = 0; k < nVertices[2]; ++k) {
                double z = segmentationMin[2] + delta[2] / nCells[2] * k;

                long vertexId = i + nVertices[0] * j + nVertices[0] * nVertices[1] * k;

                mesh->addVertex({{x, y, z}}, vertexId);

            }
        }
    }

    // Create cells
    std::unordered_set<long> customCellIds;
    customCellIds.insert(171039);
    customCellIds.insert(187359);

    int cellConnectSize = bitpit::ReferenceElementInfo::MAX_ELEM_VERTICES;
    std::vector<long> cellConnect(cellConnectSize);
    for (int i = 0; i < nCells[0]; ++i) {
        for (int j = 0; j < nCells[1]; ++j) {
            for (int k = 0; k < nCells[2]; ++k) {
                long cellId = i + nCells[0] * j + nCells[0] * nCells[1] * k;
                if (customCellIds.count(cellId) != 0) {
                    continue;
                }

                cellConnect[0] =       i + nVertices[0] *       j + nVertices[0] * nVertices[1] *       k;
                cellConnect[1] = (i + 1) + nVertices[0] *       j + nVertices[0] * nVertices[1] *       k;
                cellConnect[2] = (i + 1) + nVertices[0] * (j + 1) + nVertices[0] * nVertices[1] *       k;
                cellConnect[3] =       i + nVertices[0] * (j + 1) + nVertices[0] * nVertices[1] *       k;
                cellConnect[4] =       i + nVertices[0] *       j + nVertices[0] * nVertices[1] * (k + 1);
                cellConnect[5] = (i + 1) + nVertices[0] *       j + nVertices[0] * nVertices[1] * (k + 1);
                cellConnect[6] = (i + 1) + nVertices[0] * (j + 1) + nVertices[0] * nVertices[1] * (k + 1);
                cellConnect[7] =       i + nVertices[0] * (j + 1) + nVertices[0] * nVertices[1] * (k + 1);

                mesh->addCell(bitpit::ElementType::HEXAHEDRON, cellConnect, cellId);
            }
        }
    }

    cellConnect[0] = 176377;
    cellConnect[1] = 176442;
    cellConnect[2] = 180602;
    cellConnect[3] = 180667;
    cellConnect[4] = 176376;
    cellConnect[5] = 176441;
    cellConnect[6] = 180601;
    cellConnect[7] = 180666 ;
    mesh->addCell(bitpit::ElementType::VOXEL, cellConnect);

    std::vector<long> faceStream(31);
    faceStream[ 0] = 6;
    faceStream[ 1] = 4;
    faceStream[ 2] = 197437;
    faceStream[ 3] = 193212;
    faceStream[ 4] = 193277;
    faceStream[ 5] = 197502;
    faceStream[ 6] = 4;
    faceStream[ 7] = 193212;
    faceStream[ 8] = 193211;
    faceStream[ 9] = 193276;
    faceStream[10] = 193277;
    faceStream[11] = 4;
    faceStream[12] = 197436;
    faceStream[13] = 197437;
    faceStream[14] = 197502;
    faceStream[15] = 197501;
    faceStream[16] = 4;
    faceStream[17] = 197502;
    faceStream[18] = 193277;
    faceStream[19] = 193276;
    faceStream[20] = 197501;
    faceStream[21] = 4;
    faceStream[22] = 197436;
    faceStream[23] = 193211;
    faceStream[24] = 193212;
    faceStream[25] = 197437;
    faceStream[26] = 4;
    faceStream[27] = 193211;
    faceStream[28] = 197436;
    faceStream[29] = 197501;
    faceStream[30] = 193276;
    mesh->addCell(bitpit::ElementType::POLYHEDRON, faceStream);

    return mesh;
}

/*!
* Subtest 001
*
* Testing basic features of a 3D levelset on a Cartesian mesh in default memory mode.
*/
int subtest_001()
{
    bitpit::log::cout() << std::endl;
    bitpit::log::cout() << "Testing three-dimensional levelset on a Cartesian mesh in default memory mode" << std::endl;

    // Input geometry
    bitpit::log::cout() << " - Loading geometry" << std::endl;

    std::unique_ptr<bitpit::SurfUnstructured> segmentation = generateSegmentation();
    segmentation->getVTK().setName("geometry_010");
    segmentation->write();

    bitpit::log::cout() << "n. vertex: " << segmentation->getVertexCount() << std::endl;
    bitpit::log::cout() << "n. simplex: " << segmentation->getCellCount() << std::endl;

    // Create the mesh
    bitpit::log::cout() << " - Setting mesh" << std::endl;

    std::unique_ptr<bitpit::VolCartesian> mesh = generateCartesianMesh(*segmentation);
    mesh->switchMemoryMode(bitpit::VolCartesian::MEMORY_NORMAL);
    mesh->initializeAdjacencies();
    mesh->update();

    // Initialize levelset
    bitpit::log::cout() << " - Initializing levelset" << std::endl;

    int objectId = 0;

    bitpit::LevelSet levelset ;
    levelset.setMesh(mesh.get());
    levelset.addObject(segmentation.get(), BITPIT_PI, objectId);

    // Compute levelset
    bitpit::log::cout() << " - Evaluating levelset" << std::endl;
    std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();

    bitpit::LevelSetObject *object = levelset.getObjectPtr(objectId);
    object->enableFieldCellCache(bitpit::LevelSetField::SIGN, bitpit::LevelSetCacheMode::FULL);
    object->enableFieldCellCache(bitpit::LevelSetField::VALUE, bitpit::LevelSetCacheMode::FULL);

    std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
    std::chrono::milliseconds elapsed_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
    bitpit::log::cout() << "elapsed time: " << elapsed_seconds.count() << " ms" << std::endl;

    // Store values
    long testCellId0 = 226911;
    long testCellId1 = 210719;
    long testCellId2 = 190431;

    double initialValue0 = object->evalCellValue(testCellId0, true);
    double initialValue1 = object->evalCellValue(testCellId1, true);
    double initialValue2 = object->evalCellValue(testCellId2, true);

    bitpit::log::cout() << " Initial levelset: levelset on cell " << testCellId0 << " is equal to " << initialValue0 << std::endl;
    bitpit::log::cout() << " Initial levelset: levelset on cell " << testCellId1 << " is equal to " << initialValue1 << std::endl;
    bitpit::log::cout() << " Initial levelset: levelset on cell " << testCellId2 << " is equal to " << initialValue2 << std::endl;

    // Write output
    bitpit::log::cout() << " - Writing output" << std::endl;

    levelset.getObject(objectId).enableVTKOutput(bitpit::LevelSetWriteField::SIGN);
    levelset.getObject(objectId).enableVTKOutput(bitpit::LevelSetWriteField::VALUE);
    mesh->getVTK().setName("levelset_010_cartesian_normal_initial");
    mesh->write();

    // Dump the levelset
    bitpit::log::cout() << "Dumping levelset..." << std::endl;

    int archiveVersion = 1;

    std::string header = "Cartesian levelset in normal mode";
    bitpit::OBinaryArchive binaryWriter("levelset_010_cartesian_normal", archiveVersion, header);
    levelset.dump(binaryWriter.getStream());
    binaryWriter.close();

    // Clear existing levelset
    levelset.getObject(objectId).enableVTKOutput(bitpit::LevelSetWriteField::SIGN, false);
    levelset.getObject(objectId).enableVTKOutput(bitpit::LevelSetWriteField::VALUE, false);

    levelset.clear();

    // Restore the levelset
    bitpit::log::cout() << "Restoring levelset..." << std::endl;

    bitpit::LevelSet restoredLevelset ;
    restoredLevelset.setMesh(mesh.get());
    restoredLevelset.addObject(segmentation.get(), BITPIT_PI, objectId);

    bitpit::IBinaryArchive binaryReader("levelset_010_cartesian_normal");
    restoredLevelset.restore(binaryReader.getStream());
    binaryReader.close();

    bitpit::LevelSetObject *restoredObject = restoredLevelset.getObjectPtr(objectId);

    double restoredValue0 = restoredObject->evalCellValue(testCellId0, true);
    double restoredValue1 = restoredObject->evalCellValue(testCellId1, true);
    double restoredValue2 = restoredObject->evalCellValue(testCellId2, true);

    bitpit::log::cout() << " Restored levelset: levelset on cell " << testCellId0 << " is equal to " << restoredValue0 << std::endl;
    bitpit::log::cout() << " Restored levelset: levelset on cell " << testCellId1 << " is equal to " << restoredValue1 << std::endl;
    bitpit::log::cout() << " Restored levelset: levelset on cell " << testCellId2 << " is equal to " << restoredValue2 << std::endl;

    // Write restored output
    bitpit::log::cout() << " - Writing output" << std::endl;

    mesh->getVTK().setName("levelset_010_cartesian_normal_restored");
    mesh->write();

    //
    // Comparison
    //
    bitpit::log::cout() << " Checking levelset values" << std::endl;

    if (!bitpit::utils::DoubleFloatingEqual()(initialValue0, restoredValue0)) {
        bitpit::log::cout() << "  - Restored value for test cell #0 doesn't match the initial value." << std::endl;
        return 1;
    }
    bitpit::log::cout() << "  - Restored value for test cell #0 matches the initial value." << std::endl;

    if (!bitpit::utils::DoubleFloatingEqual()(initialValue1, restoredValue1)) {
        bitpit::log::cout() << "  - Restored value for test cell #1 doesn't match the initial value." << std::endl;
        return 1;
    }
    bitpit::log::cout() << "  - Restored value for test cell #1 matches the initial value." << std::endl;

    if (!bitpit::utils::DoubleFloatingEqual()(initialValue2, restoredValue2)) {
        bitpit::log::cout() << "  - Restored value for test cell #2 doesn't match the initial value." << std::endl;
        return 1;
    }
    bitpit::log::cout() << "  - Restored value for test cell #2 matches the initial value." << std::endl;

    return 0;
}

/*!
* Subtest 010
*
* Testing basic features of a 3D levelset on a Cartesian mesh in light memory mode.
*/
int subtest_010()
{
    bitpit::log::cout() << std::endl;
    bitpit::log::cout() << "Testing three-dimensional levelset on a Cartesian mesh in light memory mode" << std::endl;

    // Input geometry
    bitpit::log::cout() << " - Loading geometry" << std::endl;

    std::unique_ptr<bitpit::SurfUnstructured> segmentation = generateSegmentation();
    segmentation->getVTK().setName("geometry_010");
    segmentation->write();

    bitpit::log::cout() << "n. vertex: " << segmentation->getVertexCount() << std::endl;
    bitpit::log::cout() << "n. simplex: " << segmentation->getCellCount() << std::endl;

    // Create the mesh
    bitpit::log::cout() << " - Setting mesh" << std::endl;

    std::unique_ptr<bitpit::VolCartesian> mesh = generateCartesianMesh(*segmentation);
    mesh->switchMemoryMode(bitpit::VolCartesian::MEMORY_LIGHT);

    // Initialize levelset
    bitpit::log::cout() << " - Initializing levelset" << std::endl;

    int objectId = 0;

    bitpit::LevelSet levelset ;
    levelset.setMesh(mesh.get());
    levelset.addObject(segmentation.get(), BITPIT_PI, objectId);

    // Compute levelset
    bitpit::log::cout() << " - Evaluating levelset" << std::endl;
    std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();

    bitpit::LevelSetObject *object = static_cast<bitpit::LevelSetObject *>(levelset.getObjectPtr(objectId));
    object->enableFieldCellCache(bitpit::LevelSetField::SIGN, bitpit::LevelSetCacheMode::FULL);
    object->enableFieldCellCache(bitpit::LevelSetField::VALUE, bitpit::LevelSetCacheMode::FULL);

    std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
    std::chrono::milliseconds elapsed_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
    bitpit::log::cout() << "elapsed time: " << elapsed_seconds.count() << " ms" << std::endl;

    // Store values
    long testCellId0 = 226911;
    long testCellId1 = 210719;
    long testCellId2 = 190431;

    double initialValue0 = object->evalCellValue(testCellId0, true);
    double initialValue1 = object->evalCellValue(testCellId1, true);
    double initialValue2 = object->evalCellValue(testCellId2, true);

    bitpit::log::cout() << " Initial levelset: levelset on cell " << testCellId0 << " is equal to " << initialValue0 << std::endl;
    bitpit::log::cout() << " Initial levelset: levelset on cell " << testCellId1 << " is equal to " << initialValue1 << std::endl;
    bitpit::log::cout() << " Initial levelset: levelset on cell " << testCellId2 << " is equal to " << initialValue2 << std::endl;

    // Write output
    bitpit::log::cout() << " - Writing output" << std::endl;

    levelset.getObject(objectId).enableVTKOutput(bitpit::LevelSetWriteField::SIGN);
    levelset.getObject(objectId).enableVTKOutput(bitpit::LevelSetWriteField::VALUE);
    mesh->getVTK().setName("levelset_010_cartesian_light_initial");
    mesh->switchMemoryMode(bitpit::VolCartesian::MEMORY_NORMAL);
    mesh->write();
    mesh->switchMemoryMode(bitpit::VolCartesian::MEMORY_LIGHT);

    // Dump the levelset
    bitpit::log::cout() << "Dumping levelset..." << std::endl;

    int archiveVersion = 1;

    std::string header = "Cartesian levelset in light mode";
    bitpit::OBinaryArchive binaryWriter("levelset_010_cartesian_light", archiveVersion, header);
    levelset.dump(binaryWriter.getStream());
    binaryWriter.close();

    // Clear existing levelset
    levelset.getObject(objectId).enableVTKOutput(bitpit::LevelSetWriteField::SIGN, false);
    levelset.getObject(objectId).enableVTKOutput(bitpit::LevelSetWriteField::VALUE, false);

    levelset.clear();

    // Restore the levelset
    bitpit::log::cout() << "Restoring levelset..." << std::endl;

    bitpit::LevelSet restoredLevelset ;
    restoredLevelset.setMesh(mesh.get());
    restoredLevelset.addObject(segmentation.get(), BITPIT_PI, objectId);

    bitpit::IBinaryArchive binaryReader("levelset_010_cartesian_light");
    restoredLevelset.restore(binaryReader.getStream());
    binaryReader.close();

    bitpit::LevelSetObject *restoredObject = restoredLevelset.getObjectPtr(objectId);

    double restoredValue0 = restoredObject->evalCellValue(testCellId0, true);
    double restoredValue1 = restoredObject->evalCellValue(testCellId1, true);
    double restoredValue2 = restoredObject->evalCellValue(testCellId2, true);

    bitpit::log::cout() << " Restored levelset: levelset on cell " << testCellId0 << " is equal to " << restoredValue0 << std::endl;
    bitpit::log::cout() << " Restored levelset: levelset on cell " << testCellId1 << " is equal to " << restoredValue1 << std::endl;
    bitpit::log::cout() << " Restored levelset: levelset on cell " << testCellId2 << " is equal to " << restoredValue2 << std::endl;

    // Write restored output
    bitpit::log::cout() << " - Writing output" << std::endl;

    mesh->getVTK().setName("levelset_010_cartesian_light_restored");
    mesh->switchMemoryMode(bitpit::VolCartesian::MEMORY_NORMAL);
    mesh->write();
    mesh->switchMemoryMode(bitpit::VolCartesian::MEMORY_LIGHT);

    //
    // Comparison
    //
    bitpit::log::cout() << " Checking levelset values" << std::endl;

    if (!bitpit::utils::DoubleFloatingEqual()(initialValue0, restoredValue0)) {
        bitpit::log::cout() << "  - Restored value for test cell #0 doesn't match the initial value." << std::endl;
        return 1;
    }
    bitpit::log::cout() << "  - Restored value for test cell #0 matches the initial value." << std::endl;

    if (!bitpit::utils::DoubleFloatingEqual()(initialValue1, restoredValue1)) {
        bitpit::log::cout() << "  - Restored value for test cell #1 doesn't match the initial value." << std::endl;
        return 1;
    }
    bitpit::log::cout() << "  - Restored value for test cell #1 matches the initial value." << std::endl;

    if (!bitpit::utils::DoubleFloatingEqual()(initialValue2, restoredValue2)) {
        bitpit::log::cout() << "  - Restored value for test cell #2 doesn't match the initial value." << std::endl;
        return 1;
    }
    bitpit::log::cout() << "  - Restored value for test cell #2 matches the initial value." << std::endl;

    return 0;
}

/*!
* Subtest 003
*
* Testing basic features of a 3D levelset on an Octreee mesh.
*/
int subtest_003()
{
    bitpit::log::cout() << std::endl;
    bitpit::log::cout() << "Testing three-dimensional levelset on an Octree mesh" << std::endl;

    // Input geometry
    bitpit::log::cout() << " - Loading geometry" << std::endl;

    std::unique_ptr<bitpit::SurfUnstructured> segmentation = generateSegmentation();
    segmentation->getVTK().setName("geometry_010");
    segmentation->write();

    bitpit::log::cout() << "n. vertex: " << segmentation->getVertexCount() << std::endl;
    bitpit::log::cout() << "n. simplex: " << segmentation->getCellCount() << std::endl;

    // Create the mesh
    bitpit::log::cout() << " - Setting mesh" << std::endl;

    std::unique_ptr<bitpit::VolOctree> mesh = generateOctreeMesh(*segmentation);
    mesh->initializeAdjacencies();
    mesh->update();

    // Initialize levelset
    bitpit::log::cout() << " - Initializing levelset" << std::endl;

    int objectId = 0;

    bitpit::LevelSet levelset ;
    levelset.setMesh(mesh.get());
    levelset.addObject(segmentation.get(), BITPIT_PI, objectId);

    // Compute levelset
    bitpit::log::cout() << " - Evaluating levelset" << std::endl;
    std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();

    bitpit::LevelSetObject *object = static_cast<bitpit::LevelSetObject *>(levelset.getObjectPtr(objectId));
    object->enableFieldCellCache(bitpit::LevelSetField::SIGN, bitpit::LevelSetCacheMode::FULL);
    object->enableFieldCellCache(bitpit::LevelSetField::VALUE, bitpit::LevelSetCacheMode::FULL);

    std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
    std::chrono::milliseconds elapsed_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
    bitpit::log::cout() << "elapsed time: " << elapsed_seconds.count() << " ms" << std::endl;

    // Store values
    long testCellId0 = 219133;
    long testCellId1 = 203775;
    long testCellId2 = 201579;

    double initialValue0 = object->evalCellValue(testCellId0, true);
    double initialValue1 = object->evalCellValue(testCellId1, true);
    double initialValue2 = object->evalCellValue(testCellId2, true);

    bitpit::log::cout() << " Initial levelset: levelset on cell " << testCellId0 << " is equal to " << initialValue0 << std::endl;
    bitpit::log::cout() << " Initial levelset: levelset on cell " << testCellId1 << " is equal to " << initialValue1 << std::endl;
    bitpit::log::cout() << " Initial levelset: levelset on cell " << testCellId2 << " is equal to " << initialValue2 << std::endl;

    // Write output
    bitpit::log::cout() << " - Writing output" << std::endl;

    levelset.getObject(objectId).enableVTKOutput(bitpit::LevelSetWriteField::SIGN);
    levelset.getObject(objectId).enableVTKOutput(bitpit::LevelSetWriteField::VALUE);
    mesh->getVTK().setName("levelset_010_octree_initial");
    mesh->write();

    // Dump the levelset
    bitpit::log::cout() << "Dumping levelset..." << std::endl;

    int archiveVersion = 1;

    std::string header = "Octree levelset";
    bitpit::OBinaryArchive binaryWriter("levelset_010_octree", archiveVersion, header);
    levelset.dump(binaryWriter.getStream());
    binaryWriter.close();

    // Clear existing levelset
    levelset.getObject(objectId).enableVTKOutput(bitpit::LevelSetWriteField::SIGN, false);
    levelset.getObject(objectId).enableVTKOutput(bitpit::LevelSetWriteField::VALUE, false);

    levelset.clear();

    // Restore the levelset
    bitpit::log::cout() << "Restoring levelset..." << std::endl;

    bitpit::LevelSet restoredLevelset ;
    restoredLevelset.setMesh(mesh.get());
    restoredLevelset.addObject(segmentation.get(), BITPIT_PI, objectId);

    bitpit::IBinaryArchive binaryReader("levelset_010_octree");
    restoredLevelset.restore(binaryReader.getStream());
    binaryReader.close();

    bitpit::LevelSetObject *restoredObject = restoredLevelset.getObjectPtr(objectId);

    double restoredValue0 = restoredObject->evalCellValue(testCellId0, true);
    double restoredValue1 = restoredObject->evalCellValue(testCellId1, true);
    double restoredValue2 = restoredObject->evalCellValue(testCellId2, true);

    bitpit::log::cout() << " Restored levelset: levelset on cell " << testCellId0 << " is equal to " << restoredValue0 << std::endl;
    bitpit::log::cout() << " Restored levelset: levelset on cell " << testCellId1 << " is equal to " << restoredValue1 << std::endl;
    bitpit::log::cout() << " Restored levelset: levelset on cell " << testCellId2 << " is equal to " << restoredValue2 << std::endl;

    // Write restored output
    bitpit::log::cout() << " - Writing output" << std::endl;

    mesh->getVTK().setName("levelset_010_octree_restored");
    mesh->write();

    //
    // Comparison
    //
    bitpit::log::cout() << " Checking levelset values" << std::endl;

    if (!bitpit::utils::DoubleFloatingEqual()(initialValue0, restoredValue0)) {
        bitpit::log::cout() << "  - Restored value for test cell #0 doesn't match the initial value." << std::endl;
        return 1;
    }
    bitpit::log::cout() << "  - Restored value for test cell #0 matches the initial value." << std::endl;

    if (!bitpit::utils::DoubleFloatingEqual()(initialValue1, restoredValue1)) {
        bitpit::log::cout() << "  - Restored value for test cell #1 doesn't match the initial value." << std::endl;
        return 1;
    }
    bitpit::log::cout() << "  - Restored value for test cell #1 matches the initial value." << std::endl;

    if (!bitpit::utils::DoubleFloatingEqual()(initialValue2, restoredValue2)) {
        bitpit::log::cout() << "  - Restored value for test cell #2 doesn't match the initial value." << std::endl;
        return 1;
    }
    bitpit::log::cout() << "  - Restored value for test cell #2 matches the initial value." << std::endl;

    return 0;
}

/*!
* Subtest 004
*
* Testing basic features of a 3D levelset on an Unstructured mesh.
*/
int subtest_004()
{
    bitpit::log::cout() << std::endl;
    bitpit::log::cout() << "Testing three-dimensional levelset on an Unstructured mesh" << std::endl;

    // Input geometry
    bitpit::log::cout() << " - Loading geometry" << std::endl;

    std::unique_ptr<bitpit::SurfUnstructured> segmentation = generateSegmentation();
    segmentation->getVTK().setName("geometry_010");
    segmentation->write();

    bitpit::log::cout() << "n. vertex: " << segmentation->getVertexCount() << std::endl;
    bitpit::log::cout() << "n. simplex: " << segmentation->getCellCount() << std::endl;

    // Create the mesh
    bitpit::log::cout() << " - Setting mesh" << std::endl;

    std::unique_ptr<bitpit::VolUnstructured> mesh = generateUnstructuredMesh(*segmentation);
    mesh->initializeAdjacencies();
    mesh->update();

    // Initialize levelset
    bitpit::log::cout() << " - Initializing levelset" << std::endl;

    int objectId = 0;

    // Compute levelset
    bitpit::log::cout() << " - Evaluating levelset" << std::endl;
    std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();

    bitpit::LevelSet levelset ;
    levelset.setMesh(mesh.get());
    levelset.addObject(segmentation.get(), BITPIT_PI, objectId);

    bitpit::LevelSetObject *object = static_cast<bitpit::LevelSetObject *>(levelset.getObjectPtr(objectId));
    object->enableFieldCellCache(bitpit::LevelSetField::SIGN, bitpit::LevelSetCacheMode::FULL);
    object->enableFieldCellCache(bitpit::LevelSetField::VALUE, bitpit::LevelSetCacheMode::FULL);

    std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
    std::chrono::milliseconds elapsed_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
    bitpit::log::cout() << "elapsed time: " << elapsed_seconds.count() << " ms" << std::endl;

    // Store values
    long testCellId0 = 226911;
    long testCellId1 = 210719;
    long testCellId2 = 190431;

    double initialValue0 = object->evalCellValue(testCellId0, true);
    double initialValue1 = object->evalCellValue(testCellId1, true);
    double initialValue2 = object->evalCellValue(testCellId2, true);

    bitpit::log::cout() << " Initial levelset: levelset on cell " << testCellId0 << " is equal to " << initialValue0 << std::endl;
    bitpit::log::cout() << " Initial levelset: levelset on cell " << testCellId1 << " is equal to " << initialValue1 << std::endl;
    bitpit::log::cout() << " Initial levelset: levelset on cell " << testCellId2 << " is equal to " << initialValue2 << std::endl;

    // Write output
    bitpit::log::cout() << " - Writing output" << std::endl;

    levelset.getObject(objectId).enableVTKOutput(bitpit::LevelSetWriteField::SIGN);
    levelset.getObject(objectId).enableVTKOutput(bitpit::LevelSetWriteField::VALUE);
    mesh->getVTK().setName("levelset_010_unstructured_initial");
    mesh->write();

    // Dump the levelset
    bitpit::log::cout() << "Dumping levelset..." << std::endl;

    int archiveVersion = 1;

    std::string header = "Unstructured levelset";
    bitpit::OBinaryArchive binaryWriter("levelset_010_unstructured", archiveVersion, header);
    levelset.dump(binaryWriter.getStream());
    binaryWriter.close();

    // Clear existing levelset
    levelset.getObject(objectId).enableVTKOutput(bitpit::LevelSetWriteField::SIGN, false);
    levelset.getObject(objectId).enableVTKOutput(bitpit::LevelSetWriteField::VALUE, false);

    levelset.clear();

    // Restore the levelset
    bitpit::log::cout() << "Restoring levelset..." << std::endl;

    bitpit::LevelSet restoredLevelset ;
    restoredLevelset.setMesh(mesh.get());
    restoredLevelset.addObject(segmentation.get(), BITPIT_PI, objectId);

    bitpit::IBinaryArchive binaryReader("levelset_010_unstructured");
    restoredLevelset.restore(binaryReader.getStream());
    binaryReader.close();

    bitpit::LevelSetObject *restoredObject = restoredLevelset.getObjectPtr(objectId);

    double restoredValue0 = restoredObject->evalCellValue(testCellId0, true);
    double restoredValue1 = restoredObject->evalCellValue(testCellId1, true);
    double restoredValue2 = restoredObject->evalCellValue(testCellId2, true);

    bitpit::log::cout() << " Restored levelset: levelset on cell " << testCellId0 << " is equal to " << restoredValue0 << std::endl;
    bitpit::log::cout() << " Restored levelset: levelset on cell " << testCellId1 << " is equal to " << restoredValue1 << std::endl;
    bitpit::log::cout() << " Restored levelset: levelset on cell " << testCellId2 << " is equal to " << restoredValue2 << std::endl;

    // Write restored output
    bitpit::log::cout() << " - Writing output" << std::endl;

    mesh->getVTK().setName("levelset_010_unstructured_restored");
    mesh->write();

    //
    // Comparison
    //
    bitpit::log::cout() << " Checking levelset values" << std::endl;

    if (!bitpit::utils::DoubleFloatingEqual()(initialValue0, restoredValue0)) {
        bitpit::log::cout() << "  - Restored value for test cell #0 doesn't match the initial value." << std::endl;
        return 1;
    }
    bitpit::log::cout() << "  - Restored value for test cell #0 matches the initial value." << std::endl;

    if (!bitpit::utils::DoubleFloatingEqual()(initialValue1, restoredValue1)) {
        bitpit::log::cout() << "  - Restored value for test cell #1 doesn't match the initial value." << std::endl;
        return 1;
    }
    bitpit::log::cout() << "  - Restored value for test cell #1 matches the initial value." << std::endl;

    if (!bitpit::utils::DoubleFloatingEqual()(initialValue2, restoredValue2)) {
        bitpit::log::cout() << "  - Restored value for test cell #2 doesn't match the initial value." << std::endl;
        return 1;
    }
    bitpit::log::cout() << "  - Restored value for test cell #2 matches the initial value." << std::endl;

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
	bitpit::log::manager().initialize(bitpit::log::MODE_COMBINE);

	// Run the subtests
	bitpit::log::cout() << "Testing basic levelset features" << std::endl;

	int status;
	try {
		status = subtest_001();
		if (status != 0) {
			return status;
		}

		status = subtest_010();
		if (status != 0) {
			return status;
		}

		status = subtest_003();
		if (status != 0) {
			return status;
		}

		status = subtest_004();
		if (status != 0) {
			return status;
		}
	} catch (const std::exception &exception) {
		bitpit::log::cout() << exception.what();
		exit(1);
	}

#if BITPIT_ENABLE_MPI==1
	MPI_Finalize();
#endif
}
