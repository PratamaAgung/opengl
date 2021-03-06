#include <bits/stdc++.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext.hpp>
#include "lib/camera.hpp"
#include "lib/shader.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"
#include "lib/smoke_particles.hpp"
#include "lib/rain_particles.hpp"
#include "lib/splash_particles.hpp"
#include <queue>

using namespace glm;
bool firstMouse;
float lastX, lastY;
Camera* camera;
int amountRain = 5000;
int amountSmoke = 1000;
SmokeParticles smoke(amountSmoke, vec3(0.85f, -0.2f, -0.2f), 0.01f);
RainParticles rain(amountRain);
int amountSplash = 5;
bool pause = false;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window, float deltaTime) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera->ProcessKeyboard(UP,  deltaTime);
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera->ProcessKeyboard(DOWN,  deltaTime);
    else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera->ProcessKeyboard(LEFT,  deltaTime);
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera->ProcessKeyboard(RIGHT,  deltaTime);
    else if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) {
        smoke.decGlobalPullX();
        rain.decGlobalPullX();
    } else if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) {
        smoke.incGlobalPullX();
        rain.incGlobalPullX();
    }  else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        pause = true;
    }  else if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        pause = false;
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    camera->ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
    camera->ProcessMouseScroll(yoffset);
}

int compileShader(unsigned int type, const std::string& source){
  unsigned int id = glCreateShader(type);
  const char* src = source.c_str();
  glShaderSource(id, 1, &src, NULL);
  glCompileShader(id);

  int result;
  glGetShaderiv(id, GL_COMPILE_STATUS, &result);
  if(result == GL_FALSE){
    int length;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
    char* msg = (char*) malloc(length);
    glGetShaderInfoLog(id, length, &length, msg);
    printf("Error %s: %s\n", (type == GL_VERTEX_SHADER?"vertex shader":"fragment shader"), msg);
    glDeleteShader(id);
    return 0;
  }
  return id;
}

int createShader(const std::string& vertexShader, const std::string& fragmentShader){
  unsigned int program = glCreateProgram();
  unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader);
  unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);
  glValidateProgram(program);

  glDeleteShader(vs);
  glDeleteShader(fs);

  return program;
}

std::string loadShader(const char * file_path){
  std::string shader_code;
	std::ifstream shader_stream(file_path, std::ios::in);
	if(shader_stream.is_open()){
		std::stringstream sstr;
		sstr << shader_stream.rdbuf();
		shader_code = sstr.str();
		shader_stream.close();
	} else {
		printf("Failed to open %s\n", file_path);
		return 0;
	}
  return shader_code;
}

void createTire(float* vertices, float x, float y, float r, int side, float* tire_color){
  float deg = 360/side;
  float offset = 0.05f;
  for(int i=0; i<(side+2)*6; i+=6){
    if(i == 0){
      vertices[i] = x;
      vertices[i+1] = y;
      vertices[i+2] = 0.0f;
    } else {
      vertices[i] = x + (r * cos((i-1)*deg*M_PI/180.0));
      vertices[i+1] = y + (r * sin((i-1)*deg*M_PI/180.0));
      vertices[i+2] = 0.0f;
    }
    vertices[i+3] = i/((side + 2)*6.0f*1.5f) * sin((i-1)*deg*M_PI/90.0);
    vertices[i+4] = i/((side + 2)*6.0f*1.5f) * sin((i-1)*deg*M_PI/90.0);
    vertices[i+5] = i/((side + 2)*6.0f*1.5f) * sin((i-1)*deg*M_PI/90.0);
  }

  for(int i=(side+2)*6; i<2*((side+2)*6); i+=6){
    if(i == (side+2)*6){
      vertices[i] = x;
      vertices[i+1] = y;
      vertices[i+2] = 0.0f;
    } else {
      vertices[i] = x + ((r-offset) * cos((i-1)*deg*M_PI/180.0));
      vertices[i+1] = y + ((r-offset) * sin((i-1)*deg*M_PI/180.0));
      vertices[i+2] = 0.0f;
    }
    vertices[i+3] = tire_color[0];
    vertices[i+4] = tire_color[1];
    vertices[i+5] = tire_color[2];
  }
}

void createVAOVBO(float* vertices, unsigned int size, unsigned int* vbo, unsigned int* vao) {
    unsigned int vbo2;
    glGenBuffers(1, &vbo2);
    glBindBuffer(GL_ARRAY_BUFFER, vbo2);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

    *vbo = vbo2;

    unsigned int vao2;
    glGenVertexArrays(1, &vao2);
    glBindVertexArray(vao2);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8*sizeof(float)));
    glEnableVertexAttribArray(3);

    *vao = vao2;
}

void createVAOVBOInstance(float* vertices, unsigned int size, glm::mat4* instanceLoc, float* alpha, unsigned int* vao, unsigned int* vbo, unsigned int* vbo_aplha){
    glGenVertexArrays(1, vao);
    glBindVertexArray(*vao);


    glGenBuffers(1, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * amountRain, instanceLoc, GL_STATIC_DRAW);

    glGenBuffers(1, vbo_aplha);
    glBindBuffer(GL_ARRAY_BUFFER, *vbo_aplha);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * amountRain, alpha, GL_STATIC_DRAW);

    unsigned int instanceVBO;
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)3);


    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

    glBindBuffer(GL_ARRAY_BUFFER, *vbo_aplha);
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);

    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);

    glBindVertexArray(0);
}

void createEmptyVAOVBOInstance(float* vertices, int size, unsigned int* vao, unsigned int* vbo, unsigned int* vbo_aplha){
    glGenVertexArrays(1, vao);
    glBindVertexArray(*vao);
    glGenBuffers(1, vbo);
    glGenBuffers(1, vbo_aplha);

    unsigned int instanceVBO;
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)3);


    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

    glBindBuffer(GL_ARRAY_BUFFER, *vbo_aplha);
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);

    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);

    glBindVertexArray(0);
}

void rotate(Shader shader, float x, float y, float z) {
    glm::mat4 transformMatrix;
    transformMatrix = glm::translate(transformMatrix, glm::vec3(x, y, z));
    transformMatrix = glm::rotate(transformMatrix, (float) glfwGetTime() * 5.0f, glm::vec3(0.0f, 0.0f, 1.0f));
    shader.use();
    shader.setMat4("model", transformMatrix);
}

void buildTexture(unsigned int *texture, const char * path){
  glGenTextures(1, &(*texture));
  glBindTexture(GL_TEXTURE_2D, *texture);
  // set the texture wrapping/filtering options (on the currently bound texture object)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // load and generate the texture
  int width, height, nrChannels;
  unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
  if (data){
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
      glGenerateMipmap(GL_TEXTURE_2D);
  } else {
      std::cout << "Failed to load texture" << std::endl;
  }
  stbi_image_free(data);
}

void createTire3d(float* vertices, float x, float y, float z, float r, int side, float* tire_color, float width){
  float deg = 360/side;
  for(int i=0; i<(side+2)*11*2; i+=11){
    if((i == 0) || (i == (side+2)*11)){
      vertices[i] = x;
      vertices[i+1] = y;
      if(i < (side+2)*11){
        vertices[i+2] = z;
      } else {
        vertices[i+2] = (z<0.0f?z+width:z-width);
      }
      vertices[i+6] = 0.5;
      vertices[i+7] = 0.5;
    } else {
      vertices[i] = x + (r * cos((i-1)*deg*M_PI/180.0));
      vertices[i+1] = y + (r * sin((i-1)*deg*M_PI/180.0));
      if(i < (side+2)*11){
        vertices[i+2] = z;
      } else {
        vertices[i+2] = (z<0.0f?z+width:z-width);
      }
      vertices[i+6] = 0.5 + (0.5 * cos((i-1)*deg*M_PI/180.0));
      vertices[i+7] = 0.5 + (0.5 * sin((i-1)*deg*M_PI/180.0));
    }
    vertices[i+3] = tire_color[0];
    vertices[i+4] = tire_color[1];
    vertices[i+5] = tire_color[2];
    vertices[i+8] = 0.0f;
    vertices[i+9] = 0.0f;
    if (i < (side + 2) * 11){
      vertices[i+10] = 1.0f;
    } else {
      vertices[i+10] = -1.0f;
    }
  }

  int j = (side+2)*11*2;
  for(int i=11; i<(side*11); i+=11, j+=55){
    for(int k=0; k<55; k++){
      if(k<11){
        vertices[j+k] = vertices[i+k];
      } else if (k<22){
        vertices[j+k] = vertices[i+k+((side+2)*11)];
      } else if (k<33){
        vertices[j+k] = vertices[i+11+k+((side+2)*11)];
      } else if (k<44) {
        vertices[j+k] = vertices[i+11+k];
      } else {
        vertices[j+k] = vertices[i+k];
      }
    }
  }
}

int main(int argc, char** argv) {
    if (!glfwInit()) {
        fprintf(stderr, "failed to initialize glfw\n");
        return -1;
    }

    firstMouse = true;

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window;
    window = glfwCreateWindow(1000, 1000, "Car", NULL, NULL);
    if(!window){
        fprintf( stderr, "Failed to open GLFW window.\n" );
        glfwTerminate();
        return -1;
    }
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwMakeContextCurrent(window);
    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    float ground_vertices[] = {
      // 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
      100.0f, -0.40941000000000005f, 100.0f, 0.0f, 0.0f, 0.0f, 100.0f, 100.0f, 0.0f, 1.0f, 0.0f,
      -100.0f, -0.40941000000000005f, 100.0f, 0.0f, 0.0f, 0.0f, 0.0f, 100.0f, 0.0f, 1.0f, 0.0f,
      -100.0f, -0.40941000000000005f, -100.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
      100.0f, -0.40941000000000005f, -100.0f, 0.0f, 0.0f, 0.0f, 100.0f, 0.0f, 0.0f, 1.0f, 0.0f,
      100.0f, -0.40941000000000005f, 100.0f, 0.0f, 0.0f, 0.0f, 100.0f, 100.0f, 0.0f, 1.0f, 0.0f,
      // 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
    };

    float car_vertices[] = {
        // left
        // 0.0f, 0.0f, 0.4f, 1.0f, 0.157f, 0.0f, 0.5f, 0.0f,
        0.0f, -0.32941000000000004f, 0.4f, 0.62745098039f, 0.157f, 1.0f,  0.0f, 0.335295f, 0.0f, -0.32941000000000004f, 1.0f,
        // back  0.0f, 0.0f,
        0.0f, -0.32941000000000004f, 0.4f, 0.62745098039f, 0.157f, 1.0f,  0.0f, 0.335295f,0.0f, -0.32941000000000004f, 1.0f,
        0.7348599999999999f, -0.32941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.8674299999999999f, 0.335295f, 0.7348599999999999f, -0.32941000000000004f, 1.0f,
        0.8577449999999999f, -0.22941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.9288725f, 0.385295f, 0.8577449999999999f, -0.22941000000000004f, 1.0f,
        0.8777449999999999f, -0.10218999999999994f, 0.4f, 1.0f, 0.157f, 0.0f,  0.9388725f, 0.44890500000000005f, 0.8777449999999999f, -0.10218999999999994f, 0.4f,
        0.8677449999999999f, 0.05218999999999994f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.9338724999999999f, 0.526095f, 0.8677449999999999f, 0.05218999999999994f, 1.0f,
        0.81141f, 0.07588999999999999f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.905705f, 0.537945f, 0.81141f, 0.07588999999999999f, 1.0f,
        0.6579649999999999f, 0.10588999999999999f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.8289825f, 0.552945f, 0.6579649999999999f, 0.10588999999999999f, 1.0f,
        // top  0.0f, 0.0f,
        0.3579649999999999f, 0.20361999999999994f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.6789824999999999f, 0.60181f, 0.3579649999999999f, 0.20361999999999994f, 1.0f,
        0.0579649999999999f, 0.20361999999999994f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5289824999999999f, 0.60181f, 0.0579649999999999f, 0.20361999999999994f, 1.0f,
        -0.22641000000000006f, 0.06930999999999997f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.386795f, 0.534655f, -0.22641000000000006f, 0.06930999999999997f, 1.0f,
        // front  0.0f, 0.0f,
        -0.7323350000000001f, -0.036025000000000064f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.13383249999999997f, 0.48198749999999996f, -0.7323350000000001f, -0.036025000000000064f, 1.0f,
        -0.7523350000000001f, -0.136025000000000064f, 0.4f, 1.0f, 0.157f, 0.0f,  0.12383249999999996f, 0.43198749999999997f,-0.7523350000000001f, -0.136025000000000064f, 1.0f,
        -0.7323350000000001f, -0.29941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.13383249999999997f, 0.35029499999999997f, -0.7323350000000001f, -0.29941000000000004f, 1.0f,
        -0.6523350000000001f, -0.32941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.17383249999999995f, 0.335295f, -0.6523350000000001f, -0.32941000000000004f, 1.0f,
        0.0f, -0.32941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.335295f, 0.0f, -0.32941000000000004f, 1.0f,

        // right
        // 0.0f, 0.0f, 0.4f, 1.0f, 0.157f, 0.0f, 0.5f, 0.0f,
        0.0f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 1.0f,  0.0f, 0.335295f, 0.0f, -0.32941000000000004f, -1.0f,
        // back  0.0f, 0.0f,
        0.0f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 1.0f,  0.0f, 0.335295f, 0.0f, -0.32941000000000004f, -1.0f,
        0.7348599999999999f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.8674299999999999f, 0.335295f, 0.7348599999999999f, -0.32941000000000004f, -1.0f,
        0.8577449999999999f, -0.22941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.9288725f, 0.385295f, 0.8577449999999999f, -0.22941000000000004f, -1.0f,
        0.8777449999999999f, -0.10218999999999994f, -0.4f, 1.0f, 0.157f, 0.0f,  0.9388725f, 0.44890500000000005f, 0.8777449999999999f, -0.10218999999999994f, -1.0f,
        0.8677449999999999f, 0.05218999999999994f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.9338724999999999f, 0.526095f, 0.8677449999999999f, 0.05218999999999994f, -1.0f,
        0.81141f, 0.07588999999999999f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.905705f, 0.537945f, 0.81141f, 0.07588999999999999f, -1.0f,
        0.6579649999999999f, 0.10588999999999999f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.8289825f, 0.552945f, 0.6579649999999999f, 0.10588999999999999f, -1.0f,
        // top  0.0f, 0.0f,
        0.3579649999999999f, 0.20361999999999994f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.6789824999999999f, 0.60181f, 0.3579649999999999f, 0.20361999999999994f, -1.0f,
        0.0579649999999999f, 0.20361999999999994f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5289824999999999f, 0.60181f, 0.0579649999999999f, 0.20361999999999994f, -1.0f,
        -0.22641000000000006f, 0.06930999999999997f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.386795f, 0.534655f, -0.22641000000000006f, 0.06930999999999997f, -1.0f,
        // front  0.0f, 0.0f,
        -0.7323350000000001f, -0.036025000000000064f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.13383249999999997f, 0.48198749999999996f, -0.7323350000000001f, -0.036025000000000064f, -1.0f,
        -0.7523350000000001f, -0.136025000000000064f, -0.4f, 1.0f, 0.157f, 0.0f,  0.12383249999999996f, 0.43198749999999997f, -0.7523350000000001f, -0.136025000000000064f, -1.0f,
        -0.7323350000000001f, -0.29941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.13383249999999997f, 0.35029499999999997f, -0.7323350000000001f, -0.29941000000000004f, -1.0f,
        -0.6523350000000001f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.17383249999999995f, 0.335295f, -0.6523350000000001f, -0.32941000000000004f, -1.0f,
        0.0f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.335295f, 0.0f, -0.32941000000000004f, -1.0f,

        // TOP
        // bottom
        0.0f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f, 0.0f, -1.0f, -0.4f,
        0.7348599999999999f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f, 0.7348599999999999f, -1.0f, -0.4f,
        0.7348599999999999f, -0.32941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.5f, 0.7348599999999999f, -1.0f, 0.4f,
        0.0f, -0.32941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f, 0.0f, -1.0f, 0.4f,
        0.0f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f, 0.0f, -1.0f, -0.4f,

        // bottom (corner)
        0.7348599999999999f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f, 0.08f, -0.098308f, 0.0f,
        0.8577449999999999f, -0.22941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f, 0.08f, -0.098308f, 0.0f,
        0.8577449999999999f, -0.22941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.5f, 0.08f, -0.098308f, 0.0f,
        0.7348599999999999f, -0.32941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f, 0.08f, -0.098308f, 0.0f,
        0.7348599999999999f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f, 0.08f, -0.098308f, 0.0f,

        // back
        0.8577449999999999f, -0.22941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f, 0.101776f, 0.016f, 0.0f,
        0.8777449999999999f, -0.10218999999999994f, -0.4f, 1.0f, 0.157f, 0.0f,  0.0f, 0.0f, 0.101776f, 0.016f, 0.0f,
        0.8777449999999999f, -0.10218999999999994f, 0.4f, 1.0f, 0.157f, 0.0f,  0.0f, 0.5f, 0.101776f, 0.016f, 0.0f,
        0.8577449999999999f, -0.22941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f, 0.101776f, 0.016f, 0.0f,
        0.8577449999999999f, -0.22941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f, 0.101776f, 0.016f, 0.0f,

        // back (corner) logo
        0.8777449999999999f, -0.10218999999999994f, -0.4f, 1.0f, 0.157f, 0.0f,  1.0f, 0.675f, 0.123504f, -0.00799999f, 0.0f,
        0.8677449999999999f, 0.05218999999999994f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  1.0f, 0.325f, 0.123504f, -0.00799999f, 0.0f,
        0.8677449999999999f, 0.05218999999999994f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.325f, 0.123504f, -0.00799999f, 0.0f,
        0.8777449999999999f, -0.10218999999999994f, 0.4f, 1.0f, 0.157f, 0.0f,  0.0f, 0.675f, 0.123504f, -0.00799999f, 0.0f,
        0.8777449999999999f, -0.10218999999999994f, -0.4f, 1.0f, 0.157f, 0.0f,  1.0f, 0.675f,0.123504f, -0.00799999f, 0.0f,

        // back (top)
        0.8677449999999999f, 0.05218999999999994f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f, 0.01896f, 0.045068f, 0.0f,
        0.81141f, 0.07588999999999999f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f, 0.01896f, 0.045068f, 0.0f,
        0.81141f, 0.07588999999999999f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.5f, 0.01896f, 0.045068f, 0.0f,
        0.8677449999999999f, 0.05218999999999994f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f, 0.01896f, 0.045068f, 0.0f,
        0.8677449999999999f, 0.05218999999999994f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f, 0.01896f, 0.045068f, 0.0f,

        // back (window)
        0.81141f, 0.07588999999999999f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f, 0.024f, 0.122756f, 0.0f,
        0.6579649999999999f, 0.10588999999999999f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f, 0.024f, 0.122756f, 0.0f,
        0.6579649999999999f, 0.10588999999999999f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.5f, 0.024f, 0.122756f, 0.0f,
        0.81141f, 0.07588999999999999f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f, 0.024f, 0.122756f, 0.0f,
        0.81141f, 0.07588999999999999f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f, 0.024f, 0.122756f, 0.0f,

        // back (window 2) kaca belakang
        0.6579649999999999f, 0.10588999999999999f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f, 0.078184f, 0.24f, 0.0f,
        0.3579649999999999f, 0.20361999999999994f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f, 0.078184f, 0.24f, 0.0f,
        0.3579649999999999f, 0.20361999999999994f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.5f, 0.078184f, 0.24f, 0.0f,
        0.6579649999999999f, 0.10588999999999999f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f, 0.078184f, 0.24f, 0.0f,
        0.6579649999999999f, 0.10588999999999999f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f, 0.078184f, 0.24f, 0.0f,

        // top
        0.3579649999999999f, 0.20361999999999994f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f, 0.0f, 0.24f, 0.0f,
        0.0579649999999999f, 0.20361999999999994f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f, 0.0f, 0.24f, 0.0f,
        0.0579649999999999f, 0.20361999999999994f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.5f, 0.0f, 0.24f, 0.0f,
        0.3579649999999999f, 0.20361999999999994f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f, 0.0f, 0.24f, 0.0f,
        0.3579649999999999f, 0.20361999999999994f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f, 0.0f, 0.24f, 0.0f,

        // top front kaca depan
        0.0579649999999999f, 0.20361999999999994f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f, -0.107448f, 0.2275f, 0.0f,
        -0.22641000000000006f, 0.06930999999999997f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f, -0.107448f, 0.2275f, 0.0f,
        -0.22641000000000006f, 0.06930999999999997f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.5f, -0.107448f, 0.2275f, 0.0f,
        0.0579649999999999f, 0.20361999999999994f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f, -0.107448f, 0.2275f, 0.0f,
        0.0579649999999999f, 0.20361999999999994f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f, -0.107448f, 0.2275f, 0.0f,

        // front
        -0.22641000000000006f, 0.06930999999999997f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 1.0f, -0.084268f, 0.40474f, 0.0f,
        -0.7323350000000001f, -0.036025000000000064f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f, -0.084268f, 0.40474f, 0.0f,
        -0.7323350000000001f, -0.036025000000000064f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  1.0f, 0.0f, -0.084268f, 0.40474f, 0.0f,
        -0.22641000000000006f, 0.06930999999999997f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  1.0f, 1.0f, -0.084268f, 0.40474f, 0.0f,
        -0.22641000000000006f, 0.06930999999999997f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 1.0f, -0.084268f, 0.40474f, 0.0f,

        // front logo
        -0.7323350000000001f, -0.036025000000000064f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.325f, -1.0f, 0.1f, 0.0f,
        -0.7523350000000001f, -0.136025000000000064f, -0.4f, 1.0f, 0.157f, 0.0f,  0.0f, 0.675f, -1.0f, 0.1f, 0.0f,
        -0.7523350000000001f, -0.136025000000000064f, 0.4f, 1.0f, 0.157f, 0.0f,  1.0f, 0.675f, -1.0f, 0.1f, 0.0f,
        -0.7323350000000001f, -0.036025000000000064f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  1.0f, 0.325f, -1.0f, 0.1f, 0.0f,
        -0.7323350000000001f, -0.036025000000000064f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.325f, -1.0f, 0.1f, 0.0f,

        // front
        -0.7523350000000001f, -0.136025000000000064f, -0.4f, 1.0f, 0.157f, 0.0f,  0.5f, 0.0f, -0.130708f, 0.016f, 0.0f,
        -0.7323350000000001f, -0.29941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f, -0.130708f, 0.016f, 0.0f,
        -0.7323350000000001f, -0.29941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.5f, -0.130708f, 0.016f, 0.0f,
        -0.7523350000000001f, -0.136025000000000064f, 0.4f, 1.0f, 0.157f, 0.0f,  0.0f, 0.0f, -0.130708f, 0.016f, 0.0f,
        -0.7523350000000001f, -0.136025000000000064f, -0.4f, 1.0f, 0.157f, 0.0f,  0.5f, 0.0f, -0.130708f, 0.016f, 0.0f,

        // front
        -0.7323350000000001f, -0.29941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f, -0.024f, -0.064f, 0.0f,
        -0.6523350000000001f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f, -0.024f, -0.064f, 0.0f,
        -0.6523350000000001f, -0.32941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f, -0.024f, -0.064f, 0.0f,
        -0.7323350000000001f, -0.29941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f, -0.024f, -0.064f, 0.0f,
        -0.7323350000000001f, -0.29941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f, -0.024f, -0.064f, 0.0f,

        // front bottom
        -0.6523350000000001f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f, 0.0f, -0.521868f, 0.0f,
        0.0f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f, 0.0f, -0.521868f, 0.0f,
        0.0f, -0.32941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.5f, 0.0f, -0.521868f, 0.0f,
        -0.6523350000000001f, -0.32941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f, 0.0f, -0.521868f, 0.0f,
        -0.6523350000000001f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f, 0.0f, -0.521868f, 0.0f,

        // window left
        0.0f, 0.1f, 0.401f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.3579649999999999f, 0.01930999999999997f, 0.401f, 0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.3579649999999999f, 0.15361999999999994f, 0.401f, 0.8f, 0.8f, 0.8f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.3279649999999999f, 0.18361999999999994f, 0.401f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.0579649999999999f, 0.18361999999999994f, 0.401f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.18641000000000006f, 0.06930999999999997f, 0.401f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.20641000000000006f, 0.02930999999999997f, 0.401f, 0.8f, 0.8f, 0.8f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.10641000000000006f, 0.00030999999999997f, 0.401f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.2579649999999999f, 0.00030999999999997f, 0.401f, 0.8f, 0.8f, 0.8f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.3579649999999999f, 0.01930999999999997f, 0.401f, 0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,

        0.4579649999999999f, 0.1f, 0.401f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.6079649999999999f, 0.08888999999999999f, 0.401f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.4079649999999999f, 0.15361999999999994f, 0.401f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.3879649999999999f, 0.15361999999999994f, 0.401f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.3879649999999999f, 0.01930999999999997f, 0.401f, 0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.6079649999999999f, 0.06588999999999999f, 0.401f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.6079649999999999f, 0.08888999999999999f, 0.401f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,

        // window right
        0.0f, 0.1f, -0.401f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        0.3579649999999999f, 0.01930999999999997f, -0.401f, 0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        0.3579649999999999f, 0.15361999999999994f, -0.401f, 0.8f, 0.8f, 0.8f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        0.3279649999999999f, 0.18361999999999994f, -0.401f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        0.0579649999999999f, 0.18361999999999994f, -0.401f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        -0.18641000000000006f, 0.06930999999999997f, -0.401f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        -0.20641000000000006f, 0.02930999999999997f, -0.401f, 0.8f, 0.8f, 0.8f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        -0.10641000000000006f, 0.00030999999999997f, -0.401f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        0.2579649999999999f, 0.00030999999999997f, -0.401f, 0.8f, 0.8f, 0.8f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        0.3579649999999999f, 0.01930999999999997f, -0.401f, 0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,

        0.4579649999999999f, 0.1f, -0.401f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        0.6079649999999999f, 0.08888999999999999f, -0.401f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        0.4079649999999999f, 0.15361999999999994f, -0.401f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        0.3879649999999999f, 0.15361999999999994f, -0.401f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        0.3879649999999999f, 0.01930999999999997f, -0.401f, 0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        0.6079649999999999f, 0.06588999999999999f, -0.401f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        0.6079649999999999f, 0.08888999999999999f, -0.401f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,

        // kaca belakang
        0.6079649999999999f, 0.12317833333333333f, -0.38f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0495165f, 0.152f, 0.0f,
        0.4079649999999999f, 0.18833166666666662f, -0.38f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0495165f, 0.152f, 0.0f,
        0.4079649999999999f, 0.18833166666666662f, 0.38f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0495165f, 0.152f, 0.0f,
        0.6079649999999999f, 0.12317833333333333f, 0.38f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0495165f, 0.152f, 0.0f,
        0.6079649999999999f, 0.12317833333333333f, -0.38f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0495165f, 0.152f, 0.0f,

        // kaca depan
        0.0234776588365224f, 0.18833166666666662f,  -0.38f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, -0.0904565f, 0.189915f, 0.0f,
        -0.22641000000000006f, 0.06930999999999997f, -0.38f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -0.0904565f, 0.189915f, 0.0f,
        -0.22641000000000006f, 0.06930999999999997f, 0.38f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, -0.0904565f, 0.189915f, 0.0f,
        0.0234776588365224f, 0.18833166666666662f,  0.38f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -0.0904565f, 0.189915f, 0.0f,
        0.0234776588365224f, 0.18833166666666662f,  -0.38f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, -0.0904565f, 0.189915f, 0.0f,

        // lampu kanan
        -0.7393350000000001f, -0.066025000000000064f, 0.225f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.325f, -0.0118f, 0.00236f, 0.0f,
        -0.7513350000000001f, -0.126025000000000064f, 0.2f, 1.0f, 0.157f, 0.0f,  0.0f, 0.675f, -0.0118f, 0.00236f, 0.0f,
        -0.7521350000000001f, -0.130025000000000064f, 0.395f, 1.0f, 0.157f, 0.0f,  1.0f, 0.675f, -0.0118f, 0.00236f, 0.0f,
        -0.7353350000000001f, -0.046025000000000064f, 0.395f, 0.62745098039f, 0.157f, 0.0f,  1.0f, 0.325f, -0.0118f, 0.00236f, 0.0f,
        -0.7393350000000001f, -0.066025000000000064f, 0.225f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.325f, -0.0118f, 0.00236f, 0.0f,

        // lampu kiri
        -0.7393350000000001f, -0.066025000000000064f, -0.225f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.325f, -0.0118f, 0.00236f, 0.0f,
        -0.7513350000000001f, -0.126025000000000064f, -0.2f, 1.0f, 0.157f, 0.0f,  0.0f, 0.675f, -0.0118f, 0.00236f, 0.0f,
        -0.7521350000000001f, -0.130025000000000064f, -0.395f, 1.0f, 0.157f, 0.0f,  1.0f, 0.675f, -0.0118f, 0.00236f, 0.0f,
        -0.7353350000000001f, -0.046025000000000064f, -0.395f, 0.62745098039f, 0.157f, 0.0f,  1.0f, 0.325f, -0.0118f, 0.00236f, 0.0f,
        -0.7393350000000001f, -0.066025000000000064f, -0.225f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.325f, -0.0118f, 0.00236f, 0.0f,

        // lampu belakang
        0.8765494954009585f, -0.08218999999999994f, 0.25f, 0.0f, 0.0f, 0.0f,  0.0f, 0.675f, 0.020876f, 0.00135225f, 0.0f,
        0.8697882568985619f, 0.02218999999999994f, 0.2f, 0.0f, 0.0f, 0.0f,  0.0f, 0.325f, 0.020876f, 0.00135225f, 0.0f,
        0.8697882568985619f, 0.02218999999999994f, 0.401f, 0.0f, 0.0f, 0.0f,  1.0f, 0.325f, 0.020876f, 0.00135225f, 0.0f,
        0.8765494954009585f, -0.08218999999999994f, 0.401f, 0.0f, 0.0f, 0.0f,  1.0f, 0.675f, 0.020876f, 0.00135225f, 0.0f,
        0.8765494954009585f, -0.08218999999999994f, 0.25f, 0.0f, 0.0f, 0.0f,  0.0f, 0.675f, 0.020876f, 0.00135225f, 0.0f,

        // lampu belakang
        0.8765494954009585f, -0.08218999999999994f, -0.25f, 0.0f, 0.0f, 0.0f,  0.0f, 0.675f, 0.020876f, 0.00135225f, 0.0f,
        0.8697882568985619f, 0.02218999999999994f, -0.2f, 0.0f, 0.0f, 0.0f,  0.0f, 0.325f, 0.020876f, 0.00135225f, 0.0f,
        0.8697882568985619f, 0.02218999999999994f, -0.401f, 0.0f, 0.0f, 0.0f,  1.0f, 0.325f, 0.020876f, 0.00135225f, 0.0f,
        0.8765494954009585f, -0.08218999999999994f, -0.401f, 0.0f, 0.0f, 0.0f,  1.0f, 0.675f, 0.020876f, 0.00135225f, 0.0f,
        0.8765494954009585f, -0.08218999999999994f, -0.25f, 0.0f, 0.0f, 0.0f,  0.0f, 0.675f, 0.020876f, 0.00135225f, 0.0f,

        // lampu belakang samping
        0.8697882568985619f, 0.02218999999999994f, 0.401f, 0.0f, 0.0f, 0.0f,  1.0f, 0.325f,  0.0f, 0.0f, 1.0f,
        0.8765494954009585f, -0.08218999999999994f, 0.401f, 0.0f, 0.0f, 0.0f,  1.0f, 0.675f,  0.0f, 0.0f, 1.0f,
        0.8265494954009585f, -0.08218999999999994f, 0.401f, 0.0f, 0.0f, 0.0f,  1.0f, 0.675f,  0.0f, 0.0f, 1.0f,
        0.8097882568985619f, 0.02218999999999994f, 0.401f, 0.0f, 0.0f, 0.0f,  1.0f, 0.325f,  0.0f, 0.0f, 1.0f,
        0.8697882568985619f, 0.02218999999999994f, 0.401f, 0.0f, 0.0f, 0.0f,  1.0f, 0.325f,  0.0f, 0.0f, 1.0f,

        0.8697882568985619f, 0.02218999999999994f, -0.401f, 0.0f, 0.0f, 0.0f,  1.0f, 0.325f,  0.0f, 0.0f, -1.0f,
        0.8765494954009585f, -0.08218999999999994f, -0.401f, 0.0f, 0.0f, 0.0f,  1.0f, 0.675f,  0.0f, 0.0f, -1.0f,
        0.8265494954009585f, -0.08218999999999994f, -0.401f, 0.0f, 0.0f, 0.0f,  1.0f, 0.675f,  0.0f, 0.0f, -1.0f,
        0.8097882568985619f, 0.02218999999999994f, -0.401f, 0.0f, 0.0f, 0.0f,  1.0f, 0.325f,  0.0f, 0.0f, -1.0f,
        0.8697882568985619f, 0.02218999999999994f, -0.401f, 0.0f, 0.0f, 0.0f,  1.0f, 0.325f,  0.0f, 0.0f, -1.0f,

    };

    float teardrop_vertices[] = {
      0.0f, 0.0f, 0.0f, 0.4f, 0.6f, 0.6f,
      0.0f, 2.0f, 1.0f, 0.4f, 0.6f, 0.6f,
      -0.6f, 2.0f, 0.6f, 0.4f, 0.6f, 0.6f,

      0.0f, 0.0f, 0.0f, -2.4f, 0.72f, 0.0f,
      -0.6f, 2.0f, 0.6f, -2.4f, 0.72f, 0.0f,
      -0.6f, 2.0f, -0.6f, -2.4f, 0.72f, 0.0f,

      0.0f, 0.0f, 0.0f, -0.8f, 0.6f, -1.2f,
      -0.6f, 2.0f, -0.6f, -0.8f, 0.6f, -1.2f,
      0.0f, 2.0f, -1.0f, -0.8f, 0.6f, -1.2f,

      0.0f, 0.0f, 0.0f, 0.8f, 0.6f, -1.2f,
      0.0f, 2.0f, -1.0f, 0.8f, 0.6f, -1.2f,
      0.6f, 2.0f, -0.6f, 0.8f, 0.6f, -1.2f,

      0.0f, 0.0f, 0.0f, 2.4f, 0.72f, 0.0f,
      0.6f, 2.0f, -0.6f, 2.4f, 0.72f, 0.0f,
      0.6f, 2.0f, 0.6f, 2.4f, 0.72f, 0.0f,

      0.0f, 0.0f, 0.0f, 0.8f, 0.6f, 1.2f,
      0.6f, 2.0f, 0.6f, 0.8f, 0.6f, 1.2f,
      0.0f, 2.0f, 1.0f, 0.8f, 0.6f, 1.2f,

      0.0f, 3.0f, 0.0f, 0.4f, 0.6f, -0.6f,
      0.0f, 2.0f, 1.0f, 0.4f, 0.6f, -0.6f,
      -0.6f, 2.0f, 0.6f, 0.4f, 0.6f, -0.6f,

      0.0f, 3.0f, 0.0f, 1.2f, 0.72f, 0.0f,
      -0.6f, 2.0f, 0.6f, 1.2f, 0.72f, 0.0f,
      -0.6f, 2.0f, -0.6f, 1.2f, 0.72f, 0.0f,

      0.0f, 3.0f, 0.0f, 0.4f, 0.6f, 0.6f,
      -0.6f, 2.0f, -0.6f, 0.4f, 0.6f, 0.6f,
      0.0f, 2.0f, -1.0f, 0.4f, 0.6f, 0.6f,

      0.0f, 3.0f, 0.0f, -0.4f, 0.6f, 0.6f,
      0.0f, 2.0f, -1.0f, -0.4f, 0.6f, 0.6f,
      0.6f, 2.0f, -0.6f, -0.4f, 0.6f, 0.6f,

      0.0f, 3.0f, 0.0f, -1.2f, 0.72f, 0.0f,
      0.6f, 2.0f, -0.6f, -1.2f, 0.72f, 0.0f,
      0.6f, 2.0f, 0.6f, -1.2f, 0.72f, 0.0f,

      0.0f, 3.0f, 0.0f, -0.4f, 0.6f, -0.6f,
      0.6f, 2.0f, 0.6f, -0.4f, 0.6f, -0.6f,
      0.0f, 2.0f, 1.0f, -0.4f, 0.6f, -0.6f,
    };

    // posX, posY, posZ, norX, norY, norZ
    float smoke_vertices[] = {
      0.0f, 0.0f, 0.0f, -0.4f, -0.6f, -0.6f,
      0.0f, 1.0f, 1.0f, -0.4f, -0.6f, -0.6f,
      -0.6f, 0.6f, 1.0f, -0.4f, -0.6f, -0.6f,

      0.0f, 0.0f, 0.0f, -1.2f, 0.0f, -0.72f,
      -0.6f, 0.6f, 1.0f, -1.2f, 0.0f, -0.72f,
      -0.6f, -0.6f, 1.0f, -1.2f, 0.0f, -0.72f,

      0.0f, 0.0f, 0.0f, -0.4f, 0.6f, -0.6f,
      -0.6f, -0.6f, 1.0f, -0.4f, 0.6f, -0.6f,
      0.0f, -1.0f, 1.0f, -0.4f, 0.6f, -0.6f,

      0.0f, 0.0f, 0.0f, 0.4f, 0.6f, -0.6f,
      0.0f, -1.0f, 1.0f, 0.4f, 0.6f, -0.6f,
      0.6f, -0.6f, 1.0f, 0.4f, 0.6f, -0.6f,

      0.0f, 0.0f, 0.0f, 1.2f, 0.0f, -0.72f,
      0.6f, -0.6f, 1.0f, 1.2f, 0.0f, -0.72f,
      0.6f, 0.6f, 1.0f, 1.2f, 0.0f, -0.72f,

      0.0f, 0.0f, 0.0f, 0.4f, -0.6f, -0.6f,
      0.6f, 0.6f, 1.0f, 0.4f, -0.6f, -0.6f,
      0.0f, 1.0f, 1.0f, 0.4f, -0.6f, -0.6f,

      0.0f, 0.0f, 2.0f, 0.4f, 0.6f, -0.6f,
      0.0f, 1.0f, 1.0f, 0.4f, 0.6f, -0.6f,
      -0.6f, 0.6f, 1.0f, 0.4f, 0.6f, -0.6f,

      0.0f, 0.0f, 2.0f, 1.2f, 0.0f, -0.72f,
      -0.6f, 0.6f, 1.0f, 1.2f, 0.0f, -0.72f,
      -0.6f, -0.6f, 1.0f, 1.2f, 0.0f, -0.72f,

      0.0f, 0.0f, 2.0f, 0.4f, -0.6f, -0.6f,
      -0.6f, -0.6f, 1.0f, 0.4f, -0.6f, -0.6f,
      0.0f, -1.0f, 1.0f, 0.4f, -0.6f, -0.6f,

      0.0f, 0.0f, 2.0f, -0.4f, -0.6f, -0.6f,
      0.0f, -1.0f, 1.0f, -0.4f, -0.6f, -0.6f,
      0.6f, -0.6f, 1.0f, -0.4f, -0.6f, -0.6f,

      0.0f, 0.0f, 2.0f, -1.2f, -0.0f, -0.72f,
      0.6f, -0.6f, 1.0f, -1.2f, -0.0f, -0.72f,
      0.6f, 0.6f, 1.0f, -1.2f, -0.0f, -0.72f,

      0.0f, 0.0f, 2.0f, -0.4f, 0.6f, -0.6f,
      0.6f, 0.6f, 1.0f, -0.4f, 0.6f, -0.6f,
      0.0f, 1.0f, 1.0f, -0.4f, 0.6f, -0.6f,
    };


    float splash_vertices[] = {
      0.0f, 0.0f, 0.0f, -0.4f, -0.6f, -0.6f,
      0.0f, 1.0f, 1.0f, -0.4f, -0.6f, -0.6f,
      -0.6f, 0.6f, 1.0f, -0.4f, -0.6f, -0.6f,

      0.0f, 0.0f, 0.0f, -1.2f, 0.0f, -0.72f,
      -0.6f, 0.6f, 1.0f, -1.2f, 0.0f, -0.72f,
      -0.6f, -0.6f, 1.0f, -1.2f, 0.0f, -0.72f,

      0.0f, 0.0f, 0.0f, -0.4f, 0.6f, -0.6f,
      -0.6f, -0.6f, 1.0f, -0.4f, 0.6f, -0.6f,
      0.0f, -1.0f, 1.0f, -0.4f, 0.6f, -0.6f,

      0.0f, 0.0f, 0.0f, 0.4f, 0.6f, -0.6f,
      0.0f, -1.0f, 1.0f, 0.4f, 0.6f, -0.6f,
      0.6f, -0.6f, 1.0f, 0.4f, 0.6f, -0.6f,

      0.0f, 0.0f, 0.0f, 1.2f, 0.0f, -0.72f,
      0.6f, -0.6f, 1.0f, 1.2f, 0.0f, -0.72f,
      0.6f, 0.6f, 1.0f, 1.2f, 0.0f, -0.72f,

      0.0f, 0.0f, 0.0f, 0.4f, -0.6f, -0.6f,
      0.6f, 0.6f, 1.0f, 0.4f, -0.6f, -0.6f,
      0.0f, 1.0f, 1.0f, 0.4f, -0.6f, -0.6f,

      0.0f, 0.0f, 2.0f, 0.4f, 0.6f, -0.6f,
      0.0f, 1.0f, 1.0f, 0.4f, 0.6f, -0.6f,
      -0.6f, 0.6f, 1.0f, 0.4f, 0.6f, -0.6f,

      0.0f, 0.0f, 2.0f, 1.2f, 0.0f, -0.72f,
      -0.6f, 0.6f, 1.0f, 1.2f, 0.0f, -0.72f,
      -0.6f, -0.6f, 1.0f, 1.2f, 0.0f, -0.72f,

      0.0f, 0.0f, 2.0f, 0.4f, -0.6f, -0.6f,
      -0.6f, -0.6f, 1.0f, 0.4f, -0.6f, -0.6f,
      0.0f, -1.0f, 1.0f, 0.4f, -0.6f, -0.6f,

      0.0f, 0.0f, 2.0f, -0.4f, -0.6f, -0.6f,
      0.0f, -1.0f, 1.0f, -0.4f, -0.6f, -0.6f,
      0.6f, -0.6f, 1.0f, -0.4f, -0.6f, -0.6f,

      0.0f, 0.0f, 2.0f, -1.2f, -0.0f, -0.72f,
      0.6f, -0.6f, 1.0f, -1.2f, -0.0f, -0.72f,
      0.6f, 0.6f, 1.0f, -1.2f, -0.0f, -0.72f,

      0.0f, 0.0f, 2.0f, -0.4f, 0.6f, -0.6f,
      0.6f, 0.6f, 1.0f, -0.4f, 0.6f, -0.6f,
      0.0f, 1.0f, 1.0f, -0.4f, 0.6f, -0.6f,
    };

    unsigned int texture_wood, texture_tire, texture_logo, texture_window, texture_rear_logo, texture_road;
    buildTexture(&texture_wood, "./src/main5_particles/assets/ferraribody.jpg");
    buildTexture(&texture_tire, "./src/main5_particles/assets/roda2.jpg");
    buildTexture(&texture_logo, "./src/main5_particles/assets/ferrarilogomerah.jpg");
    buildTexture(&texture_window, "./src/main5_particles/assets/window.jpg");
    buildTexture(&texture_rear_logo, "./src/main5_particles/assets/Ferrari.jpg");
    buildTexture(&texture_road, "./src/main5_particles/assets/road.jpg");

    std::string vertex_shader_source_code = loadShader("./src/main5_particles/vertex.vs");
    std::string fragment_shader_source_code = loadShader("./src/main5_particles/fragment.fs");
    Shader car_shader("./src/main5_particles/vertex.vs", "./src/main5_particles/fragment.fs");

    std::string particle_vertex_shader_source_code = loadShader("./src/main5_particles/particles_vertex.vs");
    std::string particle_fragment_shader_source_code = loadShader("./src/main5_particles/particles_fragment.fs");
    Shader particle_shader("./src/main5_particles/particles_vertex.vs", "./src/main5_particles/particles_fragment.fs");

    int side = 120;
    float tires[4][((side+2)*11*2) + (side+1)*11*5];
    float tire_color[3];
    tire_color[0] = 0.1f;
    tire_color[1] = 0.1f;
    tire_color[2] = 0.1f;
    createTire3d(tires[0], 0.0f, 0.0f, 0.0f, 0.17f, side, tire_color, 0.125f);
    createTire3d(tires[1], 0.0f, 0.0f, 0.0f, 0.18f, side, tire_color, 0.125f);
    createTire3d(tires[2], 0.0f, 0.0f, 0.0f, 0.17f, side, tire_color, 0.125f);
    createTire3d(tires[3], 0.0f, 0.0f, 0.0f, 0.18f, side, tire_color, 0.125f);

    unsigned int vbo, vbo_tires[4], vbo_particles;
    unsigned int vao, vao_tires[4], vao_particles;
    createVAOVBO(car_vertices, sizeof(car_vertices),&vbo,&vao);
    for(int i=0; i<4; i++){
      createVAOVBO(tires[i], sizeof(tires[i]),&vbo_tires[i],&vao_tires[i]);
    }
    unsigned int vao_ground, vbo_ground;
    createVAOVBO(ground_vertices, sizeof(ground_vertices), &vbo_ground, &vao_ground);

    // RainParticles rain(500);
    unsigned int vao_rain, vbo_rain, vbo_rain_aplha;
    createVAOVBOInstance(teardrop_vertices, sizeof(teardrop_vertices), rain.getTransitionMatrix(), rain.getAlpha(), &vao_rain, &vbo_rain, &vbo_rain_aplha);

    // SmokeParticles smoke(amountSmoke, vec3(0.85f, -0.2f, -0.2f), 0.01f);
    unsigned int vao_smoke, vbo_smoke, vbo_smoke_aplha;
    createVAOVBOInstance(smoke_vertices, sizeof(smoke_vertices), smoke.getTransitionMatrix(), smoke.getAlpha(),  &vao_smoke, &vbo_smoke, &vbo_smoke_aplha);

    unsigned int vao_splash, vbo_splash, vbo_splash_alpha;
    createEmptyVAOVBOInstance(splash_vertices, sizeof(splash_vertices), &vao_splash, &vbo_splash, &vbo_splash_alpha);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float lastFrame = 0.0f;
    float deltaTime = 0.0f;

    camera = new Camera();
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    double lastTime = glfwGetTime();
    int nbFrames = 0;

    std::vector<SplashParticles*> splash_vec;
    bool * isSplash;
    isSplash = new bool[amountRain];
    for(int i=0; i<amountRain; i++){
      isSplash[i] = 0;
    }

    std::vector<mat4>* splash_matrix_vector = new std::vector<mat4>;
    std::vector<float>* splash_alpha_vector = new std::vector<float>;

    while(!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        nbFrames++;
        if ( currentTime - lastTime >= 1.0 ){
            if (!pause) printf("FPS rate: %d fps\n", nbFrames);
            nbFrames = 0;
            lastTime += 1.0;
        }

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window, deltaTime);

        if (!pause) {
          glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

          glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), 1.0f, 0.1f, 100.0f);

          car_shader.use();
          car_shader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
          car_shader.setVec3("lightPos", camera->Position);
          car_shader.setVec3("viewPos", camera->Position);
          car_shader.setMat4("view", camera->GetViewMatrix());
          car_shader.setMat4("model", glm::mat4());
          car_shader.setMat4("projection", projection);
          car_shader.setVec3("material.ambient",glm::vec3(0.05f, 0.0f, 0.0f));
          car_shader.setVec3("material.diffuse",glm::vec3(0.5f, 0.4f, 0.4f));
          car_shader.setVec3("material.specular", glm::vec3(0.7f, 0.04f, 0.04f));
          car_shader.setFloat("material.shininess",0.078125f);

          // road
          glBindTexture(GL_TEXTURE_2D, texture_road);
          glBindVertexArray(vao_ground);
          glDrawArrays(GL_TRIANGLE_FAN, 0, 5);

          // car
          glBindTexture(GL_TEXTURE_2D, texture_wood);
          glBindVertexArray(vao);
          glDrawArrays(GL_TRIANGLE_FAN, 0, 16);
          glDrawArrays(GL_TRIANGLE_FAN, 16, 16);
          glDrawArrays(GL_TRIANGLE_FAN, 32, 5);
          glDrawArrays(GL_TRIANGLE_FAN, 37, 5);
          glDrawArrays(GL_TRIANGLE_FAN, 42, 5);
          glBindTexture(GL_TEXTURE_2D, texture_rear_logo);
          glDrawArrays(GL_TRIANGLE_FAN, 47, 5);
          glBindTexture(GL_TEXTURE_2D, texture_wood);
          glDrawArrays(GL_TRIANGLE_FAN, 52, 5);
          glDrawArrays(GL_TRIANGLE_FAN, 57, 5);
          glDrawArrays(GL_TRIANGLE_FAN, 62, 5);
          glDrawArrays(GL_TRIANGLE_FAN, 67, 5);
          glDrawArrays(GL_TRIANGLE_FAN, 72, 5);
          glDrawArrays(GL_TRIANGLE_FAN, 77, 5);
          glBindTexture(GL_TEXTURE_2D, texture_logo);
          glDrawArrays(GL_TRIANGLE_FAN, 82, 5);
          glBindTexture(GL_TEXTURE_2D, texture_wood);
          glDrawArrays(GL_TRIANGLE_FAN, 87, 5);
          glDrawArrays(GL_TRIANGLE_FAN, 92, 5);
          glDrawArrays(GL_TRIANGLE_FAN, 97, 5);
          glBindTexture(GL_TEXTURE_2D, texture_window);

          car_shader.setVec3("material.ambient", glm::vec3(0.19225f, 0.19225f, 0.19225f));
          car_shader.setVec3("material.diffuse", glm::vec3(0.50754f, 0.50754f, 0.50754f));
          car_shader.setVec3("material.specular", glm::vec3(0.502873f, 0.502873f, 0.502873f));
          car_shader.setFloat("material.shininess", 0.4f);
          // jendela kanan kiri
          glDrawArrays(GL_TRIANGLE_FAN, 102, 10);
          glDrawArrays(GL_TRIANGLE_FAN, 112, 7);
          glDrawArrays(GL_TRIANGLE_FAN, 119, 10);
          glDrawArrays(GL_TRIANGLE_FAN, 129, 7);
          // kaca depan belakang
          glDrawArrays(GL_TRIANGLE_FAN, 136, 5);
          glDrawArrays(GL_TRIANGLE_FAN, 141, 5);
          // lampu
          glDrawArrays(GL_TRIANGLE_FAN, 146, 5);
          glDrawArrays(GL_TRIANGLE_FAN, 151, 5);
          glDrawArrays(GL_TRIANGLE_FAN, 156, 5);
          glDrawArrays(GL_TRIANGLE_FAN, 161, 5);

          car_shader.setVec3("material.ambient", glm::vec3(0.02f, 0.02f, 0.02f));
          car_shader.setVec3("material.diffuse", glm::vec3(0.01f, 0.01f, 0.01f));
          car_shader.setVec3("material.specular", glm::vec3(0.4f, 0.4f, 0.4f));
          car_shader.setFloat("material.shininess", 0.078125f);
          glBindTexture(GL_TEXTURE_2D, texture_tire);
          for(int i=0; i<4; i++){
            glBindVertexArray(vao_tires[i]);
            if (i == 0) rotate(car_shader, -0.4523350000000001f, -0.22941000000000004f, 0.45f);
            else if (i == 1) rotate(car_shader, 0.6048599999999999f, -0.22941000000000004f, 0.45f);
            else if (i == 2) rotate(car_shader, -0.4523350000000001f, -0.22941000000000004f, -0.325f);
            else if (i == 3) rotate(car_shader, 0.6048599999999999f, -0.22941000000000004f, -0.325f);
            glDrawArrays(GL_TRIANGLE_FAN, 0, side+2);
            glDrawArrays(GL_TRIANGLE_FAN, side+2, side*2+2);
            for(int i=0; i<side-1; i++){
              glDrawArrays(GL_TRIANGLE_FAN, side*2+2+i*5, 5);
            }
          }

          particle_shader.use();
          particle_shader.setVec3("viewPos", camera->Position);
          particle_shader.setMat4("view", camera->GetViewMatrix());
          particle_shader.setMat4("projection", projection);
          particle_shader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
          particle_shader.setVec3("lightPos", camera->Position);

          particle_shader.setVec3("material.ambient",glm::vec3(0.25f,	0.20725f,	0.20725f));
          particle_shader.setVec3("material.diffuse",glm::vec3(1.0f,	0.829f,	0.829f));
          particle_shader.setVec3("material.specular", glm::vec3(0.296648f,	0.296648f,	0.296648f));
          particle_shader.setFloat("material.shininess",0.48125f);
          glBindVertexArray(vao_rain);
          glDrawArraysInstanced(GL_TRIANGLES, 0, 36, amountRain);
          rain.updateParticles();
          glBindBuffer(GL_ARRAY_BUFFER, vbo_rain);
          glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * amountRain, rain.getTransitionMatrix(), GL_STATIC_DRAW);

          particle_shader.setVec3("material.ambient",glm::vec3(0.05375f,	0.05f,	0.06625f));
          particle_shader.setVec3("material.diffuse",glm::vec3(0.18275f,	0.17f,	0.22525f));
          particle_shader.setVec3("material.specular", glm::vec3(0.332741f,	0.328634f,	0.346435f));
          particle_shader.setFloat("material.shininess",0.3f);
          glBindVertexArray(vao_smoke);
          glDrawArraysInstanced(GL_TRIANGLES, 0, 36, amountSmoke);
          smoke.updateParticles();
          glBindBuffer(GL_ARRAY_BUFFER, vbo_smoke);
          glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * amountSmoke, smoke.getTransitionMatrix(), GL_STATIC_DRAW);
          glBindBuffer(GL_ARRAY_BUFFER, vbo_smoke_aplha);
          glBufferData(GL_ARRAY_BUFFER, sizeof(float) * amountRain, smoke.getAlpha(), GL_STATIC_DRAW);

          for(int i=0; i<rain.getNumParticles(); i++){
            if(rain.isCollide(i, vec3(0.0f, -0.4f, 0.0f)) && !isSplash[i]){
              isSplash[i] = 1;
              float x = rain.position[i].x;
              float y = rain.position[i].y;
              float z = rain.position[i].z;
              rain.initParticle(i);
              SplashParticles * splash = new SplashParticles(amountSplash, vec3(x, y, z), 0.01f);
              splash->id = i;
              splash_vec.push_back(splash);
            }
          }

          for(int i = 0; i < splash_vec.size(); i++){
            if(splash_vec[i]->timeOut <= 0){
              isSplash[splash_vec[i]->id] = 0;
              delete splash_vec[i];
              splash_vec.erase(splash_vec.begin() + i);
            }
          }

          splash_alpha_vector->clear();
          splash_matrix_vector->clear();
          for(int i = 0; i < splash_vec.size(); i++){
              splash_vec[i]->updateParticles();
              for(int j = 0; j < amountSplash; j++){
                splash_matrix_vector->push_back(splash_vec[i]->getTransitionMatrix()[j]);
                splash_alpha_vector->push_back(splash_vec[i]->getAlpha()[j]);
              }
          }

          if (splash_vec.size() > 0){
            glBindVertexArray(vao_splash);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_splash);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * amountSplash * splash_matrix_vector->size(), &(*splash_matrix_vector)[0], GL_STATIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, vbo_splash_alpha);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * amountSplash * splash_alpha_vector->size(), &(*splash_alpha_vector)[0], GL_STATIC_DRAW);

            glDrawArraysInstanced(GL_TRIANGLES, 0, 36, amountSplash * splash_vec.size());
          }
          glfwSwapBuffers(window);
        }
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
