#include "physics_debug_renderer.hpp"
#include "glm/gtc/type_ptr.hpp"


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

glm::mat4 convert_mat4_from_jolt_to_glm(JPH::Mat44 input_matrix) {
    glm::mat4 glm_mat;
    for (int col = 0; col < 4; col ++) {
        for (int row = 0; row < 4; row ++) {
            glm_mat[col][row] = input_matrix(row, col);
        }
    }
    return glm_mat;
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
    "uniform vec4 ourColor;\n"
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
        fprintf(stderr, "Error: %s\n", "glfw couldn't be initialized");
        exit(1);
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
        fprintf(stderr, "Error: %s\n", "Failed to create GLFW window");
        glfwTerminate();
        exit(1);
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "Error: %s\n", "Failed to initialize GLAD");
        exit(1);
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
        fprintf(stderr, "Error: ERROR::SHADER::VERTEX::COMPILATION_FAILED\n %s\n", infoLog);
        exit(1);
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
        fprintf(stderr, "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n %s\n", infoLog);
        exit(1);
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
        fprintf(stderr, "ERROR::SHADER::PROGRAM::LINKING_FAILED\n %s\n", infoLog);
        exit(1);
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

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    JPH::DebugRenderer::Initialize();
}

void PhysicsDebugRenderer::DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) {

    printf("trying to draw line\n");
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

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
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

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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


void PhysicsDebugRenderer::DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3,
                                        JPH::ColorArg inColor, ECastShadow inCastShadow) {
    printf("trying to draw triangle\n");
}

void PhysicsDebugRenderer::DrawText3D(JPH::RVec3Arg inPosition, const JPH::string_view &inString, JPH::ColorArg inColor,
                                      float inHeight) {
    printf("trying to draw triangle\n");
}
