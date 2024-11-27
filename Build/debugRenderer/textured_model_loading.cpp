#include "glad/glad.h"
#include "textured_model_loading.hpp"

#include <iostream>

#include "assimp/Importer.hpp"
#include <utility>
#include "assimp/postprocess.h"
#define STB_IMAGE_IMPLEMENTATION // NOTE THIS LINE MUST NOT APPEAR ANYWHERE ELSE!!
#include "stb_image.h"

/**
 * overview:
 * 	initialization:
 * 		first we iterate through the entire assimp imported object recursively
 * 		we store all the images we found in the base model, this is the initialization phase.
 *
 * drawing:
 *	iterate through every mesh, load up it's vao and related texture and draw the vertices through the shader
 *pipeline
 *
 * notes:
 * 	the bottleneck of this program is the image loading, everything else like the vertex loading from assimp is
 *actually really fast
 *
 *
 */

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures) {
    this->vertices = std::move(vertices);
    this->indices = std::move(indices);
    this->textures = std::move(textures);
};

void Mesh::draw(GLuint shader_program_id) {
    glUseProgram(shader_program_id);
    GLuint id = this->textures.front().id;
    glBindTexture(GL_TEXTURE_2D, id);
    glBindVertexArray(vertex_attribute_object);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0); // unbinds current vertex_attribute_object
    glUseProgram(0);
};

/**
 * description:
 * 	a mesh is created with all of it's vertex data, this function
 * 	loads that data into this mesh's vao, so that all of the vertex
 * 	information for this specific mesh can be loaded by enabling the vao
 *
 * precondition:
 * 	this mesh's VAO is bound
 */
void Mesh::bind_vertex_data_to_opengl_for_later_use() {
    glGenBuffers(1, &vertex_buffer_object);
    glGenBuffers(1, &element_buffer_object);

    glBindVertexArray(vertex_attribute_object);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_object);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
};

/**
 * \pre this mesh's vao is bound
 * \warning if you get black
 *
 */
void Mesh::bind_vertex_attribute_interpretation_to_opengl_for_later_use(GLuint shader_program_id) {

    // vertex positions
    GLuint position_location = glGetAttribLocation(shader_program_id, "position");
    glEnableVertexAttribArray(position_location);
    glVertexAttribPointer(position_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);

    // vertex normals
    GLuint vnorm_location = glGetAttribLocation(shader_program_id, "normal");
    glEnableVertexAttribArray(vnorm_location);
    glVertexAttribPointer(vnorm_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));

    // vertex texture positions
    GLuint vtexpos_location = glGetAttribLocation(shader_program_id, "passthrough_texture_position");
    glEnableVertexAttribArray(vtexpos_location);
    glVertexAttribPointer(vtexpos_location, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, texture_coordinate));
};

void Mesh::configure_vertex_interpretation_for_shader(GLuint shader_program_id) {
    glGenVertexArrays(1, &vertex_attribute_object);
    this->bind_vertex_data_to_opengl_for_later_use();
    this->bind_vertex_attribute_interpretation_to_opengl_for_later_use(shader_program_id);
    // Unbind the current VAO because we're not drawing at this point in time and we don't want anyone else to
    // accidentally while drawing and plus we don't just give functions side effects for no reason.
    glBindVertexArray(0);
};

/**
 *
 * \brief loads a 3d model into wrapper structure
 *
 * \pre the shader files specified exist and are at this file path
 * \todo have a resource directory so that we don't have to specify a long relative path
 *
 * @param path the path to the model we want to load
 */
Model::Model(std::string path, GLuint shader_program_id) {
    this->shader_program_id = shader_program_id;
    this->load_model(std::move(path));
    this->configure_vertex_interpretation_for_shader();
}

void Model::draw() {
    for (unsigned int i = 0; i < this->meshes.size(); i++) {
        this->meshes[i].draw(this->shader_program_id);
    }
};

/**
 * \pre the shader pipeline must be initialized
 */
void Model::configure_vertex_interpretation_for_shader() {
    for (unsigned int i = 0; i < this->meshes.size(); i++) {
        this->meshes[i].configure_vertex_interpretation_for_shader(this->shader_program_id);
    }
}

/**
 * notes:
 * 	- this function is the entry point to the initialization process
 * 	- although this function looks short and simple, the call to process_node
 * 	is recursive and does all the work of parsing assimp's structure
 */
void Model::load_model(std::string path) {
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASIMP::" << importer.GetErrorString() << std::endl;
        return;
    }
    this->directory = path.substr(0, path.find_last_of("/"));
    printf("starting to process nodes \n");
    this->process_node(scene->mRootNode, scene);
    printf("processed all nodes\n");
};

/**
 * notes:
 * 	- recall that a node may contains a collection of meshes, and
 * 	also children nodes
 * 	- this function is guarenteed to terminate because modellers
 * 	create strctures made up of finitely many nodes and meshes.
 * 	- this can be thought of the same way that each folder in blender can have individual meshes inside or more
 * 	folders (I believe a folder is called a collection) and just represents arbitrary nesting
 */
void Model::process_node(aiNode *node, const aiScene *scene) {
    printf("stared processing meshes\n");
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        this->meshes.push_back(this->process_mesh(mesh, scene));
    }
    printf("finished processing meshes\n");
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        process_node(node->mChildren[i], scene);
    }
};

glm::vec3 assimp_to_glm_3d_vector(aiVector3D assimp_vector) {
    return {assimp_vector.x, assimp_vector.y, assimp_vector.z};
}

std::vector<Vertex> Model::process_mesh_vertices(aiMesh *mesh) {
    std::vector<Vertex> vertices;

    bool mesh_has_texture_coordinates = mesh->mTextureCoords[0] != nullptr;
    printf("This mesh has %d vertices\n", mesh->mNumVertices);
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;

        vertex.position = assimp_to_glm_3d_vector(mesh->mVertices[i]);
        vertex.normal = assimp_to_glm_3d_vector(mesh->mNormals[i]);

        if (mesh_has_texture_coordinates) {
            vertex.texture_coordinate = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        } else {
            vertex.texture_coordinate = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }
    return vertices;
}

/**
 * \brief given a mesh iterate through each face storing all indices of each vertex on that face
 */
std::vector<unsigned int> Model::process_mesh_indices(aiMesh *mesh) {
    std::vector<unsigned int> indices;
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];

        assert(face.mNumIndices == 3); // if this is false we are not working with triangles

        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }
    return indices;
}

std::vector<Texture> Model::process_mesh_materials(aiMesh *mesh, const aiScene *scene) {
    std::vector<Texture> textures;

    bool mesh_has_materials = mesh->mMaterialIndex >= 0;
    if (mesh_has_materials) {
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

        std::vector<Texture> diffuse_maps = load_material_textures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());

        std::vector<Texture> specular_maps =
            load_material_textures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specular_maps.begin(), specular_maps.end());
    }
    return textures;
}

/**
 * description
 * 	takes in assimp's interpretation of a mesh, and then parses it
 * 	into our version of a mesh
 */
Mesh Model::process_mesh(aiMesh *mesh, const aiScene *scene) {
    std::vector<Vertex> vertices = this->process_mesh_vertices(mesh);
    std::vector<unsigned int> indices = this->process_mesh_indices(mesh);
    std::vector<Texture> textures = this->process_mesh_materials(mesh, scene);
    return {vertices, indices, textures};
};

unsigned int texture_from_file(const char *path, const std::string &directory, bool gamma = false) {
    std::string filename = std::string(path);
    // we don't need this because file paths are stored correctly in the mtl file.
    // filename = directory + '/' + filename;

    unsigned int texture_id;
    glGenTextures(1, &texture_id);

    int width, height, num_components;

    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &num_components, 0);

    if (!data) {
        std::cout << "unable to load image: " << stbi_failure_reason() << "\n";
        // throw;
    }

    printf("loading file: %s with width: %d, height: %d, and has %d components\n", filename.c_str(), width, height,
           num_components);
    if (data) {
        GLenum format = 0;
        // TODO use a switch
        if (num_components == 1) {
            format = GL_RED;
        } else if (num_components == 3) {
            format = GL_RGB;
        } else if (num_components == 4) {
            format = GL_RGBA;
        }

        // TODO can I refactor this into a function called bind/prepare_texture_data_for_opengl
        glBindTexture(GL_TEXTURE_2D, texture_id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

    } else {
        std::cout << "Texture failed to load texture at path: " << path << std::endl;
    }
    stbi_image_free(data);

    return texture_id;
};

/**
 * returns if the texture is already loaded along with an index which it exists at
 * otherwise the texture is not loaded and -1 is returned as the index
 */
std::tuple<bool, int> Model::texture_already_loaded(aiString texture_path) {
    // printf("started checking for texture, so far there are %d textures to pick from\n", this->already_loaded_textures.size());
    for (unsigned int j = 0; j < this->already_loaded_textures.size(); j++) {
        const char *already_loaded_path = already_loaded_textures[j].path.data();
        const char *query_path = texture_path.C_Str();
        if (std::strcmp(already_loaded_path, query_path) == 0) {
            return std::make_tuple(true, j);
        }
    }
    return std::make_tuple(false, -1);
}

std::vector<Texture> Model::load_material_textures(aiMaterial *material, aiTextureType type, std::string type_name) {
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < material->GetTextureCount(type); i++) {
        aiString texture_path;
        material->GetTexture(type, i, &texture_path);
        bool texture_already_loaded;
        int index_of_already_loaded_texture;

        std::tie(texture_already_loaded, index_of_already_loaded_texture) = this->texture_already_loaded(texture_path);

        if (texture_already_loaded) {
            textures.push_back(this->already_loaded_textures[index_of_already_loaded_texture]);
        } else {
            Texture texture;
            texture.id = texture_from_file(texture_path.C_Str(), this->directory);
            texture.type = type_name;
            texture.path = texture_path.C_Str();
            textures.push_back(texture);
            already_loaded_textures.push_back(texture);
        }
    }
    return textures;
};
