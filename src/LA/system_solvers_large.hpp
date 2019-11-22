/*---------------------------------------------------------------------------*\
 *
 *  bitpit
 *
 *  Copyright (C) 2015-2019 OPTIMAD engineering Srl
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

#ifndef __BITPIT_SYSTEM_SOLVERS_LARGE_HPP__
#define __BITPIT_SYSTEM_SOLVERS_LARGE_HPP__

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <petscksp.h>

#include "system_matrix.hpp"

namespace bitpit {

struct KSPOptions {
    PetscInt restart;
    PetscInt maxits;
    PetscScalar rtol;

    PetscInt overlap;
    PetscInt levels;

    PetscInt sublevels;
    PetscScalar subrtol;

    KSPOptions()
        : restart(PETSC_DEFAULT), maxits(PETSC_DEFAULT), rtol(PETSC_DEFAULT),
          overlap(PETSC_DEFAULT), levels(PETSC_DEFAULT),
          sublevels(PETSC_DEFAULT), subrtol(PETSC_DEFAULT)
    {
    }
};

struct KSPStatus {
    PetscErrorCode error;
    PetscInt its;
    KSPConvergedReason convergence;

    KSPStatus()
        : error(0), its(-1), convergence(KSP_DIVERGED_BREAKDOWN)
    {
    }
};

class SystemSolver {

public:
    enum DumpFormat {
        DUMP_BINARY,
        DUMP_ASCII
    };

    static void addInitOption(const std::string &option);
    static void addInitOptions(int argc, char **args);
    static void addInitOptions(const std::vector<std::string> &options);
    static void clearInitOptions();

    SystemSolver(bool debug = false);
    SystemSolver(const std::string &prefix, bool debug = false);

    virtual ~SystemSolver();

    void clear();

    void setPermutations(long nRows, const long *rowRanks, long nCols, const long *colRanks);

    void assembly(const SparseMatrix &matrix);
    bool isAssembled() const;

    void update(const std::vector<long> &rows, const SparseMatrix &elements);

    void setUp();
    bool isSetUp() const;

    long getRowCount() const;
    long getColCount() const;
#if BITPIT_ENABLE_MPI==1
    long getRowGlobalCount() const;
    long getColGlobalCount() const;

    bool isPartitioned() const;
#endif

    void solve();
    void solve(const std::vector<double> &rhs, std::vector<double> *solution);

    void dump(const std::string &directory, const std::string &prefix = "",
              DumpFormat matrixFormat = DUMP_BINARY, DumpFormat rhsFormat = DUMP_BINARY,
              DumpFormat solutionFormat = DUMP_BINARY) const;

    void setNullSpace();
    void unsetNullSpace();

    KSPOptions & getKSPOptions();
    const KSPOptions & getKSPOptions() const;
    const KSPStatus & getKSPStatus() const;

    double * getRHSRawPtr();
    const double * getRHSRawPtr() const;
    const double * getRHSRawReadPtr() const;
    void restoreRHSRawPtr(double *raw_rhs);
    void restoreRHSRawReadPtr(const double *raw_rhs) const;

    double * getSolutionRawPtr();
    const double * getSolutionRawPtr() const;
    const double * getSolutionRawReadPtr() const;
    void restoreSolutionRawPtr(double *raw_solution);
    void restoreSolutionRawReadPtr(const double *raw_solution) const;

protected:
    Mat m_A;
    Vec m_rhs;
    Vec m_solution;

    KSP m_KSP;
    KSPOptions m_KSPOptions;
    KSPStatus m_KSPStatus;

    void matrixInit(const SparseMatrix &matrix);
    void matrixFill(const SparseMatrix &matrix);
    void matrixUpdate(const std::vector<long> &rows, const SparseMatrix &elements);

#if BITPIT_ENABLE_MPI == 1
    void vectorsInit(const std::vector<long> &ghosts);
#else
    void vectorsInit();
#endif
    void vectorsPermute(bool invert);
    void vectorsFill(const std::vector<double> &rhs, std::vector<double> *solution);
    void vectorsExport(std::vector<double> *solution);

    virtual void preKSPSetupActions();
    virtual void postKSPSetupActions();

    virtual void preKSPSolveActions();
    virtual void postKSPSolveActions();

#if BITPIT_ENABLE_MPI==1
    const MPI_Comm & getCommunicator() const;
#endif

private:
    static int m_nInstances;
    static bool m_optionsEditable;
    static std::vector<std::string> m_options;

    std::string m_prefix;

    bool m_assembled;
    bool m_setUp;

#if BITPIT_ENABLE_MPI==1
    MPI_Comm m_communicator;

    bool m_partitioned;

    long m_rowGlobalOffset;
    long m_colGlobalOffset;
#endif

    IS m_rowPermutation;
    IS m_colPermutation;

#if BITPIT_ENABLE_MPI==1
    void setCommunicator(MPI_Comm communicator);
    void freeCommunicator();
#endif

    void resetPermutations();

};

}

#endif
