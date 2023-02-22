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
#include <random> 

using namespace std;

struct LandCell{
	bool fire = false;
	double fuel;
};

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
double calculateDistance(int xOne, int yOne, int xTwo, int yTwo) {
	return sqrt(pow(xOne - xTwo, 2) + pow((yOne - yTwo), 2));
}

void printData(vector<vector<LandCell> >& state){
	//prints our levels really quickly
	/*
	for(int i = 0; i < state.size(); i++){
		for(int j = 0; j < state[0].size(); j++){
			cout << state[i][j].fuel << " ";
		}
		cout << endl;
	}
	*/
	for(int i = 0; i < state.size(); i++){
		for(int j = 0; j < state[0].size(); j++){
			cout << state[i][j].fire << " ";
		}
		cout << endl;
	}

}

void runFuelDepletion(vector<vector<LandCell> >& state){
	for(int i = 0; i < state.size(); i++){
		for(int j = 0; j < state[0].size(); j++)
			if(state[i][j].fire){
				state[i][j].fuel = max(double(0), state[i][j].fuel - 1);
				if(!state[i][j].fuel){
					state[i][j].fire = false;
				}
			}
	}
}


//Presumes fire can only spread across five cells
vector<vector<LandCell> > sampleNextState(vector<vector<LandCell> >& state, double distanceConstant){

	random_device rd;
	mt19937 gen(rd());
	vector<vector<LandCell> > newState = state;
	int observationDistance = 2;
	//Goes through each point
	for(int i = 0; i < state.size(); i++){
		for(int j = 0; j < state[0].size(); j++){

			if(!state[i][j].fire && state[i][j].fuel > 0){
				double prob = 1;

				for(int nI = max(0, i - observationDistance); nI < min(int(state.size()), i + observationDistance); nI++){
					for(int nJ = max(0, j - observationDistance); nJ < min(int(state.size()), j + observationDistance); nJ++){
						if(i != nI && j != nJ){
							prob *= 1 - (pow(1.0 / calculateDistance(i, j, nI, nJ) / distanceConstant, 2) * (state[nI][nJ].fire ? 1 : 0));
						}
					}
				}
				prob = 1 - prob;
				bernoulli_distribution b(prob);
				newState[i][j].fire = b(gen);
			}
		}
	}

	return newState;
}

void runSimulation(int gridDim, double distanceConstant, int burnRate){
	random_device rd;
    mt19937 gen(rd());

	vector<vector<LandCell> > state(gridDim, vector<LandCell>(gridDim, LandCell()));
	
	//Places initial fire seeds
	int burnCount = 2;
	for(int i = 0; i < burnCount; i++){
		state[rand() % gridDim][rand() % gridDim].fire = true;
	}

	//Sets fuel levels
	normal_distribution<double> normal(10, 3);
	for(int i = 0; i < state.size(); i++){
		for(int j = 0; j < state[0].size(); j++){
			state[i][j].fuel = max(double(0), normal(gen));
		}
	}

	//Runs for x timesteps
	for(int i = 0; i < 30; i++){
		runFuelDepletion(state);
		state = sampleNextState(state, distanceConstant);
		if(i % 5 == 0){
			printData(state);
			cout << endl;
		}
			

			
	}

	

}

int main()
{	
	// Initialize random seed, grid dimension, and constant for proportionality of distance
	srand (time(NULL));
	int gridDim = 10;
	double distanceConstant = 2;
    int burnRate = 5;
	
	/*
	Initial stochastic model for fuel of a cell
	*/
	cout << "Modeling of amount of fuel!" << endl << endl;
	    
	runSimulation(gridDim, distanceConstant, burnRate);
	exit(0);
    // Defining burning rate
    int totalFuel = 50;
    
    // Changing of the amout of fuel
    for (int i = 0; i < 10; i++) {
		// Check if cell is burning and reduce accordingly
    	if (isCellBurning()) {
    		totalFuel = max(0, totalFuel - burnRate);
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
	double probSpread = 1;
	for (int i = 0; i < 8; i++) {
		// Determine if current cell is burning
		bool currBurning = isCellBurning();
		
		// Check to see if we can multiply this probability
		if ((0 <= pointsAround[i][0] && pointsAround[i][0] < gridDim) && (0 <= pointsAround[i][1] && pointsAround[i][1] < gridDim)) {
			double pointDistance = distanceConstant * calculateDistance(x, y, pointsAround[i][0], pointsAround[i][1]);
			probSpread *= 1 - (pow(double(1 / pointDistance), 2) * currBurning);
		}
		
	}
	
	// Simple if to check for fuel and to subtract
	cout << "Probability of igniting based on proximity to burning cells: ";
    if (totalFuel > 0) {
    	cout << double(1) - probSpread << endl;
	} else {
		cout << 0 << endl;
	}
    
    return 0;
}
