#include <pybind11/pybind11.h>

namespace py = pybind11;

void export_utils(py::module&);
void export_core(py::module&);

PYBIND11_MODULE(pycppe, pycppe) {
    pycppe.doc() = "Python interface for CPPE";
    export_core(pycppe);
    export_utils(pycppe);
}