#include "GLUT/glut.h"
#include "math.h"

#define OFFSET 1.3
#define WIDTH 1200
#define HEIGHT 600
#define SCALE 0.7
#define STEP 5
#define FUN_STEP 0.05

enum Mode {
    M_CRUSH = 0,
    M_SINGLE,
    M_TRIAD_X,
    M_TRIAD_Z,
    M_TRIAD_Y,
    M_TRIAD_THROUGH_1_1_1,
    M_TRIAD_ON_ONE_AXE_GROUP_ROTATE,
    M_TRIAD_ON_ONE_AXE_EACH_ROTATES,
    M_TRIAD_ON_ONE_AXE_ROTATES_SMALL_ONLY,
    M_SINGLE_FUNNY,
    M_FOUR
};

enum FunnySnowmanCircleZone {
    CZ_1 = 0,
    CZ_2,
    CZ_3,
    CZ_4
};

int currentMode = M_CRUSH;

void processKeys(int key, int x, int y);
void processRotation();
void processSteppingForFunnnySnowman(bool directionIsRight);

void Initialize();
void Display();

void drawScene();
void drawAxes();
void drawSnowman(bool toProcessRotation);
void drawSnowmanCrush();
void drawSnowmenWithOffsets(int count, float x, float y, float z);
void drawSnowmenOnOneAxe(bool independentRotation, bool oneOnlyRotates);
void drawFunnySnowman();
void drawFourSnowmen();

float bodyRadius = 0.5f;
float headRadius = 0.25f;
float eyeRadius = 0.05f;
int ex = 0;
float alfaX = 0;
float alfaY = 0;
float steppingForFun = 0;
int currentFunState = 0;

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB); //- используем буфер глубины, двойную буферизацию и представление цвета триадой RGB.
    glutInitWindowSize(WIDTH, HEIGHT);
    glutInitWindowPosition(10, 20);
    glutCreateWindow("lab-2");
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
            alfaX-=STEP;
            break;
        case GLUT_KEY_DOWN:
            alfaX+=STEP;
            break;
        case GLUT_KEY_LEFT:
            if (currentMode == M_SINGLE_FUNNY)
                processSteppingForFunnnySnowman(false);
            else
                alfaY-=STEP;
            break;
        case GLUT_KEY_RIGHT:
            if (currentMode == M_SINGLE_FUNNY)
                processSteppingForFunnnySnowman(true);
            else
                alfaY+=STEP;
            break;
        default:{
            ex = key;
            alfaX = 0;
            alfaY = 0;
            switch (ex) {
                case GLUT_KEY_F1:
                    currentMode = M_CRUSH;
                    break;
                case GLUT_KEY_F2:
                    currentMode = M_SINGLE;
                    break;
                case GLUT_KEY_F3:
                    currentMode = M_TRIAD_X;
                    break;
                case GLUT_KEY_F4:
                    currentMode = M_TRIAD_Z;
                    break;
                case GLUT_KEY_F5:
                    currentMode = M_TRIAD_Y;
                    break;
                case GLUT_KEY_F6:
                    currentMode = M_TRIAD_THROUGH_1_1_1;
                    break;
                case GLUT_KEY_F7:
                    currentMode = M_TRIAD_ON_ONE_AXE_GROUP_ROTATE;
                    break;
                case GLUT_KEY_F8:
                    currentMode = M_TRIAD_ON_ONE_AXE_EACH_ROTATES;
                    break;
                case GLUT_KEY_F9:
                    currentMode = M_TRIAD_ON_ONE_AXE_ROTATES_SMALL_ONLY;
                    break;
                case GLUT_KEY_F10:
                    currentMode = M_SINGLE_FUNNY;
                    break;
                case GLUT_KEY_F12:
                    currentMode = M_FOUR;
                    break;
                default:
                    currentMode = M_TRIAD_X;
                    return;
            }
        }

    }
    glutPostRedisplay(); // принудительный вызов функции визуализации
}

void Display() {
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, WIDTH/2, HEIGHT);
    glLoadIdentity();
    drawScene();

    glViewport(WIDTH/2, 0, WIDTH/2, HEIGHT);
    glLoadIdentity();
    glRotatef(90, 1, 0, 0);
    drawScene();

    glutSwapBuffers();
}

void drawScene() {
    switch (currentMode) {
        case M_CRUSH:
            drawSnowmanCrush();
            break;
        case M_SINGLE:
            drawAxes();
            drawSnowman(true);
            break;
        case M_TRIAD_X:
            drawSnowmenWithOffsets(3, bodyRadius, 0, 0);
            break;
        case M_TRIAD_Z:
            drawSnowmenWithOffsets(3, 0, 0, bodyRadius);
            break;
        case M_TRIAD_Y:
            drawSnowmenWithOffsets(3, 0, bodyRadius + headRadius, 0);
            break;
        case M_TRIAD_THROUGH_1_1_1:
            drawSnowmenWithOffsets(3, bodyRadius/sqrt(3.0), bodyRadius/sqrt(3.0), bodyRadius/sqrt(3.0));
            break;
        case M_TRIAD_ON_ONE_AXE_GROUP_ROTATE:
            drawSnowmenOnOneAxe(false, false);
            break;
        case M_TRIAD_ON_ONE_AXE_EACH_ROTATES:
            drawSnowmenOnOneAxe(true, false);
            break;
        case M_TRIAD_ON_ONE_AXE_ROTATES_SMALL_ONLY:
            drawSnowmenOnOneAxe(true, true);
            break;
        case M_SINGLE_FUNNY:
            drawFunnySnowman();
            break;
        case M_FOUR:
            drawFourSnowmen();
            break;
        default:
            break;
    }
}

void drawAxes() {
    glBegin(GL_LINES);
    glColor3f(1.0,0,0);
    glVertex3f(0, 0, 0);
    glVertex3f(OFFSET, 0, 0);
    glEnd();
    glBegin(GL_LINES);
    glColor3f(0,1.0,0);
    glVertex3f(0, 0, 0);
    glVertex3f(0, OFFSET, 0);
    glEnd();
    glBegin(GL_LINES);
    glColor3f(0,0,1.0);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, OFFSET);
    glEnd();
}

void drawSnowmanEye(bool toTheRight) {
    glColor3f(0,0,0);
    glTranslatef(0.3f * headRadius * (toTheRight ? 1 : -1), 0.5f * headRadius, headRadius);
    glutSolidSphere(eyeRadius, 50, 50);
    glColor3f(1.0,1.0,1.0);
}

void drawSnowmanHead() {
    glutSolidSphere(headRadius, 50, 50);
    glPushMatrix();
    drawSnowmanEye(true);
    glPopMatrix();
    glPushMatrix();
    drawSnowmanEye(false);
    glPopMatrix();

    glColor3f(1, 0, 0);
    glTranslatef(0, 0, headRadius);
    glutSolidCone(headRadius / 4.0, headRadius, 50,50);
}

void drawSnowman(bool toProcessRotation) {
    glPushMatrix();

    if (toProcessRotation)
        processRotation();

    glColor3f(1.0,1.0,1.0);
    glutSolidSphere(bodyRadius, 50, 50);
    glTranslatef(0, bodyRadius+headRadius, 0);
    drawSnowmanHead();
    glPopMatrix();
}

void drawSnowmenWithOffsets(int count, float x, float y, float z) {
    processRotation();
    for (int i = 0; i < count; ++i) {
        drawAxes();
        drawSnowman(false);
        glTranslatef(x, y, z);
        glScalef(SCALE, SCALE, SCALE);
        glTranslatef(x, y, z);
    }
}

void drawSnowmenOnOneAxe(bool independentRotation, bool oneOnlyRotates) {
    if (!(independentRotation || oneOnlyRotates))
        processRotation();

    glPushMatrix();
    drawAxes();
    glTranslatef(0, bodyRadius, 0);
    drawSnowman(independentRotation && !oneOnlyRotates);
    drawAxes();
    glTranslatef(bodyRadius, -bodyRadius, 0);
    glScalef(SCALE, SCALE, SCALE);
    glTranslatef(bodyRadius, bodyRadius, 0);
    drawSnowman(independentRotation && !oneOnlyRotates);
    drawAxes();
    glTranslatef(bodyRadius, -bodyRadius, 0);
    glScalef(SCALE, SCALE, SCALE);
    glTranslatef(bodyRadius, bodyRadius, 0);
    drawSnowman(oneOnlyRotates || independentRotation);

    glPopMatrix();
}

void drawSnowmanCrush() {
    processRotation();
    glRotatef(alfaX, 1.0, 0, 0);
    glRotatef(alfaY, 0.0, 1.0, 0.0);
    drawAxes();
    //body
    glPushMatrix();
    glColor3f(1.0,1.0,1.0);
    glutSolidSphere(bodyRadius, 50, 50);
    //head
    glTranslatef(bodyRadius+headRadius+headRadius*0.2f, 0, 0);
    glutSolidSphere(headRadius, 50, 50);
    //eyes
    glColor3f(0,0,0);
    glTranslatef(headRadius + eyeRadius + 0.2f * headRadius, 0, 0);
    glutSolidSphere(eyeRadius, 50, 50);
    glTranslatef(eyeRadius*2 + 0.2f * headRadius, 0, 0);
    glutSolidSphere(eyeRadius, 50, 50);
    glTranslatef(eyeRadius + headRadius, 0, 0);

    glColor3f(1, 0, 0);
    glutSolidCone(headRadius / 4.0, headRadius, 50,50);
    glColor3f(0, 0, 0);
    glPopMatrix();
}

void processRotation() {
    glRotatef(alfaX, 1.0, 0, 0);
    glRotatef(alfaY, 0.0, 1.0, 0.0);
}

void processSteppingForFunnnySnowman(bool directionIsRight) {
    float multiplier = directionIsRight ? 1 : -1;

    switch (currentFunState) {
        case CZ_1:
            steppingForFun+= FUN_STEP * multiplier;
            if (steppingForFun > bodyRadius + headRadius)
                currentFunState = CZ_2;
            else if (steppingForFun < 0)
                currentFunState = CZ_4;
            break;
        case CZ_2:
            steppingForFun-= FUN_STEP * multiplier;
            if (steppingForFun > bodyRadius + headRadius)
                currentFunState = CZ_1;
            else if (steppingForFun < 0)
                currentFunState = CZ_3;
            break;
        case CZ_3:
            steppingForFun-= FUN_STEP * multiplier;
            if (steppingForFun > 0)
                currentFunState = CZ_2;
            else if (steppingForFun < -(bodyRadius + headRadius))
                currentFunState = CZ_4;
            break;
        case CZ_4:
            steppingForFun+= FUN_STEP * multiplier;
            if (steppingForFun < -(bodyRadius + headRadius))
                currentFunState = CZ_3;
            else if (steppingForFun > 0)
                currentFunState = CZ_1;
            break;
        default:
            currentFunState = CZ_1;
            break;
    }
}

void drawFunnySnowman(){
    glPushMatrix();
    drawAxes();
    glColor3f(1, 1, 1);
    glutSolidSphere(bodyRadius, 50, 50);
    float yMultiplier = (currentFunState == CZ_1 || currentFunState == CZ_4) ? 1 : -1;
    float yTranslation = yMultiplier * sqrt(abs(pow(bodyRadius + headRadius, 2.0f) - pow(steppingForFun, 2.0f)));
    glTranslatef(steppingForFun, yTranslation, 0);
    drawSnowmanHead();
    glPopMatrix();
}

void drawFourSnowmen(){
    glPushMatrix();
    drawAxes();
    drawSnowman(true); //c1

    glRotatef(alfaY, 0, 1.0, 0);
    glTranslatef(bodyRadius * 2, 0, 0); //c2
    glScalef(SCALE, SCALE, SCALE);
    drawAxes();
    drawSnowman(true);

    glPushMatrix();
    glRotatef(alfaY, 0, 0, 1.0);
    glTranslatef(0, bodyRadius*3, 0); //c3
    glScalef(SCALE, SCALE, SCALE);
    drawAxes();
    drawSnowman(true);
    glPopMatrix();

    glRotatef(alfaY, 0, 1.0, 0);
    glTranslatef(bodyRadius*2, bodyRadius+headRadius, 0);
    float doubledScale = SCALE * SCALE;
    glScalef(doubledScale, doubledScale, doubledScale);
    drawAxes();
    drawSnowman(true);

    glPopMatrix();
}