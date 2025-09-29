> When bumping the version, you need to update the following files:

1. [ ] [../CMakeLists.txt]() Bump `GSL_VERSION`
1. [ ] [../README.md]() Bump `GIT_TAG`
1. [ ] [../.github/workflows/ios.yml]() Bump `MACOSX_BUNDLE_BUNDLE_VERSION` and
`MACOSX_BUNDLE_SHORT_VERSION_STRING`

> After updating, you need to create a new GitHub release:

1. [ ] [https://github.com/microsoft/GSL/releases/new]()

Be sure to update the release notes accordingly and properly mention open-source
contributors.

> After a new release exists, update the `ms-gsl` vcpkg port:

1. [ ] [https://github.com/microsoft/vcpkg/tree/master/ports/ms-gsl]()

Be sure to monitor the PR that updates the port for any feedback from vcpkg maintainers.
