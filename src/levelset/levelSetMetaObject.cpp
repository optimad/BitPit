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

# include "bitpit_common.hpp"

# include "levelSetObject.hpp"
# include "levelSetMetaObject.hpp"

namespace bitpit {

/*!
	@interface LevelSetMetaObject
	@ingroup levelset
	@brief Interface class for all objects, which depend on other LevelSetObjects
*/

/*!
 * Constructor
 */
LevelSetMetaObject::LevelSetMetaObject(int id) : LevelSetObject(id){
}

/*!
 * If the object is primary 
 * @return true
*/
bool LevelSetMetaObject::isPrimary() const{
    return false;
} 

}
