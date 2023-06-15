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

# ifndef __BITPIT_LEVELSET_COMMON_HPP__
# define __BITPIT_LEVELSET_COMMON_HPP__

// Standard Template Library
# include <array>
# include <unordered_map>
# include <unordered_set>
# include <vector>

namespace bitpit{

/*!
 * @ingroup levelset
 * @brief namespace containing default values
 */
namespace levelSetDefaults{
    const double                            VALUE = 1.e18 ;             /**< Default value for levelset function */
    const std::array<double,3>              GRADIENT = {{0.,0.,0.}};    /**< Default value for levelset gradient */
    const std::array<double,3>              POINT = {{0.,0.,0.}};       /**< Default value for levelset projection point */
    const short                             SIGN = 1;                   /**< Default value for the sign */
    const double                            SIZE = 1.e18 ;              /**< Default value for surface feature */
    const int                               OBJECT = -1 ;               /**< Default value for closest object  */
    const int                               PART  = -1 ;                /**< Default value for closest patch  */
    const long                              SUPPORT = -1 ;              /**< Default value for closest support element */
    const double                            NARROWBAND_SIZE = -1 ;      /**< Default value for the narrowband size */
};

struct LevelSetInfo{
    double                                  value ;                     /**< Levelset value */
    std::array<double,3>                    gradient ;                  /**< Levelset gradient */

    LevelSetInfo() ;
    LevelSetInfo( double , const std::array<double,3> &) ;
};

/*!
 * @ingroup levelsetEnums
 * Enum class defining different boolean operations
 */
enum class LevelSetBooleanOperation{
    UNION =0,                                                           /**< Union between two objects */
    INTERSECTION =1,                                                    /**< Intersection between two objects */
    SUBTRACTION =2                                                      /**< Substract object two from object one */
};

/*!
 * @ingroup levelsetEnums
 * Enum class defining the intersection between a cell and the zero levelset.
 * For details see LevelSetObject::intersectSurface()
 */
enum class LevelSetIntersectionStatus{
    FALSE=0,                                                            /**< Cell does not intersect zero levelset */
    TRUE=1,                                                             /**< Cell does intersect zero levelset */
    CLOSE=2                                                             /**< Zero levelset lies within tangent and bounding radius of a cell */
};

/*!
 * @ingroup levelsetEnums
 * Enum class describing the how the intersection between cell and zero levelset 
 * should be computed
 */
enum class LevelSetIntersectionMode{
    FAST_FUZZY=0,                   /**< Compares levelset value to tangent and bounding radius of a cell */
    FAST_GUARANTEE_TRUE=1,          /**< All LevelSetIntersectionStatus::TRUE are accurate but LevelSetIntersectionStatus::FALSE may be wrong */
    FAST_GUARANTEE_FALSE=2,         /**< All LevelSetIntersectionStatus::FALSE are accurate but LevelSetIntersectionStatus::TRUE may be wrong */
    ACCURATE=3                      /**< Accurate but more costly checks */
};

/*!
 * @ingroup levelsetEnums
 * Enum class containing the possible sotrage modes.
 */
enum class LevelSetStorageType{
    SPARSE=0,                       /**< Sparse storage, to be used when only a small portion of the domain is inside the narrow band */
    DENSE=1,                        /**< Dense storage, to be used when almost all the domain is inside the narrow band */
};

/*!
 * @ingroup levelsetEnums
 * Enum class containing the possible level set fields
 */
enum class LevelSetField{
    BEGIN = 0,
    VALUE = BEGIN,                  /**< level set value */
    GRADIENT,                       /**< level set gradient */
    PART,                           /**< part identifier at projection point */
    NORMAL,                         /**< body normal at projection point */
    END,
    COUNT = END - BEGIN
};

/*!
 * Hasher for the LevelSetWriteField enum.
 */
struct LevelSetFieldHasher
{
    std::size_t operator()(const LevelSetField &field) const
    {
        return static_cast<int>(field);
    }
};

/*!
 * Set of field
 */
typedef std::unordered_set<LevelSetField> LevelSetFieldset;

/*!
 * Map of write fields.
 */
template<typename value_t>
using LevelSetFieldMap = std::unordered_map<LevelSetField, value_t, LevelSetFieldHasher>;

/*!
 * @ingroup levelsetEnums
 * Enum class containing the possible fields to be added to the VTK file
 */
enum class LevelSetWriteField{
    VALUE    = static_cast<int>(LevelSetField::VALUE),     /**< adds level set value to VTK*/
    GRADIENT = static_cast<int>(LevelSetField::GRADIENT),  /**< adds level set gradient to VTK*/
    PART     = static_cast<int>(LevelSetField::PART),      /**< adds part identifier at projection point to VTK*/
    NORMAL   = static_cast<int>(LevelSetField::NORMAL),    /**< adds body normal at projection point to VTK*/
    ALL,                                                   /**< adds level set value, gradient, normal and projection point to VTK*/
    DEFAULT                                                /**< adds levelset value and gradient to VTK*/
};

}

#endif
