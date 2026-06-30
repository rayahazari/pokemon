#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include "test.cpp"

using namespace std;
namespace py = pybind11;

PYBIND11_MODULE(poke_env_cpp, m) {
    py::class_<StepResult>(m, "StepResult")
        .def_readonly("next_state", &StepResult::next_state)
        .def_readonly("reward", &StepResult::reward)
        .def_readonly("done", &StepResult::done);

    py::class_<Pokemon>(m, "Pokemon")
        .def(py::init<>());

    m.def("getRandomPokemon", &getRandomPokemon);

    py::class_<RLEnvironment>(m, "RLEnvironment")
        .def(py::init<std::vector<Pokemon>, std::vector<Pokemon>>())
        .def("reset", &RLEnvironment::reset)
        .def("step", &RLEnvironment::step, py::arg("action"), py::arg("opp_action") = -1)
        .def("render", &RLEnvironment::render)
        .def("showMoves", &RLEnvironment::showMoves)
        .def("showSwitches", &RLEnvironment::showSwitches)
        .def("displayParty", &RLEnvironment::displayParty)
        .def("startBattleWithModel", &RLEnvironment::startBattleWithModel);;
}