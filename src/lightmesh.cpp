#include <GL/glew.h>
#ifdef __APPLE__
	#include <OpenGL/gl3.h>
	#include <GLUT/glut.h>
#else
	#include <GL/gl.h>
	#include <GL/freeglut.h>
#endif

#include <stdio.h>
#include <iostream>
#include <time.h>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <object.h>
#include <texture.h>
#include <shader.h>
#include <camera.h>
#include <terrain.h>
#include <vector>
#include <light.h>
#include <fbo.h>
#include <heightfield.h>
#include <lightmesh.h>
#include <skybox.h>

float wvertices[] = {
    -1, 1, 0,
    1, 1, 0,
    1, -1, 0,
    -1, -1, 0
};

float wtexcoords[] = {
    0, 0,
    1, 0,
    1, 1,
    0, 1
};

float wnormals[] = {
    0, 0, 1,
    0, 0, 1,
    0, 0, 1,
    0, 0, 1
};

int windices[] = {
    0, 1, 3,
    1, 2, 3
};

LightMesh::LightMesh(int detail, float scale) {
    this->detail = detail;
    this->scale = scale;

    generateGeometry();
    prepareBuffers();
    prepareShaders();
}

void LightMesh::generateGeometry() {
    std::vector<glm::vec3> vertices;
    std::vector<unsigned> indices;

    for(int i = 0; i < detail; i++) {
        for(int j = 0; j < detail; j++) {
            vertices.push_back(glm::vec3((j / (float) detail) * 2 - 1, (i / (float) detail) * 2 - 1, 0));
            // std::cout << glm::to_string(glm::vec3((j / (float) detail) * 2 - 1, (i / (float) detail) * 2 - 1, 0)) << std::endl;

            if(i == 0 || j == 0) continue;

            // Top left triangle
            indices.push_back(((i - 1) * detail) + j - 1);
            indices.push_back(((i - 1) * detail) + j);
            indices.push_back((i * detail) + j - 1);

            // Bottom right triangle
            indices.push_back(((i - 1) * detail) + j);
            indices.push_back((i * detail) + j);
            indices.push_back((i * detail) + j - 1);
        }
    }

    std::cout << "Generated " << vertices.size() << " vertices and " << indices.size() << " indices for lightmesh." << std::endl;

    // Convert data into C arrays
    float *vertexArray = (float*) malloc(vertices.size() * 3 * sizeof(float));

    for(int i = 0; i < vertices.size(); i++) {
        vertexArray[i * 3] = vertices[i][0];
        vertexArray[i * 3 + 1] = vertices[i][1];
        vertexArray[i * 3 + 2] = vertices[i][2];
    }

    plane = new Object();
    plane->loadVertices(vertexArray, &indices[0], vertices.size(), indices.size());
    plane->scale(scale, 1 , scale);
    // plane->loadVertices(wvertices, wtexcoords, wnormals, windices, 4, 6);
}

void LightMesh::prepareBuffers() {
    caustics = new FBO(2048, 2048);
}
void LightMesh::prepareShaders() {
    causticShader = new Shader();
    std::cout << "Compiling caustic vertex shader..." << std::endl;
    causticShader->attachShader(GL_VERTEX_SHADER, "shaders/caustics_vertex.glsl");
    std::cout << "Compiling caustic fragment shader..." << std::endl;
    causticShader->attachShader(GL_FRAGMENT_SHADER, "shaders/caustics_fragment.glsl");
    if(!causticShader->compile()) std::cout << "Error compiling caustic shader!" << std::endl;
}

FBO *LightMesh::draw(glm::mat4 projectionMatrix, Camera *camera, DirectionalLight *light) {

    caustics->bind();
    glViewport(0, 0, 2048, 2048);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    causticShader->attach();
    // causticShader->loadProjectionMatrix(projectionMatrix);
    // causticShader->loadViewMatrix(camera->getViewMatrix());
    causticShader->loadLight(light);
    causticShader->enableTexture();

    plane->setShader(causticShader);
    plane->draw();

    int causticDetail = 835;

    // float pixels[causticDetail * causticDetail * 3];
    // glReadPixels(0, 0, causticDetail, causticDetail, GL_RGB, GL_FLOAT, pixels);
    // for(int i = 0; i < causticDetail; i++) {
    //     for(int j = 0; j < causticDetail; j++) {
    //         std::cout << "(";
    //         for(int k = 0; k < 3; k++) {
    //             std::cout << pixels[(i * (causticDetail * 3)) + (j * 3) + k] << ",";
    //         }
    //         std::cout << ") ";
    //     }
    //     std::cout << std::endl;
    // }

    caustics->unbind();
    causticShader->detach();

    return caustics;
}