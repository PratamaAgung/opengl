#include <bits/stdc++.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext.hpp>
#include "camera.hpp"

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
        camera->ProcessKeyboard(FORWARD,  deltaTime);
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera->ProcessKeyboard(BACKWARD,  deltaTime);
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    *vao = vao2;
}

void rotate(unsigned int transform, float x, float y, float z) {
    glm::mat4 transformMatrix;
    transformMatrix = glm::translate(transformMatrix, glm::vec3(x, y, z));
    transformMatrix = glm::rotate(transformMatrix, (float) glfwGetTime() * 5.0f, glm::vec3(0.0f, 0.0f, 1.0f));
    glUniformMatrix4fv(transform, 1, GL_FALSE, glm::value_ptr(transformMatrix));
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
      0.0f, 0.0f, 0.0f, 1.0f, 0.157f, 0.0f,
      // back
      0.0f, -0.32941000000000004f, 0.0f, 0.62745098039f, 0.157f, 0.0f,
      0.7348599999999999f, -0.32941000000000004f, 0.0f, 0.62745098039f, 0.157f, 0.0f,
      0.8577449999999999f, -0.22941000000000004f, 0.0f, 0.62745098039f, 0.157f, 0.0f,
      0.8777449999999999f, -0.10218999999999994f, 0.0f, 1.0f, 0.157f, 0.0f,
      0.8677449999999999f, 0.05218999999999994f, 0.0f, 0.62745098039f, 0.157f, 0.0f,
      0.81141f, 0.07588999999999999f, 0.0f, 0.62745098039f, 0.157f, 0.0f,
      0.6579649999999999f, 0.10588999999999999f, 0.0f, 0.62745098039f, 0.157f, 0.0f,
      // top
      0.3579649999999999f, 0.20361999999999994f, 0.0f, 0.62745098039f, 0.157f, 0.0f,
      0.0579649999999999f, 0.20361999999999994f, 0.0f, 0.62745098039f, 0.157f, 0.0f,
      -0.22641000000000006f, 0.06930999999999997f, 0.0f, 0.62745098039f, 0.157f, 0.0f,
      // front
      -0.7323350000000001f, -0.036025000000000064f, 0.0f, 0.62745098039f, 0.157f, 0.0f,
      -0.7523350000000001f, -0.136025000000000064f, 0.0f, 1.0f, 0.157f, 0.0f,
      -0.7323350000000001f, -0.29941000000000004f, 0.0f, 0.62745098039f, 0.157f, 0.0f,
      -0.6523350000000001f, -0.32941000000000004f, 0.0f, 0.62745098039f, 0.157f, 0.0f,
      0.0f, -0.32941000000000004f, 0.0f, 0.62745098039f, 0.157f, 0.0f,

      // window shadow
      0.0f, 0.1f, 0.0f, 0.62745098039f, 0.157f, 0.0f,
      0.81141f, 0.07588999999999999f, 0.0f, 0.62745098039f, 0.157f, 0.0f,
      0.6579649999999999f, 0.10588999999999999f, 0.0f, 0.62745098039f, 0.157f, 0.0f,
      0.3579649999999999f, 0.20361999999999994f, 0.0f, 0.62745098039f, 0.157f, 0.0f,
      0.0579649999999999f, 0.20361999999999994f, 0.0f, 0.62745098039f, 0.157f, 0.0f,
      -0.22641000000000006f, 0.06930999999999997f, 0.0f, 0.62745098039f, 0.157f, 0.0f,
      -0.7323350000000001f, -0.036025000000000064f, 0.0f, 0.62745098039f, 0.157f, 0.0f,
      -0.10641000000000006f, -0.03030999999999997f, 0.0f, 0.62745098039f, 0.157f, 0.0f,
      0.2579649999999999f, -0.03030999999999997f, 0.0f, 0.62745098039f, 0.157f, 0.0f,
      0.2579649999999999, -0.02930999999999997f, 0.0f, 0.62745098039f, 0.157f, 0.0f,
      0.6079649999999999f, 0.03588999999999999f, 0.0f, 0.62745098039f, 0.157f, 0.0f,
      0.81141f, 0.07588999999999999f, 0.0f, 0.62745098039f, 0.157f, 0.0f,

      // window
      0.0f, 0.1f, 0.0f, 1.0f, 1.0f, 1.0f,
      0.3579649999999999f, 0.01930999999999997f, 0.0f, 0.5f, 0.5f, 0.5f,
      0.3579649999999999f, 0.15361999999999994f, 0.0f, 0.8f, 0.8f, 0.8f,
      0.3279649999999999f, 0.18361999999999994f, 0.0f, 1.0f, 1.0f, 1.0f,
      0.0579649999999999f, 0.18361999999999994f, 0.0f, 1.0f, 1.0f, 1.0f,
      -0.18641000000000006f, 0.06930999999999997f, 0.0f, 1.0f, 1.0f, 1.0f,
      -0.20641000000000006f, 0.02930999999999997f, 0.0f, 0.8f, 0.8f, 0.8f,
      -0.10641000000000006f, 0.00030999999999997f, 0.0f, 1.0f, 1.0f, 1.0f,
      0.2579649999999999f, 0.00030999999999997f, 0.0f, 0.8f, 0.8f, 0.8f,
      0.3579649999999999f, 0.01930999999999997f, 0.0f, 0.5f, 0.5f, 0.5f,

      0.4579649999999999f, 0.1f, 0.0f, 1.0f, 1.0f, 1.0f,
      0.6079649999999999f, 0.08888999999999999f, 0.0f, 1.0f, 1.0f, 1.0f,
      0.4079649999999999f, 0.15361999999999994f, 0.0f, 1.0f, 1.0f, 1.0f,
      0.3879649999999999f, 0.15361999999999994f, 0.0f, 1.0f, 1.0f, 1.0f,
      0.3879649999999999f, 0.01930999999999997f, 0.0f, 0.5f, 0.5f, 0.5f,
      0.6079649999999999f, 0.06588999999999999f, 0.0f, 1.0f, 1.0f, 1.0f,
      0.6079649999999999f, 0.08888999999999999f, 0.0f, 1.0f, 1.0f, 1.0f,

      //botom shadow
      0.0f, -0.30941000000000004f, 0.0f, 0.62745098039f, 0.157f, 0.0f,
      -0.3523350000000001f, -0.32941000000000004f, 0.0f, 0.62745098039f, 0.157f, 0.0f,
      0.7348599999999999f, -0.32941000000000004f, 0.0f, 0.62745098039f, 0.157f, 0.0f,
      0.7348599999999999f, -0.25941000000000004f, 0.0f, 0.62745098039f, 0.157f, 0.0f,
      -0.3523350000000001f, -0.25941000000000004f, 0.0f, 0.62745098039f, 0.157f, 0.0f,
      -0.3523350000000001f, -0.32941000000000004f, 0.0f, 0.62745098039f, 0.157f, 0.0f,

    };

    int side = 120;
    float tire_front_vertices[((side+2)*6)*2];
    float tire_back_vertices[((side+2)*6)*2];
    float tire_color[3];
    tire_color[0] = 0.9f;
    tire_color[1] = 0.9f;
    tire_color[2] = 0.9f;
    createTire(tire_front_vertices, 0.0f, 0.0f, 0.15f, side, tire_color);
    createTire(tire_back_vertices, 0.0f, 0.0f, 0.16f, side, tire_color);

    std::string vertex_shader_source_code = loadShader("./src/main3_car3d/CarVS.vs");
    std::string fragment_shader_source_code = loadShader("./src/main3_car3d/CarFS.fs");

    unsigned int shader_program = createShader(vertex_shader_source_code, fragment_shader_source_code);

    unsigned int vbo, vbo2, vbo3;
    unsigned int vao, vao2, vao3;
    createVAOVBO(car_vertices, sizeof(car_vertices),&vbo,&vao);
    createVAOVBO(tire_front_vertices, sizeof(tire_front_vertices),&vbo2,&vao2);
    createVAOVBO(tire_back_vertices, sizeof(tire_back_vertices),&vbo3,&vao3);
    glUseProgram(shader_program);

    GLuint modelLoc = glGetUniformLocation(shader_program, "model");
    GLuint viewLoc = glGetUniformLocation(shader_program, "view");
    GLuint projectionLoc = glGetUniformLocation(shader_program, "projection");
    float lastFrame = 0.0f;
    float deltaTime = 0.0f;

    camera = new Camera();
    glfwSetCursorPosCallback(window, mouse_callback);

    glm::mat4 projection = glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 100.0f);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    while(!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        processInput(window, deltaTime);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4()));
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 16);
        glDrawArrays(GL_TRIANGLE_FAN, 16, 12);
        glDrawArrays(GL_TRIANGLE_FAN, 28, 10);
        glDrawArrays(GL_TRIANGLE_FAN, 38, 7);
        glDrawArrays(GL_TRIANGLE_FAN, 45, 6);

        rotate(modelLoc, -0.4523350000000001f, -0.22941000000000004f, 0.0f);
        glBindVertexArray(vao2);
        glDrawArrays(GL_TRIANGLE_FAN, 0, side+2);
        glDrawArrays(GL_TRIANGLE_FAN, side+2, side*2+2);

        rotate(modelLoc,0.6348599999999999f, -0.22941000000000004f, 0.0f);
        glBindVertexArray(vao3);
        glDrawArrays(GL_TRIANGLE_FAN, 0, side+2);
        glDrawArrays(GL_TRIANGLE_FAN, side+2, side*2+2);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
