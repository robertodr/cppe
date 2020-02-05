#include "bmatrix.hh"
#include "math.hh"

namespace libcppe {

Eigen::VectorXd BMatrix::compute_apply(Eigen::VectorXd induced_moments) {
  return compute_apply_slice(induced_moments, 0, m_n_polsites);
}

Eigen::VectorXd BMatrix::compute_apply_slice(Eigen::VectorXd induced_moments, int start,
                                             int stop) {
  std::vector<Eigen::MatrixXi> Tk_coeffs = Tk_coefficients(5);
  Eigen::VectorXd ret                    = Eigen::VectorXd::Zero(induced_moments.size());
  if (start < 0 || stop > m_n_polsites) {
    throw std::runtime_error("Invalid range in compute_apply_slice.");
  }

#pragma omp parallel for firstprivate(Tk_coeffs)
  for (int i = start; i < stop; ++i) {
    int l           = i * 3;
    Potential& pot1 = m_polsites[i];
    for (auto j : m_polmask[i]) {
      int m                = j * 3;
      Potential& pot2      = m_polsites[j];
      Eigen::Vector3d diff = pot2.get_site_position() - pot1.get_site_position();
      Eigen::VectorXd T2;
      if (m_options.damp_induced) {
        Polarizability& alpha_i = pot1.get_polarizability();
        Polarizability& alpha_j = pot2.get_polarizability();
        T2 = Tk_tensor(2, diff, Tk_coeffs, m_options.damping_factor_induced,
                       alpha_i.get_isotropic_value(), alpha_j.get_isotropic_value());
      } else {
        T2 = Tk_tensor(2, diff, Tk_coeffs);
      }
      Eigen::Matrix3d T2m = triangle_to_mat(T2);
      ret.segment<3>(l) -= T2m * induced_moments.segment<3>(m);
    }
    ret.segment<3>(l) += m_alpha_inverse[i] * induced_moments.segment<3>(l);
  }
  return ret;
}

Eigen::VectorXd BMatrix::compute_apply_diagonal(Eigen::VectorXd in) {
  Eigen::VectorXd ret = Eigen::VectorXd::Zero(in.size());

#pragma omp parallel for
  for (int i = 0; i < m_n_polsites; ++i) {
    int l                   = i * 3;
    Polarizability& alpha_i = m_polsites[i].get_polarizability();
    ret.segment<3>(l)       = alpha_i.get_matrix() * in.segment<3>(l);
  }
  return ret;
}

}  // namespace libcppe
