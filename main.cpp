#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <GL/glew.h>
#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "ShaderProgram.h"
#include "ObjMesh.h"

#define SCALE_FACTOR 2.0f


int width, height;

// Lamp
GLuint lampProgramId;
GLuint lampVertexBuffer;
GLuint lampIndexBuffer;
GLenum lampPositionBufferId;
GLuint lamp_positions_vbo = 0;
GLuint lamp_texCoords_vbo = 0;
GLuint lamp_normals_vbo = 0;
GLuint lmap_colours_vbo = 0;
unsigned int numVerticesLamp;
float t = 0.0f; // Jumping height

// pixar text
GLuint textProgramId;
GLuint textVertexBuffer;
GLuint textIndexBuffer;
GLenum textPositionBufferId;
GLuint text_positions_vbo = 500;
GLuint text_texCoords_vbo = 500;
GLuint text_normals_vbo = 500;
GLuint text_colours_vbo = 500;
unsigned int numVerticesText;

bool scaling = false;
bool rotating = true;
float xAngle = 0.0f;
float yAngle = 0.0f;
float zAngle = 0.0f;
float lightOffsetY = 0.0f;
glm::vec3 eyePosition(40, 30, 30);
float lastX = std::numeric_limits<float>::infinity();
float lastY = std::numeric_limits<float>::infinity();

float scaleFactor = 1.0f;


static void loadModel(const std::string filename, GLuint& positions_vbo,
                      GLuint& texCoords_vbo, GLuint& normals_vbo,
                      unsigned int& numVertices, GLuint& indexBuffer) {
  ObjMesh mesh;
  mesh.load(filename, true, false);

  numVertices = mesh.getNumIndexedVertices();
  Vector3* vertexPositions = mesh.getIndexedPositions();
  Vector2* vertexTextureCoords = mesh.getIndexedTextureCoords();
  Vector3* vertexNormals = mesh.getIndexedNormals();

  glGenBuffers(1, &positions_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, positions_vbo);
  glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(Vector3), vertexPositions,
               GL_STATIC_DRAW);

  glGenBuffers(1, &texCoords_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, texCoords_vbo);
  glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(Vector2),
               vertexTextureCoords, GL_STATIC_DRAW);

  glGenBuffers(1, &normals_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
  glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(Vector3), vertexNormals,
               GL_STATIC_DRAW);

  unsigned int* indexData = mesh.getTriangleIndices();
  int numTriangles = mesh.getNumTriangles();

  glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * numTriangles * 3,
               indexData, GL_STATIC_DRAW);
}

static void loadModels(void) {
  loadModel("meshes/pixarlamp/pixarlamp.obj", lamp_positions_vbo,
            lamp_texCoords_vbo, lamp_normals_vbo, numVerticesLamp,
            lampIndexBuffer);
  loadModel("meshes/pixarlamp/pixar.obj", text_positions_vbo,
            text_texCoords_vbo, text_normals_vbo, numVerticesText,
            textIndexBuffer);

}

static void loadShader(const std::string vertShaderName,
                       const std::string fragShaderName,
                       GLuint& modelProgramId) {
  ShaderProgram shaderProgram;
  shaderProgram.loadShaders(vertShaderName, fragShaderName);
  modelProgramId = shaderProgram.getProgramId();
}

static void loadShaders() {
  loadShader("shaders/vertex.glsl", "shaders/fragment.glsl", lampProgramId);
  loadShader("shaders/vertex.glsl", "shaders/fragment.glsl", textProgramId);
}


static void update(void) {
    int milliseconds = glutGet(GLUT_ELAPSED_TIME);

    /*
    // move the light position over time along the x-axis, so we can see how it affects the shading
    if (animateLight) {
      float tLight = milliseconds / 1000.0f;
      lightOffsetY = sinf(tLight) * 100;
    }
    */

    t += 0.013;
    // Prevent overflow
    if (t == 360) {
      t = 0;
    }

    glutPostRedisplay();
}


static void renderModel(GLuint programId, GLuint& positions_vbo,
                        GLuint& texCoords_vbo, GLuint& normals_vbo,
                        GLuint& indexBuffer, unsigned int numVertices,
                        glm::vec3 translation, glm::vec3 scale,
                        glm::vec3 rotation, glm::vec3 alphaColour,
                        float shininess) {
  // Draw the lamp model
	glUseProgram(programId);

  // projection matrix - perspective projection
  // FOV:           45°
  // Aspect ratio:  4:3 ratio
  // Z range:       between 0.1 and 100.0
  float aspectRatio = (float)width / (float)height;
  glm::mat4 projection = glm::perspective(glm::radians(30.0f), aspectRatio,
                                          0.1f, 1000.0f);

  // projection matrix - orthographic (non-perspective) projection
  // Note:  These are in world coordinates
  // xMin:          -10
  // xMax:          +10
  // yMin:          -10
  // yMax:          +10
  // zMin:           0
  // zMax:          +100
  //glm::mat4 projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f);

  // view matrix - orient everything around our preferred view
  glm::mat4 view = glm::lookAt(
      eyePosition,
      glm::vec3(0,0,0),    // where to look
      glm::vec3(0,1,0)     // up
  );

  // Earth model matrix: translate, scale, and rotate the model
  glm::vec3 rotationAxis(0,1,0);
  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, translation);
  model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1, 0, 0)); // rotate about the x-axis
  model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0, 1, 0)); // rotate about the y-axis
  model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0, 0, 1)); // rotate about the z-axis
  model = glm::scale(model, scale * scaleFactor);

  // Model-view-projection matrix
  glm::mat4 mvp = projection * view * model;
  GLuint mvpMatrixId = glGetUniformLocation(programId, "u_MVP");
  glUniformMatrix4fv(mvpMatrixId, 1, GL_FALSE, &mvp[0][0]);

  // Earth MV matrix
  glm::mat4 mv = view * model;
  GLuint mvMatrixId = glGetUniformLocation(programId, "u_MV");
  glUniformMatrix4fv(mvMatrixId, 1, GL_FALSE, &mv[0][0]);

  // find the names (ids) of each vertex attribute
  GLint positionAttribId = glGetAttribLocation(programId, "position");
  GLint textureCoordsAttribId = glGetAttribLocation(programId, "textureCoords");
  GLint normalAttribId = glGetAttribLocation(programId, "normal");

  // Set model colour
  GLuint diffuseColourId = glGetUniformLocation(programId, "u_DiffuseColour");
  glUniform4f(diffuseColourId, alphaColour.x, alphaColour.y, alphaColour.z, 1.0);

  // Set position of light for directional light
  GLuint lightDirectionId = glGetUniformLocation(programId, "u_LightDirection");
  glUniform4f(lightDirectionId, 10, -100, -50, 0);

  // Set shininess
  GLuint shininessId = glGetUniformLocation(programId, "u_Shininess");
  glUniform1f(shininessId, shininess);

  // the position of our camera/eye
  GLuint eyePosId = glGetUniformLocation(programId, "u_EyePosition");
  glUniform3f(eyePosId, eyePosition.x, eyePosition.y, eyePosition.z);

  // provide the vertex positions to the shaders
  glBindBuffer(GL_ARRAY_BUFFER, positions_vbo);
  glEnableVertexAttribArray(positionAttribId);
  glVertexAttribPointer(positionAttribId, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

  // provide the vertex texture coordinates to the shaders
  glBindBuffer(GL_ARRAY_BUFFER, texCoords_vbo);
  glEnableVertexAttribArray(textureCoordsAttribId);
  glVertexAttribPointer(textureCoordsAttribId, 2, GL_FLOAT, GL_FALSE, 0,
                        nullptr);

  // provide the vertex normals to the shaders
  glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
  glEnableVertexAttribArray(normalAttribId);
  glVertexAttribPointer(normalAttribId, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// draw the triangles
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glDrawElements(GL_TRIANGLES, numVertices, GL_UNSIGNED_INT, (void*)0);

  // disable the attribute arrays
  glDisableVertexAttribArray(positionAttribId);
  glDisableVertexAttribArray(textureCoordsAttribId);
  glDisableVertexAttribArray(normalAttribId);
}


static void render(void) {
  // Render the lamp
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Turn on depth buffering
  glEnable(GL_DEPTH_TEST);
  //glDepthFunc(GL_LESS);

  // Render "PIXAR"
  renderModel(textProgramId, text_positions_vbo, text_texCoords_vbo,
              text_normals_vbo, textIndexBuffer, numVerticesText,
              glm::vec3(6.0, 0.0, 0.0), glm::vec3(7.5, 7.5, 7.5),
              glm::vec3(-15.0, 8.0, 15.0), glm::vec3(0.2, 0.2, 0.2), 50);

  // Render lamp
  renderModel(lampProgramId, lamp_positions_vbo, lamp_texCoords_vbo,
              lamp_normals_vbo, lampIndexBuffer, numVerticesLamp,
              glm::vec3(-0.4, 9.0 + sinf(t) * 3.0, 8.6), glm::vec3(0.0075, 0.0075 + (cos(t) - sinf(t)) / 1500, 0.0075),
              glm::vec3(0.0, -80.0, 4.0), glm::vec3(0.8, 0.8, 0.8), 100);

  // make the draw buffer to display buffer (i.e. display what we have drawn)
  glutSwapBuffers();
}


static void reshape(int w, int h) {
    glViewport(0, 0, w, h);

    width = w;
    height = h;
}

/*
static void drag(int x, int y) {
  if (!std::isinf(lastX) && !std::isinf(lastY)) {
    float dx = lastX - (float)x;
    float dy = lastY - (float)y;

    if (scaling) {
      float distance = sqrt(dx * dx + dy * dy);

      if (dy > 0.0f) {
          scaleFactor = SCALE_FACTOR / distance;
        } else {
          scaleFactor = distance / SCALE_FACTOR;
      }
    } else if (rotating) {
      xAngle = dx;
      zAngle = dy;
    }
  } else {
    lastX = (float)x;
    lastY = (float)y;
  }
}
*/

/*
static void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
      std::cout << "Starting scale" << std::endl;
      lastX = std::numeric_limits<float>::infinity();
      lastY = std::numeric_limits<float>::infinity();
      scaling = true;
    }
    if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
      std::cout << "Stopping scale" << std::endl;
      lastX = std::numeric_limits<float>::infinity();
      lastY = std::numeric_limits<float>::infinity();
      scaling = false;
    }
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
      std::cout << "Starting rotation" << std::endl;
      lastX = std::numeric_limits<float>::infinity();
      lastY = std::numeric_limits<float>::infinity();
      rotating = true;
    }
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP) {
      std::cout << "Stopping rotation" << std::endl;
      lastX = std::numeric_limits<float>::infinity();
      lastY = std::numeric_limits<float>::infinity();
      rotating = false;
    }
}
*/

/*
static void keyboard(unsigned char key, int x, int y) {
    std::cout << "Key pressed: " << key << std::endl;
    if (key == 'l') {
      animateLight = !animateLight;
    } else if (key == 'r') {
      rotateObject = !rotateObject;
    } else if (key == 'o') {
      moonOrbiting = !moonOrbiting;
    } else if (key == 'p') {
      // Switches between using point light and using directional light
      usePointLight = !usePointLight;
      if (usePointLight) {
        std::cout << "Switching to point light shaders" << std::endl;
        loadPointLightShaders();
      } else {
        std::cout << "Switching to directional light shaders" << std::endl;
        loadDirectionalLightShaders();
      }
    } else if (key == 'w') {
      // Increase shininess
      shininess += SHININESS_STEP;
    } else if (key == 's') {
      // Decrease shininess
        shininess -= SHININESS_STEP;
    }
}
*/

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("CSCI_3090U_FinalProject");
    glutIdleFunc(&update);
    glutDisplayFunc(&render);
    glutReshapeFunc(&reshape);
    //glutMotionFunc(&drag);
    //glutMouseFunc(&mouse);
    //glutKeyboardFunc(&keyboard);

    glewInit();
    if (!GLEW_VERSION_2_0) {
        std::cerr << "OpenGL 2.0 not available" << std::endl;
        return 1;
    }
    std::cout << "Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
		std::cout << "Using OpenGL " << glGetString(GL_VERSION) << std::endl;

    // Load models
    loadModels();

    // Load shaders
    loadShaders();


    glutMainLoop();


    return 0;
}
