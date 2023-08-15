/*  ______________________________________________________________________
 *
 *  ExaDG - High-Order Discontinuous Galerkin for the Exa-Scale
 *
 *  Copyright (C) 2021 by the ExaDG authors
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  ______________________________________________________________________
 */

#ifndef INCLUDE_FUNCTIONALITIES_CALCULATE_CHARACTERISTIC_ELEMENT_LENGTH_H_
#define INCLUDE_FUNCTIONALITIES_CALCULATE_CHARACTERISTIC_ELEMENT_LENGTH_H_

// deal.II
#include <deal.II/base/mpi.h>
#include <deal.II/fe/mapping.h>
#include <deal.II/grid/tria.h>

namespace ExaDG
{
/**
 *  This function calculates the characteristic element length h
 *  defined as h = min_{e=1,...,N_el} h_e, where h_e is the
 *  minimum vertex distance of element e.
 */
template<int dim>
inline double
calculate_minimum_vertex_distance(dealii::Triangulation<dim> const & triangulation,
                                  dealii::Mapping<dim> const &       mapping,
                                  MPI_Comm const &                   mpi_comm)
{
  double min_cell_diameter = std::numeric_limits<double>::max();

  for(auto const & cell : triangulation.active_cell_iterators())
  {
    if(cell->is_locally_owned())
    {
      auto const vertices = mapping.get_vertices(cell);
      for(unsigned int i = 0; i < vertices.size(); ++i)
        for(unsigned int j = i + 1; j < vertices.size(); ++j)
          min_cell_diameter = std::min(min_cell_diameter, vertices[i].distance(vertices[j]));
    }
  }

  return dealii::Utilities::MPI::min(min_cell_diameter, mpi_comm);
}

/**
 * This function calculates a characteristic resolution limit for high-order Lagrange polynomials
 * given a mesh size h. This one-dimensional resolution limit is calculated as the grid size divided
 * by the number of nodes per coordinate direction. Hence, the result depends on the function space
 * (H^1 vs. L^2).
 */
template<typename Number>
inline Number
calculate_high_order_element_length(Number const       element_length,
                                    unsigned int const fe_degree,
                                    bool const         is_dg)
{
  unsigned int n_nodes_1d = fe_degree;

  if(is_dg)
    n_nodes_1d += 1;

  return element_length / ((double)n_nodes_1d);
}

} // namespace ExaDG

#endif /* INCLUDE_FUNCTIONALITIES_CALCULATE_CHARACTERISTIC_ELEMENT_LENGTH_H_ */
