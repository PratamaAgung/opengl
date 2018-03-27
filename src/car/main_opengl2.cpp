#include <GL/glut.h>

void display() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBegin(GL_POLYGON);
    glColor3f(1.0f, 1.0f, 1.0f);
    //back
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.8677449999999999f, 0.05218999999999994f, 1.0f);
    glVertex3f(0.81141f, 0.07588999999999999f, 1.0f);
    glVertex3f(0.6579649999999999f, 0.10588999999999999f, 1.0f);
    // top
    glVertex3f(0.3579649999999999f, 0.20361999999999994f, 1.0f);
    glVertex3f(0.0579649999999999f, 0.20361999999999994f, 1.0f);
    glVertex3f(-0.22641000000000006f, 0.06930999999999997f, 1.0f);
    // front
    glVertex3f(-0.7323350000000001f, -0.036025000000000064f, 1.0f);
    glVertex3f(-0.7523350000000001f, -0.136025000000000064f, 1.0f);
    glVertex3f(-0.7323350000000001f, -0.29941000000000004f, 1.0f);
    glVertex3f(-0.6523350000000001f, -0.32941000000000004f, 1.0f);
    // back
    glVertex3f(0.7348599999999999f, -0.32941000000000004f, 1.0f);
    glVertex3f(0.8577449999999999f, -0.22941000000000004f, 1.0f);
    glVertex3f(0.8777449999999999f, -0.10218999999999994f, 1.0f);
    glVertex3f(0.8677449999999999f, 0.05218999999999994f, 1.0f);
    glEnd();

   glFlush();
}

int main(int argc, char** argv) {
   glutInit(&argc, argv);
   glutInitWindowSize(1024, 768);
   glutInitWindowPosition(50, 50);
   glutCreateWindow("Main 1");
   glutDisplayFunc(display);
   glutMainLoop();
   return 0;
}