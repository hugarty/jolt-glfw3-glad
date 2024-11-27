add_library(debugRenderer OBJECT
    physics_debug_renderer.cpp
#   car_maintenance.cpp
)
target_include_directories(debugRenderer PUBLIC .)
target_link_libraries(debugRenderer PUBLIC Jolt PRIVATE glfw glad glm)