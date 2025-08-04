#include <iostream>

using namespace std;

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

void mvp(int x, int y, char c){
    moveCurser(x, y);
    cout << c << endl;
}