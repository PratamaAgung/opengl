CC = g++-4.9
INCLUDE_PATHS = -I/usr/local/include -I/opt/X11/include
LIBRARY_PATHS = -L/usr/local/lib -I/opt/X11/lib
LINKER_FLAGS = -framework OpenGL -lglfw -lglew

all: task_1 task_2 task_3 task_4 task_5

task_1:
	g++ src/main1_triangle/main.cpp -o bin/main1 -lglfw -lGLEW -lGL
	g++ src/main1_triangle/main_uniform.cpp -o bin/main1_uniform -lglfw -lGLEW -lGL
	g++ src/main1_triangle/main_opengl2.cpp -o bin/main1_opengl2 -lglut -lGL

task_2: src/main2_car/main.cpp
	g++ src/main2_car/main.cpp -o bin/main2_car -lglfw -lGL -lGLEW
	g++ src/main2_car/main_opengl2.cpp -o bin/main2_car_opengl2 -lglut -lGL -lGLU

task_3: src/main3_car3d/main.cpp
	g++ src/main3_car3d/main_combine.cpp -o bin/main3_car -lglfw -lGL -lGLEW

task_4: src/main4_lighting/main_combine.cpp
	g++ src/main4_lighting/main_combine.cpp -o bin/main4_light -lglfw -lGL -lGLEW

task_5: src/main5_particles/main.cpp
	g++ src/main5_particles/main.cpp -o bin/main5_particles -lglfw -lGL -lGLEW -std=c++11

task_2_mac:
	$(CC) src/main2_car/main.cpp -o bin/main2_car $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(LINKER_FLAGS)

task_3_mac:
	$(CC) src/main3_car3d/main.cpp -o bin/main3_car3d $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(LINKER_FLAGS)

task_5_mac: src/main5_particles/main.cpp
	$(CC) src/main5_particles/main.cpp -o bin/main5_particles $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(LINKER_FLAGS) -std=c++11
