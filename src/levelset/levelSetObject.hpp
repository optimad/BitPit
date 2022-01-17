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

# ifndef __BITPIT_LEVELSET_OBJECT_HPP__
# define __BITPIT_LEVELSET_OBJECT_HPP__

// Standard Template Library
# include <iostream>
# include <array>
# include <vector>
# include <unordered_map>

# include "bitpit_IO.hpp"
# if BITPIT_ENABLE_MPI
# include "bitpit_communications.hpp"
# endif
# include "levelSetCommon.hpp"

namespace bitpit{

namespace adaption{
    struct Info;
}
class SendBuffer;
class RecvBuffer;

class LevelSet;
class LevelSetKernel;

class LevelSetObject : public VTKBaseStreamer{

    friend LevelSet;

    private:
    int                                         m_id;           /**< identifier of object */

    void                                        setId(int id);

    protected:
    LevelSetObject(int);

    void                                        setKernel(LevelSetKernel *);
    LevelSetKernel *                            getKernel();

    void                                        clear();

    void                                        setSizeNarrowBand(double) ;

    virtual void                                computeLSInNarrowBand(bool);
    virtual void                                updateLSInNarrowBand(const std::vector<adaption::Info> &, bool);
    void                                        clearAfterMeshAdaption(const std::vector<adaption::Info>&);

    short                                       evalValueSign(double) const ;

    void                                        dump(std::ostream &);
    void                                        restore(std::istream &);

# if BITPIT_ENABLE_MPI
    void                                        exchangeGhosts() ;
    void                                        startExchange( const std::unordered_map<int,std::vector<long>> &, DataCommunicator * );
    void                                        completeExchange( const std::unordered_map<int,std::vector<long>> &, DataCommunicator * );
# endif


    LevelSetKernel*                             m_kernelPtr;    /**< pointer to kernel */
    double                                      m_narrowBand;   /**< Size of narrow band */

    virtual void                                _clear();
    virtual void                                _clearAfterMeshAdaption(const std::vector<adaption::Info>&) ;
    virtual void                                _dump(std::ostream &);
    virtual void                                _restore( std::istream &);

# if BITPIT_ENABLE_MPI
    virtual void                                writeCommunicationBuffer( const std::vector<long> &, SendBuffer & ) ; 
    virtual void                                _writeCommunicationBuffer(const std::vector<long>&, SendBuffer&)  ;
    virtual void                                readCommunicationBuffer( const std::vector<long> &, RecvBuffer & ) ; 
    virtual void                                _readCommunicationBuffer(const std::vector<long>&, RecvBuffer&)  ;
# endif 

    public:
    virtual ~LevelSetObject() = default;

    const LevelSetKernel *                      getKernel() const;

    virtual LevelSetObject*                     clone() const =0;

    int                                         getId() const ;
    virtual bool                                isPrimary() const ;

    BITPIT_DEPRECATED(virtual LevelSetInfo      getLevelSetInfo(long ) const) =0;
    BITPIT_DEPRECATED(virtual double            getLS(long ) const) =0;
    virtual double                              getValue(long ) const =0;
    virtual short                               getSign(long ) const ;
    virtual std::array<double,3>                getGradient(long ) const =0 ;
    std::array<double,3>                        computeProjectionPoint(long ) const;

    std::array<double,3>                        computeVertexProjectionPoint(long ) const;

    virtual int                                 getPart(long ) const ;
    virtual std::array<double,3>                getNormal(long ) const;
    virtual LevelSetInfo                        computeLevelSetInfo(const std::array<double,3> &) const =0;
    std::array<double,3>                        computeProjectionPoint(const std::array<double,3> &) const;

    virtual bool                                isInNarrowBand(long ) const = 0;

    double                                      getSizeNarrowBand() const;

    LevelSetIntersectionStatus                  intersectSurface(long, LevelSetIntersectionMode=LevelSetIntersectionMode::FAST_FUZZY) const;
    virtual double                              getSurfaceFeatureSize(long ) const;
    virtual double                              getMinSurfaceFeatureSize() const;
    virtual double                              getMaxSurfaceFeatureSize() const;

    void                                        enableVTKOutput(LevelSetWriteField writeField, bool enable=true);
    void                                        enableVTKOutput(LevelSetWriteField writeField, const std::string &objectName, bool enable=true);
    void                                        flushData(std::fstream &, const std::string &, VTKFormat) override;


};

}

#endif
