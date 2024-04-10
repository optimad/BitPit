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

#ifndef __BITPIT_POINT_KERNEL_HPP__
#define __BITPIT_POINT_KERNEL_HPP__

#include "patch_kernel.hpp"

namespace bitpit {

class PointKernel : public PatchKernel {

public:
    int getVolumeCodimension() const override;
    int getSurfaceCodimension() const override;
    int getLineCodimension() const override;
    int getPointCodimension() const override;

    double evalCellSize(long id) const override;
    virtual double evalPointsDistance(long id1, long id2) const;
    virtual std::array<double, 3> evalPointsDirection(long id1, long id2) const;

protected:
#if BITPIT_ENABLE_MPI==1
    PointKernel(MPI_Comm communicator, AdaptionMode adaptionMode);
    PointKernel(int dimension, MPI_Comm communicator, AdaptionMode adaptionMode);
    PointKernel(int id, int dimension, MPI_Comm communicator, AdaptionMode adaptionMode);
#else
    PointKernel(AdaptionMode adaptionMode);
    PointKernel(int dimension, AdaptionMode adaptionMode);
    PointKernel(int id, int dimension, AdaptionMode adaptionMode);
#endif

private:
    void initialize();

};

}

#endif
