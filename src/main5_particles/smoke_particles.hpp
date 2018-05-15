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
    public:
        SmokeParticles(int, vec3, float);
        void updateParticles();
        mat4* getTransitionMatrix();
        int getMatrixSize();
};

SmokeParticles::SmokeParticles(int numParticles, vec3 initPosition, float offset){
    this->numParticles = numParticles;
    this->initPosition = initPosition;
    this->offsetInit = offset;

    this->globalPull.x = 0.0025f;
    this->globalPull.y = 0.0f;
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
        this->position[i].x = initPosition.x + (rand()%20 - 10) / 10.0f * offset;
        this->position[i].y = initPosition.y + (rand()%20 - 10) / 10.0f * offset;
        this->position[i].z = initPosition.z + (rand()%20 - 10) / 10.0f * offset;
        this->direction[i] = 0.0f;
        this->scaleFactor[i] = (rand() % 2) / 100.0f + 0.01;
        this->age[i] = 0.0f;
        this->lifespan[i] = (((rand()%10+1)))/10.0f;
        this->movement[i].x = (((((((2) * rand()%11) + 1)) * rand()%11) + 1) * 0.000035) - (((((((2) * rand()%11) + 1)) * rand()%11) + 1) * 0.000035);
        this->movement[i].y = ((((((5) * rand()%11) + 3)) * rand()%11) + 7) * 0.00005; 
        this->movement[i].z = (((((((2) * rand()%11) + 1)) * rand()%11) + 1) * 0.000015) - (((((((2) * rand()%11) + 1)) * rand()%11) + 1) * 0.000015);
    }
}

void SmokeParticles::updateParticles(){
    // std::cout << position[0].x << " " << position[0].y << " " << position[0].z << std::endl;
    for(int i = 0; i < numParticles; i++){
        age[i] += 0.002f;
        // scaleFactor[i] += 0.001f;
        direction[i] += ((((((int)(0.5) * rand()%11) + 1)) * rand()%11) + 1);
        position[i].x += movement[i].x + globalPull.x;
        position[i].y += movement[i].y + globalPull.y;
        position[i].z += movement[i].z + globalPull.z;
        
        if (age[i] > lifespan[i]){
            this->position[i].x = initPosition.x + (rand()%20 - 10) / 10.0f * offsetInit;
            this->position[i].y = initPosition.y + (rand()%20 - 10) / 10.0f * offsetInit;
            this->position[i].z = initPosition.z + (rand()%20 - 10) / 10.0f * offsetInit;
            this->direction[i] = 0.0f;
            this->scaleFactor[i] = (rand() % 2) / 100.0f + 0.01;
            this->age[i] = 0.0f;
            this->lifespan[i] = (((rand()%10+1)))/10.0f;
            this->movement[i].x = (((((((2) * rand()%11) + 1)) * rand()%11) + 1) * 0.000035) - (((((((2) * rand()%11) + 1)) * rand()%11) + 1) * 0.000035);
        this->movement[i].y = ((((((5) * rand()%11) + 3)) * rand()%11) + 7) * 0.00005; 
        this->movement[i].z = (((((((2) * rand()%11) + 1)) * rand()%11) + 1) * 0.000015) - (((((((2) * rand()%11) + 1)) * rand()%11) + 1) * 0.000015);
            // this->movement[i].xposition /= 10.0f;
            // this->movement[i].y /= 100.0f;
            // this->movement[i].z /= 100.0f;
        }
    }
}

mat4* SmokeParticles::getTransitionMatrix() {
    for(int i = 0; i < numParticles; i++){
        mat4 matrix;
        matrix = translate(matrix, position[i]);
        matrix = rotate(matrix, direction[i] - 90, vec3(0.0f, 0.0f, 1.0f));
        matrix = scale(matrix, vec3(scaleFactor[i]));
        transitionMatrix[i] = matrix;
    }
    return transitionMatrix;
}

int SmokeParticles::getMatrixSize() {
    return sizeof(transitionMatrix);
}

#endif