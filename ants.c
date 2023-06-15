#include "header.h"

typedef struct {
    float x;
    float y;
    float angle;
    int speed;
} Ant;

int numAnts = 50;
Ant* ants;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void initializeAnts() {
    ants = malloc(numAnts * sizeof(Ant));

    for (int i = 0; i < numAnts; i++) {
        ants[i].x = rand() % (WINDOW_WIDTH - ANT_WIDTH);
        ants[i].y = rand() % (WINDOW_HEIGHT - ANT_HEIGHT);
        ants[i].angle = rand() % 360;
        ants[i].speed = rand() % 10 + 1;
    }
}

void* updateAntPosition(void* arg) {
    int antIndex = (int)arg;
    free(arg);

    while (1) {
        pthread_mutex_lock(&mutex);

        float radians = ants[antIndex].angle * (3.1415926535 / 180.0);
        float dx = ants[antIndex].speed * cos(radians);
        float dy = ants[antIndex].speed * sin(radians);

        ants[antIndex].x += dx;
        ants[antIndex].y += dy;

        // Check if the ant hits the window boundaries
        if (ants[antIndex].x < 0 || ants[antIndex].x > WINDOW_WIDTH - ANT_WIDTH) {
            ants[antIndex].angle += 45;
        }

        if (ants[antIndex].y < 0 || ants[antIndex].y > WINDOW_HEIGHT - ANT_HEIGHT) {
            ants[antIndex].angle += 45;
        }

        pthread_mutex_unlock(&mutex);

        usleep(UPDATE_INTERVAL_MS * 1000); // Delay to control the speed of ants
    }

    return NULL;
}

void drawAnts() {
    glClear(GL_COLOR_BUFFER_BIT);

    for (int i = 0; i < numAnts; i++) {
        glColor3f(0.0f, 0.0f, 1.0f); // Blue color for ants

        float left = ants[i].x;
        float right = ants[i].x + ANT_WIDTH;
        float top = ants[i].y + ANT_HEIGHT;
        float bottom = ants[i].y;

        glRectf(left, bottom, right, top);
    }

    glFlush();
    glutSwapBuffers();
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void update(int value) {
    glutPostRedisplay();
    glutTimerFunc(UPDATE_INTERVAL_MS, update, 0);
}

int main(int argc, char** argv) {
    if (argc >= 2) {
        numAnts = atoi(argv[1]);
    }

    srand(time(NULL));

    initializeAnts();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Ants Simulation");

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // White background color

    glutDisplayFunc(drawAnts);
    glutReshapeFunc(reshape);
    glutTimerFunc(UPDATE_INTERVAL_MS, update, 0);

    pthread_t* threads = malloc(numAnts * sizeof(pthread_t));
    for (int i = 0; i < numAnts; i++) {
        int* antIndex = malloc(sizeof(int));
        *antIndex = i;
        pthread_create(&threads[i], NULL, updateAntPosition, antIndex);
    }

    glutMainLoop();

    free(threads);
    free(ants);

    return 0;
}