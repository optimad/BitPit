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
# include <unordered_set>

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

class LevelSetObjectInterface {

    public:
    virtual ~LevelSetObjectInterface() = default;

    virtual LevelSetKernel *                    getKernel() = 0;
    virtual const LevelSetKernel *              getKernel() const = 0;
    virtual void                                setKernel(LevelSetKernel *) = 0;

    BITPIT_DEPRECATED(virtual LevelSetInfo      getLevelSetInfo(long ) const) = 0;
    BITPIT_DEPRECATED(virtual double            getLS(long ) const) = 0;
    virtual double                              getValue(long ) const = 0;
    virtual short                               getSign(long ) const = 0 ;
    virtual std::array<double,3>                getGradient(long ) const = 0 ;

    virtual bool                                isInNarrowBand(long ) const = 0;

};

class LevelSetObject : public VTKBaseStreamer, public virtual LevelSetObjectInterface {

    friend LevelSet;

    private:
    int                                         m_id;           /**< identifier of object */

    std::size_t                                 m_nReferences;

    void                                        setId(int id);

    std::size_t                                 incrementReferenceCount();
    std::size_t                                 decrementReferenceCount();

    protected:
    enum UpdateStrategy {
        UPDATE_STRATEGY_EXCHANGE,
        UPDATE_STRATEGY_EVALUATE,
    };

    LevelSetFieldMap<std::string>               m_enabledOutputFields;

    LevelSetObject(int);
    LevelSetObject(const LevelSetObject &other);
    LevelSetObject(LevelSetObject &&other);

    void                                        setKernel(LevelSetKernel *) override;
    LevelSetKernel *                            getKernel() override;

    void                                        clear();
    void                                        update( const std::vector<adaption::Info> &adaptionData, bool signedDistance );

    void                                        setSizeNarrowBand(double) ;

# if BITPIT_ENABLE_MPI
    virtual UpdateStrategy                      getPartitioningUpdateStrategy() const;
# endif

    virtual void                                computeNarrowBand(bool);
    virtual void                                updateNarrowBand(const std::vector<long> &cellIds, bool signd);
    virtual void                                pruneNarrowBand(const std::vector<long> &cellIds);

    short                                       evalValueSign(double) const ;

    void                                        dump(std::ostream &);
    void                                        restore(std::istream &);

# if BITPIT_ENABLE_MPI
    void                                        exchangeGhosts() ;
    void                                        startExchange( const std::unordered_map<int,std::vector<long>> &, DataCommunicator * );
    void                                        completeExchange( const std::unordered_map<int,std::vector<long>> &, DataCommunicator * );
# endif


    LevelSetKernel*                             m_kernel;           /**< Levelset kernel */
    double                                      m_narrowBandSize;   /**< Size of narrow band */

    virtual void                                _clear();
    virtual void                                _dump(std::ostream &);
    virtual void                                _restore( std::istream &);

# if BITPIT_ENABLE_MPI
    virtual void                                writeCommunicationBuffer( const std::vector<long> &, SendBuffer & ) ; 
    virtual void                                _writeCommunicationBuffer(const std::vector<long>&, SendBuffer&)  ;
    virtual void                                readCommunicationBuffer( const std::vector<long> &, RecvBuffer & ) ; 
    virtual void                                _readCommunicationBuffer(const std::vector<long>&, RecvBuffer&)  ;
# endif 

    bool                                        hasVTKOutputData(LevelSetField field, const std::string &objectName) const;
    void                                        removeVTKOutputData(LevelSetField field, const std::string &objectName);
    virtual void                                addVTKOutputData(LevelSetField field, const std::string &objectName);
    std::string                                 getVTKOutputDataName(LevelSetField field, const std::string &objectName) const;
    virtual std::string                         getVTKOutputFieldName(LevelSetField field) const;
    virtual void                                flushVTKOutputData(LevelSetField field, std::fstream &stream, VTKFormat format) const;

    public:
    virtual ~LevelSetObject();

    const LevelSetKernel *                      getKernel() const override;

    virtual LevelSetObject*                     clone() const =0;

    virtual LevelSetFieldset                    getSupportedFields() const;

    int                                         getId() const ;
    virtual bool                                isPrimary() const ;

    std::size_t                                 getReferenceCount() const ;

    short                                       getSign(long ) const override;

    std::array<double,3>                        computeProjectionPoint(long ) const;
    std::array<double,3>                        computeVertexProjectionPoint(long ) const;

    BITPIT_DEPRECATED(LevelSetInfo              getLevelSetInfo(long ) const override);
    BITPIT_DEPRECATED(double                    getLS(long ) const override);
    virtual LevelSetInfo                        computeLevelSetInfo(const std::array<double,3> &) const =0;
    std::array<double,3>                        computeProjectionPoint(const std::array<double,3> &) const;

    double                                      getSizeNarrowBand() const;

    LevelSetIntersectionStatus                  intersectSurface(long, LevelSetIntersectionMode=LevelSetIntersectionMode::FAST_FUZZY) const;

    void                                        enableVTKOutput(LevelSetWriteField field, bool enable=true);
    void                                        enableVTKOutput(const LevelSetFieldset &fieldset, bool enable=true);
    void                                        enableVTKOutput(LevelSetField field, bool enable=true);
    void                                        enableVTKOutput(LevelSetWriteField fieldset, const std::string &objectName, bool enable=true);
    void                                        enableVTKOutput(const LevelSetFieldset &fieldset, const std::string &objectName, bool enable=true);
    void                                        enableVTKOutput(LevelSetField field, const std::string &objectName, bool enable=true);
    void                                        flushData(std::fstream &, const std::string &, VTKFormat) override;


};

}

#endif
