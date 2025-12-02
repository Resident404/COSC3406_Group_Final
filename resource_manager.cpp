#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iostream>
#include <SOIL/SOIL.h>

#include "resource_manager.h"

namespace game {

ResourceManager::ResourceManager(void){
}


ResourceManager::~ResourceManager(){
}


void ResourceManager::AddResource(ResourceType type, const std::string name, GLuint resource, GLsizei size){

    Resource *res;

    res = new Resource(type, name, resource, size);

    resource_.push_back(res);
}


void ResourceManager::AddResource(ResourceType type, const std::string name, GLuint array_buffer, GLuint element_array_buffer, GLsizei size){

    Resource *res;

    res = new Resource(type, name, array_buffer, element_array_buffer, size);

    resource_.push_back(res);
}


void ResourceManager::LoadResource(ResourceType type, const std::string name, const char *filename){

    // Call appropriate method depending on type of resource
    if (type == Material){
        LoadMaterial(name, filename);
    } else if (type == Texture){
        LoadTexture(name, filename);
    } else {
        throw(std::invalid_argument(std::string("Invalid type of resource")));
    }
}


Resource *ResourceManager::GetResource(const std::string name) const {

    // Find resource with the specified name
    for (int i = 0; i < resource_.size(); i++){
        if (resource_[i]->GetName() == name){
            return resource_[i];
        }
    }
    return NULL;
}


void ResourceManager::LoadMaterial(const std::string name, const char *prefix){

    // Load vertex program source code
    std::string filename = std::string(prefix) + std::string(VERTEX_PROGRAM_EXTENSION);
    std::string vp = LoadTextFile(filename.c_str());

    // Load fragment program source code
    filename = std::string(prefix) + std::string(FRAGMENT_PROGRAM_EXTENSION);
    std::string fp = LoadTextFile(filename.c_str());

    // Create a shader from the vertex program source code
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    const char *source_vp = vp.c_str();
    glShaderSource(vs, 1, &source_vp, NULL);
    glCompileShader(vs);

    // Check if shader compiled successfully
    GLint status;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE){
        char buffer[512];
        glGetShaderInfoLog(vs, 512, NULL, buffer);
        throw(std::ios_base::failure(std::string("Error compiling vertex shader: ")+std::string(buffer)));
    }

    // Create a shader from the fragment program source code
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    const char *source_fp = fp.c_str();
    glShaderSource(fs, 1, &source_fp, NULL);
    glCompileShader(fs);

    // Check if shader compiled successfully
    glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE){
        char buffer[512];
        glGetShaderInfoLog(fs, 512, NULL, buffer);
        throw(std::ios_base::failure(std::string("Error compiling fragment shader: ")+std::string(buffer)));
    }

    // Create a shader program linking both vertex and fragment shaders
    // together
    GLuint sp = glCreateProgram();
    glAttachShader(sp, vs);
    glAttachShader(sp, fs);
    glLinkProgram(sp);

    // Check if shaders were linked successfully
    glGetProgramiv(sp, GL_LINK_STATUS, &status);
    if (status != GL_TRUE){
        char buffer[512];
        glGetShaderInfoLog(sp, 512, NULL, buffer);
        throw(std::ios_base::failure(std::string("Error linking shaders: ")+std::string(buffer)));
    }

    // Delete memory used by shaders, since they were already compiled
    // and linked
    glDeleteShader(vs);
    glDeleteShader(fs);

    // Add a resource for the shader program
    AddResource(Material, name, sp, 0);
}


std::string ResourceManager::LoadTextFile(const char *filename){

    // Open file
    std::ifstream f;
    f.open(filename);
    if (f.fail()){
        throw(std::ios_base::failure(std::string("Error opening file ")+std::string(filename)));
    }

    // Read file
    std::string content;
    std::string line;
    while(std::getline(f, line)){
        content += line + "\n";
    }

    // Close file
    f.close();

    return content;
}


void ResourceManager::LoadTexture(const std::string name, const char *filename){

    // Load image from file using SOIL
    int width, height;
    unsigned char* image = SOIL_load_image(filename, &width, &height, 0, SOIL_LOAD_RGBA);
    if (!image){
        throw(std::ios_base::failure(std::string("Error loading texture file: ")+std::string(filename)+std::string(" - ")+std::string(SOIL_last_result())));
    }

    // Create OpenGL texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Free image data
    SOIL_free_image_data(image);

    // Add texture resource
    AddResource(Texture, name, texture, 0);
}


void ResourceManager::CreateTorus(std::string object_name, float loop_radius, float circle_radius, int num_loop_samples, int num_circle_samples){

    // Create a torus
    // The torus is built from a large loop with small circles around the loop

    // Number of vertices and faces to be created
    // Check the construction algorithm below to understand the numbers
    // specified below
    const GLuint vertex_num = num_loop_samples*num_circle_samples;
    const GLuint face_num = num_loop_samples*num_circle_samples*2;

    // Number of attributes for vertices and faces
    const int vertex_att = 11;
    const int face_att = 3;

    // Data buffers for the torus
    GLfloat *vertex = NULL;
    GLuint *face = NULL;

    // Allocate memory for buffers
    try {
        vertex = new GLfloat[vertex_num * vertex_att]; // 11 attributes per vertex: 3D position (3), 3D normal (3), RGB color (3), 2D texture coordinates (2)
        face = new GLuint[face_num * face_att]; // 3 indices per face
    }
    catch  (std::exception &e){
        throw e;
    }

    // Create vertices 
    float theta, phi; // Angles for circles
    glm::vec3 loop_center;
    glm::vec3 vertex_position;
    glm::vec3 vertex_normal;
    glm::vec3 vertex_color;
    glm::vec2 vertex_coord;

    for (int i = 0; i < num_loop_samples; i++){ // large loop
        
        theta = 2.0*glm::pi<GLfloat>()*i/num_loop_samples; // loop sample (angle theta)
        loop_center = glm::vec3(loop_radius*cos(theta), loop_radius*sin(theta), 0); // centre of a small circle

        for (int j = 0; j < num_circle_samples; j++){ // small circle
            
            phi = 2.0*glm::pi<GLfloat>()*j/num_circle_samples; // circle sample (angle phi)
            
            // Define position, normal and color of vertex
            vertex_normal = glm::vec3(cos(theta)*cos(phi), sin(theta)*cos(phi), sin(phi));
            vertex_position = loop_center + vertex_normal*circle_radius;
            vertex_color = glm::vec3(1.0 - ((float) i / (float) num_loop_samples), 
                                            (float) i / (float) num_loop_samples, 
                                            (float) j / (float) num_circle_samples);
            vertex_coord = glm::vec2(theta / 2.0*glm::pi<GLfloat>(),
                                     phi / 2.0*glm::pi<GLfloat>());

            // Add vectors to the data buffer
            for (int k = 0; k < 3; k++){
                vertex[(i*num_circle_samples+j)*vertex_att + k] = vertex_position[k];
                vertex[(i*num_circle_samples+j)*vertex_att + k + 3] = vertex_normal[k];
                vertex[(i*num_circle_samples+j)*vertex_att + k + 6] = vertex_color[k];
            }
            vertex[(i*num_circle_samples+j)*vertex_att + 9] = vertex_coord[0];
            vertex[(i*num_circle_samples+j)*vertex_att + 10] = vertex_coord[1];
        }
    }

    // Create triangles
    for (int i = 0; i < num_loop_samples; i++){
        for (int j = 0; j < num_circle_samples; j++){
            // Two triangles per quad
            glm::vec3 t1(((i + 1) % num_loop_samples)*num_circle_samples + j, 
                         i*num_circle_samples + ((j + 1) % num_circle_samples),
                         i*num_circle_samples + j);    
            glm::vec3 t2(((i + 1) % num_loop_samples)*num_circle_samples + j,
                         ((i + 1) % num_loop_samples)*num_circle_samples + ((j + 1) % num_circle_samples),
                         i*num_circle_samples + ((j + 1) % num_circle_samples));
            // Add two triangles to the data buffer
            for (int k = 0; k < 3; k++){
                face[(i*num_circle_samples+j)*face_att*2 + k] = (GLuint) t1[k];
                face[(i*num_circle_samples+j)*face_att*2 + k + face_att] = (GLuint) t2[k];
            }
        }
    }

    // Create OpenGL buffers and copy data
    //GLuint vao;
    //glGenVertexArrays(1, &vao);
    //glBindVertexArray(vao);

    GLuint vbo, ebo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_num * vertex_att * sizeof(GLfloat), vertex, GL_STATIC_DRAW);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, face_num * face_att * sizeof(GLuint), face, GL_STATIC_DRAW);

    // Free data buffers
    delete [] vertex;
    delete [] face;

    // Create resource
    AddResource(Mesh, object_name, vbo, ebo, face_num * face_att);
}


void ResourceManager::CreateSphere(std::string object_name, float radius, int num_samples_theta, int num_samples_phi){

    // Create a sphere using a well-known parameterization

    // Number of vertices and faces to be created
    const GLuint vertex_num = num_samples_theta*num_samples_phi;
    const GLuint face_num = num_samples_theta*(num_samples_phi-1)*2;

    // Number of attributes for vertices and faces
    const int vertex_att = 11;
    const int face_att = 3;

    // Data buffers 
    GLfloat *vertex = NULL;
    GLuint *face = NULL;

    // Allocate memory for buffers
    try {
        vertex = new GLfloat[vertex_num * vertex_att]; // 11 attributes per vertex: 3D position (3), 3D normal (3), RGB color (3), 2D texture coordinates (2)
        face = new GLuint[face_num * face_att]; // 3 indices per face
    }
    catch  (std::exception &e){
        throw e;
    }

    // Create vertices 
    float theta, phi; // Angles for parametric equation
    glm::vec3 vertex_position;
    glm::vec3 vertex_normal;
    glm::vec3 vertex_color;
    glm::vec2 vertex_coord;
   
    for (int i = 0; i < num_samples_theta; i++){
            
        theta = 2.0*glm::pi<GLfloat>()*i/(num_samples_theta-1); // angle theta
            
        for (int j = 0; j < num_samples_phi; j++){
                    
            phi = glm::pi<GLfloat>()*j/(num_samples_phi-1); // angle phi

            // Define position, normal and color of vertex
            vertex_normal = glm::vec3(cos(theta)*sin(phi), sin(theta)*sin(phi), -cos(phi));
            // We need z = -cos(phi) to make sure that the z coordinate runs from -1 to 1 as phi runs from 0 to pi
            // Otherwise, the normal will be inverted
            vertex_position = glm::vec3(vertex_normal.x*radius, 
                                        vertex_normal.y*radius, 
                                        vertex_normal.z*radius),
            vertex_color = glm::vec3(((float)i)/((float)num_samples_theta), 1.0-((float)j)/((float)num_samples_phi), ((float)j)/((float)num_samples_phi));
            vertex_coord = glm::vec2(((float)i)/((float)num_samples_theta), 1.0-((float)j)/((float)num_samples_phi));

            // Add vectors to the data buffer
            for (int k = 0; k < 3; k++){
                vertex[(i*num_samples_phi+j)*vertex_att + k] = vertex_position[k];
                vertex[(i*num_samples_phi+j)*vertex_att + k + 3] = vertex_normal[k];
                vertex[(i*num_samples_phi+j)*vertex_att + k + 6] = vertex_color[k];
            }
            vertex[(i*num_samples_phi+j)*vertex_att + 9] = vertex_coord[0];
            vertex[(i*num_samples_phi+j)*vertex_att + 10] = vertex_coord[1];
        }
    }

    // Create faces
    for (int i = 0; i < num_samples_theta; i++){
        for (int j = 0; j < (num_samples_phi-1); j++){
            // Two triangles per quad
            glm::vec3 t1(((i + 1) % num_samples_theta)*num_samples_phi + j, 
                         i*num_samples_phi + (j + 1),
                         i*num_samples_phi + j);
            glm::vec3 t2(((i + 1) % num_samples_theta)*num_samples_phi + j, 
                         ((i + 1) % num_samples_theta)*num_samples_phi + (j + 1), 
                         i*num_samples_phi + (j + 1));
            // Add two triangles to the data buffer
            for (int k = 0; k < 3; k++){
                face[(i*(num_samples_phi-1)+j)*face_att*2 + k] = (GLuint) t1[k];
                face[(i*(num_samples_phi-1)+j)*face_att*2 + k + face_att] = (GLuint) t2[k];
            }
        }
    }

    // Create OpenGL buffers and copy data
    //GLuint vao;
    //glGenVertexArrays(1, &vao);
    //glBindVertexArray(vao);

    GLuint vbo, ebo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_num * vertex_att * sizeof(GLfloat), vertex, GL_STATIC_DRAW);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, face_num * face_att * sizeof(GLuint), face, GL_STATIC_DRAW);

    // Free data buffers
    delete [] vertex;
    delete [] face;

    // Create resource
    AddResource(Mesh, object_name, vbo, ebo, face_num * face_att);
}


void ResourceManager::CreateCylindricalGeometry(std::string object_name, float top_radius, float bottom_radius, float height, int linear_samples, int circle_samples) {

    if (linear_samples < 2) { linear_samples = 2; }

    //Specify the number of vertices and faces that will be created. This is done to determine how big the buffers must be for the model.
    const GLuint vertex_num = linear_samples * circle_samples + 2;
    const GLuint face_num = linear_samples * circle_samples * 2 + (circle_samples * 2);

    //Specify the number of attributes each vertex and face will have. Again, this is used to determine the size of the buffers for the model.
    const int vertex_att = 11;
    const int face_att = 3;

    //Get some pointers ready for the data of our buffers.
    GLfloat* vertex = NULL;
    GLuint* face = NULL;

    //Try to allocate the memory for buffers of the model.
    try {
        vertex = new GLfloat[vertex_num * vertex_att];
        face = new GLuint[face_num * face_att];
    }
    catch (std::exception& e) {
        throw e;
    }

    //Create vertices
    float theta; //Angle increment between points along the circumference of a circle.
    glm::vec3 circle_center;
    glm::vec3 vertex_position;
    glm::vec3 vertex_normal;
    glm::vec3 vertex_color;
    glm::vec2 vertex_coord;

    for (int i = 0; i < linear_samples; i++) { //Iterate along the line our cylinder will be made along.

        circle_center = glm::vec3(0, height * (((float)i / (linear_samples - 1)) - 0.5), 0);         //The starting point of the line at the center of the cylinder.

        for (int j = 0; j < circle_samples; j++) {  //For creating the vertices of each circle centered at each point along the line of the cylinder.

            theta = 2 * glm::pi<GLfloat>() * j / circle_samples;

            //interpolated value for different top and bottom radii
            //radius = ((((float)i / (linear_samples - 1)) * top_radius) + ((1 - ((float)i / (linear_samples - 1))) * bottom_radius))
            // Define position, normal and color of vertex for point along the circumference of the current circle.
            vertex_normal = glm::vec3(((((float)i / (linear_samples - 1)) * top_radius) + ((1 - ((float)i / (linear_samples - 1))) * bottom_radius)) * cos(theta),
                0,
                ((((float)i / (linear_samples - 1)) * top_radius) + ((1 - ((float)i / (linear_samples - 1))) * bottom_radius)) * sin(theta));
            vertex_position = circle_center + vertex_normal * ((((float)i / (linear_samples - 1)) * top_radius) + ((1 - ((float)i / (linear_samples - 1))) * bottom_radius));
            //vertex_position = glm::vec3(vertex_normal.x * ((((float)i / (linear_samples - 1)) * top_radius) + ((1 - ((float)i / (linear_samples - 1))) * bottom_radius)),
            //                            vertex_normal.y * ((((float)i / (linear_samples - 1)) * top_radius) + ((1 - ((float)i / (linear_samples - 1))) * bottom_radius)),
            //                            vertex_normal.z * ((((float)i / (linear_samples - 1)) * top_radius) + ((1 - ((float)i / (linear_samples - 1))) * bottom_radius)));
            vertex_color = glm::vec3(1.0 - ((float)i / (float)linear_samples),
                (float)i / (float)linear_samples,
                (float)j / (float)circle_samples);
            vertex_coord = glm::vec2(theta / 2.0 * glm::pi<GLfloat>(), i);

            // Add vectors to the data buffer.
            for (int k = 0; k < 3; k++) {
                vertex[(i * circle_samples + j) * vertex_att + k + 0] = vertex_position[k]; //3D position     (3)
                vertex[(i * circle_samples + j) * vertex_att + k + 3] = vertex_normal[k];   //3D normal       (3)
                vertex[(i * circle_samples + j) * vertex_att + k + 6] = vertex_color[k];    //RGB color       (3)
            }
            vertex[(i * circle_samples + j) * vertex_att + 9] = vertex_coord[0];        //Texture coord x (1)
            vertex[(i * circle_samples + j) * vertex_att + 10] = vertex_coord[1];       //Texture coord y (1)
        }
    }

    //End caps
    //bottom center data
    vertex_normal = glm::vec3(0., -1., 0.);
    vertex_position = glm::vec3(0, height * ((0 / (linear_samples - 1)) - 0.5), 0);
    vertex_color = glm::vec3(1.0 - (0. / (float)linear_samples),
        0. / (float)linear_samples,
        0. / (float)circle_samples);
    vertex_coord = glm::vec2(0, 0);

    //add the bottom center's vertex to vertex data.
    for (int k = 0; k < 3; k++) {
        vertex[((linear_samples - 1) * circle_samples + (circle_samples - 1)) * vertex_att + vertex_att + k + 0] = vertex_position[k];
        vertex[((linear_samples - 1) * circle_samples + (circle_samples - 1)) * vertex_att + vertex_att + k + 3] = vertex_normal[k];
        vertex[((linear_samples - 1) * circle_samples + (circle_samples - 1)) * vertex_att + vertex_att + k + 6] = vertex_color[k];
    }
    vertex[((linear_samples - 1) * circle_samples + (circle_samples - 1)) * vertex_att + vertex_att + 9] = vertex_coord[0];
    vertex[(((int)-1) * circle_samples + (circle_samples - 1)) * vertex_att + vertex_att + 10] = vertex_coord[1];

    //top center data
    vertex_normal = glm::vec3(0., -1., 0.);
    vertex_position = glm::vec3(0, height * (((linear_samples - 1) / (linear_samples - 1)) - 0.5), 0);
    vertex_color = glm::vec3(1.0 - (((float)linear_samples - 1) / (float)linear_samples),
        ((float)linear_samples - 1) / (float)linear_samples,
        ((float)circle_samples - 1) / (float)circle_samples);
    vertex_coord = glm::vec2(1, 1);

    //add the top center's vertex to vertex data.
    for (int k = 0; k < 3; k++) {
        vertex[((linear_samples - 1) * circle_samples + (circle_samples - 1)) * vertex_att + (vertex_att * 2) + k + 0] = vertex_position[k];
        vertex[((linear_samples - 1) * circle_samples + (circle_samples - 1)) * vertex_att + (vertex_att * 2) + k + 3] = vertex_normal[k];
        vertex[((linear_samples - 1) * circle_samples + (circle_samples - 1)) * vertex_att + (vertex_att * 2) + k + 6] = vertex_color[k];
    }
    vertex[((linear_samples - 1) * circle_samples + (circle_samples - 1)) * vertex_att + (vertex_att * 2) + 9] = vertex_coord[0];
    vertex[((linear_samples - 1) * circle_samples + (circle_samples - 1)) * vertex_att + (vertex_att * 2) + 10] = vertex_coord[1];

    //Create triangles
    for (int i = 0; i < linear_samples; i++) {
        for (int j = 0; j < circle_samples; j++) {
            // Two triangles per quad
            glm::vec3 t1(((i + 1) % linear_samples) * circle_samples + j,   //Describes where the first triangle is located in the buffer.
                i * circle_samples + ((j + 1) % circle_samples),
                i * circle_samples + j);
            glm::vec3 t2(((i + 1) % linear_samples) * circle_samples + j,   //Describes where the second triangle is located in the buffer.
                ((i + 1) % linear_samples) * circle_samples + ((j + 1) % circle_samples),
                i * circle_samples + ((j + 1) % circle_samples));
            // Add two triangles to the data buffer
            for (int k = 0; k < 3; k++) {
                face[(i * circle_samples + j) * face_att * 2 + k + face_att] = (GLuint)t2[k];
                face[(i * circle_samples + j) * face_att * 2 + k] = (GLuint)t1[k];
            }

            //Create triangles for end caps of cylinder.
            if (i == linear_samples - 1 && j == circle_samples - 1) {
                for (int g = 0; g < circle_samples; g++) {
                    glm::vec3 t3((g + 1) % circle_samples,
                        linear_samples * circle_samples,
                        g);
                    glm::vec3 t4(linear_samples * circle_samples + 1,
                        ((linear_samples - 1) * circle_samples) + ((g + 1) % circle_samples),
                        ((linear_samples - 1) * circle_samples) + g);
                    //Add the triangles to the buffer accounting for some offset. (triangles fo caps are stored at the end of the buffer)
                    for (int k = 0; k < 3; k++) {
                        face[((linear_samples - 1) * circle_samples + (circle_samples - 1) + g) * face_att * 2 + k + (face_att * 2)] = (GLuint)t4[k];
                        face[((linear_samples - 1) * circle_samples + (circle_samples - 1) + g) * face_att * 2 + k + (face_att)] = (GLuint)t3[k];
                    }
                }
            }
        }
    }

    // Create OpenGL buffers and copy data
    GLuint vbo, ebo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_num * vertex_att * sizeof(GLfloat), vertex, GL_STATIC_DRAW);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, face_num * face_att * sizeof(GLuint), face, GL_STATIC_DRAW);

    // Free data buffers
    delete[] vertex;
    delete[] face;

    // Create resource
    AddResource(Mesh, object_name, vbo, ebo, face_num * face_att);
}

// Create the geometry for a cube centered at (0, 0, 0) with sides of length 1
void ResourceManager::CreateCube(std::string object_name){

    // This construction uses shared vertices, following the same data
    // format as the other functions 
    // However, vertices are repeated since their normals at each face
    // are different
    // Each face of the cube is defined by four vertices (with the same normal) and two triangles

    // Vertices used to build the cube
    // 11 attributes per vertex: 3D position (3), 3D normal (3), RGB color (3), texture coordinates (2)
    GLfloat vertex[] = {
        // First cube face 
        -0.5, -0.5,  0.5,    0.0,  0.0,  1.0,    1.0, 0.0, 0.0,    0.0, 0.0,
         0.5, -0.5,  0.5,    0.0,  0.0,  1.0,    0.0, 1.0, 0.0,    1.0, 0.0,
         0.5,  0.5,  0.5,    0.0,  0.0,  1.0,    0.0, 0.0, 1.0,    1.0, 1.0,
        -0.5,  0.5,  0.5,    0.0,  0.0,  1.0,    1.0, 0.0, 1.0,    0.0, 1.0,
        // Second cube face
         0.5, -0.5, -0.5,    1.0,  0.0,  0.0,    1.0, 0.0, 0.0,    0.0, 0.0,
         0.5,  0.5, -0.5,    1.0,  0.0,  0.0,    0.0, 1.0, 0.0,    1.0, 0.0,
         0.5,  0.5,  0.5,    1.0,  0.0,  0.0,    0.0, 0.0, 1.0,    1.0, 1.0,
         0.5, -0.5,  0.5,    1.0,  0.0,  0.0,    1.0, 0.0, 1.0,    0.0, 1.0,
        // Third cube face
         0.5, -0.5, -0.5,    0.0,  0.0, -1.0,    1.0, 0.0, 0.0,    0.0, 0.0,
        -0.5, -0.5, -0.5,    0.0,  0.0, -1.0,    0.0, 1.0, 0.0,    1.0, 0.0,
        -0.5,  0.5, -0.5,    0.0,  0.0, -1.0,    0.0, 0.0, 1.0,    1.0, 1.0,
         0.5,  0.5, -0.5,    0.0,  0.0, -1.0,    1.0, 0.0, 1.0,    0.0, 1.0,
        // Fourth cube face
        -0.5,  0.5, -0.5,   -1.0,  0.0,  0.0,    1.0, 0.0, 0.0,    0.0, 0.0,
        -0.5, -0.5, -0.5,   -1.0,  0.0,  0.0,    0.0, 1.0, 0.0,    1.0, 0.0,
        -0.5, -0.5,  0.5,   -1.0,  0.0,  0.0,    0.0, 0.0, 1.0,    1.0, 1.0,
        -0.5,  0.5,  0.5,   -1.0,  0.0,  0.0,    1.0, 0.0, 1.0,    0.0, 1.0,
        // Fifth cube face
        -0.5,  0.5, -0.5,    0.0,  1.0,  0.0,    1.0, 0.0, 0.0,    0.0, 0.0,
        -0.5,  0.5,  0.5,    0.0,  1.0,  0.0,    0.0, 1.0, 0.0,    0.0, 1.0,
         0.5,  0.5,  0.5,    0.0,  1.0,  0.0,    0.0, 0.0, 1.0,    1.0, 1.0,
         0.5,  0.5, -0.5,    0.0,  1.0,  0.0,    1.0, 0.0, 1.0,    1.0, 0.0,
        // Sixth cube face
         0.5, -0.5, -0.5,    0.0, -1.0,  0.0,    1.0, 0.0, 0.0,    0.0, 0.0,
        -0.5, -0.5, -0.5,    0.0, -1.0,  0.0,    0.0, 1.0, 0.0,    1.0, 0.0,
        -0.5, -0.5,  0.5,    0.0, -1.0,  0.0,    0.0, 0.0, 1.0,    1.0, 1.0,
         0.5, -0.5,  0.5,    0.0, -1.0,  0.0,    1.0, 0.0, 1.0,    0.0, 1.0,
    };

    // Triangles
    GLuint face[] = {
        // First cube face, with two triangles
        0, 1, 2,
        0, 2, 3,
        // Second face
        4, 5, 6,
        4, 6, 7,
        // Third face
        8, 9, 10,
        8, 10, 11,
        // Fourth face
        12, 13, 14,
        12, 14, 15,
        // Fifth face
        16, 17, 18,
        16, 18, 19,
        // Sixth face
        20, 21, 22,
        20, 22, 23,
    };

    // Create OpenGL buffers and copy data
    GLuint vbo, ebo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(face), face, GL_STATIC_DRAW);

    // Create resource
    AddResource(Mesh, object_name, vbo, ebo, sizeof(face) / sizeof(GLfloat));
}

} // namespace game;
