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

#ifndef __BITPIT_SYSTEM_MATRIX_HPP__
#define __BITPIT_SYSTEM_MATRIX_HPP__

#include <vector>

#if BITPIT_ENABLE_MPI==1
#   include <mpi.h>
#endif

#include "bitpit_containers.hpp"

namespace bitpit {

class SparseMatrix {

public:
    SparseMatrix();
    SparseMatrix(long nRows, long nCols, long nNZ = 0);
#if BITPIT_ENABLE_MPI==1
    SparseMatrix(MPI_Comm communicator);
    SparseMatrix(MPI_Comm communicator, bool partitioned, long nRows, long nCols, long nNZ = 0);
#endif

    ~SparseMatrix();

    void initialize(long nRows, long nCols, long nNZ = 0);
#if BITPIT_ENABLE_MPI==1
    void initialize(bool partitioned, long nRows, long nCols, long nNZ = 0);
#endif
    void clear(bool release = false);
    void squeeze();

    void assembly();
    bool isAssembled() const;

    long countMissingRows() const;
    long countAddedRows() const;

    long getRowCount() const;
    long getColCount() const;

    long getNZCount() const;
    long getRowNZCount(long row) const;
    long getMaxRowNZCount() const;

#if BITPIT_ENABLE_MPI==1
    bool isPartitioned() const;

    const MPI_Comm & getCommunicator() const;

    long getRowGlobalCount() const;
    long getRowGlobalOffset() const;

    long getColGlobalCount() const;
    long getColGlobalOffset() const;

    long getNZGlobalCount() const;
    long getMaxRowNZGlobalCount() const;

    std::vector<long> extractLocalGlobalRows() const;
    std::vector<long> extractGhostGlobalRows() const;

    std::vector<long> extractLocalGlobalCols() const;
    std::vector<long> extractGhostGlobalCols() const;
#endif

    void addRow(const std::vector<long> &rowPattern, const std::vector<double> &rowValues);
    void addRow(long nRowNZ, const long *rowPattern, const double *rowValues);

    ConstProxyVector<long> getRowPattern(long row) const;
    void getRowPattern(long row, ConstProxyVector<long> *pattern) const;

    ConstProxyVector<double> getRowValues(long row) const;
    void getRowValues(long row, ConstProxyVector<double> *values) const;

    std::unique_ptr<SparseMatrix> computeTranspose() const;

protected:
    long m_nRows;
    long m_nCols;
    long m_nNZ;
    long m_maxRowNZ;

    long m_lastRow;

    bool m_assembled;

#if BITPIT_ENABLE_MPI==1
    bool m_partitioned;
    MPI_Comm m_communicator;

    long m_global_nRows;
    long m_global_nCols;
    long m_global_nNZ;
    long m_global_maxRowNZ;
    long m_global_rowOffset;
    long m_global_colOffset;
#endif

    FlatVector2D<long> m_pattern;
    std::vector<double> m_values;

private:
    void _initialize(long nRows, long nCols, long nNZ = 0);
#if BITPIT_ENABLE_MPI==1
    void _initialize(bool partitioned, long nRows, long nCols, long nNZ = 0);
#endif

#if BITPIT_ENABLE_MPI==1
    void setCommunicator(MPI_Comm communicator);
    void freeCommunicator();
#endif

};

}

#endif
