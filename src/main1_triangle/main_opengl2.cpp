#include <GL/glut.h>

void display() {
   glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
   glClear(GL_COLOR_BUFFER_BIT);

   glBegin(GL_TRIANGLES);
   glColor3f(1.0f, 1.0f, 1.0f);
   glVertex2f(-0.5f, -0.5f);
   glVertex2f( 0.5f, -0.5f);
   glVertex2f( 0.0f,  0.5f);
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