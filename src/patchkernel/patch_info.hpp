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

#ifndef __BITPIT_PATCH_INFO_HPP__
#define __BITPIT_PATCH_INFO_HPP__

#include "bitpit_common.hpp"

#include <unordered_map>
#include <vector>

namespace bitpit {

class PatchKernel;

class PatchInfo {

public:
	virtual ~PatchInfo() = default;

	PatchKernel const & getPatch() const;
	void setPatch(PatchKernel const *patch);

	void reset();
	void extract();
	void update();

protected:
	PatchKernel const *m_patch;

	PatchInfo(PatchKernel const *patch);

	void setPatch(PatchKernel const *patch, bool initialize);

	virtual void _init() = 0;
	virtual void _reset() = 0;
	virtual void _extract() = 0;

};

class PatchNumberingInfo : public PatchInfo {

public:
	PatchNumberingInfo(PatchKernel const *patch = nullptr);

	std::size_t getCellConsecutiveOffset() const;
	std::size_t getCellConsecutiveId(long id) const;
	const std::unordered_map<long, std::size_t> & getCellConsecutiveMap() const;

#if BITPIT_ENABLE_MPI==1
	std::size_t getCellGlobalCount() const;
	std::size_t getCellGlobalCountOffset() const;
	std::size_t getCellGlobalCountOffset(int rank) const;

	BITPIT_DEPRECATED_FOR(std::size_t getCellGlobalId(long id) const, long getCellConsecutiveId(std::size_t id) const);
	BITPIT_DEPRECATED_FOR(const std::unordered_map<long BITPIT_COMMA std::size_t> & getCellGlobalMap() const, const std::unordered_map<long BITPIT_COMMA std::size_t> & getCellConsecutiveMap() const);

	int getCellOwnerFromLocal(long id) const;
	int getCellOwnerFromConsecutive(std::size_t id) const;
	BITPIT_DEPRECATED_FOR(int getCellOwnerFromGlobal(std::size_t id) const, int getCellOwnerFromConsecutive(std::size_t id) const);
#endif

protected:
	void _init() override;
	void _reset() override;
	void _extract() override;

private:
	std::size_t m_cellConsecutiveOffset;
	std::unordered_map<long, std::size_t> m_cellLocalToConsecutiveMap;
#if BITPIT_ENABLE_MPI==1
	std::vector<size_t> m_nGlobalInternalCells;
#endif

};

}

#endif
