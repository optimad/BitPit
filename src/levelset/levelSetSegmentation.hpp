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

# ifndef __BITPIT_LEVELSET_SEGMENTATION_HPP__
# define __BITPIT_LEVELSET_SEGMENTATION_HPP__

// Standard Template Library
# include <array>
# include <vector>
# include <unordered_set>
# include <unordered_map>

#include "bitpit_surfunstructured.hpp"

#include "levelSetCommon.hpp"

namespace bitpit{

namespace adaption{
    struct Info;
}
class SurfaceSkdTree;

class SendBuffer;
class RecvBuffer;

class LevelSetKernel;
class LevelSetCartesian;
class LevelSetOctree;
class LevelSetCachedObject;

class SegmentationKernel {

public:
    SegmentationKernel();
    SegmentationKernel(const SurfUnstructured *surface, double featureAngle);
    SegmentationKernel(std::unique_ptr<const SurfUnstructured> &&surface, double featureAngle);

    const SurfUnstructured & getSurface() const;
    double getFeatureAngle() const;

    const SurfaceSkdTree & getSearchTree() const ;

    int getSegmentInfo( const std::array<double,3> &pointCoords, long segmentId, bool signd, double &distance, std::array<double,3> &gradient, std::array<double,3> &normal ) const;

private:
    typedef std::pair<long, int> SegmentVertexKey;

    const SurfUnstructured *m_surface;
    std::unique_ptr<const SurfUnstructured> m_ownedSurface;
    double m_featureAngle;

    std::unique_ptr<SurfaceSkdTree> m_searchTree;

    PiercedStorage<std::size_t> m_segmentVertexOffset;

    mutable PiercedStorage<bool> m_segmentNormalsValid;
    mutable PiercedStorage<std::array<double,3>> m_segmentNormalsStorage;
    mutable PiercedStorage<bool> m_unlimitedVertexNormalsValid;
    mutable PiercedStorage<std::array<double,3>> m_unlimitedVertexNormalsStorage;
    mutable std::vector<bool> m_limitedSegmentVertexNormalValid;
    mutable std::unordered_map<SegmentVertexKey, std::array<double,3>, utils::hashing::hash<SegmentVertexKey>> m_limitedSegmentVertexNormalStorage;

    void setSurface( const SurfUnstructured *surface, double featureAngle);

    std::array<double,3> computePseudoNormal( const SurfUnstructured::CellConstIterator &segmentIterator, const double *lambda ) const;
    std::array<double,3> computeSurfaceNormal( const SurfUnstructured::CellConstIterator &segmentIterator, const double *lambda ) const;

    std::array<double,3> computeSegmentNormal( const SurfUnstructured::CellConstIterator &segmentIterator ) const;
    std::array<double,3> computeSegmentEdgeNormal( const SurfUnstructured::CellConstIterator &segmentIterator, int edge ) const;
    std::array<double,3> computeSegmentVertexNormal( const SurfUnstructured::CellConstIterator &segmentIterator, int vertex, bool limited ) const;

};

class LevelSetSegmentation : public LevelSetCachedObject {

    private:
    std::shared_ptr<const SegmentationKernel> m_segmentation;

    NarrowBandStorage<long>                     m_narrowBandSupportIds;      /** Support ids of the cells inside the narrow band */
    NarrowBandStorage<std::array<double,3>>     m_narrowBandSurfaceNormals;  /** Surface normal associated with the cells inside the narrow band */

    double                                      getSegmentSize( long ) const;

    protected:

    void                                        _dump( std::ostream &) override ;
    void                                        _restore( std::istream &) override ;

    void                                        getBoundingBox( std::array<double,3> &, std::array<double,3> &) const override;
# if BITPIT_ENABLE_MPI
    void                                        getGlobalBoundingBox( std::array<double,3> &, std::array<double,3> &) const override;
#endif

    void                                        computeLSInNarrowBand( LevelSetCartesian *, bool);
    void                                        computeLSInNarrowBand( LevelSetOctree *, bool);
    void                                        updateLSInNarrowBand(LevelSetOctree *, const std::vector<adaption::Info> &, bool);

    void                                        setNarrowBandEntry(NarrowBandIterator itr, double value, const std::array<double, 3> &gradient, long semgnetId, const std::array<double, 3> &surfaceNormal);

# if BITPIT_ENABLE_MPI
    std::size_t                                 getNarrowBandEntryBinarySize() const override;
    void                                        writeNarrowBandEntryCommunicationBuffer( NarrowBandIterator narrowBandItr, SendBuffer &dataBuffer ) override;
    void                                        readNarrowBandEntryCommunicationBuffer( NarrowBandIterator narrowBandItr, RecvBuffer &dataBuffer ) override;
#endif

    public:
    LevelSetSegmentation(int);
    LevelSetSegmentation(int, std::unique_ptr<const SurfUnstructured> &&, double featureAngle = 2. * BITPIT_PI);
    LevelSetSegmentation(int, const SurfUnstructured*, double featureAngle = 2. * BITPIT_PI);

    LevelSetSegmentation*                       clone() const override ;

    void                                        setSegmentation(std::unique_ptr<const SurfUnstructured> &&patch, double featureAngle = 2. * BITPIT_PI) ;
    void                                        setSegmentation(const SurfUnstructured *patch, double featureAngle = 2. * BITPIT_PI) ;
    const SegmentationKernel &                  getSegmentation() const ;

    virtual int                                 getPart(long ) const override;
    virtual std::array<double,3>                getNormal(long ) const override;
    long                                        getSupport(long id) const;

    double                                      getSurfaceFeatureSize(long ) const override;
    double                                      getMinSurfaceFeatureSize() const override;
    double                                      getMaxSurfaceFeatureSize() const override;

    void                                        computeLSInNarrowBand(bool) override;
    void                                        updateLSInNarrowBand(const std::vector<adaption::Info> &, bool) override;

    LevelSetInfo                                computeLevelSetInfo(const std::array<double,3> &) const override;

};

}

#endif
