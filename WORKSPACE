load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# Keep aligned with tests/CMakeLists.txt
http_archive(
    name = "com_github_catchorg_catch2",
    build_file = "//third_party:catch2.BUILD",
    sha256 = "5f31b93712e65d363f257ad0f0c02cfbed7a3988979d5f320ad7771e513d4cc8",
    strip_prefix = "Catch2-2.0.1",
    urls = ["https://github.com/catchorg/Catch2/archive/v2.0.1.tar.gz"],
)
