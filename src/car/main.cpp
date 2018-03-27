#include <bits/stdc++.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

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

const char * loadShader(const char * file_path){
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
  return shader_code.c_str();
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
    window = glfwCreateWindow( 800, 600, "Car", NULL, NULL);
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
      // back
      0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
      0.8677449999999999f, 0.05218999999999994f, 1.0f, 0.0f, 0.0f, 0.0f,
      0.81141f, 0.07588999999999999f, 1.0f, 0.0f, 0.0f, 0.0f,
      0.6579649999999999f, 0.10588999999999999f, 1.0f, 0.0f, 0.0f, 0.0f,
      // top
      0.3579649999999999f, 0.20361999999999994f, 1.0f, 0.0f, 0.0f, 0.0f,
      0.0579649999999999f, 0.20361999999999994f, 1.0f, 0.0f, 0.0f, 0.0f,
      -0.22641000000000006f, 0.06930999999999997f, 1.0f, 0.0f, 0.0f, 0.0f,
      // front
      -0.7323350000000001f, -0.036025000000000064f, 1.0f, 0.0f, 0.0f, 0.0f,
      -0.7523350000000001f, -0.136025000000000064f, 1.0f, 0.0f, 0.0f, 0.0f,
      -0.7323350000000001f, -0.29941000000000004f, 1.0f, 0.0f, 0.0f, 0.0f,
      -0.6523350000000001f, -0.32941000000000004f, 1.0f, 0.0f, 0.0f, 0.0f,
      // back
      0.7348599999999999f, -0.32941000000000004f, 1.0f, 0.0f, 0.0f, 0.0f,
      0.8577449999999999f, -0.22941000000000004f, 1.0f, 0.0f, 0.0f, 0.0f,
      0.8777449999999999f, -0.10218999999999994f, 1.0f, 0.0f, 0.0f, 0.0f,
      0.8677449999999999f, 0.05218999999999994f, 1.0f, 0.0f, 0.0f, 0.0f,
    };

    const char* vertex_shader_source_code = loadShader("./src/car/CarVS.vs");
    const char* fragment_shader_source_code = loadShader("./src/car/CarFS.fs");

    unsigned int shader_program = createShader(vertex_shader_source_code, fragment_shader_source_code);

    unsigned int vbo;
    glGenBuffers(1, &vbo);
    unsigned int vao;
    glGenVertexArrays(1, &vao);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glUseProgram(shader_program);
    glBindVertexArray(vao);

    while(!glfwWindowShouldClose(window)) {
        processInput(window);

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLE_FAN, 0, 28);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
