#include <gtest/gtest.h>
#include <pybind11/embed.h>

namespace py = pybind11;

TEST(PythonBindings, BasicTest) {
    py::scoped_interpreter guard{};

    try {
        auto sys = py::module_::import("sys");
        // Assicurati che il percorso sia corretto per la tua struttura di build
        sys.attr("path").attr("append")("build/lib/Release"); 

        py::print("Python Path:", sys.attr("path"));

        auto example_module = py::module_::import("python_bindings");
        auto result = example_module.attr("add")(1, 2).cast<int>();
        ASSERT_EQ(result, 3);
    } catch (py::error_already_set &e) {
        FAIL() << "Python error: " << e.what();
    }
}

// Il main è gestito da gtest_main, non è necessario definirlo qui
// se si collega GTest::gtest_main