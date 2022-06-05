/**
 * @file FAA_Common.hpp
 * @author Andrew Eliseev (JointPoints), 2022, github.com/jointpoints
 */
#ifndef __FASOLVER_FAA_COMMON_HPP__
#define __FASOLVER_FAA_COMMON_HPP__





#include "../SubjectType/SubjectType.hpp"
#include "../Logger/Logger.hpp"

#define FACILITY_ARRANGEMENT_ALGORITHM(name)                                                                    \
template                                                                                                        \
<                                                                                                               \
    typename DistanceType,                                                                                      \
    typename CoordinateType,                                                                                    \
    typename AreaInputType,                                                                                     \
    typename SubjectCountInputType,                                                                             \
    typename SubjectCountOutputType,                                                                            \
    typename UnitInputType,                                                                                     \
    typename UnitOutputType,                                                                                    \
    typename PriceType                                                                                          \
>                                                                                                               \
void name                                                                                                       \
(                                                                                                               \
    UnaryMap<Point<CoordinateType, AreaInputType, SubjectCountOutputType>> &points,                             \
    PlanarMetric<DistanceType> const &distance,                                                                 \
    BinaryPairMap<UnitOutputType> &flows,                                                                       \
    UnaryMap<SubjectType<AreaInputType, SubjectCountInputType, UnitInputType, PriceType>> const &subject_types, \
    BinaryMap<UnitInputType> const &total_flows,                                                                \
    Logger const &logger,                                                                                       \
    bool const warm_start = false                                                                               \
)





/**
 * @namespace faa
 * @brief Facility arrangement algorithms
 *
 * Algorithms contained in this namespace are building blocks used to perform an
 * arrangement strategy set by a user.
 * 
 * @subsection faa_development_policy Development policy
 * Each member of this namespace \b must satisfy the following requirements:
 * 
 * * it \b must be a function template accepting the following types in the exactly same
 * order:
 *   * \c DistanceType,
 *   * \c CoordinateType,
 *   * \c AreaInputType,
 *   * \c SubjectCountInputType,
 *   * \c SubjectCountOutputType,
 *   * \c UnitInputType,
 *   * \c UnitOutputType,
 *   * \c PriceType,
 * * it \b must be of \c void type,
 * * it \b must accept formal arguments listed below in the exactly same order:
 * 
 * @param points An instance of `UnaryMap<Point<CoordinateType, AreaInputType, SubjectCountOutputType>> &`.
 *               This parameter will be supplied with a reference to Facility::_points.
 * @param distance An instance of `PlanarMetric<DistanceType> const &`. This parameter
 *                 will be supplied with a reference to Facility::_distance.
 * @param flows An instance of `BinaryPairMap<UnitOutputType> &`. This parameter will be
 *              supplied with a reference to Facility::_flows.
 * @param subject_types An instance of `UnaryMap<SubjectType<AreaInputType, SubjectCountInputType, UnitInputType, PriceType>> const &`.
 *                      This parameter will be supplied with a map of descriptions of
 *                      available subject types that a user needs to arrange within the
 *                      facility.
 * @param total_flows An instance of `BinaryMap<UnitInputType> const &`. This parameter
 *                    will be supplied with data of total desired flows from all subjects
 *                    of each type into all subjects of each type.
 * @param logger An instance of `Logger const &`. This parameter will be supplied with a
 *               reference to a logger where logs \b should be written.
 * @param warm_start An instance of `bool const`. This parameter specified whether an
 *                   arrangement that is present in the concerned instance of Facility
 *                   shall be viewed as a starting point for its further improvement
 *                   (\c true) or it shall be erased and the process shall start over
 *                   (\c false). If the sequence of algorithms in the arrangement
 *                   strategy has length at least 2, all algorithms except, maybe, the
 *                   first one will be supplied with value \c true for \c warm_start.
 * 
 * Each facility arrangement algorithm is expected to arrange subjects and assign object
 * flows between them within the facility explicitly explained with parameters \c points,
 * \c distance and \c flows. All modifications \c must be done in place meaning that
 * the final arrangement \c must be saved in parameters \c points and \c flows which
 * will, evidently, affect the internal members of the original Facility instance.
 * 
 * One can easily add a new member into this namespace by declaring/defining a function
 * template with the help of pre-defined macro `FACILITY_ARRANGEMENT_FUNCTION(name)`,
 * which automatically expands to the correct signature.
 */
namespace faa {}





#endif // __FASOLVER_FAA_COMMON_HPP__
