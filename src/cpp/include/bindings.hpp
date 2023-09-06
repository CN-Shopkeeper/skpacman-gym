#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <unordered_map>
#include <variant>

#include "consts.hpp"
#include "context.hpp"
#include "game_context.hpp"

namespace py = pybind11;

using MonsterDict = std::unordered_map<
    std::string, std::variant<std::array<int, 2>, int>>;

using ObservationDict = std::unordered_map<
    std::string,
    std::variant<MonsterDict,
                 std::tuple<MonsterDict, MonsterDict, MonsterDict, MonsterDict>,
                 std::array<int, MapWidth * MapHeight>, int>>;

std::unordered_map<std::string, int> Init(const std::string& baseDir,
                                          bool debug = false);

void Quit();

ObservationDict GetObservation();

void Reset(std::optional<int> randomSeed);

std::tuple<int, bool> Update(int intentionCode);

void Render();

void draw();
