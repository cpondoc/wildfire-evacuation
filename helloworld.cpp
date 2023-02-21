#include <algorithm>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <vector> 

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
    //drawGridworld();
	
	// Initialize random seed
	srand (time(NULL));
	
	/*
	Initial stochastic model for burning of the fire
	*/
	    
    // Defining burning rate
    int burningRate = 5;
    int totalFuel = 50;
    
    for (int i = 0; i < 10; i++) {
    	// Generate random number and decide to turn on or off with even probability
		int burningBool = rand() % 10 + 1;
    	if (burningBool % 2 == 0) {
    		totalFuel = max(0, totalFuel - burningRate);
		}
		
		cout << "Remaining fuel left: " << totalFuel << endl;
	}
    
    return 0;
}
