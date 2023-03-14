import sys

# OpenAI Gym Libraries
import gym
from gym import spaces
import time

# Custom Environment
import build.fire_environment

# Numpy, for dealing with all of the representations of the observation space
import numpy as np

# Stable Baselines Stuff
from stable_baselines3 import DQN
from stable_baselines3.common.env_checker import check_env

total_time = 0
leaving_time, enter_time, external_time = 0, 0, 0

class WildfireEnv(gym.Env):
    '''
    Sets up the basic FireEnvironment object from C++, an observation space, and an action space
    '''
    def __init__(self):
        # Fire Environment example
        self.fire_env = build.fire_environment.FireEnvironment(20)

        # Set the observation space
        sep = (self.fire_env.getState())
        self.observation_space = spaces.Box(low=0, high=200, shape = sep.shape, dtype=np.float64)


        # Set the action space
        actions = self.fire_env.getActions()

        #I'm not doing a fancy prefix sum solution for an action space on the order of 10
        self.ind_to_pair = [[populated_area, path] for populated_area, path_count in enumerate(actions) for path in range(path_count)]
        self.action_space = spaces.Discrete(n = len(self.ind_to_pair), start=0)

        # Creating starting times
        self.total_time = 0
        self.leaving_time, self.enter_time, self.external_time = 0, 0, 0

    '''
    Reset the entire environment by creating a new environment.
    '''
    def reset(self):
        self.fire_env = build.fire_environment.FireEnvironment(20)
        sep = self.fire_env.getState()
        return sep, {"": ""}

    
    '''
    Take a step and advance the environment after taking an action.
    '''
    def step(self, action):
        # Get start of time call
        self.enter_time = time.time()
        if (self.leaving_time != 0):
            self.external_time += self.enter_time - self.leaving_time
        start = time.time()

        # Call C++ function to take the action
        actionTuple = self.ind_to_pair[action]

        rewards = self.fire_env.inputAction(actionTuple[0] - 1, actionTuple[1])

        # Gather the observations, rewards, terminated, and truncated
        observations = self.fire_env.getState()
        terminated = self.fire_env.getTerminated()
        truncated = False

        # Get end of time call
        end = time.time()
        self.total_time += (end - start)
        self.leaving_time = time.time()

        # Return necessary 4 tuple
        return observations, rewards, terminated, truncated, {"": ""}

    '''
    Useful in debugging and seeing the progression of the environment over time.
    '''
    def print_environment(self):
        self.fire_env.printData()
        print("")
    


def run_simulation(train):
    # Set up the basic environment
    env = WildfireEnv()
    #check_env(env)

    # Set up DQN Model
    model = None
    if train:
        model = DQN("CnnPolicy", env, verbose=1, policy_kwargs=dict(normalize_images=False))
        print("begin training")
        model.learn(total_timesteps=110000, log_interval=100)
        print(env.total_time)
        print(env.external_time)
        model.save("Trained Policy")
    else:
        model = DQN.load("Trained Policy")


    # Run a random sampling basically for 20 iterations
    for _ in range(10):
        # Get the initial stuff
        obs = env.reset()[0]
        for _ in range(99):
            action, _states = model.predict(obs, deterministic=True)
            print(action)
            observation, reward, terminated, truncated, info = env.step(action)
            env.print_environment()
            print(reward)

def main(train):
    train = int(train)
    run_simulation(train)

'''
Call with CLI argument "1" if you want to train, "0" if you don't
'''
if __name__ == "__main__":
    main(sys.argv[1])