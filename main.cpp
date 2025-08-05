#include <iostream>
#include <thread>
#include <queue>
#include <sys/ioctl.h>
#include <unistd.h>

#include "tComands.cpp"
#include "rawmode.cpp"
#include "asciiArt.cpp"


/*
Work to do:
- rewrite fruitGen()
- fix intro sequence
- bugs
    - hitting the bottom on the game causes funny graphics
*/


/***************************************************************************\
*                                                                           *
*                                  Variables                                *
*                                                                           *
\***************************************************************************/


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
int width = 0;
int height = 0;
int heightBuffer = 2;
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
        if ((newX == t->x && newY == t->y) || (newX < 0 || newY < heightBuffer || newX > width || newY > height)){
            isGameOver = true;
            return true;
        }
        t = t->next;
    }
    return false;
}

//This is eneffiecient and should be rewritten
void fruitGen(snakeNode* snakeHead){
    bool fruitWorks = false;
    int newX = 0;
    int newY = 0;

    int i = 0;
    while (!fruitWorks && i < 50){
        newX = randi(0, width);
        newY = randi(heightBuffer, height);

        bool collision = false;
        snakeNode* tS = snakeHead;
        for (int i = 0; i < length; i ++){
            if (tS->x == newX && tS->y == newY){
                collision = true;
                break;
            }
            tS = tS->next;
        }

        if (collision){
            fruitWorks = true;
        }
        i++;
    }

    fruitX = newX;
    fruitY = newY;

    mvp(fruitX, fruitY, fruitChar);
}

//Initialize the snake
pair<snakeNode*, snakeNode*> snakeINI(int startPos[2]){
    snakeNode* snakeHead = new snakeNode;
    snakeHead->x = startPos[0];
    snakeHead->y = startPos[1];

    snakeNode* t = snakeHead;
    for (int i = 1; i < length + 1; i++){
        snakeNode* n = new snakeNode;
        n->x = startPos[0] - i;
        n->y = startPos[1];

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



//This needs more work
void introsequence(){
    
    //Check if we should use intro sequence
    char yOrN;

    printf("Use intro sequence (y/n): ");
    cin >> yOrN;

    if (!(yOrN == 'y')){
        hideCurser();
        clearScreen();
        get_terminal_size(width, height);
        return;
    }

    //Start intro sequence
    clearScreen();
    snake3D();
    controls();


    //Start speed

    //Get input from user
    char input[5];
    printf("Set start speed (0 - 100): ");
    cin >> input;

    /*
    //Clean any non ints from it
    int index = 0;
    char save[5];
    for(int i = 0; i < 6; i++){
        if (int(input[i]) >= 48 && int(input[i]) <= 57){
            save[index] = input[i];
            index++;
        }
    }*/

    moveCurser(1,1);
    printf("From input: %d, input: %s", speed, input);
    usleep(4000 * millis);

    //Convert it to a string
    speed = stoi(input);

    moveCurser(1,1);
    printf("After stoi %d", speed);
    usleep(4000 * millis);


    //If speed is out of bounds, fix it
    if (speed > maxSpeed){
        speed = maxSpeed;
    } else if (speed < minSpeed){
        speed = minSpeed;
    }

    moveCurser(1,1);
    printf("%d", speed);
        usleep(4000 * millis);


/*

    //Custom size
    printf("Set custom size (y/n): ");
    cin >> yOrN;
    if (yOrN == 'y'){
        char screenSizeIn[250];
        printf("Select size (x,y); space for default: ");
        //set width and height her based in parsing of the inputed values
    } else{

    }

    //This is left out until parcing is implremented
    //Get the size of the terminal
    hideCurser();
    get_terminal_size(width, height);
    printf("%d, %d", width, height);

    //Display the controls and wait until a char in imputed
    //controls();
    moveCurser(0, 0);

    usleep(500 * millis);
    getchar();
    getchar();


    //Clear screen before clearing box
    clearScreen();

    /*
    //print walls
    moveCurser(0, 2);
    for (int i = 0; i < width; i++){
        cout << "#";
        for(int j = 0; j < width - 4; j++){
            if (i == 0 || i == height - 1){
                cout << "#";
            } else{
                cout << " ";
            }
        }
        cout << "#\n";
    }*/
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
        moveCurser((width - 9) / 2, 0);
        printf("Score: %d  ", score);
        printf("Speed: %d  ", speed);
        
        if (readKeyBoardOn || !isGameOver){
            cin >> c;
        }

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
    if (1){
        introsequence();
    } else{
        hideCurser();
        clearScreen();
        get_terminal_size(width, height);
    }

    //After startup, enable start raw mode
    enableRawMode();

    //Initialize the snake
    int startPos[2] = {width / 2, 20};
    pair<snakeNode*, snakeNode*> result = snakeINI(startPos);

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