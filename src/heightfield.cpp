#include <iostream>
#include <vector>

#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <heightfield.h>
#include <fbo.h>
#include <shader.h>
#include <object.h>

float qvertices[] = {
    -1, 1, 0,
    1, 1, 0,
    1, -1, 0,
    -1, -1, 0
};

float qtexcoords[] = {
    0, 0,
    1, 0,
    1, 1,
    0, 1
};

float qnormals[] = {
    0, 0, 1,
    0, 0, 1,
    0, 0, 1,
    0, 0, 1
};

int qindices[] = {
    0, 1, 3,
    1, 2, 3
};

Heightfield::Heightfield(int detail, float scale) {
    this->detail = detail * scale;
    
    this->modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0.2, 0));
    this->modelMatrix = glm::scale(this->modelMatrix, glm::vec3(scale, 1, scale));
    prepareBuffers();
    prepareShaders();
    generateGeometry();
    connectTexture();

    o = new Object();
    o->loadVertices(qvertices, qtexcoords, qnormals, qindices, 4, 6);
}

void Heightfield::connectTexture() {
    glGenTextures(0, &refractTextureLocation);
    glGenTextures(1, &reflectTextureLocation);
}

void Heightfield::prepareBuffers() {
    glGenBuffers(1, &verticesVBO);
    glGenBuffers(1, &indicesVBO);
    glGenVertexArrays(1, &vaoId);

    heightA = new FBO(detail, detail);
    heightB = new FBO(detail, detail);
    normals = new FBO(detail, detail);
}

void Heightfield::prepareShaders() {
    addHeightShader = new Shader();
    std::cout << "Compiling addheight vertex shader..." << std::endl;
    addHeightShader->attachShader(GL_VERTEX_SHADER, "shaders/addheight_vertex.glsl");
    std::cout << "Compiling addheight fragment shader..." << std::endl;
    addHeightShader->attachShader(GL_FRAGMENT_SHADER, "shaders/addheight_fragment.glsl");
    if(!addHeightShader->compile()) std::cout << "Failed to compile addheight shader!" << std::endl;

    addHeightWidthLocation = addHeightShader->getUniformLocation("width");
    addHeightHeightLocation = addHeightShader->getUniformLocation("height");
    addHeightLocationLocation = addHeightShader->getUniformLocation("location");
    addHeightAmountLocation = addHeightShader->getUniformLocation("amount");

    stepSimulationShader = new Shader();
    std::cout << "Compiling stepsimulation vertex shader..." << std::endl;
    stepSimulationShader->attachShader(GL_VERTEX_SHADER, "shaders/stepsimulation_vertex.glsl");
    std::cout << "Compiling stepsimulation fragment shader..." << std::endl;
    stepSimulationShader->attachShader(GL_FRAGMENT_SHADER, "shaders/stepsimulation_fragment.glsl");
    if(!stepSimulationShader->compile()) std::cout << "Failed to compile stepsimulation shader!" << std::endl;

    stepSimulationWidthLocation = stepSimulationShader->getUniformLocation("width");
    stepSimulationHeightLocation = stepSimulationShader->getUniformLocation("height");

    normalCalculationShader = new Shader();
    std::cout << "Compiling calculatenormals vertex shader..." << std::endl;
    normalCalculationShader->attachShader(GL_VERTEX_SHADER, "shaders/calculatenormals_vertex.glsl");
    std::cout << "Compiling calculatenormals fragment shader..." << std::endl;
    normalCalculationShader->attachShader(GL_FRAGMENT_SHADER, "shaders/calculatenormals_fragment.glsl");
    if(!normalCalculationShader->compile()) std::cout << "Failed to compile calculatenormals shader!" << std::endl;

    normalCalculationWidthLocation = normalCalculationShader->getUniformLocation("width");
    normalCalculationHeightLocation = normalCalculationShader->getUniformLocation("height");

}

void Heightfield::generateGeometry() {
    std::vector<glm::vec3> vertices;
    std::vector<unsigned> indices;

    for(int i = 0; i < detail; i++) {
        for(int j = 0; j < detail; j++) {
            vertices.push_back(glm::vec3(i / (float) detail, ((i + j) / 8) / (float) detail - 0.1, -j / (float) detail));

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

    std::cout << "Generated " << vertices.size() << " vertices and " << indices.size() << " indices for heightmap." << std::endl;

    this->nVertices = vertices.size();
    this->nIndices = indices.size();

    // Convert data into C arrays
    float *vertexArray = (float*) malloc(vertices.size() * 3 * sizeof(float));

    for(int i = 0; i < vertices.size(); i++) {
        vertexArray[i * 3] = vertices[i][0];
        vertexArray[i * 3 + 1] = vertices[i][1];
        vertexArray[i * 3 + 2] = vertices[i][2];
    }

    // Set up data in VBOs
    glBindBuffer(GL_ARRAY_BUFFER, this->verticesVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * 3 * sizeof(float), vertexArray, GL_STATIC_DRAW);

    glBindVertexArray(vaoId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->indicesVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, this->verticesVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Heightfield::draw(Shader *shader) {
    // std::cout << "Drawing water" << std::endl;
    shader->loadModelMatrix(modelMatrix);
    shader->enableTexture();

    heightA->bindColorTexture(GL_TEXTURE0);
    // float pixels[width * height * 3];
    // glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, pixels);
    // for(int i = 0; i < height; i++) {
    //     for(int j = 0; j < width; j++) {
    //         std::cout << "(";
    //         for(int k = 0; k < 3; k++) {
    //             std::cout << pixels[(i * (width * 3)) + (j * 3) + k] << ",";
    //         }
    //         std::cout << ") ";
    //     }
    //     std::cout << std::endl;
    // }
    // std::cout << std::endl;

    glBindVertexArray(vaoId);
    glEnableVertexAttribArray(0);
    
    glDrawElements(GL_TRIANGLES, this->nIndices, GL_UNSIGNED_INT, 0);
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR)
    {
        std::cout << "Error in Object::draw 0: " << err << std::endl;
    }
    glDisableVertexAttribArray(0);
    glBindVertexArray(0);

    heightA->unbindColorTexture();
}

void Heightfield::addHeight(float amount, glm::vec2 location) {
    addHeightShader->attach();
    addHeightShader->enableTexture();

    // Object *o = new Object();
    // o->loadVertices(qvertices, qtexcoords, qnormals, qindices, 4, 6);

    glUniform2fv(addHeightLocationLocation, 1, &location[0]);
    glUniform1f(addHeightAmountLocation, amount);
    glUniform1f(addHeightWidthLocation, detail);
    glUniform1f(addHeightHeightLocation, detail);

    heightB->bind(); // render to height B
    heightA->bindColorTexture(GL_TEXTURE0);

    o->setShader(addHeightShader);
    o->draw();
    
    // float pixels[16 * 16 * 3];
    // glReadPixels(0, 0, 16, 16, GL_RGB, GL_FLOAT, pixels);
    // for(int i = 0; i < 16; i++) {
    //     for(int j = 0; j < 16; j++) {
    //         std::cout << "(";
    //         for(int k = 0; k < 3; k++) {
    //             std::cout << pixels[(i * (16 * 3)) + (j * 3) + k] << ",";
    //         }
    //         std::cout << ") ";
    //     }
    //     std::cout << std::endl;
    // }

    heightA->unbindColorTexture();
    heightB->unbind();

    addHeightShader->detach();

    swapBuffers();
}

void Heightfield::stepSimulation() {
    stepSimulationShader->attach();
    stepSimulationShader->enableTexture();

    // Object *o = new Object();
    // o->loadVertices(qvertices, qtexcoords, qnormals, qindices, 4, 6);
    
    glUniform1f(stepSimulationWidthLocation, detail);
    glUniform1f(stepSimulationHeightLocation, detail);

    heightB->bind(); // render to height B
    heightA->bindColorTexture(GL_TEXTURE0);

    o->setShader(stepSimulationShader);
    o->draw();
    
    // float pixels[16 * 16 * 3];
    // glReadPixels(0, 0, 16, 16, GL_RGB, GL_FLOAT, pixels);
    // for(int i = 0; i < 16; i++) {
    //     for(int j = 0; j < 16; j++) {
    //         std::cout << "(";
    //         for(int k = 0; k < 3; k++) {
    //             std::cout << pixels[(i * (16 * 3)) + (j * 3) + k] << ",";
    //         }
    //         std::cout << ") ";
    //     }
    //     std::cout << std::endl;
    // }

    heightA->unbindColorTexture();
    heightB->unbind();

    stepSimulationShader->detach();

    swapBuffers();
}

void Heightfield::calculateNormals() {
    normalCalculationShader->attach();
    normalCalculationShader->enableTexture();

    // Object *o = new Object();
    // o->loadVertices(qvertices, qtexcoords, qnormals, qindices, 4, 6);
    
    glUniform1f(normalCalculationWidthLocation, detail);
    glUniform1f(normalCalculationHeightLocation, detail);

    heightB->bind(); // render to height B
    heightA->bindColorTexture(GL_TEXTURE0);

    o->setShader(normalCalculationShader);
    o->draw();
    
    // float pixels[16 * 16 * 3];
    // glReadPixels(0, 0, 16, 16, GL_RGB, GL_FLOAT, pixels);
    // for(int i = 0; i < 16; i++) {
    //     for(int j = 0; j < 16; j++) {
    //         std::cout << "(";
    //         for(int k = 0; k < 3; k++) {
    //             std::cout << pixels[(i * (16 * 3)) + (j * 3) + k] << ",";
    //         }
    //         std::cout << ") ";
    //     }
    //     std::cout << std::endl;
    // }

    heightA->unbindColorTexture();
    heightB->unbind();

    normalCalculationShader->detach();

    swapBuffers();
}

void Heightfield::swapBuffers() {
    // Swap buffers around
    FBO* temp = heightA;
    heightA = heightB;
    heightB = temp;
}

void Heightfield::bindNormalMap() {
    heightA->bindColorTexture(GL_TEXTURE0);
}

void Heightfield::unbindNormalMap() {
    heightA->unbindColorTexture();
}