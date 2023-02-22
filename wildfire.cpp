/*
File: wildfire.cpp
Initial start with modeling wildfire spread!
*/
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <vector> 

using namespace std;
//bruh
/*
Function to help draw grid on the screen
*/
void drawGridworld(int gridDim) {
    cout << "Picture of Grid World" << endl;

    // Simple CLI grid
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

/*
Function to help with boolean variable signaling of cell burning
*/
bool isCellBurning() {
	return ((rand() % 10 + 1) % 2 == 0);
}

/*
Function to calculte Euclidean distance between two points
*/
float calculateDistance(int xOne, int yOne, int xTwo, int yTwo) {
	return sqrt(pow(xOne - xTwo, 2) + pow((yOne - yTwo), 2));
}

int main()
{	
	// Initialize random seed, grid dimension, and constant for proportionality of distance
	srand (time(NULL));
	int gridDim = 5;
	int distanceConstant = 3;
	
	/*
	Initial stochastic model for fuel of a cell
	*/
	cout << "Modeling of amount of fuel!" << endl << endl;
	    
    // Defining burning rate
    int burningRate = 5;
    int totalFuel = 50;
    
    // Changing of the amout of fuel
    for (int i = 0; i < 10; i++) {
		// Check if cell is burning and reduce accordingly
    	if (isCellBurning()) {
    		totalFuel = max(0, totalFuel - burningRate);
		}
		
		cout << "Remaining fuel left: " << totalFuel << endl;
	}
	
	/*
	Initial stochastic model for movement of the fire
	*/
	cout << endl << "Modeling spread of the fire!" << endl << endl;
	
	// Generate random points on the plane
	int x = rand() % gridDim;
	int y = rand() % gridDim;
	
	// Investigate all existing points around it
	int pointsAround[8][2] = {{x + 1, y}, {x - 1, y}, {x, y + 1}, {x, y - 1}, {x + 1, y + 1}, {x + 1, y - 1}, {x - 1, y - 1}, {x - 1, y + 1}};
	float probSpread = 1;
	for (int i = 0; i < 8; i++) {
		// Determine if current cell is burning
		bool currBurning = isCellBurning();
		
		// Check to see if we can multiply this probability
		if ((0 <= pointsAround[i][0] && pointsAround[i][0] < gridDim) && (0 <= pointsAround[i][1] && pointsAround[i][1] < gridDim)) {
			float pointDistance = distanceConstant * calculateDistance(x, y, pointsAround[i][0], pointsAround[i][1]);
			probSpread *= 1 - (pow(float(1 / pointDistance), 2) * currBurning);
		}
		
	}
	
	// Simple if to check for fuel and to subtract
	cout << "Probability of igniting based on proximity to burning cells: ";
    if (totalFuel > 0) {
    	cout << float(1) - probSpread << endl;
	} else {
		cout << 0 << endl;
	}
    
    return 0;
}
