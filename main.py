import gym
from gym import spaces
import build.fire_environment

class WildfireEnv(gym.Env):
    def __init__(self):
        # Observation space:
        # Currently just the array of all the vector areas
        # In the future: a tensor to pass into a stable baselines learning algorithm
        self.observation_space = spaces.Dict(
            {
                "agents": spaces.Discrete(4),
                "targets": spaces.Discrete(3)
            }
        )

        # Action space eventually has to be:
        # (# of populated areas, # of discrete actions to take)
        self.action_space = spaces.Tuple(spaces.Discrete(4), spaces.Discrete(2))

print("Hey there!")