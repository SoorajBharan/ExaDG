/*
 * enum_types.h
 *
 *  Created on: Dec 20, 2018
 *      Author: fehn
 */

#ifndef INCLUDE_CONVECTION_DIFFUSION_USER_INTERFACE_ENUM_TYPES_H_
#define INCLUDE_CONVECTION_DIFFUSION_USER_INTERFACE_ENUM_TYPES_H_

#include <string>

namespace ConvDiff
{
/**************************************************************************************/
/*                                                                                    */
/*                                 MATHEMATICAL MODEL                                 */
/*                                                                                    */
/**************************************************************************************/

/*
 *  ProblemType describes whether a steady or an unsteady problem has to be solved
 */
enum class ProblemType
{
  Undefined,
  Steady,
  Unsteady
};

std::string
enum_to_string(ProblemType const enum_type);

/*
 *  EquationType describes the physical/mathematical model that has to be solved,
 *  i.e., diffusion problem, convective problem or convection-diffusion problem
 */
enum class EquationType
{
  Undefined,
  Convection,
  Diffusion,
  ConvectionDiffusion
};

std::string
enum_to_string(EquationType const enum_type);

/*
 * This parameter describes the type of velocity field for the convective term.
 * Analytical means that an analytical velocity field is prescribed, while Numerical
 * means that a discrete (potentially discontinuous) velocity field is prescribed that
 * is the result of numerical computations, e.g., the velocity field obtained as the
 * solution of the incompressible Navier-Stokes equations.
 */
enum class TypeVelocityField
{
  Analytical,
  Numerical
};

std::string
enum_to_string(TypeVelocityField const enum_type);

/**************************************************************************************/
/*                                                                                    */
/*                                 PHYSICAL QUANTITIES                                */
/*                                                                                    */
/**************************************************************************************/

// there are currently no enums for this section



/**************************************************************************************/
/*                                                                                    */
/*                             TEMPORAL DISCRETIZATION                                */
/*                                                                                    */
/**************************************************************************************/

/*
 *  Temporal discretization method:
 *  ExplRK: Explicit Runge-Kutta methods (implemented for orders 1-4)
 *  BDF: backward differentiation formulae (implemented for order 1-3)
 */
enum class TemporalDiscretization
{
  Undefined,
  ExplRK,
  BDF
};

std::string
enum_to_string(TemporalDiscretization const enum_type);

/*
 *  For the BDF time integrator, the convective term can be either
 *  treated explicitly or implicitly
 */
enum class TreatmentOfConvectiveTerm
{
  Undefined,
  Explicit,    // additive decomposition (IMEX)
  ExplicitOIF, // operator-integration-factor splitting (Maday et al. 1990)
  Implicit
};

std::string
enum_to_string(TreatmentOfConvectiveTerm const enum_type);

/*
 *  Temporal discretization method for OIF splitting:
 *
 *    Explicit Runge-Kutta methods
 */
enum class TimeIntegratorRK
{
  Undefined,
  ExplRK1Stage1,
  ExplRK2Stage2,
  ExplRK3Stage3,
  ExplRK4Stage4,
  ExplRK3Stage4Reg2C,
  ExplRK3Stage7Reg2, // optimized for maximum time step sizes in DG context
  ExplRK4Stage5Reg2C,
  ExplRK4Stage8Reg2, // optimized for maximum time step sizes in DG context
  ExplRK4Stage5Reg3C,
  ExplRK5Stage9Reg2S
};

std::string
enum_to_string(TimeIntegratorRK const enum_type);

/*
 * calculation of time step size
 */
enum class TimeStepCalculation
{
  Undefined,
  UserSpecified,
  CFL,
  Diffusion,
  CFLAndDiffusion,
  MaxEfficiency
};

std::string
enum_to_string(TimeStepCalculation const enum_type);

/**************************************************************************************/
/*                                                                                    */
/*                               SPATIAL DISCRETIZATION                               */
/*                                                                                    */
/**************************************************************************************/

/*
 * Triangulation type
 */
enum class TriangulationType
{
  Undefined,
  Distributed,
  FullyDistributed
};

std::string
enum_to_string(TriangulationType const enum_type);

/*
 *  Mapping type (polynomial degree)
 */
enum class MappingType
{
  Affine,
  Isoparametric
};

std::string
enum_to_string(MappingType const enum_type);

/*
 *  Numerical flux formulation of convective term
 */

enum class NumericalFluxConvectiveOperator
{
  Undefined,
  CentralFlux,
  LaxFriedrichsFlux
};

std::string
enum_to_string(NumericalFluxConvectiveOperator const enum_type);

/**************************************************************************************/
/*                                                                                    */
/*                                       SOLVER                                       */
/*                                                                                    */
/**************************************************************************************/

/*
 *   Solver for linear system of equations
 */
enum class Solver
{
  Undefined,
  CG,
  GMRES,
  FGMRES // flexible GMRES
};

std::string
enum_to_string(Solver const enum_type);

/*
 *  Preconditioner type for solution of linear system of equations
 */
enum class Preconditioner
{
  Undefined,
  None,
  InverseMassMatrix,
  PointJacobi,
  BlockJacobi,
  Multigrid
};

std::string
enum_to_string(Preconditioner const enum_type);

/*
 * Specify the operator type to be used for multigrid (which can differ from the
 * equation type)
 */
enum class MultigridOperatorType
{
  Undefined,
  ReactionDiffusion,
  ReactionConvection,
  ReactionConvectionDiffusion
};

std::string
enum_to_string(MultigridOperatorType const enum_type);

/**************************************************************************************/
/*                                                                                    */
/*                               OUTPUT AND POSTPROCESSING                            */
/*                                                                                    */
/**************************************************************************************/

// there are currently no enums for this section

} // namespace ConvDiff



#endif /* INCLUDE_CONVECTION_DIFFUSION_USER_INTERFACE_ENUM_TYPES_H_ */
