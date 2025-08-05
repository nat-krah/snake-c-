#include <iostream>

using namespace std;

//All of these functions use ansi escape codes

void hideCurser(){
    cout << "\033[?25l" << endl;
}

void showCurser(){
    cout << "\033[?25h" << endl;
}

void moveCurser(int x, int y){
    cout << "\033[" << y << ";" << x << "H" << endl;
}

void clearScreen(){
    cout << "\033[2J" << endl;
    moveCurser(0,1);
}

//Move and place a char
void mvp(int x, int y, char c){
    moveCurser(x, y);
    cout << c << endl;
}

//Reset text modes and colors
void resetText(){
    cout << "\033[0m";
}


/*
Set foreground (text) color using 256 color options

0-7: standard colors (as in ESC [ 30–37 m)
8–15: high intensity colors (as in ESC [ 90–97 m)
16-231: 6 × 6 × 6 cube (216 colors): 16 + 36 × r + 6 × g + b (0 ≤ r, g, b ≤ 5)
Some emulators interpret these steps as linear increments (256 / 24) on all three channels while others may explicitly define these values.
232-255: grayscale from dark to light in 24 steps.
*/
void setFColor256(int id){
    cout << "\033[38;5;" << id << "m";
}

/*
Set background color using 256 color options

0-7: standard colors (as in ESC [ 30–37 m)
8–15: high intensity colors (as in ESC [ 90–97 m)
16-231: 6 × 6 × 6 cube (216 colors): 16 + 36 × r + 6 × g + b (0 ≤ r, g, b ≤ 5)
Some emulators interpret these steps as linear increments (256 / 24) on all three channels while others may explicitly define these values.
232-255: grayscale from dark to light in 24 steps.
*/
void setBColor256(int id){
    cout << "\033[48;5;" << id << "m";
}

/*
Set foreground (text) color used 24 bit rgb color
*/
void setFColorRGB(int r, int g, int b){
    cout << "\033[38;2;" << r << ";" << g << ";" << b << "m";
}

/*
Set background color used 24 bit rgb color
*/
void setBColorRGB(int r, int g, int b){
    cout << "\033[48;2;" << r << ";" << g << ";" << b << "m";
}