#include <bits/stdc++.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext.hpp>
#include "camera.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace glm;
bool firstMouse;
float lastX, lastY;
Camera* camera;

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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);

    *vao = vao2;
}

void rotate(unsigned int transform, float x, float y, float z) {
    glm::mat4 transformMatrix;
    transformMatrix = glm::translate(transformMatrix, glm::vec3(x, y, z));
    transformMatrix = glm::rotate(transformMatrix, (float) glfwGetTime() * 5.0f, glm::vec3(0.0f, 0.0f, 1.0f));
    glUniformMatrix4fv(transform, 1, GL_FALSE, glm::value_ptr(transformMatrix));
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
  for(int i=0; i<(side+2)*8*2; i+=8){
    if((i == 0) || (i == (side+2)*8)){
      vertices[i] = x;
      vertices[i+1] = y;
      if(i < (side+2)*8){
        vertices[i+2] = z;
      } else {
        vertices[i+2] = (z<0.0f?z+width:z-width);
      }
      vertices[i+6] = 0.5;
      vertices[i+7] = 0.5;
    } else {
      vertices[i] = x + (r * cos((i-1)*deg*M_PI/180.0));
      vertices[i+1] = y + (r * sin((i-1)*deg*M_PI/180.0));
      if(i < (side+2)*8){
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
    // vertices[i+6] = 0.5 + (0.25 * cos((i-1)*deg*M_PI/180.0));
    // vertices[i+7] = 0.5 + (0.25 * sin((i-1)*deg*M_PI/180.0));
    std::cout << vertices[i+6] << " " << vertices[i+7] << std::endl;
    // vertices[i+6] = (vertices[i] + vertices[i+2]) / 2;
    // vertices[i+7] = (vertices[i+1] + vertices[i+2]) / 2;
  }

  int j = (side+2)*8*2;
  for(int i=8; i<(side*8); i+=8, j+=40){
    for(int k=0; k<40; k++){
      if(k<8){
        vertices[j+k] = vertices[i+k];
      } else if (k<16){
        vertices[j+k] = vertices[i+k+((side+2)*8)];
      } else if (k<24){
        vertices[j+k] = vertices[i+8+k+((side+2)*8)];
      } else if (k<32) {
        vertices[j+k] = vertices[i+8+k];
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
    window = glfwCreateWindow( 800, 800, "Car", NULL, NULL);
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

    float car_vertices[] = {
        // left
        // 0.0f, 0.0f, 0.4f, 1.0f, 0.157f, 0.0f, 0.5f, 0.0f,
        0.0f, -0.32941000000000004f, 0.4f, 0.62745098039f, 0.157f, 1.0f,  0.0f, 0.335295f,
        // back  0.0f, 0.0f,
        0.0f, -0.32941000000000004f, 0.4f, 0.62745098039f, 0.157f, 1.0f,  0.0f, 0.335295f,
        0.7348599999999999f, -0.32941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.8674299999999999f, 0.335295f,
        0.8577449999999999f, -0.22941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.9288725f, 0.385295f,
        0.8777449999999999f, -0.10218999999999994f, 0.4f, 1.0f, 0.157f, 0.0f,  0.9388725f, 0.44890500000000005f,
        0.8677449999999999f, 0.05218999999999994f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.9338724999999999f, 0.526095f,
        0.81141f, 0.07588999999999999f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.905705f, 0.537945f,
        0.6579649999999999f, 0.10588999999999999f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.8289825f, 0.552945f,
        // top  0.0f, 0.0f,
        0.3579649999999999f, 0.20361999999999994f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.6789824999999999f, 0.60181f,
        0.0579649999999999f, 0.20361999999999994f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5289824999999999f, 0.60181f,
        -0.22641000000000006f, 0.06930999999999997f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.386795f, 0.534655f,
        // front  0.0f, 0.0f,
        -0.7323350000000001f, -0.036025000000000064f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.13383249999999997f, 0.48198749999999996f,
        -0.7523350000000001f, -0.136025000000000064f, 0.4f, 1.0f, 0.157f, 0.0f,  0.12383249999999996f, 0.43198749999999997f,
        -0.7323350000000001f, -0.29941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.13383249999999997f, 0.35029499999999997f,
        -0.6523350000000001f, -0.32941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.17383249999999995f, 0.335295f,
        0.0f, -0.32941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.335295f,

        // right
        // 0.0f, 0.0f, 0.4f, 1.0f, 0.157f, 0.0f, 0.5f, 0.0f,
        0.0f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 1.0f,  0.0f, 0.335295f,
        // back  0.0f, 0.0f,
        0.0f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 1.0f,  0.0f, 0.335295f,
        0.7348599999999999f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.8674299999999999f, 0.335295f,
        0.8577449999999999f, -0.22941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.9288725f, 0.385295f,
        0.8777449999999999f, -0.10218999999999994f, -0.4f, 1.0f, 0.157f, 0.0f,  0.9388725f, 0.44890500000000005f,
        0.8677449999999999f, 0.05218999999999994f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.9338724999999999f, 0.526095f,
        0.81141f, 0.07588999999999999f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.905705f, 0.537945f,
        0.6579649999999999f, 0.10588999999999999f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.8289825f, 0.552945f,
        // top  0.0f, 0.0f,
        0.3579649999999999f, 0.20361999999999994f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.6789824999999999f, 0.60181f,
        0.0579649999999999f, 0.20361999999999994f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5289824999999999f, 0.60181f,
        -0.22641000000000006f, 0.06930999999999997f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.386795f, 0.534655f,
        // front  0.0f, 0.0f,
        -0.7323350000000001f, -0.036025000000000064f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.13383249999999997f, 0.48198749999999996f,
        -0.7523350000000001f, -0.136025000000000064f, -0.4f, 1.0f, 0.157f, 0.0f,  0.12383249999999996f, 0.43198749999999997f,
        -0.7323350000000001f, -0.29941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.13383249999999997f, 0.35029499999999997f,
        -0.6523350000000001f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.17383249999999995f, 0.335295f,
        0.0f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.335295f,

        // TOP
        // bottom
        0.0f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f,
        0.7348599999999999f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.7348599999999999f, -0.32941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.5f,
        0.0f, -0.32941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f,
        0.0f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f,

        // bottom (corner)
        0.7348599999999999f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f,
        0.8577449999999999f, -0.22941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.8577449999999999f, -0.22941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.5f,
        0.7348599999999999f, -0.32941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.7348599999999999f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f,

        // back
        0.8577449999999999f, -0.22941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f,
        0.8777449999999999f, -0.10218999999999994f, -0.4f, 1.0f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.8777449999999999f, -0.10218999999999994f, 0.4f, 1.0f, 0.157f, 0.0f,  0.0f, 0.5f,
        0.8577449999999999f, -0.22941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.8577449999999999f, -0.22941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f,

        // back (corner)
        0.8777449999999999f, -0.10218999999999994f, -0.4f, 1.0f, 0.157f, 0.0f,  0.5f, 0.0f,
        0.8677449999999999f, 0.05218999999999994f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.8677449999999999f, 0.05218999999999994f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.5f,
        0.8777449999999999f, -0.10218999999999994f, 0.4f, 1.0f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.8777449999999999f, -0.10218999999999994f, -0.4f, 1.0f, 0.157f, 0.0f,  0.5f, 0.0f,

        // back (top)
        0.8677449999999999f, 0.05218999999999994f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f,
        0.81141f, 0.07588999999999999f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.81141f, 0.07588999999999999f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.5f,
        0.8677449999999999f, 0.05218999999999994f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.8677449999999999f, 0.05218999999999994f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f,

        // back (window)
        0.81141f, 0.07588999999999999f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f,
        0.6579649999999999f, 0.10588999999999999f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.6579649999999999f, 0.10588999999999999f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.5f,
        0.81141f, 0.07588999999999999f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.81141f, 0.07588999999999999f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f,

        // back (window 2)
        0.6579649999999999f, 0.10588999999999999f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f,
        0.3579649999999999f, 0.20361999999999994f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.3579649999999999f, 0.20361999999999994f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.5f,
        0.6579649999999999f, 0.10588999999999999f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.6579649999999999f, 0.10588999999999999f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f,

        // top
        0.3579649999999999f, 0.20361999999999994f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f,
        0.0579649999999999f, 0.20361999999999994f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.0579649999999999f, 0.20361999999999994f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.5f,
        0.3579649999999999f, 0.20361999999999994f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.3579649999999999f, 0.20361999999999994f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f,

        // top front
        0.0579649999999999f, 0.20361999999999994f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f,
        -0.22641000000000006f, 0.06930999999999997f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        -0.22641000000000006f, 0.06930999999999997f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.5f,
        0.0579649999999999f, 0.20361999999999994f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.0579649999999999f, 0.20361999999999994f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f,

        // front
        -0.22641000000000006f, 0.06930999999999997f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f,
        -0.7323350000000001f, -0.036025000000000064f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        -0.7323350000000001f, -0.036025000000000064f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.5f,
        -0.22641000000000006f, 0.06930999999999997f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        -0.22641000000000006f, 0.06930999999999997f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f,

        // front
        -0.7323350000000001f, -0.036025000000000064f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  1.0f, 0.0f,
        -0.7523350000000001f, -0.136025000000000064f, -0.4f, 1.0f, 0.157f, 0.0f,  0.0f, 0.0f,
        -0.7523350000000001f, -0.136025000000000064f, 0.4f, 1.0f, 0.157f, 0.0f,  0.0f, 0.5f,
        -0.7323350000000001f, -0.036025000000000064f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        -0.7323350000000001f, -0.036025000000000064f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f,

        // front
        -0.7523350000000001f, -0.136025000000000064f, -0.4f, 1.0f, 0.157f, 0.0f,  0.5f, 0.0f,
        -0.7323350000000001f, -0.29941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        -0.7323350000000001f, -0.29941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.5f,
        -0.7523350000000001f, -0.136025000000000064f, 0.4f, 1.0f, 0.157f, 0.0f,  0.0f, 0.0f,
        -0.7523350000000001f, -0.136025000000000064f, -0.4f, 1.0f, 0.157f, 0.0f,  0.5f, 0.0f,

        // front
        -0.7323350000000001f, -0.29941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        -0.6523350000000001f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        -0.6523350000000001f, -0.32941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        -0.7323350000000001f, -0.29941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        -0.7323350000000001f, -0.29941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,

        // front bottom
        -0.6523350000000001f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f,
        0.0f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.0f, -0.32941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.5f,
        -0.6523350000000001f, -0.32941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        -0.6523350000000001f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.5f, 0.0f,
    };
    
    unsigned int texture_wood, texture_tire;
    buildTexture(&texture_wood, "./src/main3_car3d/metalplate.jpg");
    buildTexture(&texture_tire, "./src/main3_car3d/roda2.jpg");

    std::string vertex_shader_source_code = loadShader("./src/main3_car3d/vertex.vs");
    std::string fragment_shader_source_code = loadShader("./src/main3_car3d/fragment.fs");

    unsigned int shader_program = createShader(vertex_shader_source_code, fragment_shader_source_code);

    int side = 120;
    float tires[4][((side+2)*8*2) + (side+1)*8*5];
    float tire_color[3];
    tire_color[0] = 0.1f;
    tire_color[1] = 0.1f;
    tire_color[2] = 0.1f;
    createTire3d(tires[0], 0.0f, 0.0f, 0.0f, 0.17f, side, tire_color, 0.125f);
    createTire3d(tires[1], 0.0f, 0.0f, 0.0f, 0.18f, side, tire_color, 0.125f);
    createTire3d(tires[2], 0.0f, 0.0f, 0.0f, 0.17f, side, tire_color, 0.125f);
    createTire3d(tires[3], 0.0f, 0.0f, 0.0f, 0.18f, side, tire_color, 0.125f);

    unsigned int vbo, vbo_tires[4];
    unsigned int vao, vao_tires[4];
    createVAOVBO(car_vertices, sizeof(car_vertices),&vbo,&vao);
    for(int i=0; i<4; i++){
      createVAOVBO(tires[i], sizeof(tires[i]),&vbo_tires[i],&vao_tires[i]);
    }

    glUseProgram(shader_program);
    glEnable(GL_DEPTH_TEST);

    GLuint modelLoc = glGetUniformLocation(shader_program, "model");
    GLuint viewLoc = glGetUniformLocation(shader_program, "view");
    GLuint projectionLoc = glGetUniformLocation(shader_program, "projection");
    float lastFrame = 0.0f;
    float deltaTime = 0.0f;

    camera = new Camera();
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    while(!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        processInput(window, deltaTime);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));

        glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), 1.0f, 0.1f, 100.0f);
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4()));
        glBindTexture(GL_TEXTURE_2D, texture_wood);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 16);
        glDrawArrays(GL_TRIANGLE_FAN, 16, 16);
        glDrawArrays(GL_TRIANGLE_FAN, 32, 5);
        glDrawArrays(GL_TRIANGLE_FAN, 37, 5);
        glDrawArrays(GL_TRIANGLE_FAN, 42, 5);
        glDrawArrays(GL_TRIANGLE_FAN, 47, 5);
        glDrawArrays(GL_TRIANGLE_FAN, 52, 5);
        glDrawArrays(GL_TRIANGLE_FAN, 57, 5);
        glDrawArrays(GL_TRIANGLE_FAN, 62, 5);
        glDrawArrays(GL_TRIANGLE_FAN, 67, 5);
        glDrawArrays(GL_TRIANGLE_FAN, 72, 5);
        glDrawArrays(GL_TRIANGLE_FAN, 77, 5);
        glDrawArrays(GL_TRIANGLE_FAN, 82, 5);
        glDrawArrays(GL_TRIANGLE_FAN, 87, 5);
        glDrawArrays(GL_TRIANGLE_FAN, 92, 5);
        glDrawArrays(GL_TRIANGLE_FAN, 97, 5);
        glDrawArrays(GL_TRIANGLE_FAN, 102, 5);
        glDrawArrays(GL_TRIANGLE_FAN, 107, 5);
        glDrawArrays(GL_TRIANGLE_FAN, 112, 5);
        glDrawArrays(GL_TRIANGLE_FAN, 117, 5);

        glBindTexture(GL_TEXTURE_2D, texture_tire);
        for(int i=0; i<4; i++){
          glBindVertexArray(vao_tires[i]);
          if (i == 0) rotate(modelLoc, -0.4523350000000001f, -0.22941000000000004f, 0.45f);
          else if (i == 1) rotate(modelLoc, 0.6048599999999999f, -0.22941000000000004f, 0.45f);
          // else if (i == 2) rotate(modelLoc, -0.4523350000000001f, -0.22941000000000004f, -0.45f);
          // else if (i == 3) rotate(modelLoc, 0.6048599999999999f, -0.22941000000000004f, -0.45f); 
          else if (i == 2) rotate(modelLoc, -0.4523350000000001f, -0.22941000000000004f, -0.325f);
          else if (i == 3) rotate(modelLoc, 0.6048599999999999f, -0.22941000000000004f, -0.325f);
          glDrawArrays(GL_TRIANGLE_FAN, 0, side+2);
          glDrawArrays(GL_TRIANGLE_FAN, side+2, side*2+2);
          for(int i=0; i<side-1; i++){
            glDrawArrays(GL_TRIANGLE_FAN, side*2+2+i*5, 5);
          }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
