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

random_device rd;
mt19937 gen(rd());

/*
Struct to represent a land cell. It currently contains...
-A boolean to represent if there is a fire
-A double to represent the amount of fuel left in the land cell
*/
struct LandCell{
	bool fire = false;
	double fuel;
	bool populated = false;
};

/*
Struct to represent a populated area. It currently contains...
-ints i and j to represent the location of the populated area [TO-DO: make this cleaner?]
-A boolean to indicate if the populated area is evacuating or not
-An int, where if set to 0, means there are no longer people there (either evacuated or died)
*/
struct populatedArea{
	int i;
	int j;
	bool evacuating = false;
	int remainingTime;
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

/*
Currently a replacement for `drawGridworld` -- helps to visualize the look of the canvas
*/
void printData(vector<vector<LandCell> >& state){
	for(int i = 0; i < state.size(); i++){
		for(int j = 0; j < state[0].size(); j++){
			cout << state[i][j].fire << " ";
		}
		cout << endl;
	}

}

/*
Helper function to that updates the states through all deterministic changes in the time step. 
Check through all states and deplete fire by 1.
If there is no more fire, then we set the boolean equal to 0.
*/
void runDetForward(vector<vector<LandCell> >& state, vector<populatedArea>& actionSpace){
	// Deplete fire and check if there is still a fire
	for(int i = 0; i < state.size(); i++){
		for(int j = 0; j < state[0].size(); j++)
			if (state[i][j].fire) {
				state[i][j].fuel = max(double(0), state[i][j].fuel - 1);
				if (!state[i][j].fuel) {
					state[i][j].fire = false;
				}
			}
	}

	// Decrease amount of time remaining for populated areas if already evacuating
	for(int i = 0; i < actionSpace.size(); i++){
		if (actionSpace[i].evacuating && actionSpace[i].remainingTime) actionSpace[i].remainingTime--;
	}
}


/*
At each point in the grid world, determine if there will be a fire by looking at the amount of fuel and the surrounding states.
*/
vector<vector<LandCell> > sampleNextState(vector<vector<LandCell> >& state, double distanceConstant){

	// Create new state and set observation distance
	vector<vector<LandCell> > newState = state;
	int observationDistance = 2;

	// Iterate through each point
	for (int i = 0; i < state.size(); i++) {
		for (int j = 0; j < state[0].size(); j++) {
			
			// Check whether a certain state doesn't have a fire, but DOES have fuel
			if (!state[i][j].fire && state[i][j].fuel > 0) {

				// Utilize equation from SISL paper to calculate probability of there being a fire
				double prob = 1;
				for (int nI = max(0, i - observationDistance); nI < min(int(state.size()), i + observationDistance); nI++) {
					for (int nJ = max(0, j - observationDistance); nJ < min(int(state.size()), j + observationDistance); nJ++) {
						if (i != nI && j != nJ) {
							prob *= 1 - (distanceConstant * pow(1.0 / calculateDistance(i, j, nI, nJ), 2) * (state[nI][nJ].fire ? 1 : 0));
						}
					}
				}

				// Calculate final probability, and then use a Bernoulli distribution to determine if there will be a fire
				prob = 1 - prob;
				bernoulli_distribution b(prob);
				newState[i][j].fire = b(gen);
			}
		}
	}

	return newState;
}

/*
PLACEHOLDER: shows how to update in place, since passing in by reference
*/
vector<vector<vector<LandCell> > > sampleNextStates(vector<vector<LandCell> >& state, double distanceConstant, int total){
	vector<vector<vector<LandCell> > > futureStates;

	vector<vector<double> > probabilities(state.size(), vector<double>(state[0].size()));
	int observationDistance = 2;
	//Goes through each point to build probability of there being a fire
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
				probabilities[i][j] = prob;
			}
			else
				probabilities[i][j] = 0;
		}
	}

	//Builds a future sample for each 
	for(int t = 0; t < total; t++){

		futureStates.push_back(vector<vector<LandCell> >(state.size(), vector<LandCell>(state[0].size())));
		
		// Uses Bernoulli distribution in order to model if a fire exists or doesn't exist at a certain place
		for(int i = 0; i < state.size(); i++){
			for(int j = 0; j < state[0].size(); j++){
				futureStates[futureStates.size() - 1][i][j] = state[i][j];
				if(!state[i][j].fire){
					bernoulli_distribution b(probabilities[i][j]);
					futureStates[futureStates.size() - 1][i][j].fire = b(gen);
				}
				
			}
		}
	}

	for(int i = 0 ; i < total; i++){
		printData(futureStates[i]);
		cout << endl;
	}

	exit(0);

	return futureStates;
}

/*
Function to calculate the total utility of all of the states
TO-DO: should the first check see whether or not the action space is evacuating?
TO-DO: should the second case consider whether or not the spot is on fire but they are not evacuated?
*/
int getStateUtility(vector<vector<LandCell> >& state, vector<populatedArea>& actionSpace){
	int reward = 0;
	for(int i = 0; i < actionSpace.size(); i++) {
		// If the populated area still has remaining time left, but the area is already on fire, incur -100 reward.
		if (actionSpace[i].remainingTime && state[actionSpace[i].i][actionSpace[i].j].fire){
			reward -= 100;
			actionSpace[i].remainingTime = 0;
		}

		// If the current populated area is not evacuating, add 1 reward
		else if (!actionSpace[i].evacuating) {
			reward += 1;
		}
	}

	return reward;
}

/*
* First is action -1 means do nothing, i means begin evacuation of the i'th area in the actionSpace
* Important that we aren't passive the action space by reference
*/
pair<int,int> sparseSampling(vector<vector<LandCell> >& state, vector<populatedArea> actionSpace) {
	//The base reward this state must get
	int reward = getStateUtility(state, actionSpace);
	

	return {-1, reward};

}

/*
Main function that controls the simulation
*/
void runSimulation(int gridDim, double distanceConstant, int burnRate) {
	// Set up randomness
	random_device rd;
    mt19937 gen(rd());

	// Various hyperparameters
	int timeToEvacuate = 4;

	// Create the state of the simulation
	vector<vector<LandCell> > state(gridDim, vector<LandCell>(gridDim, LandCell()));

	// Create an action space of populated areas
	vector<populatedArea> actionSpace;
	actionSpace.push_back({4, 9, false, timeToEvacuate});
	actionSpace.push_back({2, 1, false, timeToEvacuate});

	// Indicate populated areas also on game state
	state[4][9].populated = true;
	state[2][1].populated = true;
	
	// Places initial fire seeds
	int burnCount = 2;
	for(int i = 0; i < burnCount; i++){
		state[rand() % gridDim][rand() % gridDim].fire = true;
	}

	// Sets fuel levels
	normal_distribution<double> normal(10, 3);
	for(int i = 0; i < state.size(); i++){
		for(int j = 0; j < state[0].size(); j++){
			state[i][j].fuel = max(double(0), normal(gen));
		}
	}

	// Run simulation for x timesteps
	for (int i = 0; i < 30; i++) {
		runDetForward(state, actionSpace);
		state = sampleNextState(state, distanceConstant);
		if (i % 5 == 0) {
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
