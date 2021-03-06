#ifndef RAIN_PARTICLES_H
#define RAIN_PARTICLES_H

#include <glm/glm.hpp>
#include <time.h>
#include <stdio.h>

using namespace glm;

class RainParticles {
    private:
        int numParticles;
        vec3 initPosition;
        float offsetInit;
        float* direction;
        float* scaleFactor;
        float* age;
        float* lifespan;
        vec3 globalPull;
        vec3* movement;
        mat4* transitionMatrix;
        float radius;
        float offset;
        float* alpha;
    public:
        vec3* position;
        RainParticles(int);
        void updateParticles();
        mat4* getTransitionMatrix();
        int getMatrixSize();
        void initParticle(int);
        bool isCollide(int, vec3);
        int getNumParticles();
        float* getAlpha();
        void incGlobalPullX();
        void decGlobalPullX();
};

RainParticles::RainParticles(int numParticles){
    this->numParticles = numParticles;

    this->globalPull.x = 0.0f;
    this->globalPull.y = -0.01f;
    this->globalPull.z = 0.0f;
    this->position = new vec3[this->numParticles];
    this->direction = new float[this->numParticles];
    this->scaleFactor = new float[this->numParticles];
    this->age = new float[this->numParticles];
    this->lifespan = new float[this->numParticles];
    this->movement = new vec3[this->numParticles];
    this->transitionMatrix = new mat4[this->numParticles];
    this->radius = 1.0f;
    this->offset = 3.0f;
    this->alpha = new float[numParticles];
    srand(time(NULL));
    for (int i = 0; i < this->numParticles; i++){
      this->initParticle(i);
    }
}

void RainParticles::updateParticles(){
    for(int i = 0; i < this->numParticles; i++){
        age[i] += 0.001f;
        scaleFactor[i] -= 0.00001f;
        direction[i] += 0.0f;
        position[i].x += movement[i].x + globalPull.x;
        position[i].y += movement[i].y + globalPull.y;
        position[i].z += movement[i].z + globalPull.z;
        if (age[i] > lifespan[i]){
            this->initParticle(i);
        }
    }
}

mat4* RainParticles::getTransitionMatrix() {
    for(int i = 0; i < numParticles; i++){
        mat4 matrix;
        matrix = translate(matrix, position[i]);
        // matrix = rotate(matrix, direction[i] - 90, vec3(0.0f, 0.0f, 1.0f));
        matrix = scale(matrix, vec3(scaleFactor[i]));
        transitionMatrix[i] = matrix;
    }
    return transitionMatrix;
}

int RainParticles::getMatrixSize() {
    return sizeof(transitionMatrix);
}

void RainParticles::initParticle(int i){
    float angle = (float)i / (float)numParticles * 360.0f;
    float displacement = (rand() % (int)(2 * this->offset * 100)) / 100.0f - this->offset;
    float x = sin(angle) * this->radius + displacement;
    displacement = (rand() % (int)(2 * this->offset * 100)) / 100.0f - this->offset;
    float y = 20.0f + abs(displacement * 10.0f);
    displacement = (rand() % (int)(2 * this->offset * 100)) / 100.0f - this->offset;
    float z = cos(angle) * this->radius + displacement;

    this->position[i].x = x + (rand()%20 - 10) / 10.0f * 0.1;
    this->position[i].y = y + (rand()%20 - 10) / 10.0f * 0.1;
    this->position[i].z = z +  (rand()%20 - 10) / 10.0f * 0.1;

    y = -0.40941000000000005f;
    // this->splashVertices[i * 6 * 3] = x;
    // this->splashVertices[i * 6 * 3 + 1] = y;
    // this->splashVertices[i * 6 * 3 + 2] = z;

    // this->splashVertices[i * 6 * 3 + 3] = x + 0.5f;
    // this->splashVertices[i * 6 * 3 + 4] = y;
    // this->splashVertices[i * 6 * 3 + 5] = z + 0.5f;

    // this->splashVertices[i * 6 * 3 + 6] = x - 0.5f;
    // this->splashVertices[i * 6 * 3 + 7] = y;
    // this->splashVertices[i * 6 * 3 + 8] = z + 0.5f;

    // this->splashVertices[i * 6 * 3 + 9] = x - 0.5f;
    // this->splashVertices[i * 6 * 3 + 10] = y;
    // this->splashVertices[i * 6 * 3 + 11] = z - 0.5f;

    // this->splashVertices[i * 6 * 3 + 12] = x + 0.5f;
    // this->splashVertices[i * 6 * 3 + 13] = y;
    // this->splashVertices[i * 6 * 3 + 14] = z - 0.5f;

    // this->splashVertices[i * 6 * 3 + 15] = x + 0.5f;
    // this->splashVertices[i * 6 * 3 + 16] = y;
    // this->splashVertices[i * 6 * 3 + 17] = z + 0.5f;

    // this->splashVertices[i * 6 * 3 + 18] = x;
    // this->splashVertices[i * 6 * 3 + 19] = y;
    // this->splashVertices[i * 6 * 3 + 20] = z;

    this->direction[i] = 0.0f;
    this->scaleFactor[i] = 0.0f;
    this->age[i] = 0.0f;
    this->lifespan[i] = (((rand()%10+1)))/10.0f;
    this->movement[i].x = 0.0f;
    this->movement[i].y = -0.05f;
    this->movement[i].z = 0.0f;
    this->alpha[i] = 1.0f;
}

bool RainParticles::isCollide(int i, vec3 plane){
    if(this->position[i].y <= plane.y) return true;
    return false;
}


int RainParticles::getNumParticles(){
    return this->numParticles;
}

float* RainParticles::getAlpha(){
    return alpha;
}

void RainParticles::incGlobalPullX() {
    this->globalPull.x += 0.00005f;
}

void RainParticles::decGlobalPullX() {
    this->globalPull.x -= 0.00005f;
}

#endif
