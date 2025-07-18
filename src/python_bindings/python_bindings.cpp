#include <pybind11/pybind11.h>
#include "CoreAPI.h"

namespace py = pybind11;

// Funzione di esempio da esporre in Python
std::string get_core_version() {
    // In un'applicazione reale, questa funzione potrebbe interagire
    // con CoreAPI per ottenere informazioni.
    return "1.0.0";
}

PYBIND11_MODULE(python_bindings, m) {
    m.doc() = "Python bindings for CoreBaseApplication"; // Optional module docstring

    m.def("get_core_version", &get_core_version, "A function that returns the core version");
}