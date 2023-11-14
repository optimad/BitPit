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

# ifndef __BITPIT_LEVELSET_SEGMENTATION_OBJECT_TPP__
# define __BITPIT_LEVELSET_SEGMENTATION_OBJECT_TPP__

namespace bitpit {

/*!
	@ingroup levelset
	@interface LevelSetSegmentationNarrowBandCacheBase
	@brief Base class for defining segmentation narrow band caches.
*/

/*!
 * Constructor.
 */
template<typename storage_manager_t>
LevelSetSegmentationNarrowBandCacheBase<storage_manager_t>::LevelSetSegmentationNarrowBandCacheBase() : LevelSetNarrowBandCacheBase<storage_manager_t>() {

}

/*!
 * Set the specified cache entry.
 *
 * \param itr is an iterator pointing to the narrow band entry
 * \param value is the levelset value
 * \param gradient is the levelset gradient
 * \param supportId is the support id
 * \param normal is the surface normal at the projection point
 */
template<typename storage_manager_t>
void LevelSetSegmentationNarrowBandCacheBase<storage_manager_t>::set(const KernelIterator &itr, double value, const std::array<double, 3> &gradient, long supportId, const std::array<double, 3> &surfaceNormal) {

    LevelSetNarrowBandCacheBase<storage_manager_t>::set(itr, value, gradient);

    long &cachedSupportId = getSupportId(itr);
    cachedSupportId = supportId;

    std::array<double, 3> &cachedSurfaceNormal = getSurfaceNormal(itr);
    cachedSurfaceNormal = surfaceNormal;

}

/*!
 * Exchanges the content of the cache with the content the specified other
 * cache.
 *
 * \param other is another cache whose content is swapped with that of this
 * cache
 */
template<typename storage_manager_t>
void LevelSetSegmentationNarrowBandCacheBase<storage_manager_t>::swap(LevelSetSegmentationNarrowBandCacheBase<storage_manager_t> &other) noexcept {

    LevelSetNarrowBandCacheBase<storage_manager_t>::swap(other);
}

/*!
	@ingroup levelset
	@interface LevelSetNarrowBandCache
	@brief Narrow band cache.
*/

/*!
	@class      LevelSetSegmentation
	@ingroup    levelset
	@brief      Implements visitor pattern fo segmentated geometries
*/

/*!
 * Constructor
 * @param[in] id identifier of object
 */
template<typename narrow_band_cache_t>
LevelSetSegmentationObject<narrow_band_cache_t>::LevelSetSegmentationObject(int id)
    : LevelSetSegmentationKernel(),
      LevelSetCachedObject<narrow_band_cache_t>(id),
      m_narrowBandSize(levelSetDefaults::NARROWBAND_SIZE)
{
}

/*!
 * Constructor
 * @param[in] id identifier of object
 * @param[in] STL unique pointer to surface mesh
 * @param[in] featureAngle feature angle. If the angle between two segments is bigger than this angle, the enclosed edge is considered as a sharp edge
 */
template<typename narrow_band_cache_t>
LevelSetSegmentationObject<narrow_band_cache_t>::LevelSetSegmentationObject( int id, std::unique_ptr<const SurfUnstructured> &&STL, double featureAngle)
    : LevelSetSegmentationKernel(std::move(STL), featureAngle),
      LevelSetCachedObject<narrow_band_cache_t>(id),
      m_narrowBandSize(levelSetDefaults::NARROWBAND_SIZE)
{
}

/*!
 * Constructor
 * @param[in] id identifier of object
 * @param[in] STL pointer to surface mesh
 * @param[in] featureAngle feature angle; if the angle between two segments is bigger than this angle, the enclosed edge is considered as a sharp edge.
 */
template<typename narrow_band_cache_t>
LevelSetSegmentationObject<narrow_band_cache_t>::LevelSetSegmentationObject( int id, const SurfUnstructured *STL, double featureAngle)
    : LevelSetSegmentationKernel(STL, featureAngle),
      LevelSetCachedObject<narrow_band_cache_t>(id),
      m_narrowBandSize(levelSetDefaults::NARROWBAND_SIZE)
{
}

/*!
 * Clones the object
 * @return pointer to cloned object
 */
template<typename narrow_band_cache_t>
LevelSetSegmentationObject<narrow_band_cache_t> * LevelSetSegmentationObject<narrow_band_cache_t>::clone() const {
    return new LevelSetSegmentationObject( *this );
}

/*!
 * Get the list of supported field.
 * @result The list of supported field.
 */
template<typename narrow_band_cache_t>
LevelSetFieldset LevelSetSegmentationObject<narrow_band_cache_t>::getSupportedFields() const {

    LevelSetFieldset supportedFields = LevelSetCachedObject<narrow_band_cache_t>::getSupportedFields();
    supportedFields.insert(LevelSetField::PART);
    supportedFields.insert(LevelSetField::NORMAL);

    return supportedFields;

}

/*!
 * Gets the closest support within the narrow band of cell
 * @param[in] id index of cell
 * @return closest segment in narrow band
 */
template<typename narrow_band_cache_t>
int LevelSetSegmentationObject<narrow_band_cache_t>::getPart( long id ) const{

    long supportId = getSupport(id);

    if( supportId != levelSetDefaults::SUPPORT){
        const SurfUnstructured &m_surface = getSurface();
        return m_surface.getCell(supportId).getPID();
    } else {
        return levelSetDefaults::PART ;
    }

}

/*!
 * Gets the surface normal at the projection point
 * @param[in] id index of cell
 * @return surface normal
 */
template<typename narrow_band_cache_t>
std::array<double,3> LevelSetSegmentationObject<narrow_band_cache_t>::getNormal( long id ) const{

    const narrow_band_cache_t *narrowBandCache = this->getNarrowBandCache();
    typename narrow_band_cache_t::KernelIterator narrowBandCacheItr = narrowBandCache->find(id) ;
    if( narrowBandCacheItr != narrowBandCache->end() ){
        return narrowBandCache->getSurfaceNormal(narrowBandCacheItr);
    }

    return levelSetDefaults::GRADIENT ;

}


/*!
 * Gets the closest support within the narrow band of cell
 * @param[in] id index of cell
 * @return closest segment in narrow band
 */
template<typename narrow_band_cache_t>
long LevelSetSegmentationObject<narrow_band_cache_t>::getSupport( long id ) const{

    const narrow_band_cache_t *narrowBandCache = this->getNarrowBandCache();
    typename narrow_band_cache_t::KernelIterator narrowBandCacheItr = narrowBandCache->find(id) ;
    if( narrowBandCacheItr != narrowBandCache->end() ){
        return narrowBandCache->getSupportId(narrowBandCacheItr);
    }

    return levelSetDefaults::SUPPORT ;

}

/*!
 * Get size of support triangle
 * @param[in] id cell id
 * @return characteristic size of support triangle
 */
template<typename narrow_band_cache_t>
double LevelSetSegmentationObject<narrow_band_cache_t>::getSurfaceFeatureSize( long id ) const {

    long support = getSupport(id);
    if (support == levelSetDefaults::SUPPORT) {
        return (- levelSetDefaults::SIZE);
    }

    return getSegmentSize(support);
}

/*!
 * Get the smallest characteristic size within the triangulation
 * @return smallest characteristic size within the triangulation
 */template<typename narrow_band_cache_t>
double LevelSetSegmentationObject<narrow_band_cache_t>::getMinSurfaceFeatureSize( ) const {

    return getMinSegmentSize();
}

/*!
 * Get the largest characteristic size within the triangulation
 * @return largest characteristic size within the triangulation
 */
template<typename narrow_band_cache_t>
double LevelSetSegmentationObject<narrow_band_cache_t>::getMaxSurfaceFeatureSize( ) const {

    return getMaxSegmentSize();
}

/*!
 * Computes axis aligned global bounding box of object
 * @param[out] minP minimum point
 * @param[out] maxP maximum point
 */
template<typename narrow_band_cache_t>
void LevelSetSegmentationObject<narrow_band_cache_t>::getBoundingBox( std::array<double,3> &minP, std::array<double,3> &maxP ) const {
    const SurfUnstructured &m_surface = getSurface();
    m_surface.getBoundingBox(minP,maxP) ;
}

#if BITPIT_ENABLE_MPI
/*!
 * Computes axis aligned bounding box of object
 *
 * The current process may only have the portion of the object needed for
 * evaluating the levelset on the interior cells, this function allows to
 * evaluate the overall bounding box across all process.
 *
 * @param[out] minP minimum point
 * @param[out] maxP maximum point
 */
template<typename narrow_band_cache_t>
void LevelSetSegmentationObject<narrow_band_cache_t>::getGlobalBoundingBox( std::array<double,3> &minP, std::array<double,3> &maxP ) const {
    getBoundingBox(minP, maxP);

    if (this->m_kernel->getMesh()->isPartitioned()) {
        MPI_Comm communicator = this->m_kernel->getCommunicator();

        MPI_Allreduce(MPI_IN_PLACE, minP.data(), 3, MPI_DOUBLE, MPI_MIN, communicator);
        MPI_Allreduce(MPI_IN_PLACE, maxP.data(), 3, MPI_DOUBLE, MPI_MAX, communicator);
    }
}
#endif

/*!
 * Computes the levelset function within the narrow band
 * @param[in] signd if signed- or unsigned- distance function should be calculated
 * @param[in] narrowBandSize size of the narrow band
 */
template<typename narrow_band_cache_t>
void LevelSetSegmentationObject<narrow_band_cache_t>::computeNarrowBand(bool signd, double narrowBandSize){

    log::cout() << "Computing levelset within the narrow band... " << std::endl;

    // Set the size of the narrowband
    m_narrowBandSize = narrowBandSize;

    // Cartesian patches are handled separately
    if( LevelSetCartesianKernel* lsCartesian = dynamic_cast<LevelSetCartesianKernel*>(this->m_kernel) ){
        computeNarrowBand( lsCartesian, signd) ;
        return ;
    }

    // All other patches are handled with the same method.
    computeNarrowBand(this->m_kernel, signd);

}

/*!
 * Updates the narrow band levelset function of the specified cells.
 * @param[in] cellIds are the ids of the cells that will be updated
 * @param[in] signd if signed- or unsigned- distance function should be calculated
 */
template<typename narrow_band_cache_t>
void LevelSetSegmentationObject<narrow_band_cache_t>::updateNarrowBand( const std::vector<long> &cellIds, bool signd){

    log::cout() << "Updating levelset within the narrow band... " << std::endl;

    // Cartesian patches are handled separately
    //
    // Update is not implemented for Cartesian patches, the levelset is cleared and
    // rebuild from scratch.
    if( LevelSetCartesianKernel* lsCartesian= dynamic_cast<LevelSetCartesianKernel*>(this->m_kernel) ){
        this->clear( ) ;
        computeNarrowBand( lsCartesian, signd) ;
        return;
    }

    // All other patches are handled with the same method
    updateNarrowBand(this->m_kernel, cellIds, signd);

}

/*!
 * Computes the levelset within the narrow band on an cartesian grid.
 * The levelset can be computed also when the patch is in memory-light mode.
 * If the size of the narrow band has been set, the method will compute the
 * levelset values only of those cells within the threshold.
 * In case the size of the narrow band has not been set, levelset will be
 * evaluated only on the cells that intersect the surface and on all their
 * first neighbours.
 * @param[in] levelsetKernel the octree LevelSetKernel
 * @param[in] signd whether signed distance should be calculated
 */
template<typename narrow_band_cache_t>
void LevelSetSegmentationObject<narrow_band_cache_t>::computeNarrowBand( LevelSetCartesianKernel *levelsetKernel, bool signd){

    // Get mesh information
    const VolCartesian &mesh = *(levelsetKernel->getMesh() ) ;
    int meshDimension = mesh.getDimension();
    VolCartesian::MemoryMode meshMemoryMode = mesh.getMemoryMode();

    ElementType meshCellType = mesh.getCellType();
    const ReferenceElementInfo &meshCellTypeInfo = ReferenceElementInfo::getInfo(meshCellType);
    int meshCellFaceCount = meshCellTypeInfo.nFaces;

    std::array<double,3> meshMinPoint;
    std::array<double,3> meshMaxPoint;
    mesh.getBoundingBox(meshMinPoint, meshMaxPoint) ;

    // Get surface information
    const SurfUnstructured &surface = getSurface();

    // Define search radius
    //
    // Search radius should be equal to the maximum between the narrow band
    // size and the radius of the bounding sphere. This guarantees that, when
    // the narrow band size is equal or less than zero, the levelset will be
    // evaluated on the cells that intersect the surface and on all their
    // first neighbours.
    double searchRadius = std::max(this->m_narrowBandSize, 2 * levelsetKernel->getCellBoundingRadius());

    // Initialize process list
    //
    // Process list is initialized with cells that are certainly inside the
    // narrow band. Those cells are the one that contain the vertices of the
    // segments or the intersection between the segments and the bounding box
    // of the patch.
    std::unordered_set<long> processList;

    std::vector<std::array<double,3>> intersectionPoints;
    std::vector<std::array<double,3>> segmentVertexCoords;
    for (const Cell &segment : surface.getCells()) {
        // Get segment info
        //
        // Since vertex information will be passed to the CG module, we need to
        // get the vertices in counter-clockwise order.
        ConstProxyVector<long> segmentVertexIds = surface.getFacetOrderedVertexIds(segment);
        std::size_t nSegmentVertices = segmentVertexIds.size();

        // Get segment coordinates
        segmentVertexCoords.resize(nSegmentVertices);
        surface.getVertexCoords(nSegmentVertices, segmentVertexIds.data(), segmentVertexCoords.data());

        // Add to the process list the cells that contain the vertices of the
        // segment or the intersection between the segment and the bounding box
        // of the patch.
        int nInnerVertices = 0;
        for (const std::array<double,3> &vertexPoint : segmentVertexCoords) {
            long cellId = mesh.locatePoint(vertexPoint);
            if (cellId < 0) {
                continue;
            }

            processList.insert(cellId);
            ++nInnerVertices;
        }

        if (nInnerVertices == 0) {
            if (CGElem::intersectBoxPolygon(meshMinPoint, meshMaxPoint, segmentVertexCoords, false, true, true, intersectionPoints, meshDimension)) {
                for (const std::array<double,3> &intersectionPoint : intersectionPoints){
                    long cellId = mesh.locateClosestCell(intersectionPoint);
                    processList.insert(cellId);
                }
            }
        }
    }

    // Evaluate the levelset within the narrow band
    //
    // The initial process list is gradually expanded considering all the
    // neighbours with a distance less than the search radius.
    narrow_band_cache_t *narrowBandCache = this->getNarrowBandCache();

    std::unordered_set<long> outsideNarrowBand;
    while (!processList.empty()) {
        // Get the cell to process
        long cellId = *(processList.begin());
        processList.erase(processList.begin());

        // Find segment associated to the cell
        std::array<double,3> cellCentroid = levelsetKernel->computeCellCentroid(cellId);

        long segmentId;
        double distance;
        getSearchTree().findPointClosestCell(cellCentroid, searchRadius, &segmentId, &distance);
        if(segmentId < 0){
            outsideNarrowBand.insert(cellId);
            continue;
        }

        // Evaluate levelset information
        std::array<double, 3> gradient;
        std::array<double, 3> normal;
        int error = getSegmentInfo(cellCentroid, segmentId, signd, distance, gradient, normal);
        if (error) {
            throw std::runtime_error ("Unable to extract the levelset information from segment " + std::to_string(segmentId) + ".");
        }


        typename narrow_band_cache_t::KernelIterator narrowBandCacheItr = narrowBandCache->insert(cellId, true) ;
        narrowBandCache->set(narrowBandCacheItr, distance, gradient, segmentId, normal);

        // Add cell neighbours to the process list
        if (meshMemoryMode == VolCartesian::MEMORY_LIGHT) {
            for (int face = 0; face < meshCellFaceCount; ++face) {
                long neighId = mesh.getCellFaceNeighsLinearId(cellId, face);
                if (neighId >= 0) {
                    if (!this->isInNarrowBand(neighId) && (outsideNarrowBand.count(neighId) == 0)) {
                        processList.insert(neighId);
                    }
                }
            }
        } else {
            const Cell &cell = mesh.getCell(cellId);
            const long *neighbours = cell.getAdjacencies() ;
            int nNeighbours = cell.getAdjacencyCount() ;
            for (int n = 0; n < nNeighbours; ++n) {
                long neighId = neighbours[n];
                if (!this->isInNarrowBand(neighId) && (outsideNarrowBand.count(neighId) == 0)) {
                    processList.insert(neighId);
                }
            }
        }
    }
}

/*!
 * Computes the levelset within the narrow band.
 * If the size of the narrow band has been set, the method will compute the
 * levelset values on the cells that intersect the surface, on all their
 * first neighbours and on the cells with a distance from the surface less
 * than the threshold.
 * In case the size of the narrow band has not been set, levelset will be
 * evaluated only on the cells that intersect the surface and on all their
 * first neighbours.
 * \param[in] levelsetKernel the levelset mesh kernel
 * \param[in] signd whether signed distance should be calculated
 */
template<typename narrow_band_cache_t>
void LevelSetSegmentationObject<narrow_band_cache_t>::computeNarrowBand( LevelSetKernel *levelsetKernel, bool signd){

    // Get mesh information
    const VolumeKernel &mesh = *(levelsetKernel->getMesh()) ;

    // Get narrowband information
    narrow_band_cache_t *narrowBandCache = this->getNarrowBandCache();

    // Evaluate levelset information for intersected cells
    VolumeKernel::CellConstIterator cellBegin = mesh.cellConstBegin();
    VolumeKernel::CellConstIterator cellEnd   = mesh.cellConstEnd();

    std::unordered_set<std::size_t> intersectedRawCellIds;
    for (VolumeKernel::CellConstIterator cellItr = cellBegin; cellItr != cellEnd; ++cellItr) {
        // Identify the segment associated with the cell
        //
        // The search radius is evaluated as the maximum value between the
        // narroband size and the distance above which the cell will surely
        // not intersect the surface. In this way, cells that intersect the
        // surface are always included in the narrow band, even if their
        // distance from the surface is greater than the narrow band size
        // explicitly set by the user.
        //
        // If no segment is identified the cell is not processed.
        long cellId = cellItr.getId();
        std::array<double,3> cellCentroid = levelsetKernel->computeCellCentroid(cellId);
        double cellBoundingRadius = levelsetKernel->computeCellBoundingRadius(cellId);

        double searchRadius = std::max(this->m_narrowBandSize, cellBoundingRadius);

        long segmentId;
        double distance;
        getSearchTree().findPointClosestCell(cellCentroid, searchRadius, &segmentId, &distance);
        if(segmentId < 0){
            continue;
        }

        // Evaluate levelset information
        std::array<double,3> gradient;
        std::array<double,3> normal;
        int error = getSegmentInfo(cellCentroid, segmentId, signd, distance, gradient, normal);
        if (error) {
            throw std::runtime_error ("Unable to extract the levelset information from segment " + std::to_string(segmentId) + ".");
        }

        typename narrow_band_cache_t::KernelIterator narrowBandCacheItr = narrowBandCache->insert(cellId, true) ;
        narrowBandCache->set(narrowBandCacheItr, distance, gradient, segmentId, normal);

        // Update the list of cells that intersects the surface
        //
        // When the narrowband size is not explicitly set, the cell will always
        // intersects the surface because only cells that intersect the surface
        // are considered, otherwise we need to check if the absolute distance
        // associated with the cell is lower than the intersection distance.
        if (this->m_narrowBandSize < 0 || cellBoundingRadius < std::abs(distance)) {
            std::size_t cellRawId = cellItr.getRawIndex();
            intersectedRawCellIds.insert(cellRawId);
        }

    }

    // Process the neighbours of the cells that intersect the surface
    //
    // If a cell intersects the surface, we need to evaluate the levelset
    // of all its neigbours.
    for (std::size_t cellRawId : intersectedRawCellIds) {
        // Compute cell projection point
        VolumeKernel::CellConstIterator cellItr = mesh.getCells().rawFind(cellRawId);
        std::array<double,3> cellProjectionPoint = this->computeProjectionPoint(cellItr.getId());

        // Process cell adjacencies
        const long *neighbours = cellItr->getAdjacencies() ;
        int nNeighbours = cellItr->getAdjacencyCount() ;
        for (int n = 0; n < nNeighbours; ++n) {
            // Skip the neighbour if it has already been processed
            //
            // The neighbour may already have been processed either because
            // it distance from the segmentation is within the search radius,
            // or because is a neighbour of an intersected cells already
            // processed.
            long neighId = neighbours[n];
            if( narrowBandCache->contains(neighId) ){
                continue;
            }

            // Identify the segment associated with the neighbour
            std::array<double,3> neighCentroid = levelsetKernel->computeCellCentroid(neighId);

            double searchRadius = 1.05 * norm2(neighCentroid - cellProjectionPoint);

            long segmentId;
            double distance;
            getSearchTree().findPointClosestCell(neighCentroid, searchRadius, &segmentId, &distance);
            if (segmentId < 0) {
                assert(false && "Should not pass here");
            }

            // Evaluate negihbour leveset information
            std::array<double,3> gradient;
            std::array<double,3> normal;
            int error = getSegmentInfo(neighCentroid, segmentId, signd, distance, gradient, normal);
            if (error) {
                throw std::runtime_error ("Unable to extract the levelset information from segment " + std::to_string(segmentId) + ".");
            }

            typename narrow_band_cache_t::KernelIterator narrowBandCacheItr = narrowBandCache->insert(neighId, true) ;
            narrowBandCache->set(narrowBandCacheItr, distance, gradient, segmentId, normal);
        }
    }
}

/*!
 * Updates the narrow band levelset function of the specified cells.
 * If the size of the narrow band has been set, the method will compute the
 * levelset values on the cells that intersect the surface, on all their
 * first neighbours and on the cells with a distance from the surface less
 * than the threshold.
 * In case the size of the narrow band has not been set, levelset will be
 * evaluated only on the cells that intersect the surface and on all their
 * first neighbours.
 * @param[in] levelsetKernel the octree LevelSetKernel
 * @param[in] cellIds are the ids of the cells that will be updated
 * @param[in] signd whether signed distance should be calculated
 */
template<typename narrow_band_cache_t>
void LevelSetSegmentationObject<narrow_band_cache_t>::updateNarrowBand( LevelSetKernel *levelsetKernel, const std::vector<long> &cellIds, bool signd){

    VolumeKernel &mesh = *(levelsetKernel->getMesh()) ;
    narrow_band_cache_t *narrowBandCache = this->getNarrowBandCache();

    std::vector<long> cellsOutsideNarrowband;

    // Evaluate the levelset of the cells
    //
    // When searching for the segment associated to a cell, the search radius
    // is evaluated as the maximum value between the narroband size and the
    // distance above which the cell will surely not intersect the surface.
    for( long cellId : cellIds ){

        // Identify the segment associated with the cell
        //
        // The search radius is evaluated as the maximum value between the
        // narroband size and the distance above which the cell will surely
        // not intersect the surface. In this way, cells that intersect the
        // surface are always included in the narrow band, even if their
        // distance from the surface is greater than the narrow band size
        // explicitly set by the user.
        //
        // If no segment is identified the cell is not processed.
        std::array<double,3> centroid = levelsetKernel->computeCellCentroid(cellId);

        double searchRadius = std::max(this->m_narrowBandSize, levelsetKernel->computeCellBoundingRadius(cellId));

        long segmentId;
        double distance;
        getSearchTree().findPointClosestCell(centroid, searchRadius, &segmentId, &distance);
        if (segmentId < 0) {
            cellsOutsideNarrowband.push_back(cellId);
            continue;
        }

        // Evaluate levelset information
        std::array<double,3> gradient;
        std::array<double,3> normal;
        int error = getSegmentInfo(centroid, segmentId, signd, distance, gradient, normal);
        if (error) {
            throw std::runtime_error ("Unable to extract the levelset information from segment " + std::to_string(segmentId) + ".");
        }

        typename narrow_band_cache_t::KernelIterator narrowBandCacheItr = narrowBandCache->insert(cellId, true) ;
        narrowBandCache->set(narrowBandCacheItr, distance, gradient, segmentId, normal);
    }

    // Cells with neighbours that intersect the surface need to be added to
    // the narrowband even if they don't intersect the surface themself or
    // have a distance from the surface greater than the narroband size.
    for( long cellId : cellsOutsideNarrowband){
        const Cell &cell = mesh.getCell(cellId);

        // Consider only cells with a neighbour that intersects the surface
        //
        // Care must be take to use only information from cells inside the
        // narrow band, that's because values outside the narrowband are not
        // up-to-date at this stage.
        const long *neighbours = cell.getAdjacencies() ;
        int nNeighbours = cell.getAdjacencyCount() ;

        long intersectedNeighId = Cell::NULL_ID;
        for (int n = 0; n < nNeighbours; ++n) {
            long neighId = neighbours[n];
            if (!this->isInNarrowBand(neighId)) {
                continue;
            }

            if( this->intersectSurface(neighId,LevelSetIntersectionMode::FAST_GUARANTEE_FALSE) == LevelSetIntersectionStatus::TRUE){
                intersectedNeighId = neighId;
                break;
            }
        }

        if (intersectedNeighId == Cell::NULL_ID) {
            continue;
        }

        // Identify the segment associated with the cell
        std::array<double,3> cellCentroid = levelsetKernel->computeCellCentroid(cellId);
        std::array<double,3> neighProjectionPoint = this->computeProjectionPoint(intersectedNeighId);

        double searchRadius = 1.05 * norm2(cellCentroid - neighProjectionPoint);

        long segmentId;
        double distance;
        getSearchTree().findPointClosestCell(cellCentroid, searchRadius, &segmentId, &distance);
        if (segmentId < 0) {
            assert(false && "Should not pass here");
            continue;
        }

        // Evaluate levelset information for the cell
        std::array<double,3> gradient;
        std::array<double,3> normal;
        int error = getSegmentInfo(cellCentroid, segmentId, signd, distance, gradient, normal);
        if (error) {
            throw std::runtime_error ("Unable to extract the levelset information from segment " + std::to_string(segmentId) + ".");
        }

        typename narrow_band_cache_t::KernelIterator narrowBandCacheItr = narrowBandCache->insert(cellId, true) ;
        narrowBandCache->set(narrowBandCacheItr, distance, gradient, segmentId, normal);
    }
}

/*!
 * Computes the LevelSetInfo of a point
 * \param[in] coords coordinates of the point
 * \return the LevelSetInfo
 */
template<typename narrow_band_cache_t>
LevelSetInfo LevelSetSegmentationObject<narrow_band_cache_t>::computeLevelSetInfo(const std::array<double,3> &coords) const {

    long segmentId;
    double distance;
    std::array<double,3> gradient;
    std::array<double,3> normal;

    getSearchTree().findPointClosestCell(coords, &segmentId, &distance);

    int error = getSegmentInfo(coords, segmentId, false, distance, gradient, normal);
    if (error) {
        throw std::runtime_error ("Unable to extract the levelset information from segment " + std::to_string(segmentId) + ".");
    }

    return LevelSetInfo(distance,gradient);

}

/*!
 * Add the VTK data associated with the specified field.
 *
 * @param[in] field is the field
 * @param[in] objectName is the name that will be associated with the object
 */
template<typename narrow_band_cache_t>
void LevelSetSegmentationObject<narrow_band_cache_t>::addVTKOutputData( LevelSetField field, const std::string &objectName) {

    VTK &vtkWriter = this->m_kernel->getMesh()->getVTK() ;
    std::string name = this->getVTKOutputDataName(field, objectName);

    switch(field) {

        case LevelSetField::NORMAL:
            vtkWriter.addData<double>( name, VTKFieldType::VECTOR, VTKLocation::CELL, this);
            break;

        case LevelSetField::PART:
            vtkWriter.addData<int>( name, VTKFieldType::SCALAR, VTKLocation::CELL, this);
            break;

        default:
            LevelSetCachedObject<narrow_band_cache_t>::addVTKOutputData(field, objectName);
            break;

    }

}

/*!
 * Get the name that will be used by the VTK writer for the specifed field.
 *
 * @param[in] field is the field
 * @result The name that will be used by the VTK writer for the specifed field.
 */
template<typename narrow_band_cache_t>
std::string LevelSetSegmentationObject<narrow_band_cache_t>::getVTKOutputFieldName( LevelSetField field) const {

    switch(field) {

        case LevelSetField::NORMAL:
            return "Normal";

        case LevelSetField::PART:
            return "PartId";

        default:
            return LevelSetCachedObject<narrow_band_cache_t>::getVTKOutputFieldName(field);

    }

}

/*!
 * Write the specified field to the given stream.
 *
 * @param[in] field is the field that will be written
 * @param[in] stream output stream
 * @param[in] format is the format which must be used. Supported options
 * are "ascii" or "appended". For "appended" type an unformatted binary
 * stream must be used
 */
template<typename narrow_band_cache_t>
void LevelSetSegmentationObject<narrow_band_cache_t>::flushVTKOutputData(LevelSetField field, std::fstream &stream, VTKFormat format) const {

    switch(field) {

    case LevelSetField::SUPPORT:
    {
        for( const Cell &cell : this->m_kernel->getMesh()->getVTKCellWriteRange() ){
            long cellId = cell.getId();
            long value = getSupport(cellId);
            this->flushValue(stream, format, value);
        }

        break;
    }


    case LevelSetField::PART:
    {
        for( const Cell &cell : this->m_kernel->getMesh()->getVTKCellWriteRange() ){
            long cellId = cell.getId();
            int value = getPart(cellId);
            this->flushValue(stream, format, value);
        }

        break;
    }

    case LevelSetField::NORMAL:
    {
        for( const Cell &cell : this->m_kernel->getMesh()->getVTKCellWriteRange() ){
            long cellId = cell.getId();
            const std::array<double,3> &value = getNormal(cellId);
            this->flushValue(stream, format, value);
        }

        break;
    }

    default:
    {
        LevelSetCachedObject<narrow_band_cache_t>::flushVTKOutputData(field, stream, format);

        break;
    }

    }
}

}

# endif
