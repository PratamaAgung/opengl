#ifndef SPLASH_PARTICLES_H
#define SPLASH_PARTICLES_H

#include <glm/glm.hpp>
#include <time.h>
#include <stdio.h>
using namespace glm;

class SplashParticles {
    private:
        int numParticles;
        vec3 initPosition;
        float offsetInit;
        vec3* position;
        float* direction;
        float* scaleFactor;
        float* age;
        float* lifespan;
        vec3 globalPull;
        vec3* movement;
        mat4* transitionMatrix;
        void initParticle(int);
    public:
        int id;
        int timeOut;
        SplashParticles(int, vec3, float);
        ~SplashParticles();
        void updateParticles();
        mat4* getTransitionMatrix();
        int getMatrixSize();
        unsigned int* vao;
        unsigned int* vbo;
};

SplashParticles::SplashParticles(int numParticles, vec3 initPosition, float offset){
    this->numParticles = numParticles;
    this->initPosition = initPosition;
    this->offsetInit = offset;
    this->timeOut = 5;

    this->globalPull.x = 0.0f;
    this->globalPull.y = 0.002f;
    this->globalPull.z = 0.0f;
    this->position = new vec3[numParticles];
    this->direction = new float[numParticles];
    this->scaleFactor = new float[numParticles];
    this->age = new float[numParticles];
    this->lifespan = new float[numParticles];
    this->movement = new vec3[numParticles];
    this->transitionMatrix = new mat4[numParticles];
    srand(time(NULL));
    for (int i = 0; i < numParticles; i++){
        initParticle(i);
    }
}


SplashParticles::~SplashParticles(){
    delete [] this->position;
    delete [] this->direction;
    delete [] this->scaleFactor;
    delete [] this->age;
    delete [] this->lifespan;
    delete [] this->movement;
    delete [] this->transitionMatrix;
}

void SplashParticles::updateParticles(){
    this->timeOut -= 1;
    for(int i = 0; i < numParticles; i++){
        age[i] += 0.003f;
        scaleFactor[i] += 0.00001f;
        direction[i] += ((((((int)(0.5) * rand()%11) + 1)) * rand()%11) + 1);
        position[i].x += movement[i].x + globalPull.x;
        position[i].y += movement[i].y + globalPull.y;
        position[i].z += movement[i].z + globalPull.z;

        if (age[i] > lifespan[i]){
            initParticle(i);
        }
    }
}

mat4* SplashParticles::getTransitionMatrix() {
    for(int i = 0; i < numParticles; i++){
        mat4 matrix;
        matrix = translate(matrix, position[i]);
        matrix = rotate(matrix, direction[i] - 90, vec3(1.0f, 1.0f, 1.0f));
        matrix = scale(matrix, vec3(scaleFactor[i]));
        transitionMatrix[i] = matrix;
    }
    return transitionMatrix;
}

int SplashParticles::getMatrixSize() {
    return sizeof(transitionMatrix);
}

void SplashParticles::initParticle(int i) {
    this->position[i].x = this->initPosition.x + (rand()%20 - 10) / 10.0f * this->offsetInit;
    this->position[i].y = this->initPosition.y + (rand()%20 - 10) / 10.0f * this->offsetInit;
    this->position[i].z = this->initPosition.z + (rand()%20 - 10) / 10.0f * this->offsetInit;
    this->direction[i] = 0.0f;
    this->scaleFactor[i] = (rand() % 8) / 1000.0f + 0.005;
    this->age[i] = 0.0f;
    this->lifespan[i] = (((rand()%10+1)))/10.0f;
    this->movement[i].x = (((((((2) * rand()%11) + 1)) * rand()%11) + 1) * 0.000035) - (((((((2) * rand()%11) + 1)) * rand()%11) + 1) * 0.000035);
    this->movement[i].y = ((((((5) * rand()%11) + 3)) * rand()%11) + 7) * 0.00005;
    this->movement[i].z = (((((((2) * rand()%11) + 1)) * rand()%11) + 1) * 0.000015) - (((((((2) * rand()%11) + 1)) * rand()%11) + 1) * 0.000015);
}

#endif
