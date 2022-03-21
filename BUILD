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

cc_binary(
    name = "libgame.dll",
    srcs = [
        "gem_game.cpp",
        "movement.h",
    ],
    linkshared = True,
    linkstatic = True,
    deps = [],
)

cc_binary(
    name = "main",
    srcs = ["main.cpp"],
    deps = [
        "game",
    ],
)
