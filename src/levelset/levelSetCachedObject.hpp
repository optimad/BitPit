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

# ifndef __BITPIT_LEVELSET_CACHED_HPP__
# define __BITPIT_LEVELSET_CACHED_HPP__

// Standard Template Library
# include <array>
# include <vector>
# include <unordered_set>

# include "bitpit_containers.hpp"

namespace bitpit{

namespace adaption{
    struct Info;
}

class SendBuffer;
class RecvBuffer;

class LevelSetKernel;
class LevelSetObject;

class LevelSetCachedObject : public LevelSetObject{

    private:
    static const int                            PROPAGATION_STATUS_EXTERNAL;
    static const int                            PROPAGATION_STATUS_WAITING;
    static const int                            PROPAGATION_STATUS_REACHED;

    static const int                            PROPAGATION_SIGN_UNDEFINED;
    static const int                            PROPAGATION_SIGN_DUMMY;

    void                                        setSign( long id, int sign ) ;

    void                                        initializeCellSignPropagation( long cellId, int cellSign,
                                                                               const std::array<double, 3> &objectBoxMin,
                                                                               const std::array<double, 3> &objectBoxMax,
                                                                               int *cellStatus, std::vector<long> *seeds,
                                                                               long *nWaiting, long *nExternal,
                                                                               int *externalSign ) ;

    void                                        propagateSeedSign( const std::vector<long> &seeds,
                                                                   PiercedStorage<int, long> *status,
                                                                   long *nWaiting, int *externalSign ) ;

    protected:
    PiercedVector<LevelSetInfo>                 m_ls ;          /**< Levelset information for each cell */
    virtual void                                getBoundingBox( std::array<double,3> &, std::array<double,3> & )const =0  ;
# if BITPIT_ENABLE_MPI
    virtual void                                getGlobalBoundingBox( std::array<double,3> &, std::array<double,3> & )const =0  ;
#endif

    void                                        _clear( ) override ;
    virtual void                                __clear() ;


    void                                        _clearAfterMeshAdaption(const std::vector<adaption::Info> & ) override ;
    virtual void                                __clearAfterMeshAdaption(const std::vector<adaption::Info> & ) ;

    void                                        _dump( std::ostream &) override ;
    virtual void                                __dump(std::ostream &) ; 
    void                                        _restore( std::istream &) override ;
    virtual void                                __restore(std::istream &) ; 

# if BITPIT_ENABLE_MPI
    void                                        _writeCommunicationBuffer( const std::vector<long> &, SendBuffer & ) override ;
    void                                        _readCommunicationBuffer( const std::vector<long> &, RecvBuffer & )  override;
# endif 

    public:
    LevelSetCachedObject(int);

    LevelSetInfo                                getLevelSetInfo(long ) const override ;
    double                                      getLS(long ) const override ;
    double                                      getValue(long ) const override ;
    std::array<double,3>                        getGradient(long ) const override ;

    void                                        propagateSign() override ;

};



}

#endif 
