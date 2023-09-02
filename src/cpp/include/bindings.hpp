#include <pybind11/pybind11.h>

#include "consts.hpp"
#include "context.hpp"
#include "game_context.hpp"

namespace py = pybind11;

py::dict Init();

void Quit();

// py::dict GetObservation();

void Reset(std::optional<int> randomSeed);

std::tuple<int, bool> Update(int intentionCode);

void Render();

void draw();
