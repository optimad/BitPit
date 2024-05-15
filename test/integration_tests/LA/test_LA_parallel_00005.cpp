/*---------------------------------------------------------------------------*\
 *
 *  bitpit
 *
 *  Copyright (C) 2015-2023 OPTIMAD engineering Srl
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

#include "bitpit_common.hpp"
#include "bitpit_LA.hpp"

#include <mpi.h>

using namespace bitpit;

const double SOLVER_RTOL = 1e-7;

/*!
 * Solve the test linear system using the specified split strategy.
 *
 * \param rank is the rank of the process
 * \param nProcs is the number of processes
 * \param splitStrategy is the strategy that will be used for splitting the system
 */
int solve_linear_system(int rank, int nProcs, SystemSplitStrategy splitStrategy)
{
    int nRows;
    int nCols;
    int nNZ;
    if (nProcs == 1) {
        nRows = 10;
        nCols = 10;
        nNZ   = 10;
    } else if (nProcs == 2) {
        if (rank == 0) {
            nRows = 6;
            nCols = 6;
            nNZ   = 6;
        } if (rank == 1) {
            nRows = 4;
            nCols = 4;
            nNZ   = 4;
        }
    } else {
        if (rank == 0) {
            nRows = 2;
            nCols = 2;
            nNZ   = 2;
        } if (rank == 1) {
            nRows = 5;
            nCols = 5;
            nNZ   = 5;
        } if (rank == 2) {
            nRows = 3;
            nCols = 3;
            nNZ   = 3;
        } else {
            nRows = 0;
            nCols = 0;
            nNZ   = 0;
        }
    }

    int blockSize = 5;

    // Build matrix
    log::cout() << "Building matrix..." << std::endl;

    std::vector<long> rowPattern(1);
    std::vector<double> rowValues(blockSize * blockSize);

    SparseMatrix matrix(MPI_COMM_WORLD, true, blockSize, nRows, nCols, nNZ);

    int rowOffset = matrix.getRowGlobalOffset();
    int colOffset = matrix.getColGlobalOffset();

    for (int i = 0; i < nRows; ++i) {
        rowPattern[0] = colOffset + i;
        for (int k = 0; k < blockSize; ++k) {
            int kk = linearalgebra::linearIndexRowMajor(k, k, blockSize, blockSize);

            rowValues[kk]  = 1. / (double) (blockSize * (rowOffset + i) + k + 1);
        }

        matrix.addRow(rowPattern, rowValues);
    }

    matrix.assembly();

    // Build solver
    log::cout() << "Building solver..." << std::endl;

    bool multigrid = false;
    bool debug     = false;
    bool transpose = false;
    SplitSystemSolver solver(transpose, multigrid, debug);

    std::vector<int> splitSizes(2);
    splitSizes[0] = 4;
    splitSizes[1] = blockSize - splitSizes[0];

    solver.assembly(matrix, splitStrategy, splitSizes);

    double *rhs = solver.getRHSRawPtr();
    for (int i = 0; i < nCols; ++i) {
        int field = 0;
        for (std::size_t split = 0; split < splitSizes.size(); ++split) {
            for (int k = 0; k < splitSizes[split]; ++k) {
                rhs[blockSize * i + field] = std::pow(10, split);
                ++field;

            }
        }
    }
    solver.restoreRHSRawPtr(rhs);

    double *initialSolution = solver.getSolutionRawPtr();
    for (int i = 0; i < nRows; ++i) {
        initialSolution[i] = 0;
    }
    solver.restoreSolutionRawPtr(initialSolution);

    // Set KSP options
    for (int i = 0; i < solver.getSplitCount(); ++i) {
        KSPOptions &options = solver.getSplitKSPOptions(i);
        options.rtol = SOLVER_RTOL;
    }

    // Solve system
    log::cout() << "Solve linear system..." << std::endl;
    solver.solve();

    for (int split = 0; split < solver.getSplitCount(); ++split) {
        const KSPStatus &status = solver.getSplitKSPStatus(split);
        log::cout() << "  Split #" << split << " convergence reason: " << status.convergence << " in its: " << status.its << std::endl;
    }
    log::cout() << "Linear system solved." << std::endl;

    // Compare computed solution with expected one
    //
    // Set comparison tolerance to the linear system relative tolerance
    log::cout() << "Comparing solutions..." << std::endl;

    log::cout() << "    Tolerance = " << SOLVER_RTOL << std::endl;

    std::size_t nRowElements = solver.getRowElementCount();

    const double *solution = solver.getSolutionRawReadPtr();
    for (std::size_t i = 0; i < nRowElements; ++i) {
        std::size_t globalDOF = rowOffset * blockSize + i;

        // Check for absolute tolerance only
        double expectedSolution;
        if (globalDOF == 0) {
            expectedSolution = 1;
        } else if ((globalDOF == 1) || ((globalDOF + 1) % blockSize) != 0) {
            expectedSolution = static_cast<double>(globalDOF + 1);
        } else {
            expectedSolution = static_cast<double>(10 * (globalDOF + 1));
        }

        if (!utils::DoubleFloatingEqual()(expectedSolution - solution[i], 0.0, SOLVER_RTOL, SOLVER_RTOL)) {
            std::stringstream message;
            message << "Solutions do not match for global DOF #" << globalDOF << ".";
            message << " Expected solution is " << expectedSolution;
            message << ", current solution is " << solution[i] << ".";
            log::cout() << message.str() << std::endl;
            throw std::runtime_error("  The solution of the system doesn't match the expected one.");
        }
    }
    solver.restoreSolutionRawReadPtr(solution);

    return 0;
}

/*!
 * Subtest 001
 *
 * \param rank is the rank of the process
 * \param nProcs is the number of processes
 *
 * Testing solution of a split system in DIAGONAL mode.
 */
int subtest_001(int rank, int nProcs)
{
    log::cout() << std::endl;
    log::cout() << ">> Testing solution of a split system in DIAGONAL mode" << std::endl;

    return solve_linear_system(rank, nProcs, SystemSplitStrategy::SYSTEM_SPLIT_STRATEGY_DIAGONAL);
}

/*!
 * Subtest 002
 *
 * \param rank is the rank of the process
 * \param nProcs is the number of processes
 *
 * Testing solution of a split system in LOWER mode.
 */
int subtest_002(int rank, int nProcs)
{
    log::cout() << std::endl;
    log::cout() << ">> Testing solution of a split system in LOWER mode" << std::endl;

    return solve_linear_system(rank, nProcs, SystemSplitStrategy::SYSTEM_SPLIT_STRATEGY_LOWER);
}

/*!
 * Subtest 003
 *
 * \param rank is the rank of the process
 * \param nProcs is the number of processes
 *
 * Testing solution of a split system in FULL mode.
 */
int subtest_003(int rank, int nProcs)
{
    log::cout() << std::endl;
    log::cout() << ">> Testing solution of a split system in FULL mode" << std::endl;

    return solve_linear_system(rank, nProcs, SystemSplitStrategy::SYSTEM_SPLIT_STRATEGY_FULL);
}

/*!
* Main program.
*/
int main(int argc, char *argv[])
{
    MPI_Init(&argc,&argv);

    // Initialize the logger
    int nProcs;
    int rank;
    MPI_Comm_size(MPI_COMM_WORLD, &nProcs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    log::manager().initialize(log::COMBINED, true, nProcs, rank);
    log::cout().setDefaultVisibility(log::GLOBAL);

    // Run the subtests
    log::cout() << "Testing solution of block linear systems..." << std::endl;

    int status;
    try {
        status = subtest_001(rank, nProcs);
        if (status != 0) {
            return status;
        }

        status = subtest_002(rank, nProcs);
        if (status != 0) {
            return status;
        }

        status = subtest_003(rank, nProcs);
        if (status != 0) {
            return status;
        }
    } catch (const std::exception &exception) {
        log::cout() << exception.what();
        exit(1);
    }

    MPI_Finalize();
}
