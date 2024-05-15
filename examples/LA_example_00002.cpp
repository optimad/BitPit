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

#include <array>
#if BITPIT_ENABLE_MPI==1
#include <mpi.h>
#endif

#include "bitpit_common.hpp"
#include "bitpit_LA.hpp"

using namespace bitpit;

const double SOLVER_RTOL   = 1e-7;
const double SOLVER_ATOL   = 1e-50;
const int    SOLVER_MAXITS = 100;

/*!
 * \example LA_example_00001.cpp
 *
 * \brief This example shows how to dump and restore a SplitSystemSolver.
 *
 * This example builds a small linear system, it dumps it to the disk,
 * it solves the linear system and it exports the solution to disk.
 * Dump files will be saved in the current directory and they names are:
 *   LA_example_0002_linear_system_info.dat
 *   LA_example_0002_linear_system_A_split_00.dat.info
 *   LA_example_0002_linear_system_A_split_00.dat
 *   LA_example_0002_linear_system_A_split_01.dat.info
 *   LA_example_0002_linear_system_A_split_01.dat
 *   LA_example_0002_linear_system_A_split_10.dat.info
 *   LA_example_0002_linear_system_A_split_10.dat
 *   LA_example_0002_linear_system_A_split_11.dat.info
 *   LA_example_0002_linear_system_A_split_11.dat
 *   LA_example_0002_linear_system_rhs_split_0.dat.info
 *   LA_example_0002_linear_system_rhs_split_0.dat
 *   LA_example_0002_linear_system_rhs_split_1.dat.info
 *   LA_example_0002_linear_system_rhs_split_1.dat
 *   LA_example_0002_linear_system_solution_split_0.dat.info
 *   LA_example_0002_linear_system_solution_split_0.dat
 *   LA_example_0002_linear_system_solution_split_1.dat.info
 *   LA_example_0002_linear_system_solution_split_1.dat
 *
 * This example contains an extension of the bare SplitSystemSolver class
 * adding the ability to solve using algebraic multigrid (PETSc GAMG)
 * preconditioner. Only non-block matrices can be solved using GAMG.
 * A check disables algebraic multigrid for block matrices with a
 * message in the logger.
 *
 * <b>To run</b>: ./LA_example_00001.cpp \n
 */

/*!
 * \class AMGSplitSystemSolver
 *
 * \brief The class AMGSplitSystemSolver is derived form SplitSystemSolver and it
 * allows to solve the linear system using PETSc GAMG preconditioner.
 * Only non-block matrices can be solved using GAMG.
 */
class AMGSplitSystemSolver : public SplitSystemSolver {

public:
    /*!
     * Constructor.
     *
     * \param transpose if true the solver solves the transposed linear system.
     * \param multigrid if true the solver uses algebraic multigrid as preconditioner.
     * \param debug if true PETSc monitors on residual, reason and singular values are turned on.
     */
    AMGSplitSystemSolver(bool transpose, bool multigrid, bool debug)
        : SplitSystemSolver(multigrid, transpose, debug),
          m_multigrid(multigrid)
    {
    }

protected:
    const static PetscInt GAMG_MAX_LEVELS;
    const static PetscReal GAMG_THRESHOLD;
    const static PetscReal GAMG_THRESHOLD_SCALE;

    bool m_multigrid;

    /*!
    * Set up the specified preconditioner using the given options.
    *
    * \param pc is the preconditioner to set up
    * \param options are the options that will be used to set up the preconditioner
    */
    void setupPreconditioner(PC pc, const KSPOptions &options) const override
    {
        // Early return for non-multigrid
        if (!m_multigrid) {
            SplitSystemSolver::setupPreconditioner(pc, options);
            return;
        }

        // Set multigrid options
        KSPGetPC(this->m_KSP, &pc);
        PCSetType(pc, PCGAMG);

        std::vector<PetscReal> thresholds(GAMG_MAX_LEVELS);
        for (int i = 0; i < GAMG_MAX_LEVELS; ++i) {
            thresholds[i] = GAMG_THRESHOLD;
        }

        PCGAMGSetNSmooths(pc, 0);
#if (PETSC_VERSION_MAJOR >= 3 && PETSC_VERSION_MINOR <= 17)
#if BITPIT_ENABLE_MPI == 1
        if (this->isPartitioned()) {
            PCGAMGSetSymGraph(pc, PETSC_TRUE);
        }
#endif
#endif

        PCGAMGSetNlevels(pc, GAMG_MAX_LEVELS);
#if (PETSC_VERSION_MAJOR >= 3 && PETSC_VERSION_MINOR >= 16)
        PCGAMGSetThreshold(pc, thresholds.data(), GAMG_MAX_LEVELS);
        PCGAMGSetThresholdScale(pc, GAMG_THRESHOLD_SCALE);
#else
        PCGAMGSetThreshold(pc, GAMG_THRESHOLD);
#endif

        // Setup
        PCSetUp(pc);

        // Set options for the multigrid coarse level
        KSP coarseKSP;
        PCMGGetCoarseSolve(pc, &coarseKSP);

        PC coarsePreconditioner;
        KSPGetPC(coarseKSP, &coarsePreconditioner);
        PCSetType(coarsePreconditioner, PCSVD);
    }
};

const PetscInt AMGSplitSystemSolver::GAMG_MAX_LEVELS       = 10;
const PetscReal AMGSplitSystemSolver::GAMG_THRESHOLD       = 0.02;
const PetscReal AMGSplitSystemSolver::GAMG_THRESHOLD_SCALE = 1.0;

/*!
 * Run the example that show how to dump a systemSolver.
 *
 * \param rank is the rank of the process
 * \param nProcs is the number of processes
 */
int run_dump(int rank, int nProcs)
{
    log::cout() << std::endl;
    log::cout() << "Running dump example..." << std::endl;

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

#if BITPIT_ENABLE_MPI==1
    SparseMatrix matrix(MPI_COMM_WORLD, true, blockSize, nRows, nCols, nNZ);

    int rowOffset = matrix.getRowGlobalOffset();
    int colOffset = matrix.getColGlobalOffset();
#else
    SparseMatrix matrix(blockSize, nRows, nCols, nNZ);

    int rowOffset = 0;
    int colOffset = 0;
#endif

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
    bool debug     = true;
    bool transpose = false;
    AMGSplitSystemSolver solver(transpose, multigrid, debug);

    std::vector<int> splitSizes(2);
    splitSizes[0] = 4;
    splitSizes[1] = blockSize - splitSizes[0];

    solver.assembly(matrix, SystemSplitStrategy::SYSTEM_SPLIT_STRATEGY_FULL, splitSizes);

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

    // Export system
    solver.exportMatrix("LA_example_0002_matrix.txt", SystemSolver::FILE_ASCII);
    solver.exportRHS("LA_example_0002_rhs.txt", SystemSolver::FILE_ASCII);
    solver.exportSolution("LA_example_0002_solution.txt", SystemSolver::FILE_ASCII);

    // Dump system
    log::cout() << "Dumping initialized linear system..." << std::endl;

    solver.dumpSystem("bitpit linear system", ".", "LA_example_0002_linear_system_");

    // Set KSP options
    KSPOptions options;
    options.atol             = SOLVER_ATOL;
    options.rtol             = SOLVER_RTOL;
    options.maxits           = SOLVER_MAXITS;
    options.initial_non_zero = PETSC_FALSE;

    KSPOptions &externalOptions = solver.getKSPOptions();
    externalOptions = options;

    for (int i = 0; i < solver.getSplitCount(); ++i) {
        KSPOptions &splitOptions = solver.getSplitKSPOptions(i);
        splitOptions = options;
    }

    // Solve System
    log::cout() << "Solve linear system..." << std::endl;
    solver.solve();

    const KSPStatus &externalStatus = solver.getKSPStatus();
    log::cout() << "  External convergence reason: " << externalStatus.convergence << " in its: " << externalStatus.its << std::endl;

    for (int split = 0; split < solver.getSplitCount(); ++split) {
        const KSPStatus &splitStatus = solver.getSplitKSPStatus(split);
        log::cout() << "  Split #" << split << " convergence reason: " << splitStatus.convergence << " in its: " << splitStatus.its << std::endl;
    }
    log::cout() << "Linear system solved." << std::endl;

    // Export solution
    log::cout() << "Export solution..." << std::endl;
    solver.exportSolution("LA_example_0002_linear_system_solution.dat", bitpit::SystemSolver::FILE_BINARY);

    // Compare computed solution with expected one
    //
    // Set comparison tolerance to the linear system relative tolerance
    log::cout() << "Comparing solutions..." << std::endl;

    log::cout() << "    Tolerance = " << SOLVER_RTOL << std::endl;

    std::size_t nRowElements = solver.getRowElementCount();

    bool isSolutionCorrect = true;
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
            isSolutionCorrect = false;
        }
    }
    solver.restoreSolutionRawReadPtr(solution);
    if (isSolutionCorrect) {
        log::cout() << "Solutions match." << std::endl;
    }

    return 0;
}

/*!
 * Run the example that show how to restore a systemSolver.
 *
 * \param rank is the rank of the process
 * \param nProcs is the number of processes
 */
int run_restore(int rank, int nProcs)
{
    BITPIT_UNUSED(rank);
    BITPIT_UNUSED(nProcs);

    log::cout() << std::endl;
    log::cout() << "Running restore example..." << std::endl;

    // Restore solver with initial guess
    log::cout() << "Restoring linear system ..." << std::endl;
    bool multigrid = false;
    bool debug     = true;
    bool transpose = false;
    AMGSplitSystemSolver solver(transpose, multigrid, debug);

#if BITPIT_ENABLE_MPI==1
    solver.restoreSystem(MPI_COMM_WORLD, ".", "LA_example_0002_linear_system_");
#else
    solver.restoreSystem(".", "LA_example_0002_linear_system_");
#endif
    log::cout() << "Linear system restored." << std::endl;

    // Export system
    solver.exportMatrix("LA_example_0002_restored_matrix.txt", SystemSolver::FILE_ASCII);
    solver.exportRHS("LA_example_0002_restored_rhs.txt", SystemSolver::FILE_ASCII);
    solver.exportSolution("LA_example_0002_restored_solution.txt", SystemSolver::FILE_ASCII);

    // Set KSP options
    KSPOptions options;
    options.atol             = SOLVER_ATOL;
    options.rtol             = SOLVER_RTOL;
    options.maxits           = SOLVER_MAXITS;
    options.initial_non_zero = PETSC_FALSE;

    KSPOptions &externalOptions = solver.getKSPOptions();
    externalOptions = options;

    for (int i = 0; i < solver.getSplitCount(); ++i) {
        KSPOptions &splitOptions = solver.getSplitKSPOptions(i);
        splitOptions = options;
    }

    // Solve linear system
    log::cout() << "Solving linear system..." << std::endl;
    solver.solve();

    const KSPStatus &externalStatus = solver.getKSPStatus();
    log::cout() << "  External convergence reason: " << externalStatus.convergence << " in its: " << externalStatus.its << std::endl;

    for (int split = 0; split < solver.getSplitCount(); ++split) {
        const KSPStatus &splitStatus = solver.getSplitKSPStatus(split);
        log::cout() << "  Split #" << split << " convergence reason: " << splitStatus.convergence << " in its: " << splitStatus.its << std::endl;
    }
    log::cout() << "Linear system solved." << std::endl;

    // Turn solution comparison on
    bool compareSolutions = true;
    if (compareSolutions) {
        // Store expected solution for comparison
        log::cout() << "Storing computed solution for comparison..." << std::endl;
        std::size_t nRowElements = solver.getRowElementCount();
        std::vector<double> expectedSolution(nRowElements, 0.0);
        double *computedSolution = solver.getSolutionRawPtr();
        for (std::size_t i = 0; i < nRowElements; ++i) {
            expectedSolution[i] = computedSolution[i];
        }
        solver.restoreSolutionRawPtr(computedSolution);

        // Import solution with solution from outside
        log::cout() << "Restoring solution..." << std::endl;
        solver.importSolution("LA_example_0002_linear_system_solution.dat");
        log::cout() << "Solution restored." << std::endl;

        // Compare computed solution with restored one
        //
        // Set comparison tolerance to the linear system relative tolerance
        log::cout() << "Comparing solutions..." << std::endl;

        log::cout() << "    Tolerance = " << SOLVER_RTOL << std::endl;

        bool isSolutionCorrect = true;
        const double *solution = solver.getSolutionRawReadPtr();
        for (std::size_t i = 0; i < nRowElements; ++i) {
            if (!utils::DoubleFloatingEqual()(expectedSolution[i] - solution[i], 0.0, SOLVER_RTOL, SOLVER_RTOL)) {
                std::stringstream message;
                message << "Solutions do not match for local DOF #" << i << ".";
                message << " Expected solution is " << expectedSolution[i];
                message << ", current solution is " << solution[i] << ".";
                log::cout() << message.str() << std::endl;
                isSolutionCorrect = false;
            }
        }
        solver.restoreSolutionRawReadPtr(solution);
        if (isSolutionCorrect) {
            log::cout() << "Solutions match." << std::endl;
        }
    }

    return 0;
}

/*!
 * Main program.
 */
int main(int argc, char *argv[])
{
#if BITPIT_ENABLE_MPI==1
    MPI_Init(&argc, &argv);
#else
    BITPIT_UNUSED(argc);
    BITPIT_UNUSED(argv);
#endif

    // Initialize the logger
    int nProcs;
    int rank;
#if BITPIT_ENABLE_MPI==1
    MPI_Comm_size(MPI_COMM_WORLD, &nProcs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#else
    rank = 0;
    nProcs = 1;
#endif
    log::manager().initialize(log::COMBINED, true, nProcs, rank);
    log::cout().setDefaultVisibility(log::GLOBAL);

    // Run the subtests
    log::cout() << "Build, dump, solve and restore split linear system" << std::endl;

    int status;
    try {
        status = run_dump(rank, nProcs);
        if (status != 0) {
            return status;
        }

        status = run_restore(rank, nProcs);
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
