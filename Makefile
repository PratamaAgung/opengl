CC = g++-4.9
INCLUDE_PATHS = -I/usr/local/include -I/opt/X11/include
LIBRARY_PATHS = -L/usr/local/lib -I/opt/X11/lib
LINKER_FLAGS = -framework OpenGL -lglfw -lglew

all: task_1 task_car

task_1:
	g++ src/main_1.cpp -o bin/main_1 -lglfw -lGLEW -lGL
	g++ src/main_1_uniform.cpp -o bin/main_1_uniform -lglfw -lGLEW -lGL
	g++ src/main_1_opengl2.cpp -o bin/main_1_opengl2 -lglut -lGL

task_car: src/main_car.cpp
	g++ src/main_car.cpp -o bin/main_car -lglfw -lGL -lGLEW

task_car_mac:
	 $(CC) src/car/main.cpp -o bin/main_car $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(LINKER_FLAGS)
