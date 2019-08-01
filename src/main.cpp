#include <GL/glew.h>
#include <OpenGL/gl3.h>
#include <GLUT/glut.h>
#include <stdio.h>
#include <iostream>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <object.h>
#include <texture.h>
#include <shader.h>
#include <camera.h>
#include <light.h>
#include <fbo.h>

// Shader *shader;
// Shader *simple, *simple_tex;

Shader *firstPass;
Shader *secondPass;

Object *quad;

Camera *camera;
Light *light;
Texture *texture, *normals;
FBO *fbo;

glm::mat4 projectionMatrix;

float vertices[] = {
    -1, 1, 0,
    1, 1, 0,
    1, -1, 0,
    -1, -1, 0
};

float texcoords[] = {
    0, 0,
    1, 0,
    1, 1,
    0, 1
};

float vnormals[] = {
    0, 0, 1,
    0, 0, 1,
    0, 0, 1,
    0, 0, 1
};

int indices[] = {
    0, 1, 3,
    1, 2, 3
};

void keyPressed(unsigned char c, int x, int y) {
    // switch(c) {
    //     case 'w':
    //         camera->move(0, 0, -0.02f);
    //         break;
    //     case 'd':
    //         camera->move(0.02f, 0, 0);
    //         break;
    //     case 'a':
    //         camera->move(-0.02f, 0, 0);
    //         break;
    //     case 's':
    //         camera->move(0, 0, 0.02f);
    //         break;
    //     case 'v':
    //         square->spin();
    //         break;
    // }
    
    glutPostRedisplay();
}
void update() {
    // Begin first pass of rendering (render to FBO)

    // TODO: Bind FBO for drawing

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    firstPass->attach();
    firstPass->loadProjectionMatrix(projectionMatrix);
    firstPass->loadViewMatrix(camera->getViewMatrix());

    quad->setShader(firstPass);
    quad->draw();

    firstPass->detach();

    // TODO: Unbind FBO

    // Begin second pass of rendering (render to screen)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    secondPass->attach();
    secondPass->loadProjectionMatrix(projectionMatrix);
    secondPass->loadViewMatrix(camera->getViewMatrix());
    secondPass->enableTexture();

    // TODO: Bind FBO color texture to GL_TEXTURE0

    quad->setShader(secondPass);
    quad->draw();

    // TODO: Unbind FBO color texture

    secondPass->detach();

    glutSwapBuffers();
}

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(800, 600);
	glutCreateWindow( "OpenGL" );
    
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK) printf("GLEW init failed");

     // get version info
    const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
    const GLubyte* version = glGetString(GL_VERSION); // version as a string
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version supported %s\n", version);
    
    glEnable(GL_DEPTH_TEST); 
    glDepthFunc(GL_LEQUAL);
    glClearColor(0, 0, 0, 0);
    glClearDepth(1.0);
    
    camera = new Camera();
    camera->move(0, 0, 5);

    projectionMatrix = glm::perspective(glm::radians(45.0), 4.0 / 3.0, 0.1f, 1000.0f);

    // Objects for testing FBO
    firstPass = new Shader();
    firstPass->attachShader(GL_VERTEX_SHADER, "shaders/simple_vertex.glsl");
    firstPass->attachShader(GL_FRAGMENT_SHADER, "shaders/simple_fragment.glsl");
    if(!firstPass->compile()) std::cout << "Error compiling first pass shader!" << std::endl;

    secondPass = new Shader();
    secondPass->attachShader(GL_VERTEX_SHADER, "shaders/simple_vertex.glsl");
    secondPass->attachShader(GL_FRAGMENT_SHADER, "shaders/simple_textured_fragment.glsl");
    if(!secondPass->compile()) std::cout << "Error compiling second pass shader!" << std::endl;

    quad = new Object();
    quad->loadVertices(vertices, texCoords, normals, indices, 4, 6);

    glutKeyboardFunc(&keyPressed);
    glutDisplayFunc(update);

    //Start GLUT main loop
	glutMainLoop();

    return 0;
}