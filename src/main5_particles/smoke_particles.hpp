#ifndef SMOKE_PARTICLES_H
#define SMOKE_PARTICLES_H

#include <glm/glm.hpp>
#include <time.h>
#include <stdio.h>
using namespace glm;

class SmokeParticles {
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
        float* alpha;
        void initParticle(int);
    public:
        SmokeParticles(int, vec3, float);
        void updateParticles();
        mat4* getTransitionMatrix();
        int getMatrixSize();
        float* getAlpha();
        void incGlobalPullX();
        void decGlobalPullX();
};

SmokeParticles::SmokeParticles(int numParticles, vec3 initPosition, float offset){
    this->numParticles = numParticles;
    this->initPosition = initPosition;
    this->offsetInit = offset;

    this->globalPull.x = 0.0025f;
    this->globalPull.y = 0.00002f;
    this->globalPull.z = 0.0f;
    this->position = new vec3[numParticles];
    this->direction = new float[numParticles];
    this->scaleFactor = new float[numParticles];
    this->age = new float[numParticles];
    this->lifespan = new float[numParticles];
    this->movement = new vec3[numParticles];
    this->transitionMatrix = new mat4[numParticles];
    this->alpha = new float[numParticles];
    srand(time(NULL));
    for (int i = 0; i < numParticles; i++){
        initParticle(i);
    }
}

void SmokeParticles::updateParticles(){
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

mat4* SmokeParticles::getTransitionMatrix() {
    for(int i = 0; i < numParticles; i++){
        mat4 matrix;
        matrix = translate(matrix, position[i]);
        // matrix = rotate(matrix, direction[i] - 90, vec3(1.0f, 1.0f, 1.0f));
        matrix = scale(matrix, vec3(scaleFactor[i]));
        transitionMatrix[i] = matrix;
    }
    return transitionMatrix;
}

int SmokeParticles::getMatrixSize() {
    return sizeof(transitionMatrix);
}

void SmokeParticles::initParticle(int i) {
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
    this->alpha[i] = (rand() % 5) / 10.0f + 0.1f;
}

float* SmokeParticles::getAlpha(){
    return alpha;
}

void SmokeParticles::incGlobalPullX() {
    this->globalPull.x += 0.00005f;
}

void SmokeParticles::decGlobalPullX(){
    this->globalPull.x -= 0.00005f;
}

#endif
