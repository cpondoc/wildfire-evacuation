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
        self.action_space = spaces.Tuple((spaces.Discrete(len(actions)), spaces.Discrete(max(actions))))

# Set up the basic environment
env = WildfireEnv()
