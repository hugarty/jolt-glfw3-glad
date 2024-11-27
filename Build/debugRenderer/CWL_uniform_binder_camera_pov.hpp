#ifndef CWL_UNIFORM_BINDER_CAMERA_POV_HPP
#define CWL_UNIFORM_BINDER_CAMERA_POV_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "glm/ext/vector_float3.hpp"
#include "../../interaction/camera/camera.hpp"

/**
 *
 * \brief binds matrices into uniforms given the shader program's id and a camera with yaw and pitch attributes
 *
 * \description Specifically world_to_camera translates the world coordinate system to have origin centered at the
 * characters position, facing in the direction that the character is looking
 *
 * It binds camera_to_clip to a basic perspective transformation
 *
 * It binds local_to_world as the identity, which can be overridden if needed
 *
 * \precondition the shader program uses a CWL transformation with attribute, opengl has been initialized
 * names:
 * - camera_to_clip
 * - world_to_camera
 * - local_to_world
 *
 * \param shader_program_id
 * \param screen_width
 * \param screen_height
 * \param character_position
 */
void bind_CWL_matrix_uniforms_camera_pov(GLuint shader_program_id, int screen_width, int screen_height,
                                         glm::vec3 character_position, glm::mat4 local_to_world, Camera camera,
                                         float fov, float render_distance);

#endif // CWL_UNIFORM_BINDER_CAMERA_POV_HPP
