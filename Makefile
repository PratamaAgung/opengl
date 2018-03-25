all: task_1

task_1:
	g++ src/main_1.cpp -o bin/main_1 -lglfw -lGLEW -lGL
	g++ src/main_1_uniform.cpp -o bin/main_1_uniform -lglfw -lGLEW -lGL
	g++ src/main_1_opengl2.cpp -o bin/main_1_opengl2 -lglut -lGL
