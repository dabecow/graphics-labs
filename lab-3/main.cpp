#include "GLUT/glut.h"
#include "math.h"

#define WIDTH 800
#define HEIGHT 600
#define STEP 5

void Display();
void processKeys(int key, int x, int y);
void Initialize();
void draw_model();
void processViewPortState(int mode);

int ex = 0;
float alfaX = 0;
float alfaY = 0;
int vp = WIDTH / 4;

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB); //- используем буфер глубины, двойную буферизацию и представление цвета триадой RGB.
    glutInitWindowSize(WIDTH, HEIGHT);
    glutInitWindowPosition(10, 20);
    glutCreateWindow("lab-3");
    glutDisplayFunc(Display);  //назначаем функцию визуализации
    glutSpecialFunc(processKeys); //назначаем функцию обработки нажатия специальных клавиш
    Initialize(); //дополнительные настройки конвейера
    glutMainLoop();
    return 0;
}

void Initialize() {
    glClearColor(0.3, 0.3, 0.3, 0.0); //задаем цвет заливки холста
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-2.0, 2.0, -2.0, 2.0, -2.0, 2.0); //задаем ортографическую проекцию и видимый объем
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST); // включаем тест глубины. Если этого не сделать видимыми будут объекты не  расположенные ближе всего к наблюдателю, а созданные последними.
    glLoadIdentity();
}

//Пример функции обработки нажатия специальных клавиш
void processKeys(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:
            alfaX -= STEP;
            break;
        case GLUT_KEY_DOWN:
            alfaX += STEP;
            break;
        case GLUT_KEY_LEFT:
            alfaY -= STEP;
            break;
        case GLUT_KEY_RIGHT:
            alfaY += STEP;
            break;
        default: {
            ex = key;
            alfaX = 0;
            alfaY = 0;
        }
    }
    glutPostRedisplay(); // принудительный вызов функции визуализации
}

void Display() {
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glViewport(0, vp*2, vp, vp); //1
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-2.0, 2.0, -2.0, 2.0, -2.0, 2.0); //задаем ортографическую проекцию и видимый объем
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(alfaX, 1, 0, 0);
    glRotatef(alfaY, 0, 1, 0);
    draw_model();

    glViewport(vp, vp*2, vp, vp); //2
    glLoadIdentity();
    draw_model();

    glViewport(vp*2, vp*2, vp, vp); //3
    glLoadIdentity();
    glRotatef(90, 0, -1, 0);
    draw_model();

    glViewport(vp*3, vp*2, vp, vp); //4
    glLoadIdentity();
    glRotatef(90, 1, 0, 0);
    draw_model();

    glViewport(0, vp, vp, vp); //5
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-2.0, 0, -2, 0, 2, 6);
    glTranslatef(-1.0, -1.0, -3);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    draw_model();

    glViewport(vp, vp, vp, vp); //6
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-2.0, 2.0, -2.0, 2.0, -2.0, 2.0); //задаем ортографическую проекцию и видимый объем
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    draw_model();

    glViewport(vp*2, vp, vp, vp); //7
    glLoadIdentity();
//    glRotatef(25, 1, 0, 0);
//    glRotatef(-40, 0, 1, 0);
//    glRotatef(30, 0, 1, 1);
    draw_model();

    glViewport(vp*3, vp, vp, vp); //8
    glLoadIdentity();
    glRotatef(35.264, 1, 0, 0);
    glRotatef(-45, 0, 1, 0);
    draw_model();

    glViewport(0, 0, vp, vp); //9
    glLoadIdentity();
    draw_model();

    glViewport(vp, 0, vp, vp); //10
    glLoadIdentity();
    draw_model();

    glViewport(vp*2, 0, vp, vp); //11
    glLoadIdentity();
    draw_model();

    glViewport(vp*3, 0, vp, vp); //12
    glLoadIdentity();
    draw_model();

    glutSwapBuffers();
}

void draw_model() {
    glBegin(GL_LINES);
    glColor3f(0, 1, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(1, 0, 0);
    glColor3f(1, 0, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 1, 0);
    glColor3f(0, 0, 1);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, 1);
    glEnd();
    glColor3f(1, 1, 1);

    glBegin(GL_POLYGON);
    glColor3f(1, 0, 0);
    glVertex3f(0, 0, 0.5);
    glVertex3f(0.5, 0, 0.5);
    glVertex3f(0.5, -0.5, 0.5);
    glVertex3f(0, -0.5, 0.5);
    glEnd();


    glBegin(GL_POLYGON);
    glVertex3f(-0.5, 0.5, 0.5);
    glVertex3f(0, 0.5, 0.5);
    glVertex3f(0, -0.5, 0.5);
    glVertex3f(-0.5, -0.5, 0.5);
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(0.6, 0.7, 0.9);
    glVertex3f(-0.5, -0.5, 0.5);
    glVertex3f(-0.5, -0.5, -0.5);
    glVertex3f(-0.5, 0.5, -0.5);
    glVertex3f(-0.5, 0.5, 0.5);
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(0, 1, 0);
    glVertex3f(-0.5, 0.5, 0.5);
    glVertex3f(-0.5, 0.5, -0.5);
    glVertex3f(0, 0.5, -0.5);
    glVertex3f(0, 0.5, 0.5);
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(0, 0, 1);
    glVertex3f(0, 0, 0.5);
    glVertex3f(0, 0.5, 0.5);
    glVertex3f(0, 0.5, -0.5);
    glVertex3f(0, 0, -0.5);
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(1, 1, 0);
    glVertex3f(0, 0, -0.5);
    glVertex3f(0, -0.5, -0.5);
    glVertex3f(0.5, -0.5, -0.5);
    glVertex3f(0.5, 0, -0.5);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3f(0, 0.5, -0.5);
    glVertex3f(-0.5, 0.5, -0.5);
    glVertex3f(-0.5, -0.5, -0.5);
    glVertex3f(0, -0.5, -0.5);
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(1, 0, 1);
    glVertex3f(0.5, -0.5, 0.5);
    glVertex3f(0.5, -0.5, -0.5);
    glVertex3f(-0.5, -0.5, -0.5);
    glVertex3f(-0.5, -0.5, 0.5);
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(0, 1, 1);
    glVertex3f(0.5, 0, 0.5);
    glVertex3f(0, 0, 0.5);
    glVertex3f(0, 0, -0.5);
    glVertex3f(0.5, 0, -0.5);
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(1, 1, 1);
    glVertex3f(0.5, -0.5, 0.5);
    glVertex3f(0.5, 0, 0.5);
    glVertex3f(0.5, 0, -0.5);
    glVertex3f(0.5, -0.5, -0.5);
    glEnd();
}
