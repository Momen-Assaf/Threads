#include "header.h"

typedef struct {
    float x;
    float y;
    float angle;
    int speed;
    int phermone;
} Ant;

typedef struct {
    float x;
    float y;
} Food;

int numAnts = 50;
int foodSpawnInterval = 1; // Default: 1 minute
int foodEatInterval = 55;
int phoeromoneInterval_0 = 50,phoeromoneInterval_1 = 25;
Ant* ants;
Food* foods;
int numFoods = 0;
pthread_mutex_t antMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t foodMutex = PTHREAD_MUTEX_INITIALIZER;

void initializeAnts() {
    ants = malloc(numAnts * sizeof(Ant));

    for (int i = 0; i < numAnts; i++) {
        ants[i].x = rand() % (WINDOW_WIDTH - ANT_WIDTH);
        ants[i].y = rand() % (WINDOW_HEIGHT - ANT_HEIGHT);
        ants[i].angle = rand() % 360;
        ants[i].speed = rand() % 10 + 1;
    }
}

void initializeFoods() {
    foods = malloc(numFoods * sizeof(Food));
}

void foodDetected(intptr_t antIndex, int target){
    ants[antIndex].phermone = phoeromoneInterval_0;

    float dx = foods[target].x - ants[antIndex].x;
    float dy = foods[target].y - ants[antIndex].y;

    float targetAngle = atan2(dy,dx) * (180/M_PI);
    ants[antIndex].angle = targetAngle;
    
}

void phoeromoneDetected(intptr_t antIndex, int target){
    ants[antIndex].phermone = phoeromoneInterval_1;

    float dx = ants[target].x - ants[antIndex].x;
    float dy = ants[target].y - ants[antIndex].y;

    float targetAngle = atan2(dy,dx) * (180/M_PI);
    ants[antIndex].angle = targetAngle;
}

float calculateDistance(int antIndex1, int antIndex2) {
    float dx = ants[antIndex1].x - ants[antIndex2].x;
    float dy = ants[antIndex1].y - ants[antIndex2].y;
    return sqrt(dx * dx + dy * dy);
}

void calculateFoodDistance(intptr_t antIndex){
    pthread_mutex_lock(&antMutex);
    pthread_mutex_lock(&foodMutex);

    for(int i = 0; i < numFoods; i++){
            float dx = ants[antIndex].x - foods[i].x;
            float dy = ants[antIndex].y - foods[i].y;
            float distance = sqrt( dx*dx + dy*dy);

            if( distance < foodEatInterval){
                foodDetected(antIndex,i);
            }
            
        }
    for(int i = 0; i < numAnts; i++){
        if( i != antIndex){
            float antsDistance = calculateDistance(antIndex,i);
            if(antsDistance < ants[i].phermone){
                phoeromoneDetected(antIndex,i);
            }
        }
    }  
    pthread_mutex_unlock(&foodMutex);
    pthread_mutex_unlock(&antMutex);
}


void* updateAntPosition(void* arg) {
    intptr_t antIndex = (intptr_t)arg;

    while (1) {
        pthread_mutex_lock(&antMutex);

        float radians = ants[antIndex].angle * (M_PI / 180.0);
        float dx = ants[antIndex].speed * cos(radians);
        float dy = ants[antIndex].speed * sin(radians);

        float prevX = ants[antIndex].x;
        float prevY = ants[antIndex].y;

        ants[antIndex].x += dx;
        ants[antIndex].y += dy;

        // Check if the ant hits the window boundaries
        if (ants[antIndex].x < 0 || ants[antIndex].x > WINDOW_WIDTH - ANT_WIDTH ||
            ants[antIndex].y < 0 || ants[antIndex].y > WINDOW_HEIGHT - ANT_HEIGHT) {

            // Adjust the position to the boundary and update the angle
            if (ants[antIndex].x < 0)
                ants[antIndex].x = 0;
            else if (ants[antIndex].x > WINDOW_WIDTH - ANT_WIDTH)
                ants[antIndex].x = WINDOW_WIDTH - ANT_WIDTH;

            if (ants[antIndex].y < 0)
                ants[antIndex].y = 0;
            else if (ants[antIndex].y > WINDOW_HEIGHT - ANT_HEIGHT)
                ants[antIndex].y = WINDOW_HEIGHT - ANT_HEIGHT;

            ants[antIndex].angle += (rand() % 2 == 0 ? -1 : 1) * 45;
        }

        pthread_mutex_unlock(&antMutex);
        calculateFoodDistance(antIndex);
        usleep(UPDATE_INTERVAL_MS * 1000); // Delay to control the speed of ants
    }

    return NULL;
}

void drawAnts() {
    glClear(GL_COLOR_BUFFER_BIT);

    pthread_mutex_lock(&antMutex);

    for (int i = 0; i < numAnts; i++) {
        glColor3f(0.0f, 0.0f, 1.0f); // Blue color for ants

        // Draw the ant body (rectangle)
        float left = ants[i].x;
        float right = ants[i].x + ANT_WIDTH;
        float top = ants[i].y + ANT_HEIGHT;
        float bottom = ants[i].y;

        glRectf(left, bottom, right, top);

        // Draw the ant head (circle)
        float centerX = ants[i].x + ANT_WIDTH / 2.0;
        float centerY = ants[i].y + ANT_HEIGHT / 2.0;
        float radius = ANT_WIDTH / 4.0;

        glColor3f(1.0f, 0.0f, 0.0f); // Red color for ant head

        glBegin(GL_POLYGON);
        for (int j = 0; j < 360; j++) {
            float radians = j * (M_PI / 180.0);
            float x = centerX + radius * cos(radians);
            float y = centerY + radius * sin(radians);
            glVertex2f(x, y);
        }
        glEnd();
    }

    pthread_mutex_unlock(&antMutex);

    pthread_mutex_lock(&foodMutex);

    for (int i = 0; i < numFoods; i++) {
        glColor3f(0.0f, 1.0f, 0.0f); // Green color for food

        // Draw the food (circle)
        float centerX = foods[i].x + ANT_WIDTH / 2.0;
        float centerY = foods[i].y + ANT_HEIGHT / 2.0;
        float radius = ANT_WIDTH / 4.0;

        glBegin(GL_POLYGON);
        for (int j = 0; j < 360; j++) {
            float radians = j * (M_PI / 180.0);
            float x = centerX + radius * cos(radians);
            float y = centerY + radius * sin(radians);
            glVertex2f(x, y);
        }
        glEnd();
    }

    pthread_mutex_unlock(&foodMutex);

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

void spawnFood(int value) {
    pthread_mutex_lock(&foodMutex);

    numFoods++;
    foods = realloc(foods, numFoods * sizeof(Food));
    foods[numFoods - 1].x = rand() % (WINDOW_WIDTH - ANT_WIDTH);
    foods[numFoods - 1].y = rand() % (WINDOW_HEIGHT - ANT_HEIGHT);

    pthread_mutex_unlock(&foodMutex);
    glutTimerFunc(foodSpawnInterval * 60 * 1000, spawnFood, 0);
    
}

int main(int argc, char** argv) {
    if (argc >= 2) {
        numAnts = atoi(argv[1]);
    }

    if (argc >= 3) {
        foodSpawnInterval = atoi(argv[2]);
    }

    if (argc >= 4) {
        foodEatInterval = atoi(argv[3]);
    }

    FILE *user_file = fopen("user_defined.txt", "r");
    char buffer[128];
    char* token;
    
        if(user_file == NULL){
          perror("Error opening user_defined.txt file");
        }
        else{
          while(fgets(buffer,128,user_file) != NULL)
          {
            token = strtok(buffer,",");  
            numAnts = atoi(token);
            token = strtok(NULL, ",");
            foodSpawnInterval = atoi(token);
            token = strtok(NULL,",");
            foodEatInterval = atoi(token);
            token = strtok(NULL,",");
            phoeromoneInterval_0 = atoi(token);
            token = strtok(NULL,",");
            phoeromoneInterval_1 = atoi(token);
          }
        }
    printf("The number of ants:%d\n",numAnts);
    printf("The food spawn interval is:%d\n",foodSpawnInterval);
    printf("The food eat interval is:%d\n",foodEatInterval);
    printf("The pheromone interval 1 is:%d\n",phoeromoneInterval_0);
    printf("The pheromone interval 2 is:%d\n",phoeromoneInterval_1);


    srand(time(NULL));

    initializeAnts();
    initializeFoods();

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
        pthread_create(&threads[i], NULL, updateAntPosition, (void*)(intptr_t)i);
    }

    glutTimerFunc(foodSpawnInterval * 60 * 1000, spawnFood, 0);

    glutMainLoop();

    free(threads);
    free(ants);
    free(foods);

    return 0;
}