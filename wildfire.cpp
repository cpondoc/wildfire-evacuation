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
			if (state[i][j].fire) {
				cout << "F" << " ";
				//Check check up
				if(state[i][j].populated)
					cout << "This code is broken :/" << endl;
			} else if (state[i][j].populated) {
				cout << "P" << " ";
			} else {
				cout << "_" << " ";
			}
		}
		cout << endl;
	}

}

/*
Helper function that updates the states through all deterministic changes in the time step. 
Check through all states and deplete fire by 1.
If there is no more fire, then we set the boolean equal to 0.
*/
void runDetForward(vector<vector<LandCell> >& state, vector<populatedArea>& actionSpace){
	// Deplete fire and check if there is still a fire
	for (int i = 0; i < state.size(); i++) {
		for (int j = 0; j < state[0].size(); j++)
			if (state[i][j].fire) {
				state[i][j].fuel = max(double(0), state[i][j].fuel - 1);
				if (!state[i][j].fuel) {
					state[i][j].fire = false;
				}
			}
	}

	// Decrease amount of time remaining for populated areas if already evacuating
	for (int i = 0; i < actionSpace.size(); i++){
		if (actionSpace[i].evacuating && actionSpace[i].remainingTime) {
			actionSpace[i].remainingTime--;
			if (!actionSpace[i].remainingTime) {
				state[actionSpace[i].i][actionSpace[i].j].populated = false;
			}
		}
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
						if (i != nI && j != nJ && state[nI][nJ].fire) {
							prob *= 1 - (distanceConstant * pow(1.0 / calculateDistance(i, j, nI, nJ), 2));
							//prob *= 1 - (distanceConstant * pow(1.0 / calculateDistance(i, j, nI, nJ), 2) * (state[nI][nJ].fire ? 1 : 0));
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
Function to calculate the total utility of all of the states
TO-DO: should the first check see whether or not the action space is evacuating?
TO-DO: should the second case consider whether or not the spot is on fire but they are not evacuated?
*/
int getStateUtility(vector<vector<LandCell> >& state, vector<populatedArea>& actionSpace) {
	int reward = 0;
	for (int i = 0; i < actionSpace.size(); i++) {
		// If the populated area still has remaining time left, but the area is already on fire, incur -100 reward.
		if (actionSpace[i].remainingTime && state[actionSpace[i].i][actionSpace[i].j].fire) {
			//cout << "IT'S SO OVER" << endl;
			reward -= 100;
			actionSpace[i].remainingTime = 0;
			state[actionSpace[i].i][actionSpace[i].j].populated = false;
		}

		// If the current populated area is not evacuating, add 1 reward
		else if (!actionSpace[i].evacuating && actionSpace[i].remainingTime) {
			reward += 1;
		}
	}

	return reward;
}

/*
Function to actually take an action after sampling a state
*/
vector<populatedArea> takeAction(int action, vector<populatedArea>& actionSpace) {

	vector<populatedArea> newActionSpace = actionSpace;
	// Only take an action if the action is not to do nothing
	if (action != -1) {
		actionSpace[action].evacuating = true;
	}

	return newActionSpace;
}

/*
Implements sparse sampling to approximate the value function. Note that we have the following action space:
-"-1" means to do nothing
-"i" means to evacuate the "i'th" area in the actionSpace vector of populated areas.
*/
pair<int,int> sparseSampling(vector<vector<LandCell> > state, vector<populatedArea> actionSpace, int depth, int samples, double distanceConstant) {
	// Standalone scenario -- doing nothing
	int stateReward = getStateUtility(state, actionSpace);
	pair<int, int> best = {-1, INT_MIN};

	// Base case: depth = 0, so we return doing nothing and the utility of being in the state
	if (depth == 0) {
		return {-1, stateReward};
	}

	// Recursive case: iterate through each action (each i'th area of evacuation)
	for (int i = -1; i < (int) actionSpace.size(); i++) {
		//Ignore already evacuating areas
		if(i >= 0 && actionSpace[i].evacuating)
			continue;
		// Generate a sample for each and update the utility
		int utility = 0;
		vector<populatedArea> newActionSpace = takeAction(i, actionSpace);
		vector<vector<LandCell> > resultantState = state;
		runDetForward(resultantState, newActionSpace);

		for (int j = 0; j < samples; j++) {
			// Sample a new state and reward
			vector<vector<LandCell> > sampledState = sampleNextState(resultantState, distanceConstant);
			
			// Call sparse sampling recursively and update utility (assume undiscounted)
			pair<int, int> returnPair = sparseSampling(sampledState, newActionSpace, depth - 1, samples, distanceConstant);
			utility += returnPair.second;
		}
		//the discount factor
		utility /= samples;
		utility *= .99;

		// Check if utility is better than current best
		if (utility + stateReward > best.second) {
			best = {i, utility + stateReward};
		}
	}

	// Return best
	return best;
}

/*
Main function that controls the simulation
*/
int runSimulation(int gridDim, double distanceConstant, int burnRate) {
	// Set up randomness
	random_device rd;
    mt19937 gen(rd());

	// Various hyperparameters
	int timeToEvacuate = 3;

	// Create the state of the simulation
	vector<vector<LandCell> > state(gridDim, vector<LandCell>(gridDim, LandCell()));

	// Create an action space of populated areas
	vector<populatedArea> actionSpace;
	actionSpace.push_back({11, 4, false, timeToEvacuate});
	actionSpace.push_back({3, 14, false, timeToEvacuate});

	// Indicate populated areas also on game state
	state[11][4].populated = true;
	state[3][14].populated = true;
	
	// Places initial fire seeds
	int burnCount = 2;
	for(int i = 0; i < burnCount; i++){
		state[rand() % gridDim][rand() % gridDim].fire = true;
	}

	// Sets fuel levels
	normal_distribution<double> normal(8, 3);
	for(int i = 0; i < state.size(); i++){
		for(int j = 0; j < state[0].size(); j++){
			state[i][j].fuel = max(double(0), normal(gen));
		}
	}

	// Run simulation for x timesteps
	int actualReward = 0;
	for (int i = 0; i < 100; i++) {
		// Run the next state forward and sample the next state
		int temp = getStateUtility(state, actionSpace);
		//printData(state);
		actualReward += temp;
		if(temp < 0)
			cout << "We lost them" << endl;
		runDetForward(state, actionSpace);
		state = sampleNextState(state, distanceConstant);

		// Run sparse sampling and take the next action
		pair<int, int> best = sparseSampling(state, actionSpace, 5, 3, distanceConstant);
		actionSpace = takeAction(best.first, actionSpace);
		
		cout << i << endl;
	}
	cout << "HEAR YE HEAR YE. THE KING PROCLAIMS OUR FINAL REWARD IS " << actualReward << endl;
	return actualReward;
}

/*
Calls main functions!
TO-DO: make the hyperparameters global?
*/
int main()
{	
	// Initialize random seed, grid dimension, and hyperparameters
	srand (time(NULL));
	int gridDim = 20;
	double distanceConstant = 0.124;
    int burnRate = 5;
	
	// Running simulation
	cout << "Initially running the simulation" << endl;
	double total = 0;
	for(int i = 0; i < 100; i++)
		total += runSimulation(gridDim, distanceConstant, burnRate);
	cout << "Average across all simulations is " << (total / 100) << endl;
    
    return 0;
}
