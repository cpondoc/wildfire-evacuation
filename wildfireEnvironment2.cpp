/*
File: wildfireEnvironment.cpp
Initial start with modeling wildfire spread!
Modified version of wildfire.pp built to be an environment for openai gym
*/
#include "wildfireEnvironment2.h"


using namespace std;

random_device rd;
mt19937 gen(rd());


class FireEnvironment{

	public:
	int timeStep = 0;
	int nextReward;
	double distanceConstant;
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

	//Note currently assumes paths cannot overlap
	struct evacuationPath{
		//The populated areas that are currently using the path
		//vector<populatedArea*> activatePopulatedAreas;
		vector<pair<int,int> > pathLocations;
		int evacuationTime;
		bool active = true;
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
		vector<evacuationPath*> availablePathsToTake;
		evacuationPath* currentPath = nullptr;
	};

	//Create the paths
	vector<evacuationPath> evacuationPaths;
	// Create the state of the simulation
	vector<vector<LandCell> > state;
	// Create an action space of populated areas
	vector<populatedArea> actionSpace;
	//Takes in coordinates and says if a path is located there
	unordered_map<int, unordered_map<int, int> > pathedAreas;
	//The numpy array representation of our state space for returning
	//ret[x][i][j] refers to element located at (i,j) and the x'th feature
	//0=fire, 1=fuel, 2=populated, 3=evacuating, 4=path
	vector<vector<vector<float> > > ret;

	/*
	Function to calculate Euclidean distance between two points
	*/
	double calculateDistance(int xOne, int yOne, int xTwo, int yTwo) {
		return sqrt(pow(xOne - xTwo, 2) + pow((yOne - yTwo), 2));
	}

	/*
	Currently a replacement for `drawGridworld` -- helps to visualize the look of the canvas
	*/
	void printData(/*vector<vector<LandCell> >& state*/){
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

	vector<int> returnActionSpace() {
		vector<int> possibleStates;
		for (int i = 0; i < actionSpace.size(); i++) {
			possibleStates.push_back(actionSpace[i].availablePathsToTake.size());
		}
		return possibleStates;
	}

	int returnReward() {
		return getStateUtility(state, actionSpace);
	}

	/*
	Helper function that updates the states through all deterministic changes in the time step. 
	Check through all states and deplete fire by 1.
	If there is no more fire, then we set the boolean equal to 0.
	*/
	void depleteFuel(vector<vector<LandCell> >& state, vector<populatedArea>& actionSpace){
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
	}

	void updateActionSpace(vector<vector<LandCell> >& state, vector<populatedArea>& actionSpace, vector<evacuationPath>& evacuationPaths){
		// Decrease amount of time remaining for populated areas if already evacuating
		for (int i = 0; i < actionSpace.size(); i++){
			if (actionSpace[i].evacuating && actionSpace[i].remainingTime) {
				//makes sure the path is still safe to use. If evacuating, should have an evacuation path
				if(!actionSpace[i].currentPath->active){
					actionSpace[i].remainingTime = INT_MAX;
					actionSpace[i].evacuating = false;
					actionSpace[i].currentPath = nullptr;
					ret[3][actionSpace[i].i][actionSpace[i].j] = 0;
				}
				else{
					actionSpace[i].remainingTime--;
					if (!actionSpace[i].remainingTime) {
						state[actionSpace[i].i][actionSpace[i].j].populated = false;
					}
				}
			}
		}
	}


	/*
	At each point in the grid world, determine if there will be a fire by looking at the amount of fuel and the surrounding states.
	pathAreas[i][j] refers to the path in evacuationPaths that is in the coordinates i,j
	*/
	vector<vector<LandCell> > sampleNextState(vector<vector<LandCell> >& state, double distanceConstant, 
		unordered_map<int, unordered_map<int, int> >& pathedAreas, vector<evacuationPath>& evacuationPaths){

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
							}
						}
					}

					// Calculate final probability, and then use a Bernoulli distribution to determine if there will be a fire
					prob = 1 - prob;
					bernoulli_distribution b(prob);
					newState[i][j].fire = b(gen);

					//Sees if a newly fired area is part of a path
					if(newState[i][j].fire && pathedAreas.find(i) != pathedAreas.end() && pathedAreas[i].find(j) != pathedAreas[i].end()){
						int index = pathedAreas[i][j];
						evacuationPaths[index].active = false;
						for(int x = 0; x < evacuationPaths[index].pathLocations.size(); x++){
							ret[4][evacuationPaths[index].pathLocations[x].first][evacuationPaths[index].pathLocations[x].second] = 0;
						}
					}
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
	Since we are not running sparse sampling in the environment, 
	*/
	vector<populatedArea> takeAction(pair<int,int> action, vector<populatedArea>& actionSpace) {

		vector<populatedArea> newActionSpace = actionSpace;
		int i = action.first;
		int j = action.second;
		// Only take an action if the action is not to do nothing
		// For now if we say to take an action on an already evacuated area or take a path that isn't active, do nothing
		if (action.first != -1 && newActionSpace[i].remainingTime && !newActionSpace[i].evacuating && newActionSpace[i].availablePathsToTake[j]->active) {
			newActionSpace[i].currentPath = newActionSpace[i].availablePathsToTake[j];
			newActionSpace[i].evacuating = true;
			newActionSpace[i].remainingTime = newActionSpace[i].availablePathsToTake[j]->evacuationTime;
			ret[3][i][j] = 1;
		}

		return newActionSpace;
	}


	//(gridDim, vector<LandCell>(gridDim, LandCell()))
	//Create the paths
	//vector<evacuationPath> evacuationPaths;
	// Create the state of the simulation
	//vector<vector<LandCell> > state;
	// Create an action space of populated areas
	//vector<populatedArea> actionSpace;
	//unordered_map<int, unordered_map<int, int> > pathedAreas;
	//vector<vector<vector<float> > > ret;
	FireEnvironment(int gridDim) : evacuationPaths(), state(gridDim, vector<LandCell>(gridDim, LandCell())),
		 actionSpace(), pathedAreas(), ret(5, vector<vector<float> >(state.size(), vector<float>(state[0].size(), 0))) {
		// Various hyperparameters
		srand (time(NULL));
		int timeToEvacuate = 3;
		distanceConstant = 0.094;

		//INITIALIZE STATE
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

		//INITIALIZE PATHS
		evacuationPaths.push_back({{{0,6},{1,6}}, 3, true});
		evacuationPaths.push_back({{{4,17},{4,18},{4,19}}, 3, true});
		//builds the pathedAreas struct
		for(int i = 0; i < evacuationPaths.size(); i++){
			for(int j = 0; j < evacuationPaths[i].pathLocations.size(); j++){
				pathedAreas[evacuationPaths[i].pathLocations[j].first][evacuationPaths[i].pathLocations[j].second] = i;
				if(state[evacuationPaths[i].pathLocations[j].first][evacuationPaths[i].pathLocations[j].second].fire)
					evacuationPaths[i].active = false;
			}
				
			for(int j = 0; j < evacuationPaths[i].pathLocations.size(); j++){
				ret[4][evacuationPaths[i].pathLocations[j].first][evacuationPaths[i].pathLocations[j].second] = evacuationPaths[i].active;
			}
		}


		//INITIALIZES ACTIONS
		actionSpace.push_back({2, 6, false, INT_MAX, {&evacuationPaths[0]}, nullptr});
		actionSpace.push_back({4, 16, false, INT_MAX, {&evacuationPaths[1]}, nullptr});

		// Indicate populated areas also on game state
		state[2][6].populated = true;
		state[4][16].populated = true;


	}

	//Maybe don't pass in parameters and only change by reference
	vector<vector<vector<float> > > returnState(){
		depleteFuel(state, actionSpace);
		state = sampleNextState(state, distanceConstant, pathedAreas, evacuationPaths);
		updateActionSpace(state, actionSpace, evacuationPaths);
		nextReward = getStateUtility(state, actionSpace);
		timeStep += 1;
		
		//We have run the updates. Now let's put that badboy in a numpy array shape=(5,gridDim,gridDim)

		//represents on fire or not
		for(int i = 0; i < state.size(); i++){
			for(int j = 0; j < state[0].size(); j++){
				ret[0][i][j] = state[i][j].fire;
				ret[1][i][j] = state[i][j].fuel;
				ret[2][i][j] = state[i][j].populated;
			}
		}

		return ret;
	}

	int inputAction(int first, int second){
		actionSpace = takeAction({first, second}, actionSpace);
		
		return nextReward;
	}

	bool isTerminated() {
		return (timeStep >= 100);
	}

};



PYBIND11_MODULE(fire_environment, handle) {
	handle.doc() = "Wrapper for fire simulation";

	py::class_<FireEnvironment>(
		handle, "FireEnvironment"
	)

	.def(py::init<int>())

	//Debugging purposes where we print the internal state of our simulator
	.def("printData", [](FireEnvironment& self) {
			self.printData();
		}

	)

	// Return action state to use
	.def("getActions", [](FireEnvironment& self) {
			return self.returnActionSpace();
		}

	)

	// Return reward at a time step
	.def("getReward", [](FireEnvironment& self) {
			return self.returnReward();
		}

	)

	// Return if terminated
	.def("getTerminated", [](FireEnvironment& self) {
			return self.isTerminated();
		}

	)

	// Get the current state rn
	.def("getState", [](FireEnvironment& self) {
			py::array out = py::cast(self.returnState());
			return out;
		}
	
	)
	.def("inputAction", [](FireEnvironment& self, int first, int second) {
			return self.inputAction(first, second);
		}
	
	)
	;
}






/*
void runSimulation(int gridDim, double distanceConstant, int burnRate) {
	// Set up randomness
	random_device rd;
    mt19937 gen(rd());

	// Various hyperparameters
	int timeToEvacuate = 3;

	// Run simulation for x timesteps
	int actualReward = 0;
	for (int i = 0; i < 100; i++) {
		// Run the next state forward and sample the next state
		int temp = getStateUtility(state, actionSpace);
		actualReward += temp;
		//if(temp < 0)
		//	cout << "We lost them" << endl;
		runDetForward(state, actionSpace);
		state = sampleNextState(state, distanceConstant);

		// Run sparse sampling and take the next action

		actionSpace = takeAction(best.first, actionSpace);
		
		// Print state (since we know the fire is going crazy right now)
		printData(state);
		cout << i << endl;
	}
	cout << "HEAR YE HEAR YE. THE KING PROCLAIMS OUR FINAL REWARD IS " << actualReward << endl;
}
*/

/*
Calls main functions!
TO-DO: make the hyperparameters global?
*/
int main()
{	
	FireEnvironment temp = FireEnvironment(20);
    return 0;
}