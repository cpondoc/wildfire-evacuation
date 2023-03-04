#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <vector>
#include <random> 

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


int main(){
    
}