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
#include <numeric>
#if BITPIT_ENABLE_MPI==1
#   include <mpi.h>
#endif

#include "bitpit_IO.hpp"
#include "bitpit_LA.hpp"

using namespace bitpit;

/*!
* Subtest 001
*
* Testing solution of linear systems in its natural ordering.
*/
int subtest_001()
{
    int nRows = 10;
    int nCols = 10;
    int nNZ   = nRows * nCols;

    // Build matrix
    log::cout() << "Building matrix..." << std::endl;

    std::vector<long> rowPattern(nCols);
    std::vector<double> rowValues(nCols);

#if BITPIT_ENABLE_MPI==1
    SparseMatrix matrix(MPI_COMM_WORLD, false, nRows, nCols, nNZ);
#else
    SparseMatrix matrix(nRows, nCols, nNZ);
#endif
    for (int row = 0; row < nRows; ++row) {
        std::iota(rowPattern.begin(), rowPattern.end(), 0L);

        std::fill(rowValues.begin(), rowValues.end(), 0.);
        rowValues[row]             = (row + 1);
        rowValues[nRows - row - 1] = 11 * (row + 1);

        matrix.addRow(rowPattern, rowValues);
    }
    matrix.assembly();

    // Reorder system
    NaturalSystemMatrixOrdering naturalSystemOrdering;

    // Build system
    log::cout() << "Building system..." << std::endl;

    SystemSolver system(true, false);
    system.assembly(matrix, naturalSystemOrdering);

    double *rhs = system.getRHSRawPtr();
    rhs[0] = 1101;
    rhs[1] =  895;
    rhs[2] =  713;
    rhs[3] =  555;
    rhs[4] =  421;
    rhs[5] =  311;
    rhs[6] =  225;
    rhs[7] =  163;
    rhs[8] =  125;
    rhs[9] =  111;
    system.restoreRHSRawPtr(rhs);

    double *initialSolution = system.getSolutionRawPtr();
    for (int i = 0; i < nRows; ++i) {
        initialSolution[i] = 0;
    }
    system.restoreSolutionRawPtr(initialSolution);

    // Solve system
    log::cout() << "Solving transposed system..." << std::endl;

    system.solve();

    log::cout() << std::setprecision(16) << std::scientific;

    const double *solution = system.getSolutionRawReadPtr();
    for (int i = 0; i < nRows; ++i) {
        log::cout() << "  Solution[" << i << "] = " << solution[i] << std::endl;

        double expectedSolution = i + 1;
        if (!utils::DoubleFloatingEqual()(solution[i], expectedSolution, 1e-10)) {
            log::cout() << "  Expected solution[" << i << "] = " << expectedSolution << std::endl;
            log::cout() << "  Error[" << i << "] = " << (expectedSolution - solution[i]) << std::endl;
            throw std::runtime_error("  The solution of the system doesn't match the expected one.");
        }
    }
    system.restoreSolutionRawReadPtr(solution);

    return 0;
}

/*!
* Subtest 002
*
* Testing solution of reordered linear systems.
*/
int subtest_002()
{
    int nRows = 10;
    int nCols = 10;
    int nNZ   = nRows * nCols;

    // Build matrix
    log::cout() << "Building matrix..." << std::endl;

    std::vector<long> rowPattern(nCols);
    std::vector<double> rowValues(nCols);

#if BITPIT_ENABLE_MPI==1
    SparseMatrix matrix(MPI_COMM_WORLD, false, nRows, nCols, nNZ);
#else
    SparseMatrix matrix(nRows, nCols, nNZ);
#endif
    for (int row = 0; row < nRows; ++row) {
        std::iota(rowPattern.begin(), rowPattern.end(), 0L);

        std::fill(rowValues.begin(), rowValues.end(), 0.);
        rowValues[row]             = (row + 1);
        rowValues[nRows - row - 1] = 11 * (row + 1);

        matrix.addRow(rowPattern, rowValues);
    }
    matrix.assembly();

    // Reorder system
    std::vector<long> rowOrdering(nRows);
    for (int row = 0; row < nRows; ++row) {
        rowOrdering[row] = nRows - row - 1;
    }

    std::vector<long> colOrdering(nCols);
    for (int col = 0; col < nCols; ++col) {
        colOrdering[col] = nCols - col - 1;
    }

    typedef ProxySystemMatrixOrdering<std::vector<long>, std::vector<long>> MatrixOrdering;
    MatrixOrdering systemOrdering(&rowOrdering, &colOrdering);

    // Build system
    log::cout() << "Building system..." << std::endl;

    SystemSolver system(true, false);
    system.assembly(matrix, systemOrdering);

    double *rhs = system.getRHSRawPtr();
    rhs[0] = 1101;
    rhs[1] =  895;
    rhs[2] =  713;
    rhs[3] =  555;
    rhs[4] =  421;
    rhs[5] =  311;
    rhs[6] =  225;
    rhs[7] =  163;
    rhs[8] =  125;
    rhs[9] =  111;
    system.restoreRHSRawPtr(rhs);

    double *initialSolution = system.getSolutionRawPtr();
    for (int i = 0; i < nRows; ++i) {
        initialSolution[i] = 0;
    }
    system.restoreSolutionRawPtr(initialSolution);

    // Solve system
    log::cout() << "Solving transposed system..." << std::endl;

    system.solve();

    log::cout() << std::setprecision(16) << std::scientific;

    const double *solution = system.getSolutionRawReadPtr();
    for (int i = 0; i < nRows; ++i) {
        log::cout() << "  Solution[" << i << "] = " << solution[i] << std::endl;

        double expectedSolution = i + 1;
        if (!utils::DoubleFloatingEqual()(solution[i], expectedSolution, 1e-10)) {
            log::cout() << "  Expected solution[" << i << "] = " << expectedSolution << std::endl;
            log::cout() << "  Error[" << i << "] = " << (expectedSolution - solution[i]) << std::endl;
            throw std::runtime_error("  The solution of the system doesn't match the expected one.");
        }
    }
    system.restoreSolutionRawReadPtr(solution);

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
    log::manager().initialize(log::COMBINED);

    // Run the subtests
    log::cout() << "Testing solution of transposed linear systems." << std::endl;

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
