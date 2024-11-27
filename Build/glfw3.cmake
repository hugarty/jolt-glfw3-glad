

include(FetchContent)
FetchContent_Declare(
    glfw3
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.7
    GIT_SHALLOW TRUE
    FIND_PACKAGE_ARGS
    )
FetchContent_MakeAvailable(glfw3)