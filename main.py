# OpenAI Gym Libraries
import gym
from gym import spaces

# Custom Environment
import build.fire_environment

# Numpy, for dealing with all of the representations of the observation space
import numpy as np

# Stable Baselines Stuff
from stable_baselines3 import DQN

class WildfireEnv(gym.Env):
    '''
    Sets up the basic FireEnvironment object from C++, an observation space, and an action space
    '''
    def __init__(self):
        # Fire Environment example
        self.fire_env = build.fire_environment.FireEnvironment(20)

        # Set the observation space
        self.observation_space = self.fire_env.getState()

        # Set the action space
        actions = self.fire_env.getActions()
        self.action_space = spaces.Tuple((spaces.Discrete(n = len(actions) + 1, start=-1), spaces.Discrete(n = max(actions), start=0)))

    '''
    Reset the entire environment by creating a new environment.
    '''
    def reset(self):
        self.fire_env = build.fire_environment.FireEnvironment(20)
    
    '''
    Take a step and advance the environment after taking an action.
    '''
    def step(self, action):
        # Call C++ function to take the action
        self.fire_env.inputAction(action[0], action[1])

        # Gather the observations, rewards, terminated, and truncated
        observations = self.fire_env.getState()
        rewards = self.fire_env.getReward()
        terminated = self.fire_env.getTerminated()
        truncated = False
        
        # Return necessary 4 tuple
        return observations, rewards, terminated, truncated, ""

    '''
    Useful in debugging and seeing the progression of the environment over time.
    '''
    def print_environment(self):
        self.fire_env.printData()
        print("")
    
# Set up the basic environment
env = WildfireEnv()

# Set up DQN Model
model = DQN("CnnPolicy", env, verbose=1)
model.learn(total_timesteps=30, log_interval=4)

# Run a random sampling basically for 20 iterations
for _ in range(20):
    observation, reward, terminated, truncated, info = env.step(env.action_space.sample())
    env.print_environment()