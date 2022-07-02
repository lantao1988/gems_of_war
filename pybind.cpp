#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/complex.h>
#include "gem_game.h"
namespace py = pybind11;

PYBIND11_MODULE(pybind, m) {
    py::class_<Game>(m, "Game")
        .def(py::init<bool, int, int, int>())
        .def(py::init<const std::vector<std::vector<int>> &, const std::vector<int> &, bool>())
        .def("Move", &Game::Move)
        .def("FallDown", &Game::FallDown)
        .def("CreateNewGems", &Game::CreateNewGems)
        .def("NewPos", &Game::NewPos)
        .def("Show", &Game::Show)
        .def("AllValidMove", &Game::AllValidMove)
        .def("GemCollection", &Game::GemCollection)
        .def("CurrentState", &Game::CurrentState);

    py::enum_<MoveType>(m, "MoveType")
        .value("MT_UP", MT_UP)
        .value("MT_DOWN", MT_DOWN)
        .value("MT_LEFT", MT_LEFT)
        .value("MT_RIGHT", MT_RIGHT)
        .export_values();

    py::enum_<MoveState>(m, "MoveState")
        .value("MS_INVALID", MS_INVALID)
        .value("MS_OK", MS_OK)
        .value("MS_FOUR", MS_FOUR)
        .value("MS_EXTERTURN", MS_EXTERTURN)
        .export_values();

    py::enum_<GemType>(m, "GemType")
        .value("GT_UNKNOWN", GT_UNKNOWN)
        .value("GT_EMPTY", GT_EMPTY)
        .value("GT_COPPER", GT_COPPER)
        .value("GT_SILVER", GT_SILVER)
        .value("GT_GOLD", GT_GOLD)
        .value("GT_PURSE", GT_PURSE)
        .value("GT_BOX", GT_BOX)
        .value("GT_GREENBOX", GT_GREENBOX)
        .value("GT_REDBOX", GT_REDBOX)
        .value("GT_DOOR", GT_DOOR)
        .value("GT_RED", GT_RED)
        .value("GT_GREEN", GT_GREEN)
        .value("GT_BROWN", GT_BROWN)
        .value("GT_PURPLE", GT_PURPLE)
        .value("GT_BLUE", GT_BLUE)
        .value("GT_YELLOW", GT_YELLOW)
        .value("GT_SKELETON", GT_SKELETON)
        .export_values();
    
    py::class_<PredictResult>(m, "PredictResult")
        .def(py::init<>())
        .def_readwrite("movement", &PredictResult::movement)
        .def_readwrite("map", &PredictResult::map);

    py::class_<MoveResult>(m, "MoveResult")
        .def(py::init<>())
        .def_readwrite("state", &MoveResult::state)
        .def_readwrite("snapshots", &MoveResult::snapshots);
    py::class_<MoveOptions>(m, "MoveOptions")
        .def(py::init<>())
        .def_readwrite("showState", &MoveOptions::showState);
}