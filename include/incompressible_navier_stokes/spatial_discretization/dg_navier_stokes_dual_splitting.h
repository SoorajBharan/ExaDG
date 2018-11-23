/*
 * dg_navier_stokes_dual_splitting.h
 *
 *  Created on: Jun 27, 2016
 *      Author: fehn
 */

#ifndef INCLUDE_INCOMPRESSIBLE_NAVIER_STOKES_SPATIAL_DISCRETIZATION_DG_NAVIER_STOKES_DUAL_SPLITTING_H_
#define INCLUDE_INCOMPRESSIBLE_NAVIER_STOKES_SPATIAL_DISCRETIZATION_DG_NAVIER_STOKES_DUAL_SPLITTING_H_

#include "dg_navier_stokes_projection_methods.h"
#include "dual_splitting_boundary_conditions.h"

#include "../interface_space_time/operator.h"

namespace IncNS
{
template<int dim, int degree_u, int degree_p = degree_u - 1, typename Number = double>
class DGNavierStokesDualSplitting
  : public DGNavierStokesProjectionMethods<dim, degree_u, degree_p, Number>,
    public Interface::OperatorDualSplitting<Number>
{
public:
  typedef DGNavierStokesProjectionMethods<dim, degree_u, degree_p, Number> PROJECTION_METHODS_BASE;

  typedef typename PROJECTION_METHODS_BASE::VectorType VectorType;

  typedef VectorizedArray<Number>                 scalar;
  typedef Tensor<1, dim, VectorizedArray<Number>> vector;

  typedef typename PROJECTION_METHODS_BASE::Postprocessor Postprocessor;

  typedef DGNavierStokesDualSplitting<dim, degree_u, degree_p, Number> THIS;

  DGNavierStokesDualSplitting(parallel::distributed::Triangulation<dim> const & triangulation,
                              InputParameters<dim> const &                      parameters_in,
                              std::shared_ptr<Postprocessor>                    postprocessor_in)
    : PROJECTION_METHODS_BASE(triangulation, parameters_in, postprocessor_in),
      sum_alphai_ui(nullptr),
      evaluation_time(0.0),
      scaling_factor_time_derivative_term(1.0)
  {
  }

  virtual ~DGNavierStokesDualSplitting()
  {
  }

  void
  setup_solvers(double const & scaling_factor_time_derivative_term);

  /*
   *  implicit solution of convective step
   */
  void
  solve_nonlinear_convective_problem(VectorType &       dst,
                                     VectorType const & sum_alphai_ui,
                                     double const &     eval_time,
                                     double const &     scaling_factor_mass_matrix_term,
                                     unsigned int &     newton_iterations,
                                     unsigned int &     linear_iterations);

  /*
   *  The implementation of the Newton solver requires that the underlying operator
   *  implements a function called "evaluate_nonlinear_residual"
   */
  void
  evaluate_nonlinear_residual(VectorType & dst, VectorType const & src);

  /*
   * The implementation of the Newton solver requires that the underlying operator
   * implements a function called "initialize_vector_for_newton_solver"
   */
  void
  initialize_vector_for_newton_solver(VectorType & src) const
  {
    this->initialize_vector_velocity(src);
  }

  /*
   * The implementation of the Newton solver requires that the underlying operator
   * implements a function called "set_solution_linearization"
   */
  void
  set_solution_linearization(VectorType const & solution_linearization)
  {
    this->convective_operator.set_solution_linearization(solution_linearization);
  }

  /*
   *  To solve the linearized convective problem, the underlying operator
   *  has to implement a function called "vmult"
   */
  void
  vmult(VectorType & dst, VectorType const & src) const;

  /*
   *  This function calculates the matrix vector product for the linearized convective problem.
   */
  void
  apply_linearized_convective_problem(VectorType & dst, VectorType const & src) const;

  // convective term
  void
  evaluate_convective_term_and_apply_inverse_mass_matrix(VectorType &       dst,
                                                         VectorType const & src,
                                                         double const       evaluation_time) const;

  // body forces
  void
  evaluate_body_force_and_apply_inverse_mass_matrix(VectorType & dst,
                                                    double const evaluation_time) const;

  // rhs pressure: velocity divergence
  void
  apply_velocity_divergence_term(VectorType & dst, VectorType const & src) const;

  void
  rhs_velocity_divergence_term(VectorType & dst, double const & evaluation_time) const;

  void
  rhs_ppe_div_term_body_forces_add(VectorType & dst, double const & eval_time);

  void
  rhs_ppe_div_term_convective_term_add(VectorType & dst, VectorType const & src) const;

  // rhs pressure
  void
  rhs_ppe_nbc_add(VectorType & dst, double const & evaluation_time);

  // rhs pressure: Neumann BC convective term
  void
  rhs_ppe_convective_add(VectorType & dst, VectorType const & src) const;

  // rhs pressure: Neumann BC viscous term
  void
  rhs_ppe_viscous_add(VectorType & dst, VectorType const & src) const;

  void
  rhs_ppe_laplace_add(VectorType & dst, double const & evaluation_time) const
  {
    PROJECTION_METHODS_BASE::do_rhs_ppe_laplace_add(dst, evaluation_time);
  }

  // viscous step
  unsigned int
  solve_viscous(VectorType &       dst,
                VectorType const & src,
                double const &     scaling_factor_time_derivative_term);

  // apply Helmholtz operator
  void
  apply_helmholtz_operator(VectorType & dst, VectorType const & src) const;

  void
  rhs_add_viscous_term(VectorType & dst, double const evaluation_time) const
  {
    PROJECTION_METHODS_BASE::do_rhs_add_viscous_term(dst, evaluation_time);
  }

  unsigned int
  solve_pressure(VectorType & dst, VectorType const & src) const
  {
    return PROJECTION_METHODS_BASE::do_solve_pressure(dst, src);
  }

  void
  do_postprocessing(VectorType const & velocity,
                    VectorType const & intermediate_velocity,
                    VectorType const & pressure,
                    double const       time,
                    unsigned int const time_step_number) const;

  void
  do_postprocessing_steady_problem(VectorType const & velocity,
                                   VectorType const & intermediate_velocity,
                                   VectorType const & pressure) const;

private:
  // setup of solvers
  void
  setup_convective_solver();

  virtual void
  setup_pressure_poisson_solver();

  void
  setup_helmholtz_solver(double const & scaling_factor_time_derivative_term);

  // rhs pressure: boundary conditions for intermediate velocity u_hat
  // body force term
  void
  local_rhs_ppe_div_term_body_forces(
    MatrixFree<dim, Number> const &               data,
    VectorType &                                  dst,
    VectorType const &                            src,
    std::pair<unsigned int, unsigned int> const & cell_range) const;

  void
  local_rhs_ppe_div_term_body_forces_face(
    MatrixFree<dim, Number> const &               data,
    VectorType &                                  dst,
    VectorType const &                            src,
    std::pair<unsigned int, unsigned int> const & face_range) const;

  void
  local_rhs_ppe_div_term_body_forces_boundary_face(
    MatrixFree<dim, Number> const &               data,
    VectorType &                                  dst,
    VectorType const &                            src,
    std::pair<unsigned int, unsigned int> const & face_range) const;

  // rhs pressure: NBC term
  void
  local_rhs_ppe_nbc_add(MatrixFree<dim, Number> const &               data,
                        VectorType &                                  dst,
                        VectorType const &                            src,
                        std::pair<unsigned int, unsigned int> const & cell_range) const;

  void
  local_rhs_ppe_nbc_add_face(MatrixFree<dim, Number> const &               data,
                             VectorType &                                  dst,
                             VectorType const &                            src,
                             std::pair<unsigned int, unsigned int> const & face_range) const;

  void
  local_rhs_ppe_nbc_add_boundary_face(
    MatrixFree<dim, Number> const &               data,
    VectorType &                                  dst,
    VectorType const &                            src,
    std::pair<unsigned int, unsigned int> const & face_range) const;


  // Helmholtz
  MomentumOperator<dim, degree_u, Number> helmholtz_operator;

  std::shared_ptr<PreconditionerBase<Number>> helmholtz_preconditioner;

  std::shared_ptr<IterativeSolverBase<VectorType>> helmholtz_solver;

  // Implicit solution of convective step
  VectorType         temp;
  VectorType const * sum_alphai_ui;

  // implicit solution of convective step
  std::shared_ptr<InverseMassMatrixPreconditioner<dim, degree_u, Number, dim>>
    preconditioner_convective_problem;

  std::shared_ptr<IterativeSolverBase<VectorType>> linear_solver;
  std::shared_ptr<NewtonSolver<VectorType, THIS, THIS, IterativeSolverBase<VectorType>>>
    newton_solver;


  // rhs pressure Poisson equation: Velocity divergence term
  VelocityDivergenceConvectiveTerm<dim, degree_u, degree_p, Number>
    velocity_divergence_convective_term;

  // pressure Neumann BC
  PressureNeumannBCConvectiveTerm<dim, degree_u, degree_p, Number> pressure_nbc_convective_term;

  PressureNeumannBCViscousTerm<dim, degree_u, degree_p, Number> pressure_nbc_viscous_term;

  double evaluation_time;
  double scaling_factor_time_derivative_term;
};

template<int dim, int degree_u, int degree_p, typename Number>
void
DGNavierStokesDualSplitting<dim, degree_u, degree_p, Number>::setup_solvers(
  double const & scaling_factor_time_derivative_term)
{
  ConditionalOStream pcout(std::cout, Utilities::MPI::this_mpi_process(MPI_COMM_WORLD) == 0);
  pcout << std::endl << "Setup solvers ..." << std::endl;

  // initialize vectors that are needed by the nonlinear solver
  if(this->param.equation_type == EquationType::NavierStokes &&
     this->param.treatment_of_convective_term == TreatmentOfConvectiveTerm::Implicit)
  {
    setup_convective_solver();
  }

  this->setup_pressure_poisson_solver();

  this->setup_projection_solver();

  setup_helmholtz_solver(scaling_factor_time_derivative_term);

  pcout << std::endl << "... done!" << std::endl;
}

template<int dim, int degree_u, int degree_p, typename Number>
void
DGNavierStokesDualSplitting<dim, degree_u, degree_p, Number>::setup_convective_solver()
{
  this->initialize_vector_velocity(temp);

  // preconditioner implicit convective step
  preconditioner_convective_problem.reset(
    new InverseMassMatrixPreconditioner<dim, degree_u, Number, dim>(
      this->data, this->get_dof_index_velocity(), this->get_quad_index_velocity_linear()));

  // linear solver (GMRES)
  GMRESSolverData solver_data;
  solver_data.max_iter              = this->param.max_iter_linear_convective;
  solver_data.solver_tolerance_abs  = this->param.abs_tol_linear_convective;
  solver_data.solver_tolerance_rel  = this->param.rel_tol_linear_convective;
  solver_data.right_preconditioning = this->param.use_right_preconditioning_convective;
  solver_data.max_n_tmp_vectors     = this->param.max_n_tmp_vectors_convective;

  // always use inverse mass matrix preconditioner
  solver_data.use_preconditioner = true;

  // setup linear solver
  linear_solver.reset(
    new GMRESSolver<THIS, InverseMassMatrixPreconditioner<dim, degree_u, Number, dim>, VectorType>(
      *this, *preconditioner_convective_problem, solver_data));

  // setup Newton solver
  newton_solver.reset(new NewtonSolver<VectorType, THIS, THIS, IterativeSolverBase<VectorType>>(
    this->param.newton_solver_data_convective, *this, *this, *linear_solver));
}

template<int dim, int degree_u, int degree_p, typename Number>
void
DGNavierStokesDualSplitting<dim, degree_u, degree_p, Number>::setup_pressure_poisson_solver()
{
  // Call setup function of base class
  PROJECTION_METHODS_BASE::setup_pressure_poisson_solver();

  // RHS PPE: Velocity divergence term
  VelocityDivergenceConvectiveTermData<dim> velocity_divergence_convective_data;
  velocity_divergence_convective_data.dof_index_velocity = this->get_dof_index_velocity();
  velocity_divergence_convective_data.dof_index_pressure = this->get_dof_index_pressure();
  velocity_divergence_convective_data.quad_index = this->get_quad_index_velocity_nonlinear();
  velocity_divergence_convective_data.bc         = this->boundary_descriptor_velocity;

  velocity_divergence_convective_term.initialize(this->data, velocity_divergence_convective_data);

  // Pressure NBC: Convective term
  PressureNeumannBCConvectiveTermData<dim> pressure_nbc_convective_data;
  pressure_nbc_convective_data.dof_index_velocity = this->get_dof_index_velocity();
  pressure_nbc_convective_data.dof_index_pressure = this->get_dof_index_pressure();
  pressure_nbc_convective_data.quad_index         = this->get_quad_index_velocity_nonlinear();
  pressure_nbc_convective_data.bc                 = this->boundary_descriptor_pressure;

  pressure_nbc_convective_term.initialize(this->data, pressure_nbc_convective_data);

  // Pressure NBC: Viscous term
  PressureNeumannBCViscousTermData<dim> pressure_nbc_viscous_data;
  pressure_nbc_viscous_data.dof_index_velocity = this->get_dof_index_velocity();
  pressure_nbc_viscous_data.dof_index_pressure = this->get_quad_index_pressure();
  pressure_nbc_viscous_data.quad_index         = this->get_quad_index_velocity_linear();
  pressure_nbc_viscous_data.bc                 = this->boundary_descriptor_pressure;

  pressure_nbc_viscous_term.initialize(this->data,
                                       pressure_nbc_viscous_data,
                                       this->viscous_operator);
}


template<int dim, int degree_u, int degree_p, typename Number>
void
DGNavierStokesDualSplitting<dim, degree_u, degree_p, Number>::setup_helmholtz_solver(
  double const & scaling_factor_time_derivative_term)
{
  // 1. Setup Helmholtz operator

  // setup velocity convection-diffusion operator
  MomentumOperatorData<dim> momentum_operator_data;

  // unsteady problem
  momentum_operator_data.unsteady_problem = true;

  momentum_operator_data.scaling_factor_time_derivative_term = scaling_factor_time_derivative_term;

  // convective problem = false (dual splitting scheme!)
  momentum_operator_data.convective_problem = false;

  momentum_operator_data.dof_index      = this->get_dof_index_velocity();
  momentum_operator_data.quad_index_std = this->get_quad_index_velocity_linear();

  momentum_operator_data.use_cell_based_loops = this->param.use_cell_based_face_loops;
  momentum_operator_data.implement_block_diagonal_preconditioner_matrix_free =
    this->param.implement_block_diagonal_preconditioner_matrix_free;

  momentum_operator_data.mass_matrix_operator_data = this->mass_matrix_operator_data;
  momentum_operator_data.viscous_operator_data     = this->viscous_operator_data;
  momentum_operator_data.convective_operator_data  = this->convective_operator_data;

  helmholtz_operator.initialize(this->get_data(),
                                momentum_operator_data,
                                this->mass_matrix_operator,
                                this->viscous_operator,
                                this->convective_operator);

  // 2. Setup Helmholtz preconditioner

  if(this->param.preconditioner_viscous == PreconditionerViscous::InverseMassMatrix)
  {
    helmholtz_preconditioner.reset(new InverseMassMatrixPreconditioner<dim, degree_u, Number, dim>(
      this->data, this->get_dof_index_velocity(), this->get_quad_index_velocity_linear()));
  }
  else if(this->param.preconditioner_viscous == PreconditionerViscous::PointJacobi)
  {
    helmholtz_preconditioner.reset(
      new JacobiPreconditioner<MomentumOperator<dim, degree_u, Number>>(helmholtz_operator));
  }
  else if(this->param.preconditioner_viscous == PreconditionerViscous::BlockJacobi)
  {
    helmholtz_preconditioner.reset(
      new BlockJacobiPreconditioner<MomentumOperator<dim, degree_u, Number>>(helmholtz_operator));
  }
  else if(this->param.preconditioner_viscous == PreconditionerViscous::GeometricMultigrid)
  {
    MultigridData mg_data;
    mg_data = this->param.multigrid_data_viscous;

    // use single precision for multigrid
    typedef float MultigridNumber;

    typedef MyMultigridPreconditionerVelocityDiffusion<
      dim,
      Number,
      MomentumOperator<dim, degree_u, MultigridNumber>,
      MomentumOperator<dim, degree_u, Number>>
      MULTIGRID;

    helmholtz_preconditioner.reset(new MULTIGRID());

    std::shared_ptr<MULTIGRID> mg_preconditioner =
      std::dynamic_pointer_cast<MULTIGRID>(helmholtz_preconditioner);

    mg_preconditioner->initialize(mg_data,
                                  this->dof_handler_u,
                                  this->mapping,
                                  /*helmholtz_operator.get_operator_data().bc->dirichlet_bc,*/
                                  (void *)&helmholtz_operator.get_operator_data());
  }

  // 3. Setup Helmholtz solver
  if(this->param.solver_viscous == SolverViscous::PCG)
  {
    // setup solver data
    CGSolverData solver_data;
    // use default value of max_iter
    solver_data.solver_tolerance_abs = this->param.abs_tol_viscous;
    solver_data.solver_tolerance_rel = this->param.rel_tol_viscous;
    // default value of use_preconditioner = false
    if(this->param.preconditioner_viscous == PreconditionerViscous::PointJacobi ||
       this->param.preconditioner_viscous == PreconditionerViscous::BlockJacobi ||
       this->param.preconditioner_viscous == PreconditionerViscous::InverseMassMatrix ||
       this->param.preconditioner_viscous == PreconditionerViscous::GeometricMultigrid)
    {
      solver_data.use_preconditioner = true;
    }
    solver_data.update_preconditioner = this->param.update_preconditioner_viscous;

    // setup helmholtz solver
    helmholtz_solver.reset(
      new CGSolver<MomentumOperator<dim, degree_u, Number>, PreconditionerBase<Number>, VectorType>(
        helmholtz_operator, *helmholtz_preconditioner, solver_data));
  }
  else if(this->param.solver_viscous == SolverViscous::GMRES)
  {
    // setup solver data
    GMRESSolverData solver_data;
    // use default value of max_iter
    solver_data.solver_tolerance_abs = this->param.abs_tol_viscous;
    solver_data.solver_tolerance_rel = this->param.rel_tol_viscous;
    // use default value of right_preconditioning
    // use default value of max_n_tmp_vectors
    // use default value of compute_eigenvalues
    solver_data.update_preconditioner = this->param.update_preconditioner_viscous;

    // default value of use_preconditioner = false
    if(this->param.preconditioner_viscous == PreconditionerViscous::PointJacobi ||
       this->param.preconditioner_viscous == PreconditionerViscous::BlockJacobi ||
       this->param.preconditioner_viscous == PreconditionerViscous::InverseMassMatrix ||
       this->param.preconditioner_viscous == PreconditionerViscous::GeometricMultigrid)
    {
      solver_data.use_preconditioner = true;
    }

    // setup helmholtz solver
    helmholtz_solver.reset(
      new GMRESSolver<MomentumOperator<dim, degree_u, Number>,
                      PreconditionerBase<Number>,
                      VectorType>(helmholtz_operator, *helmholtz_preconditioner, solver_data));
  }
  else if(this->param.solver_viscous == SolverViscous::FGMRES)
  {
    FGMRESSolverData solver_data;
    // use default value of max_iter
    solver_data.solver_tolerance_abs = this->param.abs_tol_viscous;
    solver_data.solver_tolerance_rel = this->param.rel_tol_viscous;
    // use default value of max_n_tmp_vectors
    solver_data.update_preconditioner = this->param.update_preconditioner_viscous;

    if(this->param.preconditioner_viscous == PreconditionerViscous::PointJacobi ||
       this->param.preconditioner_viscous == PreconditionerViscous::BlockJacobi ||
       this->param.preconditioner_viscous == PreconditionerViscous::InverseMassMatrix ||
       this->param.preconditioner_viscous == PreconditionerViscous::GeometricMultigrid)
    {
      solver_data.use_preconditioner = true;
    }

    helmholtz_solver.reset(
      new FGMRESSolver<MomentumOperator<dim, degree_u, Number>,
                       PreconditionerBase<Number>,
                       VectorType>(helmholtz_operator, *helmholtz_preconditioner, solver_data));
  }
  else
  {
    AssertThrow(this->param.solver_viscous == SolverViscous::PCG ||
                  this->param.solver_viscous == SolverViscous::GMRES ||
                  this->param.solver_viscous == SolverViscous::FGMRES,
                ExcMessage(
                  "Specified Viscous Solver not implemented - possibilities are PCG and GMRES"));
  }
}

template<int dim, int degree_u, int degree_p, typename Number>
void
DGNavierStokesDualSplitting<dim, degree_u, degree_p, Number>::solve_nonlinear_convective_problem(
  VectorType &       dst,
  VectorType const & sum_alphai_ui,
  double const &     eval_time,
  double const &     scaling_factor_mass_matrix_term,
  unsigned int &     newton_iterations,
  unsigned int &     linear_iterations)
{
  // Set sum_alphai_ui, this variable is used when evaluating the nonlinear residual
  this->sum_alphai_ui = &sum_alphai_ui;

  // set evaluation time for both the linear and the nonlinear operator
  // (=DGNavierStokesDualSplitting)
  evaluation_time = eval_time;

  // set scaling_factor_time_derivative term for both the linear and the nonlinear operator
  // (=DGNavierStokesDualSplitting)
  scaling_factor_time_derivative_term = scaling_factor_mass_matrix_term;

  // solve nonlinear problem
  newton_solver->solve(dst, newton_iterations, linear_iterations);

  // Reset sum_alphai_ui
  this->sum_alphai_ui = nullptr;
}

template<int dim, int degree_u, int degree_p, typename Number>
void
DGNavierStokesDualSplitting<dim, degree_u, degree_p, Number>::evaluate_nonlinear_residual(
  VectorType &       dst,
  VectorType const & src)
{
  if(this->param.right_hand_side == true)
  {
    this->body_force_operator.evaluate(dst, evaluation_time);
    // shift body force term to the left-hand side of the equation
    dst *= -1.0;
  }
  else // right_hand_side == false
  {
    // set dst to zero. This is necessary since the subsequent operators
    // call functions of type ..._add
    dst = 0.0;
  }

  // temp, src, sum_alphai_ui have the same number of blocks
  temp.equ(scaling_factor_time_derivative_term, src);
  temp.add(-1.0, *sum_alphai_ui);

  this->mass_matrix_operator.apply_add(dst, temp);

  this->convective_operator.evaluate_add(dst, src, evaluation_time);
}

template<int dim, int degree_u, int degree_p, typename Number>
void
DGNavierStokesDualSplitting<dim, degree_u, degree_p, Number>::vmult(VectorType &       dst,
                                                                    VectorType const & src) const
{
  apply_linearized_convective_problem(dst, src);
}

template<int dim, int degree_u, int degree_p, typename Number>
void
DGNavierStokesDualSplitting<dim, degree_u, degree_p, Number>::apply_linearized_convective_problem(
  VectorType &       dst,
  VectorType const & src) const
{
  this->mass_matrix_operator.apply(dst, src);

  dst *= scaling_factor_time_derivative_term;

  this->convective_operator.apply_add(dst, src, evaluation_time);
}

template<int dim, int degree_u, int degree_p, typename Number>
void
DGNavierStokesDualSplitting<dim, degree_u, degree_p, Number>::
  evaluate_convective_term_and_apply_inverse_mass_matrix(VectorType &       dst,
                                                         VectorType const & src,
                                                         double const       evaluation_time) const
{
  this->convective_operator.evaluate(dst, src, evaluation_time);
  this->inverse_mass_matrix_operator->apply(dst, dst);
}

template<int dim, int degree_u, int degree_p, typename Number>
void
DGNavierStokesDualSplitting<dim, degree_u, degree_p, Number>::
  evaluate_body_force_and_apply_inverse_mass_matrix(VectorType & dst,
                                                    double const evaluation_time) const
{
  this->body_force_operator.evaluate(dst, evaluation_time);

  this->inverse_mass_matrix_operator->apply(dst, dst);
}

template<int dim, int degree_u, int degree_p, typename Number>
void
DGNavierStokesDualSplitting<dim, degree_u, degree_p, Number>::apply_velocity_divergence_term(
  VectorType &       dst,
  VectorType const & src) const
{
  this->divergence_operator.apply(dst, src);
}

template<int dim, int degree_u, int degree_p, typename Number>
void
DGNavierStokesDualSplitting<dim, degree_u, degree_p, Number>::rhs_velocity_divergence_term(
  VectorType &   dst,
  double const & evaluation_time) const
{
  this->divergence_operator.rhs(dst, evaluation_time);
}

template<int dim, int degree_u, int degree_p, typename Number>
void
DGNavierStokesDualSplitting<dim, degree_u, degree_p, Number>::rhs_ppe_div_term_body_forces_add(
  VectorType &   dst,
  double const & eval_time)
{
  evaluation_time = eval_time;

  VectorType src_dummy;
  this->data.loop(&THIS::local_rhs_ppe_div_term_body_forces,
                  &THIS::local_rhs_ppe_div_term_body_forces_face,
                  &THIS::local_rhs_ppe_div_term_body_forces_boundary_face,
                  this,
                  dst,
                  src_dummy);
}

template<int dim, int degree_u, int degree_p, typename Number>
void
DGNavierStokesDualSplitting<dim, degree_u, degree_p, Number>::local_rhs_ppe_div_term_body_forces(
  MatrixFree<dim, Number> const &,
  VectorType &,
  VectorType const &,
  std::pair<unsigned int, unsigned int> const &) const
{
}

template<int dim, int degree_u, int degree_p, typename Number>
void
DGNavierStokesDualSplitting<dim, degree_u, degree_p, Number>::
  local_rhs_ppe_div_term_body_forces_face(MatrixFree<dim, Number> const &,
                                          VectorType &,
                                          VectorType const &,
                                          std::pair<unsigned int, unsigned int> const &) const
{
}

template<int dim, int degree_u, int degree_p, typename Number>
void
DGNavierStokesDualSplitting<dim, degree_u, degree_p, Number>::
  local_rhs_ppe_div_term_body_forces_boundary_face(
    MatrixFree<dim, Number> const & data,
    VectorType &                    dst,
    VectorType const &,
    std::pair<unsigned int, unsigned int> const & face_range) const
{
  unsigned int dof_index_pressure  = this->get_dof_index_pressure();
  unsigned int quad_index_pressure = this->get_quad_index_pressure();

  FEFaceEvaluation<dim, degree_p, degree_p + 1, 1, Number> fe_eval(data,
                                                                   true,
                                                                   dof_index_pressure,
                                                                   quad_index_pressure);

  for(unsigned int face = face_range.first; face < face_range.second; face++)
  {
    fe_eval.reinit(face);

    BoundaryTypeU boundary_type =
      this->boundary_descriptor_velocity->get_boundary_type(data.get_boundary_id(face));

    for(unsigned int q = 0; q < fe_eval.n_q_points; ++q)
    {
      if(boundary_type == BoundaryTypeU::Dirichlet)
      {
        Point<dim, scalar> q_points = fe_eval.quadrature_point(q);

        // evaluate right-hand side
        vector rhs = evaluate_vectorial_function(this->field_functions->right_hand_side,
                                                 q_points,
                                                 evaluation_time);

        scalar flux_times_normal = rhs * fe_eval.get_normal_vector(q);
        // minus sign is introduced here which allows to call a function of type ...add()
        // and avoids a scaling of the resulting vector by the factor -1.0
        fe_eval.submit_value(-flux_times_normal, q);
      }
      else if(boundary_type == BoundaryTypeU::Neumann || boundary_type == BoundaryTypeU::Symmetry)
      {
        // Do nothing on Neumann and Symmetry boundaries.
        // Remark: on symmetry boundaries we prescribe g_u * n = 0, and also g_{u_hat}*n = 0 in case
        // of the dual splitting scheme. This is in contrast to Dirichlet boundaries where we
        // prescribe a consistent boundary condition for g_{u_hat} derived from the convective step
        // of the dual splitting scheme which differs from the DBC g_u. Applying this consistent DBC
        // to symmetry boundaries and using g_u*n=0 as well as exploiting symmetry, we obtain
        // g_{u_hat}*n=0 on symmetry boundaries. Hence, there are no inhomogeneous contributions for
        // g_{u_hat}*n.
        scalar zero = make_vectorized_array<Number>(0.0);
        fe_eval.submit_value(zero, q);
      }
      else
      {
        AssertThrow(false, ExcMessage("Boundary type of face is invalid or not implemented."));
      }
    }
    fe_eval.integrate(true, false);
    fe_eval.distribute_local_to_global(dst);
  }
}

template<int dim, int degree_u, int degree_p, typename Number>
void
DGNavierStokesDualSplitting<dim, degree_u, degree_p, Number>::rhs_ppe_div_term_convective_term_add(
  VectorType &       dst,
  VectorType const & src) const
{
  velocity_divergence_convective_term.calculate(dst, src);
}

template<int dim, int degree_u, int degree_p, typename Number>
void
DGNavierStokesDualSplitting<dim, degree_u, degree_p, Number>::rhs_ppe_nbc_add(
  VectorType &   dst,
  double const & eval_time)
{
  evaluation_time = eval_time;

  VectorType src_dummy;
  this->data.loop(&THIS::local_rhs_ppe_nbc_add,
                  &THIS::local_rhs_ppe_nbc_add_face,
                  &THIS::local_rhs_ppe_nbc_add_boundary_face,
                  this,
                  dst,
                  src_dummy);
}

template<int dim, int degree_u, int degree_p, typename Number>
void
DGNavierStokesDualSplitting<dim, degree_u, degree_p, Number>::local_rhs_ppe_nbc_add(
  MatrixFree<dim, Number> const &,
  VectorType &,
  VectorType const &,
  std::pair<unsigned int, unsigned int> const &) const
{
}

template<int dim, int degree_u, int degree_p, typename Number>
void
DGNavierStokesDualSplitting<dim, degree_u, degree_p, Number>::local_rhs_ppe_nbc_add_face(
  MatrixFree<dim, Number> const &,
  VectorType &,
  VectorType const &,
  std::pair<unsigned int, unsigned int> const &) const
{
}


template<int dim, int degree_u, int degree_p, typename Number>
void
DGNavierStokesDualSplitting<dim, degree_u, degree_p, Number>::local_rhs_ppe_nbc_add_boundary_face(
  MatrixFree<dim, Number> const & data,
  VectorType &                    dst,
  VectorType const &,
  std::pair<unsigned int, unsigned int> const & face_range) const
{
  unsigned int dof_index_pressure  = this->get_dof_index_pressure();
  unsigned int quad_index_pressure = this->get_quad_index_pressure();

  FEFaceEvaluation<dim, degree_p, degree_p + 1, 1, Number> fe_eval(data,
                                                                   true,
                                                                   dof_index_pressure,
                                                                   quad_index_pressure);

  for(unsigned int face = face_range.first; face < face_range.second; face++)
  {
    fe_eval.reinit(face);

    types::boundary_id boundary_id = data.get_boundary_id(face);
    BoundaryTypeP      boundary_type =
      this->boundary_descriptor_pressure->get_boundary_type(boundary_id);

    for(unsigned int q = 0; q < fe_eval.n_q_points; ++q)
    {
      if(boundary_type == BoundaryTypeP::Neumann)
      {
        Point<dim, scalar> q_points = fe_eval.quadrature_point(q);

        // evaluate right-hand side
        vector rhs = evaluate_vectorial_function(this->field_functions->right_hand_side,
                                                 q_points,
                                                 evaluation_time);

        // evaluate boundary condition
        typename std::map<types::boundary_id, std::shared_ptr<Function<dim>>>::iterator it =
          this->boundary_descriptor_pressure->neumann_bc.find(boundary_id);
        vector dudt = evaluate_vectorial_function(it->second, q_points, evaluation_time);

        vector normal = fe_eval.get_normal_vector(q);

        scalar h = -normal * (dudt - rhs);

        fe_eval.submit_value(h, q);
      }
      else if(boundary_type == BoundaryTypeP::Dirichlet)
      {
        scalar zero = make_vectorized_array<Number>(0.0);
        fe_eval.submit_value(zero, q);
      }
      else
      {
        AssertThrow(false, ExcMessage("Boundary type of face is invalid or not implemented."));
      }
    }
    fe_eval.integrate(true, false);
    fe_eval.distribute_local_to_global(dst);
  }
}


template<int dim, int degree_u, int degree_p, typename Number>
void
DGNavierStokesDualSplitting<dim, degree_u, degree_p, Number>::rhs_ppe_convective_add(
  VectorType &       dst,
  VectorType const & src) const
{
  pressure_nbc_convective_term.calculate(dst, src);
}

template<int dim, int degree_u, int degree_p, typename Number>
void
DGNavierStokesDualSplitting<dim, degree_u, degree_p, Number>::rhs_ppe_viscous_add(
  VectorType &       dst,
  VectorType const & src) const
{
  pressure_nbc_viscous_term.calculate(dst, src);
}

template<int dim, int degree_u, int degree_p, typename Number>
unsigned int
DGNavierStokesDualSplitting<dim, degree_u, degree_p, Number>::solve_viscous(VectorType &       dst,
                                                                            VectorType const & src,
                                                                            double const & factor)
{
  // Update Helmholtz operator
  helmholtz_operator.set_scaling_factor_time_derivative_term(factor);

  unsigned int n_iter = helmholtz_solver->solve(dst, src);

  return n_iter;
}

template<int dim, int degree_u, int degree_p, typename Number>
void
DGNavierStokesDualSplitting<dim, degree_u, degree_p, Number>::apply_helmholtz_operator(
  VectorType &       dst,
  VectorType const & src) const
{
  // Update Helmholtz operator
  helmholtz_operator.vmult(dst, src);
}

template<int dim, int degree_u, int degree_p, typename Number>
void
DGNavierStokesDualSplitting<dim, degree_u, degree_p, Number>::do_postprocessing(
  VectorType const & velocity,
  VectorType const & intermediate_velocity,
  VectorType const & pressure,
  double const       time,
  unsigned int const time_step_number) const
{
  bool const standard = true;
  if(standard)
  {
    this->postprocessor->do_postprocessing(
      velocity, intermediate_velocity, pressure, time, time_step_number);
  }
  else // consider pressure error and velocity error
  {
    VectorType velocity_error;
    this->initialize_vector_velocity(velocity_error);

    VectorType pressure_error;
    this->initialize_vector_pressure(pressure_error);

    this->prescribe_initial_conditions(velocity_error, pressure_error, time);

    velocity_error.add(-1.0, velocity);
    pressure_error.add(-1.0, pressure);

    this->postprocessor->do_postprocessing(
      velocity_error, intermediate_velocity, pressure_error, time, time_step_number);
  }
}

template<int dim, int degree_u, int degree_p, typename Number>
void
DGNavierStokesDualSplitting<dim, degree_u, degree_p, Number>::do_postprocessing_steady_problem(
  VectorType const & velocity,
  VectorType const & intermediate_velocity,
  VectorType const & pressure) const
{
  this->postprocessor->do_postprocessing(velocity, intermediate_velocity, pressure);
}

} // namespace IncNS

#endif /* INCLUDE_INCOMPRESSIBLE_NAVIER_STOKES_SPATIAL_DISCRETIZATION_DG_NAVIER_STOKES_DUAL_SPLITTING_H_ \
        */
