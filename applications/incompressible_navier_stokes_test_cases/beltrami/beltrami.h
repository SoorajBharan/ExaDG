/*
 * beltrami.h
 *
 *  Created on: Aug 18, 2016
 *      Author: fehn
 */

#ifndef APPLICATIONS_INCOMPRESSIBLE_NAVIER_STOKES_TEST_CASES_BELTRAMI_H_
#define APPLICATIONS_INCOMPRESSIBLE_NAVIER_STOKES_TEST_CASES_BELTRAMI_H_

namespace IncNS
{
namespace Beltrami
{
template<int dim>
class AnalyticalSolutionVelocity : public Function<dim>
{
public:
  AnalyticalSolutionVelocity(double const viscosity) : Function<dim>(dim, 0.0), nu(viscosity)
  {
  }

  double
  value(const Point<dim> & p, const unsigned int component = 0) const
  {
    double const t = this->get_time();
    double const a = 0.25 * numbers::PI;
    double const d = 2 * a;

    double result = 0.0;
    // clang-format off
    if (component == 0)
      result = -a*(std::exp(a*p[0])*std::sin(a*p[1]+d*p[2]) + std::exp(a*p[2])*std::cos(a*p[0]+d*p[1]))*std::exp(-nu*d*d*t);
    else if (component == 1)
      result = -a*(std::exp(a*p[1])*std::sin(a*p[2]+d*p[0]) + std::exp(a*p[0])*std::cos(a*p[1]+d*p[2]))*std::exp(-nu*d*d*t);
    else if (component == 2)
      result = -a*(std::exp(a*p[2])*std::sin(a*p[0]+d*p[1]) + std::exp(a*p[1])*std::cos(a*p[2]+d*p[0]))*std::exp(-nu*d*d*t);
    // clang-format on

    return result;
  }

private:
  double const nu;
};

template<int dim>
class AnalyticalSolutionPressure : public Function<dim>
{
public:
  AnalyticalSolutionPressure(double const viscosity)
    : Function<dim>(1 /*n_components*/, 0.0), nu(viscosity)
  {
  }

  double
  value(const Point<dim> & p, const unsigned int /*component*/) const
  {
    double const t = this->get_time();
    double const a = 0.25 * numbers::PI;
    double const d = 2 * a;

    // clang-format off
    double result = -a*a*0.5*(std::exp(2*a*p[0]) + std::exp(2*a*p[1]) + std::exp(2*a*p[2]) +
                     2*std::sin(a*p[0]+d*p[1])*std::cos(a*p[2]+d*p[0])*std::exp(a*(p[1]+p[2])) +
                     2*std::sin(a*p[1]+d*p[2])*std::cos(a*p[0]+d*p[1])*std::exp(a*(p[2]+p[0])) +
                     2*std::sin(a*p[2]+d*p[0])*std::cos(a*p[1]+d*p[2])*std::exp(a*(p[0]+p[1]))) * std::exp(-2*nu*d*d*t);
    // clang-format on

    return result;
  }

private:
  double const nu;
};

template<int dim>
class PressureBC_dudt : public Function<dim>
{
public:
  PressureBC_dudt(double const viscosity) : Function<dim>(dim, 0.0), nu(viscosity)
  {
  }

  double
  value(const Point<dim> & p, const unsigned int component = 0) const
  {
    double const t = this->get_time();
    double const a = 0.25 * numbers::PI;
    double const d = 2 * a;

    double result = 0.0;
    // clang-format off
    if (component == 0)
      result = a*nu*d*d*(std::exp(a*p[0])*std::sin(a*p[1]+d*p[2]) + std::exp(a*p[2])*std::cos(a*p[0]+d*p[1]))*std::exp(-nu*d*d*t);
    else if (component == 1)
      result = a*nu*d*d*(std::exp(a*p[1])*std::sin(a*p[2]+d*p[0]) + std::exp(a*p[0])*std::cos(a*p[1]+d*p[2]))*std::exp(-nu*d*d*t);
    else if (component == 2)
      result = a*nu*d*d*(std::exp(a*p[2])*std::sin(a*p[0]+d*p[1]) + std::exp(a*p[1])*std::cos(a*p[2]+d*p[0]))*std::exp(-nu*d*d*t);
    // clang-format on

    return result;
  }

private:
  double const nu;
};

template<int dim, typename Number>
class Application : public ApplicationBase<dim, Number>
{
public:
  Application() : ApplicationBase<dim, Number>("")
  {
  }

  Application(std::string input_file) : ApplicationBase<dim, Number>(input_file)
  {
    // parse application-specific parameters
    ParameterHandler prm;
    this->add_parameters(prm);
    parse_input(input_file, prm, true, true);
  }

  void
  add_parameters(ParameterHandler & prm)
  {
    // clang-format off
    prm.enter_subsection("Application");
      prm.add_parameter("OutputDirectory",  output_directory, "Directory where output is written.");
      prm.add_parameter("OutputName",       output_name,      "Name of output files.");
    prm.leave_subsection();
    // clang-format on
  }

  std::string output_directory = "output/beltrami/", output_name = "test";

  double const viscosity = 0.1;

  void
  set_input_parameters(InputParameters & param)
  {
    // MATHEMATICAL MODEL
    param.dim                         = 3;
    param.problem_type                = ProblemType::Unsteady;
    param.equation_type               = EquationType::NavierStokes;
    param.formulation_viscous_term    = FormulationViscousTerm::LaplaceFormulation;
    param.formulation_convective_term = FormulationConvectiveTerm::DivergenceFormulation;
    param.right_hand_side             = false;

    // PHYSICAL QUANTITIES
    param.start_time = 0.0;
    param.end_time   = 1.0;
    param.viscosity  = viscosity;


    // TEMPORAL DISCRETIZATION
    param.solver_type = SolverType::Unsteady;
    param.temporal_discretization =
      TemporalDiscretization::BDFPressureCorrection; // BDFCoupledSolution;
    param.treatment_of_convective_term =
      TreatmentOfConvectiveTerm::Explicit; // Explicit; //Implicit;
    param.calculation_of_time_step_size = TimeStepCalculation::UserSpecified;
    param.max_velocity                  = 3.5;
    param.cfl                           = 1.0e-1;
    param.time_step_size                = 1.0e-3; // 1.0e-4;
    param.order_time_integrator         = 2;      // 1; // 2; // 3;
    param.start_with_low_order          = false;  // true; // false;

    // output of solver information
    param.solver_info_data.interval_time = (param.end_time - param.start_time) / 10;

    // SPATIAL DISCRETIZATION
    param.triangulation_type = TriangulationType::Distributed;
    param.degree_p           = DegreePressure::MixedOrder;
    param.mapping            = MappingType::Isoparametric;

    // convective term

    // viscous term
    param.IP_formulation_viscous = InteriorPenaltyFormulation::SIPG;

    // special case: pure DBC's
    param.pure_dirichlet_bc = true;
    param.adjust_pressure_level =
      AdjustPressureLevel::ApplyAnalyticalMeanValue; // ApplyAnalyticalSolutionInPoint;

    // PROJECTION METHODS

    // pressure Poisson equation
    param.solver_pressure_poisson         = SolverPressurePoisson::CG;
    param.solver_data_pressure_poisson    = SolverData(1000, 1.e-12, 1.e-8, 100);
    param.preconditioner_pressure_poisson = PreconditionerPressurePoisson::Multigrid;

    // projection step
    param.solver_projection         = SolverProjection::CG;
    param.solver_data_projection    = SolverData(1000, 1.e-20, 1.e-12);
    param.preconditioner_projection = PreconditionerProjection::InverseMassMatrix;

    // HIGH-ORDER DUAL SPLITTING SCHEME

    // formulations
    param.order_extrapolation_pressure_nbc =
      param.order_time_integrator <= 2 ? param.order_time_integrator : 2;

    // viscous step
    param.solver_viscous         = SolverViscous::CG;
    param.solver_data_viscous    = SolverData(1000, 1.e-12, 1.e-8);
    param.preconditioner_viscous = PreconditionerViscous::Multigrid;

    // PRESSURE-CORRECTION SCHEME

    // momentum step

    // Newton solver
    param.newton_solver_data_momentum = NewtonSolverData(100, 1.e-10, 1.e-8);

    // linear solver
    param.solver_momentum                = SolverMomentum::GMRES;
    param.solver_data_momentum           = SolverData(1e4, 1.e-12, 1.e-8, 100);
    param.preconditioner_momentum        = MomentumPreconditioner::InverseMassMatrix;
    param.update_preconditioner_momentum = false;

    // formulation
    param.order_pressure_extrapolation = 1;
    param.rotational_formulation       = true;


    // COUPLED NAVIER-STOKES SOLVER
    param.use_scaling_continuity = false;

    // nonlinear solver (Newton solver)
    param.newton_solver_data_coupled = NewtonSolverData(100, 1.e-12, 1.e-8);

    // linear solver
    param.solver_coupled      = SolverCoupled::GMRES;
    param.solver_data_coupled = SolverData(1e3, 1.e-12, 1.e-8, 100);

    // preconditioning linear solver
    param.preconditioner_coupled = PreconditionerCoupled::BlockTriangular;

    // preconditioner velocity/momentum block
    param.preconditioner_velocity_block = MomentumPreconditioner::Multigrid;

    // preconditioner Schur-complement block
    param.preconditioner_pressure_block =
      SchurComplementPreconditioner::PressureConvectionDiffusion; // CahouetChabard;
    param.discretization_of_laplacian = DiscretizationOfLaplacian::Classical;
  }

  void
  create_grid(std::shared_ptr<parallel::TriangulationBase<dim>> triangulation,
              unsigned int const                                n_refine_space,
              std::vector<GridTools::PeriodicFacePair<typename Triangulation<dim>::cell_iterator>> &
                periodic_faces)
  {
    (void)periodic_faces;

    const double left = -1.0, right = 1.0;
    GridGenerator::hyper_cube(*triangulation, left, right);
    triangulation->refine_global(n_refine_space);
  }


  std::shared_ptr<Function<dim>>
  set_mesh_movement_function()
  {
    std::shared_ptr<Function<dim>> mesh_motion;
    mesh_motion.reset(new Functions::ZeroFunction<dim>(dim));

    return mesh_motion;
  }

  void
  set_boundary_conditions(std::shared_ptr<BoundaryDescriptorU<dim>> boundary_descriptor_velocity,
                          std::shared_ptr<BoundaryDescriptorP<dim>> boundary_descriptor_pressure)
  {
    // set boundary conditions
    typedef typename std::pair<types::boundary_id, std::shared_ptr<Function<dim>>> pair;

    boundary_descriptor_velocity->dirichlet_bc.insert(
      pair(0, new AnalyticalSolutionVelocity<dim>(viscosity)));
    boundary_descriptor_pressure->neumann_bc.insert(pair(0, new PressureBC_dudt<dim>(viscosity)));
  }


  void
  set_field_functions(std::shared_ptr<FieldFunctions<dim>> field_functions)
  {
    field_functions->initial_solution_velocity.reset(
      new AnalyticalSolutionVelocity<dim>(viscosity));
    field_functions->initial_solution_pressure.reset(
      new AnalyticalSolutionPressure<dim>(viscosity));
    field_functions->analytical_solution_pressure.reset(
      new AnalyticalSolutionPressure<dim>(viscosity));
    field_functions->right_hand_side.reset(new Functions::ZeroFunction<dim>(dim));
  }

  std::shared_ptr<PostProcessorBase<dim, Number>>
  construct_postprocessor(InputParameters const & param, MPI_Comm const & mpi_comm)
  {
    PostProcessorData<dim> pp_data;

    // write output for visualization of results
    pp_data.output_data.write_output         = false;
    pp_data.output_data.output_folder        = output_directory;
    pp_data.output_data.output_name          = output_name;
    pp_data.output_data.output_start_time    = param.start_time;
    pp_data.output_data.output_interval_time = (param.end_time - param.start_time) / 10;
    pp_data.output_data.write_divergence     = false;
    pp_data.output_data.degree               = param.degree_u;

    // calculation of velocity error
    pp_data.error_data_u.analytical_solution_available = true;
    pp_data.error_data_u.analytical_solution.reset(new AnalyticalSolutionVelocity<dim>(viscosity));
    pp_data.error_data_u.error_calc_start_time    = param.start_time;
    pp_data.error_data_u.error_calc_interval_time = (param.end_time - param.start_time) / 10;
    pp_data.error_data_u.name                     = "velocity";

    // ... pressure error
    pp_data.error_data_p.analytical_solution_available = true;
    pp_data.error_data_p.analytical_solution.reset(new AnalyticalSolutionPressure<dim>(viscosity));
    pp_data.error_data_p.error_calc_start_time    = param.start_time;
    pp_data.error_data_p.error_calc_interval_time = (param.end_time - param.start_time) / 10;
    pp_data.error_data_p.name                     = "pressure";

    std::shared_ptr<PostProcessorBase<dim, Number>> pp;
    pp.reset(new PostProcessor<dim, Number>(pp_data, mpi_comm));

    return pp;
  }
};

} // namespace Beltrami
} // namespace IncNS

#endif /* APPLICATIONS_INCOMPRESSIBLE_NAVIER_STOKES_TEST_CASES_BELTRAMI_H_ */
