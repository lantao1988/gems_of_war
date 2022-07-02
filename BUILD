load("@pybind11_bazel//:build_defs.bzl", "pybind_extension")

cc_library(
    name = "game",
    srcs = [
        "gem_game.cpp",
        "movement.h",
    ],
    hdrs = ["gem_game.h"],
)

cc_test(
    name = "game_test",
    srcs = ["gem_game_test.cpp"],
    deps = [
        "game",
        "@com_google_googletest//:gtest_main",
    ],
)

pybind_extension(
    name = "pybind",
    srcs = [
        "pybind.cpp",
    ],
    linkstatic = True,
    deps = [":game"],
)
