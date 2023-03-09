import gym
from gym import spaces
import build.fire_environment
import numpy as np

class WildfireEnv(gym.Env):
    def __init__(self):
        # Fire Environment example
        self.fire_env = build.fire_environment.FireEnvironment(20)

        # Set the observation space
        self.observation_space = self.fire_env.getState()

        # Set the action space
        actions = self.fire_env.getActions()
        self.action_space = spaces.Tuple((spaces.Discrete(len(actions) + 1, start=-1), spaces.Discrete(max(actions), start=0)))
    
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

    def print_environment(self):
        self.fire_env.printData()
        print("")
    
# Set up the basic environment
env = WildfireEnv()

# Run a random sampling basically for 20 iterations
for _ in range(20):
    observation, reward, terminated, truncated, info = env.step(env.action_space.sample())
    env.print_environment()