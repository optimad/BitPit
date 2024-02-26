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
#include <ctime>
#include <chrono>

#if BITPIT_ENABLE_MPI==1
#include <mpi.h>
#endif

#include "bitpit_levelset.hpp"
#include "bitpit_surfunstructured.hpp"
#include "bitpit_volcartesian.hpp"
#include "bitpit_voloctree.hpp"

const int SPACE_DIMENSION = 2;

/*!
 * Generate segmentation.
 *
 * \result The generated segmentation.
 */
std::unique_ptr<bitpit::SurfUnstructured> generateSegmentation()
{
    const double R = 1.;
    const long N = 32;
    const double dtheta = 2. * BITPIT_PI / ((double) N);

    // Initialize segmentation
#if BITPIT_ENABLE_MPI
    std::unique_ptr<bitpit::SurfUnstructured> segmentation(new bitpit::SurfUnstructured(0, SPACE_DIMENSION - 1, MPI_COMM_NULL));
#else
    std::unique_ptr<bitpit::SurfUnstructured> segmentation(new bitpit::SurfUnstructured(0, SPACE_DIMENSION - 1));
#endif

    // Create vertex list
    //
    // Use non-consecutive vertex ids to test if the levelset can handle them.
    const long vertexIdOffset = 101;
    const long vertexIdStride = 2;

    std::array<double, 3> point;
    point[2] = 0.0;
    for (long i = 0; i < N; ++i) {
        double theta = i * dtheta;
        point[0] = R * cos(theta);
        point[1] = R * sin(theta);
        segmentation->addVertex(point, vertexIdOffset + vertexIdStride * i);
    }

    // Create simplex list
    //
    // Use non-consecutive cell ids to test if the levelset can handle them.
    const long cellIdOffset = 202;
    const long cellIdStride = 3;

    for (long i = 0; i < N; ++i) {
        std::vector<long> cellConnect(2, bitpit::Element::NULL_ID);
        cellConnect[0] = vertexIdOffset + vertexIdStride * i;
        cellConnect[1] = vertexIdOffset + vertexIdStride * ((i + 1) % N);
        segmentation->addCell(bitpit::ElementType::LINE, cellConnect, cellIdOffset + cellIdStride * i);
    }

    segmentation->initializeAdjacencies();

    return segmentation;
}

/*!
 * Generate the Cartesian mesh.
 *
 * \result The generated Cartesian mesh.
 */
std::unique_ptr<bitpit::VolCartesian> generateCartesianMesh(const bitpit::SurfUnstructured &segmentation)
{
    std::array<double, 3> segmentationMin;
    std::array<double, 3> segmentationMax;
    segmentation.getBoundingBox(segmentationMin, segmentationMax);

    std::array<double, 3> length = 1.1 * (segmentationMax - segmentationMin);
    std::array<double, 3> origin = -0.5 * length;

    std::array<int,3> nc = {{128, 128, 0}};

    std::unique_ptr<bitpit::VolCartesian> mesh(new bitpit::VolCartesian(SPACE_DIMENSION, origin, length, nc));

    return mesh;
}

/*!
 * Generate the Octree mesh.
 *
 * \result The generated Octree mesh.
 */
std::unique_ptr<bitpit::VolOctree> generateOctreeMesh(const bitpit::SurfUnstructured &segmentation)
{
    std::array<double, 3> segmentationMin;
    std::array<double, 3> segmentationMax;
    segmentation.getBoundingBox(segmentationMin, segmentationMax);

    double length = 0.;
    for (int i = 0; i < 3; ++i) {
        length = std::max(length, 1.1 * (segmentationMax[i] - segmentationMin[i]));
    };

    std::array<double, 3> origin = - 0.5 * std::array<double, 3>{{length, length, 0.}};

    double dh = length / 128;

#if BITPIT_ENABLE_MPI
    std::unique_ptr<bitpit::VolOctree> mesh(new bitpit::VolOctree(SPACE_DIMENSION, origin, length, dh, MPI_COMM_NULL));
#else
    std::unique_ptr<bitpit::VolOctree> mesh(new bitpit::VolOctree(SPACE_DIMENSION, origin, length, dh));
#endif

    return mesh;
}

/*!
* Subtest 001
*
* Testing dense and sparse storage on a Cartesian mesh in default memory mode.
*/
int subtest_001()
{
    bitpit::log::cout() << std::endl;
    bitpit::log::cout() << "Testing dense and sparse storage on an Cartesian mesh in default memory mode" << std::endl;

    // Input geometry
    bitpit::log::cout() << " - Loading geometry" << std::endl;

    std::unique_ptr<bitpit::SurfUnstructured> segmentation = generateSegmentation();
    segmentation->getVTK().setName("geometry_007");
    segmentation->write();

    bitpit::log::cout() << "n. vertex: " << segmentation->getVertexCount() << std::endl;
    bitpit::log::cout() << "n. simplex: " << segmentation->getCellCount() << std::endl;

    // Create the mesh
    bitpit::log::cout() << " - Setting mesh" << std::endl;

    std::unique_ptr<bitpit::VolCartesian> mesh = generateCartesianMesh(*segmentation);
    mesh->switchMemoryMode(bitpit::VolCartesian::MEMORY_NORMAL);
    mesh->initializeAdjacencies();
    mesh->update();

    // Initialize test
    long testCellId0 = 12065;
    long testCellId1 = 13671;
    long testCellId2 = 15495;

    int objectId = 0;

    //
    // Levelset - Sparse storage
    //

    // Initialize levelset
    bitpit::log::cout() << "Filling levelset cache using sprase storage... " << std::endl;
    std::chrono::time_point<std::chrono::system_clock> startSparse = std::chrono::system_clock::now();

    bitpit::LevelSet levelsetSparse(bitpit::LevelSetFillIn::SPARSE);
    levelsetSparse.setMesh(mesh.get());
    levelsetSparse.addObject(segmentation.get(), BITPIT_PI, objectId);

    bitpit::LevelSetObject &sparseObject = levelsetSparse.getObject(objectId);
    sparseObject.enableFieldCellCache(bitpit::LevelSetField::SIGN, bitpit::LevelSetCacheMode::FULL);
    sparseObject.enableFieldCellCache(bitpit::LevelSetField::VALUE, bitpit::LevelSetCacheMode::FULL);

    std::chrono::time_point<std::chrono::system_clock> endSparse = std::chrono::system_clock::now();
    std::chrono::milliseconds elapsedTimeSparse = std::chrono::duration_cast<std::chrono::milliseconds>(endSparse - startSparse);
    bitpit::log::cout() << "Computation completed in " << elapsedTimeSparse.count() << " ms" << std::endl;

    levelsetSparse.getObject(objectId).enableVTKOutput(bitpit::LevelSetWriteField::VALUE);
    mesh->getVTK().setName("levelset_007_cartesian_default_sparse");
    mesh->write();

    double sparseValue0 = levelsetSparse.getObject(objectId).evalCellValue(testCellId0, true);
    double sparseValue1 = levelsetSparse.getObject(objectId).evalCellValue(testCellId1, true);
    double sparseValue2 = levelsetSparse.getObject(objectId).evalCellValue(testCellId2, true);

    bitpit::log::cout() << " Sparse storage mode: levelset on cell " << testCellId0 << " is equal to " << sparseValue0 << std::endl;
    bitpit::log::cout() << " Sparse storage mode: levelset on cell " << testCellId1 << " is equal to " << sparseValue1 << std::endl;
    bitpit::log::cout() << " Sparse storage mode: levelset on cell " << testCellId2 << " is equal to " << sparseValue2 << std::endl;

    //
    // Levelset - Dense storage
    //

    // Initialize levelset
    bitpit::log::cout() << "Filling levelset cache using dense storage... " << std::endl;
    std::chrono::time_point<std::chrono::system_clock> startDense = std::chrono::system_clock::now();

    bitpit::LevelSet levelsetDense(bitpit::LevelSetFillIn::DENSE);
    levelsetDense.setMesh(mesh.get());
    levelsetDense.addObject(segmentation.get(), BITPIT_PI, objectId);

    bitpit::LevelSetObject &denseObject = levelsetDense.getObject(objectId);
    denseObject.enableFieldCellCache(bitpit::LevelSetField::SIGN, bitpit::LevelSetCacheMode::FULL);
    denseObject.enableFieldCellCache(bitpit::LevelSetField::VALUE, bitpit::LevelSetCacheMode::NARROW_BAND);

    std::chrono::time_point<std::chrono::system_clock> endDense = std::chrono::system_clock::now();
    std::chrono::milliseconds elapsedTimeDense = std::chrono::duration_cast<std::chrono::milliseconds>(endDense - startDense);
    bitpit::log::cout() << "Computation completed in " << elapsedTimeDense.count() << " ms" << std::endl;

    levelsetDense.getObject(objectId).enableVTKOutput(bitpit::LevelSetWriteField::VALUE);
    mesh->getVTK().setName("levelset_007_cartesian_default_dense");
    mesh->write();

    double denseValue0 = levelsetDense.getObject(objectId).evalCellValue(testCellId0, true);
    double denseValue1 = levelsetDense.getObject(objectId).evalCellValue(testCellId1, true);
    double denseValue2 = levelsetDense.getObject(objectId).evalCellValue(testCellId2, true);

    bitpit::log::cout() << " Dense storage mode: levelset on cell " << testCellId0 << " is equal to " << denseValue0 << std::endl;
    bitpit::log::cout() << " Dense storage mode: levelset on cell " << testCellId1 << " is equal to " << denseValue1 << std::endl;
    bitpit::log::cout() << " Dense storage mode: levelset on cell " << testCellId2 << " is equal to " << denseValue2 << std::endl;

    //
    // Comparison
    //

    bitpit::log::cout() << " Checking levelset values" << std::endl;

    if (!bitpit::utils::DoubleFloatingEqual()(sparseValue0, denseValue0)) {
        bitpit::log::cout() << "  - Value obtained for test cell #0 using sparse storage doesn't match the one obtained using dense storage." << std::endl;
        return 1;
    }
    bitpit::log::cout() << "  - Value obtained for test cell #0 sparse storage matches the one obtained using dense storage." << std::endl;

    if (!bitpit::utils::DoubleFloatingEqual()(sparseValue1, denseValue1)) {
        bitpit::log::cout() << "  - Value obtained for test cell #1 using sparse storage doesn't match the one obtained using dense storage." << std::endl;
        return 1;
    }
    bitpit::log::cout() << "  - Value obtained for test cell #1 sparse storage matches the one obtained using dense storage." << std::endl;

    if (!bitpit::utils::DoubleFloatingEqual()(sparseValue2, denseValue2)) {
        bitpit::log::cout() << "  - Value obtained for test cell #2 using sparse storage doesn't match the one obtained using dense storage." << std::endl;
        return 1;
    }
    bitpit::log::cout() << "  - Value obtained for test cell #2 sparse storage matches the one obtained using dense storage." << std::endl;

    return 0;
}

/*!
* Subtest 003
*
* Testing dense and sparse storage on a Cartesian mesh in light memory mode.
*/
int subtest_002()
{
    bitpit::log::cout() << std::endl;
    bitpit::log::cout() << "Testing dense and sparse storage on an Cartesian mesh in light memory mode" << std::endl;

    // Input geometry
    bitpit::log::cout() << " - Loading geometry" << std::endl;

    std::unique_ptr<bitpit::SurfUnstructured> segmentation = generateSegmentation();
    segmentation->getVTK().setName("geometry_007");
    segmentation->write();

    bitpit::log::cout() << "n. vertex: " << segmentation->getVertexCount() << std::endl;
    bitpit::log::cout() << "n. simplex: " << segmentation->getCellCount() << std::endl;

    // Create the mesh
    bitpit::log::cout() << " - Setting mesh" << std::endl;

    std::unique_ptr<bitpit::VolCartesian> mesh = generateCartesianMesh(*segmentation);
    mesh->switchMemoryMode(bitpit::VolCartesian::MEMORY_LIGHT);

    // Initialize test
    long testCellId0 = 12065;
    long testCellId1 = 13671;
    long testCellId2 = 15495;

    int objectId = 0;

    //
    // Levelset - Sparse storage
    //

    // Initialize levelset
    bitpit::log::cout() << "Filling levelset cache using sprase storage... " << std::endl;
    std::chrono::time_point<std::chrono::system_clock> startSparse = std::chrono::system_clock::now();

    bitpit::LevelSet levelsetSparse(bitpit::LevelSetFillIn::SPARSE);
    levelsetSparse.setMesh(mesh.get());
    levelsetSparse.addObject(segmentation.get(), BITPIT_PI, objectId);

    std::chrono::time_point<std::chrono::system_clock> endSparse = std::chrono::system_clock::now();
    std::chrono::milliseconds elapsedTimeSparse = std::chrono::duration_cast<std::chrono::milliseconds>(endSparse - startSparse);
    bitpit::log::cout() << "Computation completed in " << elapsedTimeSparse.count() << " ms" << std::endl;

    mesh->switchMemoryMode(bitpit::VolCartesian::MEMORY_NORMAL);
    mesh->initializeAdjacencies();
    mesh->update();

    levelsetSparse.getObject(objectId).enableVTKOutput(bitpit::LevelSetWriteField::VALUE);
    mesh->getVTK().setName("levelset_007_cartesian_light_sparse");
    mesh->write();

    mesh->switchMemoryMode(bitpit::VolCartesian::MEMORY_LIGHT);

    double sparseValue0 = levelsetSparse.getObject(objectId).evalCellValue(testCellId0, true);
    double sparseValue1 = levelsetSparse.getObject(objectId).evalCellValue(testCellId1, true);
    double sparseValue2 = levelsetSparse.getObject(objectId).evalCellValue(testCellId2, true);

    bitpit::log::cout() << " Sparse storage mode: levelset on cell " << testCellId0 << " is equal to " << sparseValue0 << std::endl;
    bitpit::log::cout() << " Sparse storage mode: levelset on cell " << testCellId1 << " is equal to " << sparseValue1 << std::endl;
    bitpit::log::cout() << " Sparse storage mode: levelset on cell " << testCellId2 << " is equal to " << sparseValue2 << std::endl;

    //
    // Levelset - Dense storage
    //

    // Initialize levelset
    bitpit::log::cout() << "Filling levelset cache using dense storage... " << std::endl;
    std::chrono::time_point<std::chrono::system_clock> startDense = std::chrono::system_clock::now();

    bitpit::LevelSet levelsetDense(bitpit::LevelSetFillIn::DENSE);
    levelsetDense.setMesh(mesh.get());
    levelsetDense.addObject(segmentation.get(), BITPIT_PI, objectId);

    std::chrono::time_point<std::chrono::system_clock> endDense = std::chrono::system_clock::now();
    std::chrono::milliseconds elapsedTimeDense = std::chrono::duration_cast<std::chrono::milliseconds>(endDense - startDense);
    bitpit::log::cout() << "Computation completed in " << elapsedTimeDense.count() << " ms" << std::endl;

    mesh->switchMemoryMode(bitpit::VolCartesian::MEMORY_NORMAL);
    mesh->initializeAdjacencies();
    mesh->update();

    levelsetDense.getObject(objectId).enableVTKOutput(bitpit::LevelSetWriteField::VALUE);
    mesh->getVTK().setName("levelset_007_cartesian_light_dense");
    mesh->write();

    mesh->switchMemoryMode(bitpit::VolCartesian::MEMORY_LIGHT);

    double denseValue0 = levelsetDense.getObject(objectId).evalCellValue(testCellId0, true);
    double denseValue1 = levelsetDense.getObject(objectId).evalCellValue(testCellId1, true);
    double denseValue2 = levelsetDense.getObject(objectId).evalCellValue(testCellId2, true);

    bitpit::log::cout() << " Dense storage mode: levelset on cell " << testCellId0 << " is equal to " << denseValue0 << std::endl;
    bitpit::log::cout() << " Dense storage mode: levelset on cell " << testCellId1 << " is equal to " << denseValue1 << std::endl;
    bitpit::log::cout() << " Dense storage mode: levelset on cell " << testCellId2 << " is equal to " << denseValue2 << std::endl;

    //
    // Comparison
    //

    bitpit::log::cout() << " Checking levelset values" << std::endl;

    if (!bitpit::utils::DoubleFloatingEqual()(sparseValue0, denseValue0)) {
        bitpit::log::cout() << "  - Value obtained for test cell #0 using sparse storage doesn't match the one obtained using dense storage." << std::endl;
        return 1;
    }
    bitpit::log::cout() << "  - Value obtained for test cell #0 sparse storage matches the one obtained using dense storage." << std::endl;

    if (!bitpit::utils::DoubleFloatingEqual()(sparseValue1, denseValue1)) {
        bitpit::log::cout() << "  - Value obtained for test cell #1 using sparse storage doesn't match the one obtained using dense storage." << std::endl;
        return 1;
    }
    bitpit::log::cout() << "  - Value obtained for test cell #1 sparse storage matches the one obtained using dense storage." << std::endl;

    if (!bitpit::utils::DoubleFloatingEqual()(sparseValue2, denseValue2)) {
        bitpit::log::cout() << "  - Value obtained for test cell #2 using sparse storage doesn't match the one obtained using dense storage." << std::endl;
        return 1;
    }
    bitpit::log::cout() << "  - Value obtained for test cell #2 sparse storage matches the one obtained using dense storage." << std::endl;

    return 0;
}

/*!
* Subtest 003
*
* Testing dense and sparse storage on an Octree mesh.
*/
int subtest_003()
{
    bitpit::log::cout() << std::endl;
    bitpit::log::cout() << "Testing dense and sparse storage on an Octree mesh" << std::endl;

    // Input geometry
    bitpit::log::cout() << " - Loading geometry" << std::endl;

    std::unique_ptr<bitpit::SurfUnstructured> segmentation = generateSegmentation();
    segmentation->getVTK().setName("geometry_007");
    segmentation->write();

    bitpit::log::cout() << "n. vertex: " << segmentation->getVertexCount() << std::endl;
    bitpit::log::cout() << "n. simplex: " << segmentation->getCellCount() << std::endl;

    // Create the mesh
    bitpit::log::cout() << " - Setting mesh" << std::endl;

    std::unique_ptr<bitpit::VolOctree> mesh = generateOctreeMesh(*segmentation);
    mesh->initializeAdjacencies();
    mesh->update();

    // Initialize test
    long testCellId0 = 9873;
    long testCellId1 = 15517;
    long testCellId2 = 10905;

    int objectId = 0;

    //
    // Levelset - Sparse storage
    //

    // Initialize levelset
    bitpit::log::cout() << "Filling levelset cache using sprase storage... " << std::endl;
    std::chrono::time_point<std::chrono::system_clock> startSparse = std::chrono::system_clock::now();

    bitpit::LevelSet levelsetSparse(bitpit::LevelSetFillIn::SPARSE);
    levelsetSparse.setMesh(mesh.get());
    levelsetSparse.addObject(segmentation.get(), BITPIT_PI, objectId);

    bitpit::LevelSetObject &sparseObject = levelsetSparse.getObject(objectId);
    sparseObject.enableFieldCellCache(bitpit::LevelSetField::SIGN, bitpit::LevelSetCacheMode::FULL);
    sparseObject.enableFieldCellCache(bitpit::LevelSetField::VALUE, bitpit::LevelSetCacheMode::NARROW_BAND);

    std::chrono::time_point<std::chrono::system_clock> endSparse = std::chrono::system_clock::now();
    std::chrono::milliseconds elapsedTimeSparse = std::chrono::duration_cast<std::chrono::milliseconds>(endSparse - startSparse);
    bitpit::log::cout() << "Computation completed in " << elapsedTimeSparse.count() << " ms" << std::endl;

    levelsetSparse.getObject(objectId).enableVTKOutput(bitpit::LevelSetWriteField::VALUE);
    mesh->getVTK().setName("levelset_007_octree_sparse");
    mesh->write();

    double sparseValue0 = levelsetSparse.getObject(objectId).evalCellValue(testCellId0, true);
    double sparseValue1 = levelsetSparse.getObject(objectId).evalCellValue(testCellId1, true);
    double sparseValue2 = levelsetSparse.getObject(objectId).evalCellValue(testCellId2, true);

    bitpit::log::cout() << " Sparse storage mode: levelset on cell " << testCellId0 << " is equal to " << sparseValue0 << std::endl;
    bitpit::log::cout() << " Sparse storage mode: levelset on cell " << testCellId1 << " is equal to " << sparseValue1 << std::endl;
    bitpit::log::cout() << " Sparse storage mode: levelset on cell " << testCellId2 << " is equal to " << sparseValue2 << std::endl;

    //
    // Levelset - Dense storage
    //

    // Initialize levelset
    bitpit::log::cout() << "Filling levelset cache using dense storage... " << std::endl;
    std::chrono::time_point<std::chrono::system_clock> startDense = std::chrono::system_clock::now();

    bitpit::LevelSet levelsetDense(bitpit::LevelSetFillIn::DENSE);
    levelsetDense.setMesh(mesh.get());
    levelsetDense.addObject(segmentation.get(), BITPIT_PI, objectId);

    bitpit::LevelSetObject &denseObject = levelsetSparse.getObject(objectId);
    denseObject.enableFieldCellCache(bitpit::LevelSetField::SIGN, bitpit::LevelSetCacheMode::FULL);
    denseObject.enableFieldCellCache(bitpit::LevelSetField::VALUE, bitpit::LevelSetCacheMode::NARROW_BAND);

    std::chrono::time_point<std::chrono::system_clock> endDense = std::chrono::system_clock::now();
    std::chrono::milliseconds elapsedTimeDense = std::chrono::duration_cast<std::chrono::milliseconds>(endDense - startDense);
    bitpit::log::cout() << "Computation completed in " << elapsedTimeDense.count() << " ms" << std::endl;

    levelsetDense.getObject(objectId).enableVTKOutput(bitpit::LevelSetWriteField::VALUE);
    mesh->getVTK().setName("levelset_007_octree_dense");
    mesh->write();

    double denseValue0 = levelsetDense.getObject(objectId).evalCellValue(testCellId0, true);
    double denseValue1 = levelsetDense.getObject(objectId).evalCellValue(testCellId1, true);
    double denseValue2 = levelsetDense.getObject(objectId).evalCellValue(testCellId2, true);

    bitpit::log::cout() << " Dense storage mode: levelset on cell " << testCellId0 << " is equal to " << denseValue0 << std::endl;
    bitpit::log::cout() << " Dense storage mode: levelset on cell " << testCellId1 << " is equal to " << denseValue1 << std::endl;
    bitpit::log::cout() << " Dense storage mode: levelset on cell " << testCellId2 << " is equal to " << denseValue2 << std::endl;

    //
    // Comparison
    //

    bitpit::log::cout() << " Checking levelset values" << std::endl;

    if (!bitpit::utils::DoubleFloatingEqual()(sparseValue0, denseValue0)) {
        bitpit::log::cout() << "  - Value obtained for test cell #0 using sparse storage doesn't match the one obtained using dense storage." << std::endl;
        return 1;
    }
    bitpit::log::cout() << "  - Value obtained for test cell #0 sparse storage matches the one obtained using dense storage." << std::endl;

    if (!bitpit::utils::DoubleFloatingEqual()(sparseValue1, denseValue1)) {
        bitpit::log::cout() << "  - Value obtained for test cell #0 using sparse storage doesn't match the one obtained using dense storage." << std::endl;
        return 1;
    }
    bitpit::log::cout() << "  - Value obtained for test cell #1 sparse storage matches the one obtained using dense storage." << std::endl;

    if (!bitpit::utils::DoubleFloatingEqual()(sparseValue2, denseValue2)) {
        bitpit::log::cout() << "  - Value obtained for test cell #2 using sparse storage doesn't match the one obtained using dense storage." << std::endl;
        return 1;
    }
    bitpit::log::cout() << "  - Value obtained for test cell #2 sparse storage matches the one obtained using dense storage." << std::endl;

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
    bitpit::log::manager().initialize(bitpit::log::COMBINED);

    // Run the subtests
    bitpit::log::cout() << "Testing storage types" << std::endl;

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

        status = subtest_003();
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
