workspace(name = "game")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "com_google_googletest",
    sha256 = "bc1cc26d1120f5a7e9eb450751c0b24160734e46a02823a573f3c6b6c0a574a7",
    strip_prefix = "googletest-e2c06aa2497e330bab1c1a03d02f7c5096eb5b0b",
    urls = ["https://github.com/google/googletest/archive/e2c06aa2497e330bab1c1a03d02f7c5096eb5b0b.zip"],
)

http_archive(
    name = "pybind11_bazel",
    strip_prefix = "pybind11_bazel-72cbbf1fbc830e487e3012862b7b720001b70672",
    urls = ["https://github.com/pybind/pybind11_bazel/archive/72cbbf1fbc830e487e3012862b7b720001b70672.zip"],
)

# We still require the pybind library.
http_archive(
    name = "pybind11",
    build_file = "@pybind11_bazel//:pybind11.BUILD",
    strip_prefix = "pybind11-b22ee64c7309eb7344031b5e5385e6907d84c7eb",
    urls = ["https://github.com/pybind/pybind11/archive/b22ee64c7309eb7344031b5e5385e6907d84c7eb.tar.gz"],
)

load("@pybind11_bazel//:python_configure.bzl", "python_configure")

python_configure(
    name = "local_config_python",
    #    python_version = "3",
)
