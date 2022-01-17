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

# if BITPIT_ENABLE_MPI
# include <mpi.h>
# include "bitpit_communications.hpp"
# endif

# include "bitpit_operators.hpp"
# include "bitpit_CG.hpp"
# include "bitpit_patchkernel.hpp"
# include "bitpit_volcartesian.hpp"
# include "bitpit_voloctree.hpp"

# include "levelSetKernel.hpp"
# include "levelSetCartesian.hpp"
# include "levelSetOctree.hpp"
# include "levelSetObject.hpp"
# include "levelSetCachedObject.hpp"
# include "levelSet.hpp"

namespace bitpit {

const int LevelSetCachedObject::PROPAGATION_STATUS_EXTERNAL = - 1;
const int LevelSetCachedObject::PROPAGATION_STATUS_WAITING  =   0;
const int LevelSetCachedObject::PROPAGATION_STATUS_REACHED  =   1;

const int LevelSetCachedObject::PROPAGATION_SIGN_DUMMY     = -2;
const int LevelSetCachedObject::PROPAGATION_SIGN_UNDEFINED = -3;

/*!
	@ingroup levelset
	@interface LevelSetCachedObject
	@brief Interface class for all objects which need to store the discrete values of levelset function.
*/

/*!
 * Constructor
 * @param[in] id id assigned to object
 */
LevelSetCachedObject::LevelSetCachedObject(int id) : LevelSetObject(id){
}

/*!
 * Get LevelSetInfo of cell
 * @param[in] i cell idex
 * @return LevelSetInfo of cell
*/
LevelSetInfo LevelSetCachedObject::getLevelSetInfo( long i)const{

    auto itr = m_ls.find(i);
    if ( itr == m_ls.end() ){
        return LevelSetInfo();
    }

    return *itr;

} 

/*!
 * Get the levelset value of cell
 * @param[in] id cell id
 * @return levelset value in cell
 */
double LevelSetCachedObject::getLS( long id)const {

    return getValue(id);

}

/*!
 * Get the levelset value of cell
 * @param[in] id cell id
 * @return levelset value in cell
 */
double LevelSetCachedObject::getValue( long id)const {

    auto itr = m_ls.find(id);
    if ( itr == m_ls.end() ){
        return levelSetDefaults::VALUE;
    }

    return itr->value;

}

/*!
 * Get the levelset gradient of cell
 * @param[in] id cell id
 * @return levelset gradient in cell 
 */
std::array<double,3> LevelSetCachedObject::getGradient(long id) const {

    auto itr = m_ls.find(id);
    if ( itr == m_ls.end() ){
        return levelSetDefaults::GRADIENT;
    }

    return itr->gradient;

}

/*! 
 * Deletes non-existing items after grid adaption.
 * @param[in] mapper mapping info
 */
void LevelSetCachedObject::_clearAfterMeshAdaption( const std::vector<adaption::Info> &mapper ){

    for ( auto & map : mapper ){
        if( map.entity == adaption::Entity::ENTITY_CELL ){
            if( map.type == adaption::Type::TYPE_DELETION || 
                map.type == adaption::Type::TYPE_PARTITION_SEND  ||
                map.type == adaption::Type::TYPE_REFINEMENT  ||
                map.type == adaption::Type::TYPE_COARSENING  ){

                for ( auto & parent : map.previous){
                    if( m_ls.exists(parent) ) 
                        m_ls.erase(parent,true) ;
                }
            }
        }
    }

    m_ls.flush() ;

}

/*! 
 * Clears all levelset information
 */
void LevelSetCachedObject::_clear( ){
    m_ls.clear() ;
}

/*!
 * Propagate the sign of the signed distance function from narrow band to
 * entire domain.
 */
void LevelSetCachedObject::propagateSign() {

    VolumeKernel const &mesh = *(m_kernelPtr->getMesh()) ;
    const PiercedVector<Cell> &cells = mesh.getCells();

    VolumeKernel::CellConstIterator cellBegin = mesh.cellConstBegin();
    VolumeKernel::CellConstIterator cellEnd   = mesh.cellConstEnd();

    std::vector<long> seeds;
    seeds.reserve(mesh.getCellCount());

    // Evaluate the bounding box of the object
    //
    // The current process may only have the portion of the object needed for
    // evaluating the levelset on the cells of its mesh, therefore we need to
    // evaluate the overall bounding box across all process.
    std::array<double,3> boxMin;
    std::array<double,3> boxMax;
#if BITPIT_ENABLE_MPI
    getGlobalBoundingBox(boxMin, boxMax);
#else
    getBoundingBox(boxMin, boxMax);
#endif

    // Set the initial propagation status of the cells
    //
    // We don't need to set the sign of cells associated to a levelset info,
    // if a cell is associated to a levelset info we know its sign (the value
    // contained in the levelset info may be a dummy value, but the sign is
    // the correct one) and the cell can be used as seeds for propagating the
    // sign to other cells. External sign should not be added to the seeds
    // because we don't need to propagate the sign in the external region.
    //
    // The sign of the cells outise the bounding box of all objects (external
    // cells) can be either positive or negative depending on the orientation
    // of the objects. There is no need to propagate the sign into those cells,
    // once the sign of the external region is know, it can be assigned to all
    // the cells in the external region. When the propagation reaches the
    // external region it can be stopped, the sign of the seed from which the
    // propagation has started will be the sign of the external region.
    PiercedStorage<int, long> propagationStatus(1, &cells);

    int externalSign = PROPAGATION_SIGN_UNDEFINED;
    long nExternal   = 0;
    long nWaiting    = mesh.getCellCount();
    for (auto itr = cellBegin; itr != cellEnd; ++itr) {
        long cellId = itr.getId();
        long cellRawId = itr.getRawIndex();
        int &cellPropagationStatus = propagationStatus.rawAt(cellRawId);

        int cellSign;
        if (m_ls.find(cellId) != m_ls.end()) {
            cellSign = getSign(cellId);
        } else {
            cellSign = PROPAGATION_SIGN_UNDEFINED;
        }

        initializeCellSignPropagation(cellId, cellSign, boxMin, boxMax,
                                      &cellPropagationStatus, &seeds,
                                      &nWaiting, &nExternal, &externalSign);
    }

    // If there are no external cells, set the sign of the external region
    // to a dummy values. In this way the routine that propagates the sign
    // will not try to detect the sign of the external region.
    if (nExternal == 0) {
        externalSign = PROPAGATION_SIGN_DUMMY;
    }

    // Use the seeds to propagate the sign
    propagateSeedSign(seeds, &propagationStatus, &nWaiting, &externalSign);

#if BITPIT_ENABLE_MPI
    // If there are cells with an unknown sign, data communication among
    // ghost cells is needed. However it is only possibly to have cells with
    // an unknown sign for partinioned patches.
    long nGlobalWaiting = nWaiting;
    if (mesh.isPartitioned()) {
        MPI_Allreduce(MPI_IN_PLACE, &nGlobalWaiting, 1, MPI_LONG, MPI_SUM, m_kernelPtr->getCommunicator());
    }

    if (nGlobalWaiting != 0) {
        assert(mesh.isPartitioned());
        assert(mesh.getProcessorCount() != 1);

        // Initialize the communicator for exchanging the sign of the ghosts
        DataCommunicator dataCommunicator(m_kernelPtr->getCommunicator());

        int sign;
        size_t dataSize = sizeof(sign);

        // Set the receives
        for (const auto &entry : mesh.getGhostCellExchangeTargets()) {
            const int rank = entry.first;
            const auto &list = entry.second;

            dataCommunicator.setRecv(rank, list.size() * dataSize);
        }

        // Set the sends
        for (const auto &entry : mesh.getGhostCellExchangeSources()) {
            const int rank = entry.first;
            auto &list = entry.second;

            dataCommunicator.setSend(rank, list.size() * dataSize);
        }

        // Communicate the sign among the partitions
        while (nGlobalWaiting != 0) {
            // Start the receives
            for (const auto &entry : mesh.getGhostCellExchangeTargets()) {
                const int rank = entry.first;
                dataCommunicator.startRecv(rank);
            }

            // Start the sends
            for (const auto &entry : mesh.getGhostCellExchangeSources()) {
                const int rank = entry.first;
                const auto &sendIds = entry.second;
                SendBuffer &buffer = dataCommunicator.getSendBuffer(rank);

                for (long cellId : sendIds) {
                    int sign = PROPAGATION_SIGN_UNDEFINED;
                    if (propagationStatus.at(cellId) == PROPAGATION_STATUS_REACHED) {
                        sign = getSign(cellId);
                    }
                    buffer << sign;
                }

                dataCommunicator.startSend(rank);
            }

            // Receive the sign and propagate the sign
            //
            // If we discover the sign of a ghost, we can use it as a seed.
            seeds.clear();
            int nCompletedRecvs = 0;
            while (nCompletedRecvs < dataCommunicator.getRecvCount()) {
                int rank = dataCommunicator.waitAnyRecv();
                const auto &recvIds = mesh.getGhostCellExchangeTargets(rank);
                RecvBuffer &buffer = dataCommunicator.getRecvBuffer(rank);

                // Receive data and detect new seeds
                for (long cellId : recvIds) {
                    buffer >> sign;
                    if (sign == PROPAGATION_SIGN_UNDEFINED) {
                        continue;
                    }

                    std::size_t cellRawId = cells.getRawIndex(cellId);
                    int &cellPropagationStatus = propagationStatus.rawAt(cellRawId);
                    if (cellPropagationStatus == PROPAGATION_STATUS_WAITING) {
                        // Set the sign
                        setSign(cellId, sign);

                        // Initialize sign propagation
                        initializeCellSignPropagation(cellId, sign, boxMin, boxMax,
                                                      &cellPropagationStatus, &seeds,
                                                      &nWaiting, &nExternal, &externalSign);
                    } else if (cellPropagationStatus == PROPAGATION_STATUS_REACHED) {
                        assert(getSign(cellId) == sign);
                    }
                }

                ++nCompletedRecvs;
            }

            if (seeds.size() > 0) {
                propagateSeedSign(seeds, &propagationStatus, &nWaiting, &externalSign);
            }

            // Wait to the sends to finish
            dataCommunicator.waitAllSends();

            // Update the global counter for cells with an unknow sign
            nGlobalWaiting = nWaiting;
            MPI_Allreduce(MPI_IN_PLACE, &nGlobalWaiting, 1, MPI_LONG, MPI_SUM, m_kernelPtr->getCommunicator());
        }
    }

    // Communicate the sign of the external region
    //
    // The sign has to be consistent among all the partitions.
    bool exchangeExternalSign;
    if (mesh.isPartitioned()) {
        exchangeExternalSign = (nExternal != 0);
        MPI_Allreduce(MPI_IN_PLACE, &exchangeExternalSign, 1, MPI_C_BOOL, MPI_LOR, m_kernelPtr->getCommunicator());
    } else {
        exchangeExternalSign = false;
    }

    if (exchangeExternalSign) {
        bool positiveExternalSign = (externalSign == 1);
        MPI_Allreduce(MPI_IN_PLACE, &positiveExternalSign, 1, MPI_C_BOOL, MPI_LOR, m_kernelPtr->getCommunicator());

        bool negativeExternalSign = (externalSign == -1);
        MPI_Allreduce(MPI_IN_PLACE, &negativeExternalSign, 1, MPI_C_BOOL, MPI_LOR, m_kernelPtr->getCommunicator());

        if (positiveExternalSign && negativeExternalSign) {
            externalSign = PROPAGATION_SIGN_UNDEFINED;
        } else if (positiveExternalSign) {
            externalSign = 1;
        } else if (negativeExternalSign) {
            externalSign = -1;
        } else {
            externalSign = PROPAGATION_SIGN_UNDEFINED;
        }
    }
#else
    // Check that the sign has been propagated into all regions
    assert(nWaiting == 0);
#endif

    // Check that the sign of the external region was correctly identified
    if (nExternal != 0 && externalSign == PROPAGATION_SIGN_UNDEFINED) {
        throw std::runtime_error("Sign of external region not properly identified!");
    }

    // Assign the sign to the external cells
    for (auto itr = cellBegin; itr != cellEnd; ++itr) {
        std::size_t cellRawId = itr.getRawIndex();
        if (propagationStatus.rawAt(cellRawId) != PROPAGATION_STATUS_EXTERNAL) {
            continue;
        }

        long cellId = itr.getId();
        setSign(cellId, externalSign);
        --nExternal;
    }

    assert(nExternal == 0);
}

/*!
 * Initialize propagation information for the specified cell.
 *
 * External cells for which the levelset sign is know can be used to define the
 * sign of the external region. Non-external cells for which the sign is known
 * will be used as seeds.
 *
 * \param cellId is the index of the cell
 * \param cellSign is the sign of the cell
 * \param objectBoxMin is the lower-left corenr of the object bounding box
 * \param objectBoxMax is the upper-right corenr of the object bounding box
 * \param[out] cellStatus on output will contain the propagation status
 * associated to the cell
 * \param[in,out] seeds are the seeds to be used for sign propagation
 * status of the cells
 * \param[in,out] nWaiting is the number of cells that are waiting for the
 * propagation to reach them.
 * \param[in,out] nExternal is the number of cells in the external region
 * \param[in,out] externalSign is the sign of the external region
 */
void LevelSetCachedObject::initializeCellSignPropagation(long cellId, int cellSign,
                                                         const std::array<double, 3> &objectBoxMin,
                                                         const std::array<double, 3> &objectBoxMax,
                                                         int *cellStatus, std::vector<long> *seeds,
                                                         long *nWaiting, long *nExternal,
                                                         int *externalSign) {

    // Detect if the cell is external
    //
    // A cell is external if it is completely outside the object bounding box.
    // If the cell may be intersected by the object (i.e., the bounding box of
    // the cell intersects the bounding box of the object), the cell cannot be
    // flagged as external.
    const VolumeKernel &mesh = *(m_kernelPtr->getMesh());
    double distanceTolerance = mesh.getTol();

    std::array<double,3> cellBoxMin;
    std::array<double,3> cellBoxMax;
    mesh.evalCellBoundingBox(cellId, &cellBoxMin, &cellBoxMax);

    bool isExternal = !CGElem::intersectBoxBox(cellBoxMin, cellBoxMax, objectBoxMin, objectBoxMax, 3, distanceTolerance);

    // Detect the status of the cell
    //
    // We can have three different cases:
    //  - the sign of the cell has been defined, in this case the propgation
    //    status is set as "REACHED";
    //  - there is no sign for the cell and the cell is external, in this case
    //    the propgation status is set as "EXTERNAL";
    //  - there is no sign for the cell and the cell is not external, in this
    //    case the propgation status is set as "WAITING".
    //
    // When the propgation reaches a cell it's possible to update the list of
    // seeds or the external sign.
    if (cellSign != PROPAGATION_SIGN_UNDEFINED) {
        *cellStatus = PROPAGATION_STATUS_REACHED;
        --(*nWaiting);

        // Non-external cells are added to the list of seeds, for external
        // cells the external sign is updated.
        if (!isExternal) {
            seeds->push_back(cellId);
        } else if (*externalSign != PROPAGATION_SIGN_DUMMY) {
            if (cellSign != PROPAGATION_SIGN_UNDEFINED) {
                if (*externalSign == PROPAGATION_SIGN_UNDEFINED) {
                    *externalSign = cellSign;
                } else if (*externalSign != cellSign) {
                    throw std::runtime_error("Mismatch in sign of external region!");
                }
            }
        }
    } else {
        if (!isExternal) {
            *cellStatus = PROPAGATION_STATUS_WAITING;
        } else {
            *cellStatus = PROPAGATION_STATUS_EXTERNAL;
            ++(*nExternal);
            --(*nWaiting);
        }
    }
}

/*!
 * Propagates the sign of the signed distance function from the specified
 * seeds to all reachable cells whose sign has not yet been assigned.
 *
 * The sign will NOT be propagated into cells flagged with the "EXTERNAL"
 * status (i.e., cells outside the bounding box of all the objects). When
 * the propagation reaches the external region it can be stopped, the sign
 * the sign of the seed from which the propagation has startedthat will be
 * the sign of the external region.
 *
 * \param seeds are the seeds to be used for sign propagation
 * \param[in,out] statuses contains the flags that defines the propagation
 * status of the cells. On output, this flag will be updated with the new
 * propagation statuses
 * \param[in,out] nWaiting is the number of cells that are waiting for the
 * propagation to reach them. On output, this number will be updated so it's
 * possible to keep track of the cells whose sign is not yet assigned
 * \param[in,out] externalSign is the sign of the external region. If the
 * external sign is not yet defined and the propagation reaches the external
 * region, the parameter will be updated with the sign of the external region
 */
void LevelSetCachedObject::propagateSeedSign(const std::vector<long> &seeds,
                                             PiercedStorage<int, long> *statuses,
                                             long *nWaiting, int *externalSign) {

    VolumeKernel const &mesh = *(m_kernelPtr->getMesh()) ;

    std::vector<long> processList;

    std::size_t seedCursor = seeds.size();
    while (seedCursor != 0) {
        // Get a seed
        --seedCursor;
        long seedId = seeds[seedCursor];

        // Get the sign of the seed
        int seedSign = getSign(seedId);
        if (seedSign == PROPAGATION_SIGN_UNDEFINED) {
            continue;
        }

        // Initialize the process list with the seed
        processList.resize(1);
        processList[0] = seedId;

        // Propagate the sign
        while (!processList.empty()) {
            long cellId = processList.back();
            processList.resize(processList.size() - 1);

            // Process non-seed cells
            bool isSeed = (cellId == seedId);
            if (!isSeed) {
                // Consider only cells waiting for the propagation to reach them
                //
                // The process list is not unique, it can contain duplicate, so
                // we need to make sure not to process a cell multiple times.
                int &cellStatus = statuses->at(cellId);
                if (cellStatus != PROPAGATION_STATUS_WAITING) {
                    continue;
                }

                // Set the sign of the cell
                setSign(cellId, seedSign);
                cellStatus = PROPAGATION_STATUS_REACHED;
                --(*nWaiting);

                // If there are no more waiting cells and we have detected the
                // sign of the external region we can exit
                if (*nWaiting == 0 && *externalSign != PROPAGATION_SIGN_UNDEFINED) {
                    return;
                }
            }

            // Process cell neighbours
            //
            // If a neighbour is waiting for the propagation, add it to the
            // process list. When the propagation reaches an external cell
            // the sign of the seed frow which the propagation started will
            // be the sign of the external region.
            const Cell &cell = mesh.getCell(cellId);
            const long *cellNeighs = cell.getAdjacencies() ;
            int nCellNeighs = cell.getAdjacencyCount() ;
            for(int n = 0; n < nCellNeighs; ++n){
                long neighId = cellNeighs[n] ;
                int neighStatus = statuses->at(neighId);
                if (neighStatus == PROPAGATION_STATUS_WAITING) {
                    processList.push_back(neighId);
                } else if (neighStatus == PROPAGATION_STATUS_EXTERNAL) {
                    // If the sign of the external region is unknown it can
                    // be assigned, otherwise check if the current sign is
                    // consistent with the previously evaluated sign.
                    if (*externalSign == PROPAGATION_SIGN_UNDEFINED) {
                        *externalSign = seedSign;
                    } else if (*externalSign != seedSign) {
                        throw std::runtime_error("Mismatch in sign of external region!");
                    }
                }
            }
        }
    }
}

/*!
 * Set the sign of the specified cell.
 *
 * \param id is the id of the cell
 * \param sign is the sign that will be assiged to the cell
 */
void LevelSetCachedObject::setSign(long id, int sign) {

    // The sign has to be set only if it is different from the default sign
    if (sign == levelSetDefaults::SIGN) {
        return;
    }

    // Get the info associated to the id
    PiercedVector<LevelSetInfo>::iterator infoItr = m_ls.find(id);
    if (infoItr == m_ls.end()){
        infoItr = m_ls.emplace(id);
    }

    // Update the sign
    (*infoItr).value = sign * levelSetDefaults::VALUE;
}

/*!
 * Writes LevelSetCachedObject to stream in binary format
 * @param[in] stream output stream
 */
void LevelSetCachedObject::_dump( std::ostream &stream ){

    utils::binary::write(stream, m_ls.size() ) ;
    bitpit::PiercedVector<LevelSetInfo>::iterator   infoItr, infoEnd = m_ls.end() ;

    for( infoItr=m_ls.begin(); infoItr!=infoEnd; ++infoItr){
        utils::binary::write(stream, infoItr.getId()) ;
        utils::binary::write(stream, infoItr->value) ;
        utils::binary::write(stream, infoItr->gradient) ;
    }
}

/*!
 * Reads LevelSetCachedObject from stream in binary format
 * @param[in] stream output stream
 */
void LevelSetCachedObject::_restore( std::istream &stream ){

    std::size_t nInfoItems;
    utils::binary::read(stream, nInfoItems);
    m_ls.reserve(nInfoItems);

    for( std::size_t i=0; i<nInfoItems; ++i){
        long id;
        utils::binary::read(stream, id) ;

        double value;
        utils::binary::read(stream, value) ;

        std::array<double,3> gradient;
        utils::binary::read(stream, gradient) ;

        m_ls.insert(id, LevelSetInfo(value, gradient)) ;
    }
}

#if BITPIT_ENABLE_MPI

/*!
 * Flushing of data to communication buffers for partitioning
 * @param[in] sendList list of cells to be sent
 * @param[in,out] dataBuffer buffer for second communication containing data
 */
void LevelSetCachedObject::_writeCommunicationBuffer( const std::vector<long> &sendList, SendBuffer &dataBuffer ){

    // Evaluate the size of the buffer
    std::size_t nInfoItems = 0;
    for( long id : sendList){
        if( m_ls.exists(id)){
            ++nInfoItems ;
        }
    }

    dataBuffer.setSize(dataBuffer.getSize() + sizeof(std::size_t) + nInfoItems* (sizeof(std::size_t) +4*sizeof(double)) ) ;

    // Fill the buffer
    dataBuffer << nInfoItems ;

    for( std::size_t k = 0; k < sendList.size(); ++k){
        long id = sendList[k];
        auto infoItr = m_ls.find(id) ;
        if( infoItr != m_ls.end() ){
            dataBuffer << k ;
            dataBuffer << infoItr->value ;
            dataBuffer << infoItr->gradient ;
        }
    }
}

/*!
 * Processing of communication buffer into data structure
 * @param[in] recvList list of cells to be received
 * @param[in,out] dataBuffer buffer containing the data
 */
void LevelSetCachedObject::_readCommunicationBuffer( const std::vector<long> &recvList, RecvBuffer &dataBuffer ){

    std::size_t nInfoItems ;
    dataBuffer >> nInfoItems ;

    for( std::size_t i=0; i<nInfoItems; ++i){
        // Determine the id of the element
        std::size_t k ;
        dataBuffer >> k ;
        long id = recvList[k] ;

        // Assign the data of the element
        auto infoItr = m_ls.find(id) ;
        if( infoItr == m_ls.end() ){
            infoItr = m_ls.emplace(id) ;
        }

        dataBuffer >> infoItr->value ;
        dataBuffer >> infoItr->gradient ;
    }
}

#endif 

}
