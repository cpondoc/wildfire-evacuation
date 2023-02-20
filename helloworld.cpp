#include <iostream>
#include <vector>
#include <string>

using namespace std;

/*
Function to help draw grid on the screen
*/
void drawGridworld() {
    cout << "Picture of Grid World" << endl;

    // Simple CLI grid
    int gridDim = 10;
    for (int i = 0; i < 1 + gridDim; i++) {
        for (int j = 0; j < gridDim; j++) {
            if (i == 0) {
                cout << "__";
            } else {
                cout << "|_";
            }
        }
        if (i > 0) {
            cout << "|" << endl;
        } else {
            cout << "_" << endl;
        }
    }
}

int main()
{
    // Initial function to draw grid world
    drawGridworld();
    return 0;
}