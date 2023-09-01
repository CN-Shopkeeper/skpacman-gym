#include "bindings.hpp"

PYBIND11_MODULE(Pacman, m) {
    m.doc() = "pybind11 example plugin";  // optional module docstring

    m.def("init", &Init, "Init SDL, Context and GameContext");
    m.def("quit", &Quit, "Quit");
}