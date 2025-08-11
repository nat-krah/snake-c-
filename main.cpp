#include <cstdio>
#include <iostream>
#include <thread>
#include <queue>
#include <sys/ioctl.h>
#include <unistd.h>

#include "tComands.cpp"
#include "rawmode.cpp"
#include "asciiArt.cpp"

/***************************************************************************\
*                                                                           *
*                                  Variables                                *
*                                                                           *
\***************************************************************************/


/*
Bugs
- Introsequence
    - No data cleaning, so if someone inputs wrong data type then things break
*/

//Snake structure
struct snakeNode{
    int x;
    int y;

    snakeNode* next;
    snakeNode* previous;
};

//Windowsize structure
struct winsize size;

//Queue to hold inputs, played by readKeyboard thread, pop by main thread
queue<char> inputChar;

//Pogram handling
int millis = 1000;
bool isGameOver = false;
bool readKeyBoardOn = true;

//Game state
int startWidth = 1; //Starting x position of game
int startHeight = 1; //Starting y position of game
int width = 0; //Width of playable game
int height = 0; //Height of playable game
int score = 0;

int speed = 50;
int maxSpeed = 100;
int minSpeed = 0;

//Snake Variables
char mainC = '*'; //Char used to represent main body
char headC = '*'; //Char used to represent head of body
int length = 10;
int direction = 0; //0 up; 1 left; 2 down; 3 right

//Fruit variables
int fruitChar = '*'; //Char used to represent fruit
int fruitX = 0;
int fruitY = 0;


/***************************************************************************\
*                                                                           *
*                              General function                             *
*                                                                           *
\***************************************************************************/


//Generate a sudo random number from x to y
int randi(int x, int y){
    srand(time(0));
    return x + (rand() % (y - x + 1));
}

//Flush standard input until enter key was hit
void flushinp(){
    while (getchar() != '\n'){}
}

//Get size of terminal
void get_terminal_size(int& width, int& height) {
    struct winsize w;
    ioctl(fileno(stdout), TIOCGWINSZ, &w);
    width = (int)(w.ws_col);
    height = (int)(w.ws_row);
}

//Walk down the linked list snake and print each charater
void printSnake(snakeNode* snakeHead){
    snakeNode* t = snakeHead; 
    for (int i = 1; i < length + 1; i++){
        mvp(t->x, t->y, mainC);
        t = t->next;
    }

    mvp(snakeHead->x, snakeHead->y, headC);
}

//Check if the snake collided with itself or the outer bounds of the game
bool checkCollision(snakeNode* snakeHead, int newX, int newY){
    snakeNode* t = snakeHead;
    for (int i = 0; i < length + 1; i++){
        if ((newX == t->x && newY == t->y) || (newX < startWidth || newY < startHeight || newX > width || newY > height)){
            isGameOver = true;
            return true;
        }
        t = t->next;
    }
    return false;
}

//Initialize the snake
pair<snakeNode*, snakeNode*> snakeINI(int startX, int startY){
    snakeNode* snakeHead = new snakeNode;
    snakeHead->x = startX;
    snakeHead->y = startY;

    snakeNode* t = snakeHead;
    for (int i = 1; i < length + 1; i++){
        snakeNode* n = new snakeNode;
        n->x = startX - i;
        n->y = startY;

        n->previous = t;
        t->next = n;
        t = t->next;
    }

    snakeNode* snakeEnd = t;

    return make_pair(snakeHead, snakeEnd);
}

//Generate fruit
void fruitGen(snakeNode* snakeHead){
    bool fruitWorks = false;
    int newX = 0;
    int newY = 0;

    while (!fruitWorks){
        newX = randi(startWidth, width);
        newY = randi(startHeight, height);

        bool collision = false;
        snakeNode* tS = snakeHead;
        for (int i = 0; i < length; i ++){
            if (tS->x == newX && tS->y == newY){
                collision = true;
                break;
            }
            tS = tS->next;
        }

        if (!collision){
            fruitWorks = true;
        }
    }

    fruitX = newX;
    fruitY = newY;
    mvp(fruitX, fruitY, fruitChar);
}

//Draws the board, defaults to the terminal size and centering the y axis
void drawBoarder(int x = width, int y = height, bool ceneterY = true){
    int xOffset = 0;
    int yOffset = 0;

    setFColorRGB(200, 200, 200);
    setBColorRGB(200, 200, 200);

    if (x >= width || x < 3){
        xOffset = 1;
        x = width ;
    } else{
        xOffset = (width - x)/2;
    }

    if (y >= height || y < 3){
        yOffset = 1;
        y = height - 2;
    } else{
        if (ceneterY){
            yOffset = (height - y) / 2;
        } else{
            yOffset = 1;
        }
    }

    for (int i = yOffset; i <= y + yOffset; i++){
        for (int j = xOffset; j <= x + xOffset; j++){
            moveCurser(j, i);

            if (i == yOffset || i == y + yOffset){
                printf("-");
            } else if (j == xOffset || j == (xOffset + 1) || j == x + xOffset || j == (x + xOffset - 1)){
                printf("|");
            }    
        }
    }

    startWidth = xOffset + 2;
    startHeight = yOffset + 1;
    width = xOffset + x - 2;
    height = yOffset + y - 1;

    resetText();
}


//Intro sequence to start off game
void introsequence(){
    //Boolean checker for intro sequence
    char yOrN;

    //Display basic game info 
    clearScreen();
    snake3D();
    controls();

    printf("Start intro sequence to costomize game (y/n): ");
    cin >> yOrN;

    //If user selected to not do any costomization, setup the game state and exit the function
    if (!(yOrN == 'y')){
        enableRawMode();

        printf("Press a letter to start");
        flushinp();
        cin >> yOrN;

        hideCurser();
        clearScreen();
        get_terminal_size(width, height);
        drawBoarder(70, 35);

        moveCurser((width - startWidth + startWidth * 2) / 2 - 11, 1);
        printf(" Score: %d  ", score);
        printf("Speed: %d ", speed);
        return;
    }


    //Start speed

    //Get input from user
    char input[5];
    flushinp();
    printf("Set start speed (0 - 100): ");
    cin >> speed;

    if (speed > maxSpeed){
        speed = maxSpeed;
    } else if (speed < minSpeed){
        speed = minSpeed;
    }

    //Custom size

    printf("Set custom size (y/n): ");
    cin >> yOrN;
    if (!(yOrN == 'y')){
        enableRawMode();

        printf("Press a letter to start");
        flushinp();
        cin >> yOrN;

        clearScreen();
        hideCurser();
        get_terminal_size(width, height);
        enableRawMode();
        drawBoarder(70, 35);
    } else{
        int inputedWidth = 0;
        int inputedHeight = 0;
        bool ceneterY = true;

        printf("Select height: ");
        flushinp();
        cin >> inputedWidth;
        printf("Select width: ");
        flushinp();
        cin >> inputedHeight;
        printf("Center along y axis (y/n): ");
        flushinp();
        cin >> yOrN;

        printf("Press a letter to start");
        flushinp();
        cin >> yOrN;


        if (!(yOrN == 'y')){
            ceneterY = false;
        }

        clearScreen();
        hideCurser();
        get_terminal_size(width, height);
        enableRawMode();
        drawBoarder(inputedWidth, inputedHeight, ceneterY);
    }
    
    moveCurser((width - startWidth + startWidth * 2) / 2 - 11, 1);
    printf(" Score: %d  ", score);
    printf("Speed: %d ", speed);
}


/***************************************************************************\
*                                                                           *
*                                   Threads                                 *
*                                                                           *
\***************************************************************************/


//A thread dedicated to reading keyboard inputs and putting them in a queue for the main program to read
void readKeyboard(){
    char c = ' ';
    while(readKeyBoardOn){
        if (readKeyBoardOn || !isGameOver){
            cin >> c;
        }
        
        //Display game info
        moveCurser((width - startWidth + startWidth * 2) / 2 - 11, 1);
        printf(" Score: %d  ", score);
        printf("Speed: %d ", speed);

        //First check if the inputed char is a control code
        //If it is, do that action
        //Else push the key onto the queue for the other function
        if (c == 'q'){
            isGameOver = true;
            disableRawMode();
        } else if (c == 'e' && speed < maxSpeed){
            speed = speed + 5;
        } else if (c == 'f' && speed > minSpeed){
            speed = speed - 5;
        } else {
            inputChar.push(c);
        }

        usleep(50 * millis);
    }
}

int main(){
    //Start intro sequence
    introsequence();

    //Initialize the snake
    int startX = startWidth + (width - startWidth) / 2;
    int startY = startHeight + (height - startHeight) / 2;
    pair<snakeNode*, snakeNode*> result = snakeINI(startX, startY);

    snakeNode* snakeHead = result.first;
    snakeNode* snakeEnd = result.second;

    //Generate first fruit
    setFColorRGB(255, 0, 0);
    fruitGen(snakeHead);

    //Start second thread to read keyboard inputs from user
    thread t1(readKeyboard);

    //Print the entire snake
    setFColorRGB(0,255, 0);
    printSnake(snakeHead);
    resetText();

    //Main game loop
    while(!isGameOver){
        //Initialize variables
        int lastDirection = direction;
        int newX;
        int newY;
        char c = ' ';

        //Populate c if a button has been pressed
        if(!inputChar.empty()){
            c = inputChar.front();
            inputChar.pop();
        } else{
            c = ' ';
        }

        //Set direction based on charater and calculate
        switch (c){
            case 'w':
                direction = 0;
                break;
            case 'a':
                direction = 1;
                break;
            case 's':
                direction = 2;
                break;
            case 'd':
                direction = 3;
                break;
        }

        //If user turned on themsevles, revert
        if (direction == 0 && lastDirection == 2 || 
            direction == 1 && lastDirection == 3 || 
            direction == 2 && lastDirection == 0 || 
            direction == 3 && lastDirection == 1){

            direction = lastDirection;
        }

        //Update newX and newY based on direction
        switch (direction){
            case 0:
                newX = snakeHead->x;
                newY = snakeHead->y - 1;
                break;
            case 1:
                newX = snakeHead->x - 1;
                newY = snakeHead->y;
                break;
            case 2:
                newX = snakeHead->x;
                newY = snakeHead->y + 1;
                break;
            case 3:
                newX = snakeHead->x + 1;
                newY = snakeHead->y;
                break;
        }

        //Check if the snake has collided with something
        checkCollision(snakeHead,  newX,  newY); 
        
        //If we collided, end game before moving snake
        if (isGameOver){
            break;
        }

        //Create new head and move linked list along and display new head location
        snakeNode* n = new snakeNode;
        n->x = newX;
        n->y = newY;

        n->next = snakeHead;
        snakeHead->previous = n;
        snakeHead = n;

        setFColorRGB(0, 255, 0);
        
        mvp(n->x, n->y, headC);
        mvp(n->next->x, n->next->y, mainC);


        resetText();

        if (newX == fruitX && newY == fruitY){ //Check if the snake has hit a piece of fruit
            //If so, generate a new fruit, and increase the score and length
            setFColorRGB(255, 0, 0);
            fruitGen(snakeHead);
            resetText();
            score++;
            length++;
        } else {
            //If the user did not hit a fruit, remove the last node of the snake
            resetText();
            mvp(snakeEnd->x, snakeEnd->y, ' ');

            snakeEnd = snakeEnd->previous;
            delete snakeEnd->next;
        }

        //Sleep to not hog cpu, and set the speed of the game
        usleep((115 - speed) * millis);
    }

    usleep(500 *millis);

    //After game has ended, stop keyboard reading thread and return the terminal to its previous conditions
    readKeyBoardOn = false;
    t1.join();
    clearScreen();
    showCurser();
    disableRawMode();
}
