#include "physics_debug_renderer.hpp"
#include "conversions.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "shader_pipeline.cpp"
#include "conversions.cpp"

#include <glad/glad.h>

long ID_TOP_MERMAO = 0;

// camera
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f, 10.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 4.0f, 0.0f);

bool firstMouse = true;
float yaw   = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch =  0.0f;
float lastX =  800.0f / 2.0;
float lastY =  600.0 / 2.0;
float fov   =  45.0f;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;



// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = static_cast<float>(2.5 * deltaTime);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f; // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
void on_window_size_change(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width
    // and height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}


float getFragmentShaderColorRed (int size) {
    if (size == 1) {
        return 1.0f;
    } else {
        return 0.3f;
    }
}

float getFragmentShaderColorGreen (int size) {
    if (size == 1) {
        return 0.5f;
    } else {
        return 0.2f;
    }
}

float getFragmentShaderColorBlue (int size) {
    if (size == 1) {
        return 0.4f;
    } else {
        return 0.8f;
    }
}

const char *vertexShaderSource ="#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "uniform mat4 local_to_world;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = projection * view * local_to_world * vec4(aPos, 1.0);\n"
    "}\0";

const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec4 ourColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = ourColor;\n"
    "}\n\0";


PhysicsDebugRenderer::PhysicsDebugRenderer() {
    unsigned int width = 500;
    unsigned int height = 500;
    bool start_in_fullscreen = false;
    bool start_with_mouse_captured = false;
    unsigned int *window_width_px = &width;
    unsigned int *window_height_px = &height;
    const char *window_name = "window";


    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        // throw std::runtime_error("glfw couldn't be initialized");
        std::cout << "Nunca deveria ter entrado aqui" << std::endl;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    if (start_in_fullscreen) {
        GLFWmonitor *monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *mode = glfwGetVideoMode(monitor);
        *window_width_px = mode->width;
        *window_height_px = mode->height;
        window = glfwCreateWindow(*window_width_px, *window_height_px, window_name, monitor, NULL);
    } else {
        window = glfwCreateWindow(*window_width_px, *window_height_px, window_name, NULL, NULL);
    }

    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        // throw std::runtime_error("failed to create window");

        std::cout << "Nunca deveria ter entrado aqui" << std::endl;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        // throw std::runtime_error("failed to initialize GLAD");
        std::cout << "Nunca deveria ter entrado aqui" << std::endl;
    }

    glEnable(GL_DEPTH_TEST);

    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    
    // link shaders
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


    // disable this for debugging so you can move the mouse outside the window
    if (start_with_mouse_captured) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }


    if (glfwRawMouseMotionSupported()) {
        // logger.info("raw mouse motion supported, using it");
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }


    shader_pipeline.load_in_shaders_from_file("/home/hpca/xdev/development/Jolt+opengl/Jolt+opengl/Build/debugRenderer/CWL_v_transformation.vert",
                                              "/home/hpca/xdev/development/Jolt+opengl/Jolt+opengl/Build/debugRenderer/fixed_color.frag");
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    JPH::DebugRenderer::Initialize();
}

// PhysicsDebugRenderer::~PhysicsDebugRenderer() = default;
// PhysicsDebugRenderer::~PhysicsDebugRenderer() {
//     glDeleteVertexArrays(1, &vao);
//     glDeleteBuffers(1, &vbo);
////    glDeleteProgram(shader_program);
//}

void PhysicsDebugRenderer::DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) {

    printf("trying to draw line\n");
    //    glUseProgram(shader_program);
    //
    //    float vertices[6] = {
    //            inFrom.GetX(), inFrom.GetY(), inFrom.GetZ(),
    //            inFrom.GetX(), inFrom.GetY(), inFrom.GetZ(),
    //    };
    //
    //    glBindVertexArray(vao);
    //
    //    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    //    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    //
    //    JPH::Vec4 color = inColor.ToVec4();
    //    float rgb_color[3] = {color.GetX(), color.GetY(), color.GetZ()};
    ////    glUniform3fv(glGetUniformLocation(shader_program, "color"), 1, &rgb_color[0]);
    //    glUniform3fv(glGetUniformLocation(shader_program, "color"), 1, rgb_color);
    //
    //    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
    //    glEnableVertexAttribArray(0);
    //
    //    glDrawArrays(GL_LINES, 0, 2);
}

JPH::DebugRenderer::Batch PhysicsDebugRenderer::CreateTriangleBatch(const Triangle *inTriangles, int inTriangleCount) {
    auto *triangle_data = new TriangleData(inTriangles, inTriangleCount);
    return triangle_data;
}

JPH::DebugRenderer::Batch PhysicsDebugRenderer::CreateTriangleBatch(const Vertex *inVertices, int inVertexCount,
                                                                    const JPH::uint32 *inIndices, int inIndexCount) {
    auto *triangle_data = new TriangleData(inVertices, inVertexCount, inIndices, inIndexCount);
    return triangle_data;
}

void PhysicsDebugRenderer::DrawGeometry(JPH::RMat44Arg inModelMatrix, const JPH::AABox &inWorldSpaceBounds,
                                        float inLODScaleSq, JPH::ColorArg inModelColor, const GeometryRef &inGeometry,
                                        ECullMode inCullMode, ECastShadow inCastShadow, EDrawMode inDrawMode) {
    /*
     * the geometry contains a list of lods, each lod contains a triangleBatch
     * which can be cast to BatchImpl. These triangle batches were created with
     * the createTriangleBatch function, so those must be implemented before you implement
     * this or else it will not work, each BatchImpl which impelments RenderPrimitive
     */
    const JPH::Array<LOD> &geometry_lods = inGeometry->mLODs;
    // use lod 0 because our game doesn't use LOD at all
    TriangleData *triangle_batch = static_cast<TriangleData *>(geometry_lods[0].mTriangleBatch.GetPtr());

    if (triangle_batch->uses_indices) {

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, 
            triangle_batch->vertices.size() * sizeof(float), 
            &triangle_batch->vertices.front(), 
            GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
            triangle_batch->indices.size() * sizeof(JPH::uint32), 
            &triangle_batch->indices.front(), 
            GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);


		float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

//=======================================

        glUseProgram(shaderProgram);
        
	  	// pass projection matrix to shader (note that in this case it could change every frame)
        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)500 / (float)500, 0.1f, 100.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);

        // camera/view transformation
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &view[0][0]);


        glm::mat4 local_to_world = convert_mat4_from_jolt_to_glm(inModelMatrix);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "local_to_world"), 1, GL_FALSE, glm::value_ptr(local_to_world));


        float red = getFragmentShaderColorRed(triangle_batch->idTriangulo);
        float green = getFragmentShaderColorGreen(triangle_batch->idTriangulo);
        float blue = getFragmentShaderColorBlue(triangle_batch->idTriangulo);
        glUniform4f(glGetUniformLocation(shaderProgram, "ourColor"), red,green,blue,1.0f);

        if(triangle_batch->vertices.size() < 200) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        glDrawElements(GL_TRIANGLES, triangle_batch->indices.size(), GL_UNSIGNED_INT, 0);

    } else {
        //        printf("this triangle batch does not use indices\n");

        // glUseProgram(shader_pipeline.shader_program_id);
        // glBindVertexArray(vao);
        // glBindBuffer(GL_ARRAY_BUFFER, vbo);
        // glBufferData(GL_ARRAY_BUFFER, triangle_batch->triangle_vertices.size() * sizeof(float),
        //              &triangle_batch->triangle_vertices.front(), GL_STATIC_DRAW);

        // GLuint position_location = glGetAttribLocation(shader_pipeline.shader_program_id, "position");
        // glEnableVertexAttribArray(position_location);
        // glVertexAttribPointer(position_location, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

        // glm::mat4 local_to_world = convert_mat4_from_jolt_to_glm(inModelMatrix);
        // GLint local_to_world_uniform_location =
        //     glGetUniformLocation(shader_pipeline.shader_program_id, "local_to_world");
        // glUniformMatrix4fv(local_to_world_uniform_location, 1, GL_FALSE, glm::value_ptr(local_to_world));

        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        // glDrawArrays(GL_TRIANGLES, 0, triangle_batch->num_triangles * 3);
        // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

// void PhysicsDebugRenderer::FinalizePrimitive()
//{
//     JPH_PROFILE_FUNCTION();
//
//     if (mLockedPrimitive != nullptr)
//     {
//         BatchImpl *primitive = static_cast<BatchImpl *>(mLockedPrimitive.GetPtr());
//
//         // Unlock the primitive
//         primitive->UnlockVertexBuffer();
//
//         // Set number of indices to draw
//         primitive->SetNumVtxToDraw(int(mLockedVertices - mLockedVerticesStart));
//
//         // Add to draw list
//         mTempPrimitives[new Geometry(mLockedPrimitive, mLockedPrimitiveBounds)].mInstances.push_back({
//         Mat44::sIdentity(), Mat44::sIdentity(), Color::sWhite, mLockedPrimitiveBounds, 1.0f });
//         ++mNumInstances;
//
//         // Clear pointers
//         mLockedPrimitive = nullptr;
//         mLockedVerticesStart = nullptr;
//         mLockedVertices = nullptr;
//         mLockedVerticesEnd = nullptr;
//         mLockedPrimitiveBounds = AABox();
//     }
// }
//
// void PhysicsDebugRenderer::EnsurePrimitiveSpace(int inVtxSize)
//{
//     const int cVertexBufferSize = 10240;
//
//     if (mLockedPrimitive == nullptr
//         || mLockedVerticesEnd - mLockedVertices < inVtxSize)
//     {
//         FinalizePrimitive();
//
//         // Create new
//         BatchImpl *primitive = new BatchImpl(mRenderer, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//         primitive->CreateVertexBuffer(cVertexBufferSize, sizeof(Vertex));
//         mLockedPrimitive = primitive;
//
//         // Lock buffers
//         mLockedVerticesStart = mLockedVertices = (Vertex *)primitive->LockVertexBuffer();
//         mLockedVerticesEnd = mLockedVertices + cVertexBufferSize;
//     }
// }

void PhysicsDebugRenderer::DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3,
                                        JPH::ColorArg inColor, ECastShadow inCastShadow) {
    printf("trying to draw triangle\n");
}

// void PhysicsDebugRenderer::DrawInstances(const Geometry *inGeometry, const Array<int> &inStartIdx)
//{
//     RenderInstances *instances_buffer = mInstancesBuffer[mRenderer->GetCurrentFrameIndex()];
//
//     if (!inStartIdx.empty())
//     {
//         // Get LODs
//         const Array<LOD> &geometry_lods = inGeometry->mLODs;
//
//         // Write instances for all LODS
//         int next_start_idx = inStartIdx.front();
//         for (size_t lod = 0; lod < geometry_lods.size(); ++lod)
//         {
//             int start_idx = next_start_idx;
//             next_start_idx = inStartIdx[lod + 1];
//             int num_instances = next_start_idx - start_idx;
//             instances_buffer->Draw(static_cast<BatchImpl *>(geometry_lods[lod].mTriangleBatch.GetPtr()), start_idx,
//             num_instances);
//         }
//     }
// }

void PhysicsDebugRenderer::DrawText3D(JPH::RVec3Arg inPosition, const JPH::string_view &inString, JPH::ColorArg inColor,
                                      float inHeight) {
    printf("trying to draw triangle\n");
}

// void PhysicsDebugRenderer::DrawLines()
//{
//     JPH_PROFILE_FUNCTION();
//
//     lock_guard lock(mLinesLock);
//
//     // Draw the lines
//     if (!mLines.empty())
//     {
//         RenderPrimitive primitive(mRenderer, D3D_PRIMITIVE_TOPOLOGY_LINELIST);
//         primitive.CreateVertexBuffer((int)mLines.size() * 2, sizeof(Line) / 2);
//         void *data = primitive.LockVertexBuffer();
//         memcpy(data, &mLines[0], mLines.size() * sizeof(Line));
//         primitive.UnlockVertexBuffer();
//         mLineState->Activate();
//         primitive.Draw();
//     }
// }
//
// void PhysicsDebugRenderer::DrawTriangles()
//{
//     JPH_PROFILE_FUNCTION();
//
//     lock_guard lock(mPrimitivesLock);
//
//     // Finish the last primitive
//     FinalizePrimitive();
//
//     // Render to shadow map texture first
//     mRenderer->SetRenderTarget(mDepthTexture);
//
//     // Clear the shadow map texture to max depth
//     mDepthTexture->ClearRenderTarget();
//
//     // Get the camera and light frustum for culling
//     Vec3 camera_pos(mRenderer->GetCameraState().mPos - mRenderer->GetBaseOffset());
//     const Frustum &camera_frustum = mRenderer->GetCameraFrustum();
//     const Frustum &light_frustum = mRenderer->GetLightFrustum();
//
//     // Resize instances buffer and copy all visible instance data into it
//     if (mNumInstances > 0)
//     {
//         // Create instances buffer
//         RenderInstances *instances_buffer = mInstancesBuffer[mRenderer->GetCurrentFrameIndex()];
//         instances_buffer->CreateBuffer(2 * mNumInstances, sizeof(Instance));
//         Instance *dst_instance = reinterpret_cast<Instance *>(instances_buffer->Lock());
//
//         // Next write index
//         int dst_index = 0;
//
//         // This keeps track of which instances use which lod, first array: 0 = light pass, 1 = geometry pass
//         Array<Array<int>> lod_indices[2];
//
//         for (InstanceMap *primitive_map : { &mPrimitives, &mTempPrimitives, &mPrimitivesBackFacing,
//         &mWireframePrimitives })
//             for (InstanceMap::value_type &v : *primitive_map)
//             {
//                 // Get LODs
//                 const Array<LOD> &geometry_lods = v.first->mLODs;
//                 size_t num_lods = geometry_lods.size();
//                 JPH_ASSERT(num_lods > 0);
//
//                 // Ensure that our lod index array is big enough (to avoid reallocating memory too often)
//                 if (lod_indices[0].size() < num_lods)
//                     lod_indices[0].resize(num_lods);
//                 if (lod_indices[1].size() < num_lods)
//                     lod_indices[1].resize(num_lods);
//
//                 // Iterate over all instances
//                 const Array<InstanceWithLODInfo> &instances = v.second.mInstances;
//                 for (size_t i = 0; i < instances.size(); ++i)
//                 {
//                     const InstanceWithLODInfo &src_instance = instances[i];
//
//                     // Check if it overlaps with the light or camera frustum
//                     bool light_overlaps = light_frustum.Overlaps(src_instance.mWorldSpaceBounds);
//                     bool camera_overlaps = camera_frustum.Overlaps(src_instance.mWorldSpaceBounds);
//                     if (light_overlaps || camera_overlaps)
//                     {
//                         // Figure out which LOD to use
//                         float dist_sq = src_instance.mWorldSpaceBounds.GetSqDistanceTo(camera_pos);
//                         for (size_t lod = 0; lod < num_lods; ++lod)
//                             if (dist_sq <= src_instance.mLODScaleSq * Square(geometry_lods[lod].mDistance))
//                             {
//                                 // Store which index goes in which LOD
//                                 if (light_overlaps)
//                                     lod_indices[0][lod].push_back((int)i);
//                                 if (camera_overlaps)
//                                     lod_indices[1][lod].push_back((int)i);
//                                 break;
//                             }
//                     }
//                 }
//
//                 // Loop over both passes: 0 = light, 1 = geometry
//                 Array<int> *start_idx[] = { &v.second.mLightStartIdx, &v.second.mGeometryStartIdx };
//                 for (int type = 0; type < 2; ++type)
//                 {
//                     // Reserve space for instance indices
//                     Array<int> &type_start_idx = *start_idx[type];
//                     type_start_idx.resize(num_lods + 1);
//
//                     // Write out geometry pass instances
//                     for (size_t lod = 0; lod < num_lods; ++lod)
//                     {
//                         // Write start index for this LOD
//                         type_start_idx[lod] = dst_index;
//
//                         // Copy instances
//                         Array<int> &this_lod_indices = lod_indices[type][lod];
//                         for (int i : this_lod_indices)
//                         {
//                             const Instance &src_instance = instances[i];
//                             dst_instance[dst_index++] = src_instance;
//                         }
//
//                         // Prepare for next iteration (will preserve memory)
//                         this_lod_indices.clear();
//                     }
//
//                     // Write out end of last LOD
//                     type_start_idx.back() = dst_index;
//                 }
//             }
//
//         instances_buffer->Unlock();
//     }
//
//     if (!mPrimitives.empty() || !mTempPrimitives.empty())
//     {
//         // Front face culling, we want to render the back side of the geometry for casting shadows
//         mShadowStateFF->Activate();
//
//         // Draw all primitives as seen from the light
//         if (mNumInstances > 0)
//             for (InstanceMap::value_type &v : mPrimitives)
//                 DrawInstances(v.first, v.second.mLightStartIdx);
//         for (InstanceMap::value_type &v : mTempPrimitives)
//             DrawInstances(v.first, v.second.mLightStartIdx);
//     }
//
//     if (!mPrimitivesBackFacing.empty())
//     {
//         // Back face culling, we want to render the front side of back facing geometry
//         mShadowStateBF->Activate();
//
//         // Draw all primitives as seen from the light
//         for (InstanceMap::value_type &v : mPrimitivesBackFacing)
//             DrawInstances(v.first, v.second.mLightStartIdx);
//     }
//
//     if (!mWireframePrimitives.empty())
//     {
//         // Switch to wireframe mode
//         mShadowStateWire->Activate();
//
//         // Draw all wireframe primitives as seen from the light
//         for (InstanceMap::value_type &v : mWireframePrimitives)
//             DrawInstances(v.first, v.second.mLightStartIdx);
//     }
//
//     // Switch to the main render target
//     mRenderer->SetRenderTarget(nullptr);
//
//     // Bind the shadow map texture
//     mDepthTexture->Bind(2);
//
//     if (!mPrimitives.empty() || !mTempPrimitives.empty())
//     {
//         // Bind the normal shader, back face culling
//         mTriangleStateBF->Activate();
//
//         // Draw all primitives
//         if (mNumInstances > 0)
//             for (InstanceMap::value_type &v : mPrimitives)
//                 DrawInstances(v.first, v.second.mGeometryStartIdx);
//         for (InstanceMap::value_type &v : mTempPrimitives)
//             DrawInstances(v.first, v.second.mGeometryStartIdx);
//     }
//
//     if (!mPrimitivesBackFacing.empty())
//     {
//         // Front face culling, the next batch needs to render inside out
//         mTriangleStateFF->Activate();
//
//         // Draw all back primitives
//         for (InstanceMap::value_type &v : mPrimitivesBackFacing)
//             DrawInstances(v.first, v.second.mGeometryStartIdx);
//     }
//
//     if (!mWireframePrimitives.empty())
//     {
//         // Wire frame mode
//         mTriangleStateWire->Activate();
//
//         // Draw all wireframe primitives
//         for (InstanceMap::value_type &v : mWireframePrimitives)
//             DrawInstances(v.first, v.second.mGeometryStartIdx);
//     }
// }
