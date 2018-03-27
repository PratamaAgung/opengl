#include <GL/glut.h>
#include <math.h>

GLfloat sudut = 0.0f;

void idle() {
   glutPostRedisplay();   // Post a re-paint request to activate display()
}

void reshape(GLsizei width, GLsizei height) {  // GLsizei for non-negative integer
   // Compute aspect ratio of the new window
   if (height == 0) height = 1;                // To prevent divide by 0
   GLfloat aspect = (GLfloat)width / (GLfloat)height;
 
   // Set the viewport to cover the new window
   glViewport(0, 0, width, height);
 
   // Set the aspect ratio of the clipping area to match the viewport
   glMatrixMode(GL_PROJECTION);  // To operate on the Projection matrix
   glLoadIdentity();
   if (width >= height) {
     // aspect >= 1, set the height from -1 to 1, with larger width
      gluOrtho2D(-1.0 * aspect, 1.0 * aspect, -1.0, 1.0);
   } else {
      // aspect < 1, set the width to -1 to 1, with larger height
     gluOrtho2D(-1.0, 1.0, -1.0 / aspect, 1.0 / aspect);
   }
}

void createTire(GLdouble radius, GLuint slices, float x, float y, float * color1, float * color2) {
    GLuint i;
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glRotatef(sudut, 0.0f, 0.0f, 1.0f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 0.0f, 1.0f);
    for (i=0;i<slices;i++) {
        GLdouble angle = (GLdouble)i*M_PI*2. / (GLdouble)slices;
        if((i / (slices/6) % 2) == 0) 
            glColor3f(color1[0], color1[1], color1[2]);        
        else 
            glColor3f(color2[0], color2[1], color2[2]);        
        glVertex3f(radius*cos(angle), radius*sin(angle), 1.0f);
    }
    glColor3f(1.0f,1.0f,1.0f);
    glEnd();
    glTranslatef(-x, -y, 0.0f);
    glPopMatrix();        
}
void display() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);     // To operate on Model-View matrix
    glLoadIdentity();

    glPushMatrix();
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
    glPopMatrix();

    float color1[3] = {0.0f, 1.0f, 0.0f};
    float color2[3] = {0.0f, 0.0f, 1.0f};
    createTire(0.15f, 180, -0.4523350000000001f, -0.22941000000000004f, color1, color2);
    createTire(0.16f, 180, 0.6348599999999999f, -0.22941000000000004f, color1, color2);

    sudut += 0.8f;

    glutSwapBuffers();
    glFlush();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE);
    glutInitWindowSize(800, 800);
    glutInitWindowPosition(50, 50);
    glutCreateWindow("Main 1");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    glutMainLoop();
    return 0;
}