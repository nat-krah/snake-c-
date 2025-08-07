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
    - Some problems with inputing data, misleading instructions
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

//Create a queue to hold inputed chars
queue<char> inputChar;

//Pogram handling
int millis = 1000;
bool isGameOver = false;
bool readKeyBoardOn = true;

//Game state
int startWidth = 1;
int startHeight = 1;
int width = 0;
int height = 0;
int score = 0;

int speed = 50;
int maxSpeed = 100;
int minSpeed = 0;

//Snake Variables
char mainC = '*';
char headC = '*';
int length = 10;
int direction = 0;

int fruitX = 0;
int fruitY = 0;
int fruitChar = '0';


/***************************************************************************\
*                                                                           *
*                              General function                             *
*                                                                           *
\***************************************************************************/


//Generate a sudo random number from x to y
int randi(int x, int y){
    srand(time(0));
    int randNum = x + (rand() % (y - x + 1));
    return randNum;
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

//Check if the snake collided with itself
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

//Generate fruit
void fruitGen(snakeNode* snakeHead){
    bool fruitWorks = false;
    int newX = 0;
    int newY = 0;

    int i = 0; //If it takes over 1000 cycles to find a fruit location, then stop looking so it doesn't hang the program
    while (!fruitWorks && i < 1000){
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
        i++;
    }
    
    if (!fruitWorks){
        moveCurser(1,1);
        printf("No fruit spawned");
        return;

    }

    fruitX = newX;
    fruitY = newY;
    mvp(fruitX, fruitY, fruitChar);
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

//Get size of terminal
void get_terminal_size(int& width, int& height) {
    struct winsize w;
    ioctl(fileno(stdout), TIOCGWINSZ, &w);
    width = (int)(w.ws_col);
    height = (int)(w.ws_row);
}

void drawBoarder(int x = width, int y = height, bool ceneterY = true){
    int xOffset = 0;
    int yOffset = 0;

    if (x >= width || x < 3){
        xOffset = 1;
        x = width ; // Might need to be width - 1 to account for going up to the edge
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
            } else if (j == xOffset || j == x + xOffset){
                printf("|");
            }    
        }
    }

    startWidth = xOffset + 1;
    startHeight = yOffset + 1;
    width = xOffset + x - 1;
    height = yOffset + y - 1;
}


//This needs more work
void introsequence(){
    //Boolean checker for intro sequence
    char yOrN;

    //Start intro sequence
    clearScreen();
    snake3D();
    controls();

    printf("Use intro sequence (y/n): ");
    cin >> yOrN;

    if (!(yOrN == 'y')){
        printf("Press enter to start");
        cin >> yOrN;

        hideCurser();
        clearScreen();
        get_terminal_size(width, height);
        enableRawMode();
        drawBoarder(70, 35);

        moveCurser((width - startWidth + startWidth * 2) / 2 - 11, 1);
        printf(" Score: %d  ", score);
        printf("Speed: %d ", speed);
        return;
    }


    //Start speed

    //Get input from user
    char input[5];
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
        printf("Press enter to start");
        getchar();

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
        cin >> inputedWidth;
        printf("Select width: ");
        cin >> inputedHeight;
        printf("Center along y axis (y/n): ");
        cin >> yOrN;

        printf("Press enter to start");
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
//Should add a thread sleep to reduce cpu usage for such a simple thread
void readKeyboard(){
    char c = ' ';
    while(readKeyBoardOn){
        if (readKeyBoardOn || !isGameOver){
            cin >> c;
        }
        
        moveCurser((width - startWidth + startWidth * 2) / 2 - 11, 1);
        printf(" Score: %d  ", score);
        printf("Speed: %d ", speed);
        
        

        //First check if the inputed char is a control code
        //If it is, do that action
        //Else push the key onto the queue for the other function
        if (c == 'q'){
            isGameOver = true;
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
    fruitGen(snakeHead);

    //Start second thread to read keyboard inputs from user
    thread t1(readKeyboard);

    //Print the entire snake
    printSnake(snakeHead);

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

        mvp(n->x, n->y, headC);
        mvp(n->next->x, n->next->y, mainC);

        if (newX == fruitX && newY == fruitY){ //Check if the snake has hit a piece of fruit
            //If so, generate a new fruit, and increase the score and length
            fruitGen(snakeHead);
            score++;
            length++;
        } else {
            //If the user did not hit a fruit, remove the last node of the snake
            mvp(snakeEnd->x, snakeEnd->y, ' ');

            snakeEnd = snakeEnd->previous;
            delete snakeEnd->next;
        }

        //Sleep to not hog cpu, and set the speed of the game
        usleep((115 - speed) * millis);
    }

    //After game has ended, stop keyboard reading thread and return the terminal to its previous conditions
    readKeyBoardOn = false;
    t1.join();
    showCurser();
    disableRawMode();
}
