all: task_1 task_car

task_1:
	g++ src/main_1.cpp -o bin/main_1 -lglfw -lGLEW -lGL
	g++ src/main_1_uniform.cpp -o bin/main_1_uniform -lglfw -lGLEW -lGL
	g++ src/main_1_opengl2.cpp -o bin/main_1_opengl2 -lglut -lGL

task_car: src/main_car.cpp
	g++ src/main_car.cpp -o bin/main_car -lglfw -lGL -lGLEW
