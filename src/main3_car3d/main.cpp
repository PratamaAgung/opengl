#include <bits/stdc++.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace glm;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
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

void createTire(float* vertices, float x, float y, float z, float r, int side, float* tire_color, float width){
  float deg = 360/side;
  for(int i=0; i<(side+2)*8; i+=8){
    if(i == 0){
      vertices[i] = x;
      vertices[i+1] = y;
      vertices[i+2] = z;
    } else {
      vertices[i] = x + (r * cos((i-1)*deg*M_PI/180.0));
      vertices[i+1] = y + (r * sin((i-1)*deg*M_PI/180.0));
      vertices[i+2] = z;
    }
    vertices[i+3] = tire_color[0];
    vertices[i+4] = tire_color[1];
    vertices[i+5] = tire_color[2];
    vertices[i+6] = 0.05f;
    vertices[i+7] = 0.1f;
  }

  for(int i=(side+2)*8; i<(side+2)*8*2; i+=8){
    if(i == (side+2)*8){
      vertices[i] = x;
      vertices[i+1] = y;
      vertices[i+2] = (z<0.0f?z+width:z-width);
    } else {
      vertices[i] = x + (r * cos((i-1)*deg*M_PI/180.0));
      vertices[i+1] = y + (r * sin((i-1)*deg*M_PI/180.0));
      vertices[i+2] = (z<0.0f?z+width:z-width);
    }
    vertices[i+3] = tire_color[0];
    vertices[i+4] = tire_color[1];
    vertices[i+5] = tire_color[2];
    vertices[i+6] = 0.05f;
    vertices[i+7] = 0.1f;
  }
  //
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

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window;
    window = glfwCreateWindow( 800, 800, "Texture", NULL, NULL);
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

    float vertices[] = {

        // left
        0.0f, 0.0f, 0.4f, 1.0f, 0.157f, 0.0f, 0.0f, 0.0f,
        // back  0.0f, 0.0f,
        0.0f, -0.32941000000000004f, 0.4f, 0.62745098039f, 0.157f, 1.0f,  0.0f, 0.0f,
        0.7348599999999999f, -0.32941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.8577449999999999f, -0.22941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 1.0f,
        0.8777449999999999f, -0.10218999999999994f, 0.4f, 1.0f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.8677449999999999f, 0.05218999999999994f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  1.0f, 0.0f,
        0.81141f, 0.07588999999999999f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.6579649999999999f, 0.10588999999999999f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 1.0f,
        // top  0.0f, 0.0f,
        0.3579649999999999f, 0.20361999999999994f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.0579649999999999f, 0.20361999999999994f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  1.0f, 0.0f,
        -0.22641000000000006f, 0.06930999999999997f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        // front  0.0f, 0.0f,
        -0.7323350000000001f, -0.036025000000000064f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 1.0f,
        -0.7523350000000001f, -0.136025000000000064f, 0.4f, 1.0f, 0.157f, 0.0f,  0.0f, 0.0f,
        -0.7323350000000001f, -0.29941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  1.0f, 0.0f,
        -0.6523350000000001f, -0.32941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.0f, -0.32941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  1.0f, 0.0f,

        // right
        0.0f, 0.0f, -0.4f, 1.0f, 0.157f, 0.0f, 0.0f, 0.0f,
        // back  0.0f, 0.0f,
        0.0f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  1.0f, 0.0f,
        0.7348599999999999f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.8577449999999999f, -0.22941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 1.0f,
        0.8777449999999999f, -0.10218999999999994f, -0.4f, 1.0f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.8677449999999999f, 0.05218999999999994f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  1.0f, 0.0f,
        0.81141f, 0.07588999999999999f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.6579649999999999f, 0.10588999999999999f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 1.0f,
        // top  0.0f, 0.0f,
        0.3579649999999999f, 0.20361999999999994f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.0579649999999999f, 0.20361999999999994f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  1.0f, 0.0f,
        -0.22641000000000006f, 0.06930999999999997f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        // front  0.0f, 0.0f,
        -0.7323350000000001f, -0.036025000000000064f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 1.0f,
        -0.7523350000000001f, -0.136025000000000064f, -0.4f, 1.0f, 0.157f, 0.0f,  0.0f, 0.0f,
        -0.7323350000000001f, -0.29941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  1.0f, 0.0f,
        -0.6523350000000001f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.0f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  1.0f, 0.0f,

        // TOP
        // bottom
        0.0f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.1f, 0.0f,
        0.7348599999999999f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.7348599999999999f, -0.32941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.1f,
        0.0f, -0.32941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.1f, 0.0f,
        0.0f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.1f, 0.0f,

        // bottom (corner)
        0.7348599999999999f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.1f, 0.0f,
        0.8577449999999999f, -0.22941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.8577449999999999f, -0.22941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.1f,
        0.7348599999999999f, -0.32941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.7348599999999999f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.1f, 0.0f,

        // back
        0.8577449999999999f, -0.22941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.1f, 0.0f,
        0.8777449999999999f, -0.10218999999999994f, -0.4f, 1.0f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.8777449999999999f, -0.10218999999999994f, 0.4f, 1.0f, 0.157f, 0.0f,  0.0f, 0.1f,
        0.8577449999999999f, -0.22941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.8577449999999999f, -0.22941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.1f, 0.0f,

        // back (corner)
        0.8777449999999999f, -0.10218999999999994f, -0.4f, 1.0f, 0.157f, 0.0f,  0.1f, 0.0f,
        0.8677449999999999f, 0.05218999999999994f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.8677449999999999f, 0.05218999999999994f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.1f,
        0.8777449999999999f, -0.10218999999999994f, 0.4f, 1.0f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.8777449999999999f, -0.10218999999999994f, -0.4f, 1.0f, 0.157f, 0.0f,  0.1f, 0.0f,

        // back (top)
        0.8677449999999999f, 0.05218999999999994f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.1f, 0.0f,
        0.81141f, 0.07588999999999999f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.81141f, 0.07588999999999999f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.1f,
        0.8677449999999999f, 0.05218999999999994f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.8677449999999999f, 0.05218999999999994f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.1f, 0.0f,

        // back (window)
        0.81141f, 0.07588999999999999f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.1f, 0.0f,
        0.6579649999999999f, 0.10588999999999999f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.6579649999999999f, 0.10588999999999999f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.1f,
        0.81141f, 0.07588999999999999f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.81141f, 0.07588999999999999f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.1f, 0.0f,

        // back (window 2)
        0.6579649999999999f, 0.10588999999999999f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.1f, 0.0f,
        0.3579649999999999f, 0.20361999999999994f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.3579649999999999f, 0.20361999999999994f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.1f,
        0.6579649999999999f, 0.10588999999999999f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.6579649999999999f, 0.10588999999999999f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.1f, 0.0f,

        // top
        0.3579649999999999f, 0.20361999999999994f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.1f, 0.0f,
        0.0579649999999999f, 0.20361999999999994f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.0579649999999999f, 0.20361999999999994f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.1f,
        0.3579649999999999f, 0.20361999999999994f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.3579649999999999f, 0.20361999999999994f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.1f, 0.0f,

        // top front
        0.0579649999999999f, 0.20361999999999994f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.1f, 0.0f,
        -0.22641000000000006f, 0.06930999999999997f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        -0.22641000000000006f, 0.06930999999999997f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.1f,
        0.0579649999999999f, 0.20361999999999994f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.0579649999999999f, 0.20361999999999994f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.1f, 0.0f,

        // front
        -0.22641000000000006f, 0.06930999999999997f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.1f, 0.0f,
        -0.7323350000000001f, -0.036025000000000064f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        -0.7323350000000001f, -0.036025000000000064f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.1f,
        -0.22641000000000006f, 0.06930999999999997f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        -0.22641000000000006f, 0.06930999999999997f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.1f, 0.0f,

        // front
        -0.7323350000000001f, -0.036025000000000064f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  1.0f, 0.0f,
        -0.7523350000000001f, -0.136025000000000064f, -0.4f, 1.0f, 0.157f, 0.0f,  0.0f, 0.0f,
        -0.7523350000000001f, -0.136025000000000064f, 0.4f, 1.0f, 0.157f, 0.0f,  0.0f, 0.1f,
        -0.7323350000000001f, -0.036025000000000064f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        -0.7323350000000001f, -0.036025000000000064f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.1f, 0.0f,

        // front
        -0.7523350000000001f, -0.136025000000000064f, -0.4f, 1.0f, 0.157f, 0.0f,  0.1f, 0.0f,
        -0.7323350000000001f, -0.29941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        -0.7323350000000001f, -0.29941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.1f,
        -0.7523350000000001f, -0.136025000000000064f, 0.4f, 1.0f, 0.157f, 0.0f,  0.0f, 0.0f,
        -0.7523350000000001f, -0.136025000000000064f, -0.4f, 1.0f, 0.157f, 0.0f,  0.1f, 0.0f,

        // front
        -0.7323350000000001f, -0.29941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        -0.6523350000000001f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        -0.6523350000000001f, -0.32941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        -0.7323350000000001f, -0.29941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        -0.7323350000000001f, -0.29941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,

        // front bottom
        -0.6523350000000001f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.1f, 0.0f,
        0.0f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        0.0f, -0.32941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.1f,
        -0.6523350000000001f, -0.32941000000000004f, 0.4f, 0.62745098039f, 0.157f, 0.0f,  0.0f, 0.0f,
        -0.6523350000000001f, -0.32941000000000004f, -0.4f, 0.62745098039f, 0.157f, 0.0f,  0.1f, 0.0f,
    };

    unsigned int texture;
    buildTexture(&texture, "./src/main3_car3d/container.jpg");

    std::string vertex_shader_source_code = loadShader("./src/main3_car3d/vertex.vs");
    std::string fragment_shader_source_code = loadShader("./src/main3_car3d/fragment.fs");

    unsigned int shader_program = createShader(vertex_shader_source_code, fragment_shader_source_code);

    int side = 120;
    float tire_front_vertices[((side+2)*8*2) + (side+1)*8*5];
    float tire_back_vertices[((side+2)*8*2) + (side+1)*8*5];
    float tire_front_vertices2[((side+2)*8*2) + (side+1)*8*5];
    float tire_back_vertices2[((side+2)*8*2) + (side+1)*8*5];
    float tire_color[3];
    tire_color[0] = 0.1f;
    tire_color[1] = 0.1f;
    tire_color[2] = 0.1f;
    createTire(tire_front_vertices, -0.4523350000000001f, -0.22941000000000004f, 0.45f, 0.17f, side, tire_color, 0.125f);
    createTire(tire_back_vertices, 0.6048599999999999f, -0.22941000000000004f, 0.45f, 0.18f, side, tire_color, 0.125f);
    createTire(tire_front_vertices2, -0.4523350000000001f, -0.22941000000000004f, -0.45f, 0.17f, side, tire_color, 0.125f);
    createTire(tire_back_vertices2, 0.6048599999999999f, -0.22941000000000004f, -0.45f, 0.18f, side, tire_color, 0.125f);


    unsigned int vbo, vbo2, vbo3, vbo4, vbo5;
    unsigned int vao, vao2, vao3, vao4, vao5;
    createVAOVBO(vertices, sizeof(vertices),&vbo,&vao);
    createVAOVBO(tire_front_vertices, sizeof(tire_front_vertices),&vbo2,&vao2);
    createVAOVBO(tire_back_vertices, sizeof(tire_back_vertices),&vbo3,&vao3);
    createVAOVBO(tire_front_vertices2, sizeof(tire_front_vertices),&vbo4,&vao4);
    createVAOVBO(tire_back_vertices2, sizeof(tire_back_vertices),&vbo5,&vao5);

    glBindTexture(GL_TEXTURE_2D, texture);
    glUseProgram(shader_program);

    // GLuint transform = glGetUniformLocation(shader_program, "transform");
    // GLuint modelLoc = glGetUniformLocation(shader_program, "model");
    glEnable(GL_DEPTH_TEST);
    while(!glfwWindowShouldClose(window)) {
        processInput(window);

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // rotate(transform, -0.4523350000000001f, -0.22941000000000004f, 0.0f);
        glm::mat4 model;
        model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
        glm::mat4 view;
        view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
        glm::mat4 projection;
        projection = glm::perspective(glm::radians(45.0f), 800.0f / 800.0f, 0.1f, 100.0f);
        unsigned int modelLoc = glGetUniformLocation(shader_program, "model");
        unsigned int viewLoc  = glGetUniformLocation(shader_program, "view");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shader_program, "projection"), 1, GL_FALSE, &projection[0][0]);

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

        glBindVertexArray(vao2);
        glDrawArrays(GL_TRIANGLE_FAN, 0, side+2);
        glDrawArrays(GL_TRIANGLE_FAN, side+2, side*2+2);
        for(int i=0; i<side-1; i++){
          glDrawArrays(GL_TRIANGLE_FAN, side*2+2+i*5, 5);
        }

        glBindVertexArray(vao3);
        glDrawArrays(GL_TRIANGLE_FAN, 0, side+2);
        glDrawArrays(GL_TRIANGLE_FAN, side+2, side*2+2);
        for(int i=0; i<side-1; i++){
          glDrawArrays(GL_TRIANGLE_FAN, side*2+2+i*5, 5);
        }

        glBindVertexArray(vao4);
        glDrawArrays(GL_TRIANGLE_FAN, 0, side+2);
        glDrawArrays(GL_TRIANGLE_FAN, side+2, side*2+2);
        for(int i=0; i<side-1; i++){
          glDrawArrays(GL_TRIANGLE_FAN, side*2+2+i*5, 5);
        }

        glBindVertexArray(vao5);
        glDrawArrays(GL_TRIANGLE_FAN, 0, side+2);
        glDrawArrays(GL_TRIANGLE_FAN, side+2, side*2+2);
        for(int i=0; i<side-1; i++){
          glDrawArrays(GL_TRIANGLE_FAN, side*2+2+i*5, 5);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
