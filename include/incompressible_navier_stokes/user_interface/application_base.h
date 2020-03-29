/*
 * application_base.h
 *
 *  Created on: 27.03.2020
 *      Author: fehn
 */

#ifndef INCLUDE_INCOMPRESSIBLE_NAVIER_STOKES_USER_INTERFACE_APPLICATION_BASE_H_
#define INCLUDE_INCOMPRESSIBLE_NAVIER_STOKES_USER_INTERFACE_APPLICATION_BASE_H_

// deal.II
#include <deal.II/distributed/tria.h>
#include <deal.II/grid/grid_generator.h>

// functionalities
#include "../../../include/functionalities/parse_input.h"

// user interface
#include "boundary_descriptor.h"
#include "field_functions.h"
#include "input_parameters.h"

// postprocessor
#include "../postprocessor/postprocessor.h"

using namespace dealii;

namespace IncNS
{
template<int dim, typename Number>
class ApplicationBase
{
public:
  virtual void
  add_parameters(ParameterHandler & prm)
  {
    (void)prm;

    // can be overwritten by derived classes and is for example necessary
    // in order to generate a default input file
  }

  ApplicationBase(std::string parameter_file)
    : parameter_file(parameter_file), n_subdivisions_1d_hypercube(1)
  {
  }

  virtual ~ApplicationBase()
  {
  }

  virtual void
  set_input_parameters(InputParameters & parameters) = 0;

  virtual void
  create_grid(std::shared_ptr<parallel::TriangulationBase<dim>> triangulation,
              unsigned int const                                n_refine_space,
              std::vector<GridTools::PeriodicFacePair<typename Triangulation<dim>::cell_iterator>> &
                periodic_faces) = 0;

  virtual void
  set_boundary_conditions(
    std::shared_ptr<BoundaryDescriptorU<dim>> boundary_descriptor_velocity,
    std::shared_ptr<BoundaryDescriptorP<dim>> boundary_descriptor_pressure) = 0;

  virtual void
  set_field_functions(std::shared_ptr<FieldFunctions<dim>> field_functions) = 0;

  virtual std::shared_ptr<Function<dim>>
  set_mesh_movement_function() = 0;

  virtual std::shared_ptr<PostProcessorBase<dim, Number>>
  construct_postprocessor(InputParameters const & param, MPI_Comm const & mpi_comm) = 0;

  void
  set_subdivisions_hypercube(unsigned int const n_subdivisions_1d)
  {
    n_subdivisions_1d_hypercube = n_subdivisions_1d;
  }

protected:
  InputParameters param;
  std::string     parameter_file;

  unsigned int n_subdivisions_1d_hypercube;
};

} // namespace IncNS



#endif /* INCLUDE_INCOMPRESSIBLE_NAVIER_STOKES_USER_INTERFACE_APPLICATION_BASE_H_ */
