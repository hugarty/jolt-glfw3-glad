#include "CWL_uniform_binder_camera_pov.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

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
                                         float fov, float render_distance) {

    // don't forget to enable shader before setting uniforms
    glUseProgram(shader_program_id);

    // view/projection transformations
    glm::mat4 camera_to_clip =
        glm::perspective(glm::radians(fov), (float)screen_width / (float)screen_height, 0.1f, render_distance);

    // TODO: swap order of these so that it mirrors the transformation application order

    GLint camera_to_clip_uniform_location = glGetUniformLocation(shader_program_id, "camera_to_clip");
    glUniformMatrix4fv(camera_to_clip_uniform_location, 1, GL_FALSE, glm::value_ptr(camera_to_clip));

    glm::mat4 world_to_camera =
        glm::lookAt(character_position, character_position + camera.look_direction, camera.up_direction);
    GLint world_to_camera_uniform_location = glGetUniformLocation(shader_program_id, "world_to_camera");
    glUniformMatrix4fv(world_to_camera_uniform_location, 1, GL_FALSE, glm::value_ptr(world_to_camera));

    GLint local_to_world_uniform_location = glGetUniformLocation(shader_program_id, "local_to_world");
    glUniformMatrix4fv(local_to_world_uniform_location, 1, GL_FALSE, glm::value_ptr(local_to_world));

    glUseProgram(0);
}
