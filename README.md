# CS 238 Final Project
By: Joey Guman, Chris Pondoc, and Joey O'Brien

## Project Description
One of the main effects of climate change in the world today is the increased frequency and intensity of wildfires. This reality has led to an increase in interest in wildfire response as demonstrated by recent work done by SISL. Our proposed final project builds off of existing research within wildfire disaster response. The central motivation is to simulate an environment over a wide area with residential sectors and a spreading wildfire. Unlike previous research, which focused on monitoring the spread of fire, we will focus on developing a community’s response to the wildfire in the form of evacuation in an environment that presumes perfect information about the fire’s current state. More specifically, the motivation will be to identify areas that must be evacuated and the ideal path to “safe areas.” We will attempt to employ deep reinforcement learning techniques to accomplish this task.

The perspective of our algorithm will have a central authority/agent that orders specific regions when and how to evacuate. This is akin to real life, where authorities declare when a certain region is to be evacuated. The model will only attempt to order the evacuation of areas that should be evacuated while avoiding any casualties. The decision-making will involve identifying areas that should be evacuated and the roads to be taken to evacuate people. The primary probability source would be the propagation of the fire – which we model stochastically – and the development of traffic while trying to evacuate, should our model become complex enough to simulate. Our base model will involve a matrix where elements (as sectors) features will be details like:
* Fire/no fire/ burned out
* Number of people in the sector
* Current evacuation order status 
* Barriers (uncrossable area representing something like a river), etc.

We can abstract away evacuation paths with a path-finding algorithm such as BFS or A*. We will experiment with action spaces that are both centralized – such as picking a sector to act upon and then making a decision – or decentralized – where each individual sector is its agent that reads in the environment and makes decisions for itself with information like distance to a shelter area or the closest fire. Each sector agent would also share the same policy to simplify training. One idea we have of representing this necessarily high-dimensional action space is to try running CNN architecture on the grid with features of each sector as depth to embed the model.

Overall, we believe this may be a basis for future work exploring the future application and scalability of RL in wide-area planning contexts.

## To Compile
```
g++ -std=c++17 -o wildfire wildfire.cpp
```

## To Run
```
./wildfire
```
