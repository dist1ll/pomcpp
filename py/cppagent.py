import pommerman
import pommerman.agents as agents
import pommerman.utility as utility
from clib import CLib
import time
import ctypes
import json

class CppAgent(agents.BaseAgent):
    def __init__(self, library_path, agent_name: str, seed: int = 42):
        super().__init__()
        self.agent_name = agent_name

        # load interface

        lib = CLib(library_path)
        self.agent_create = lib.get_fun("agent_create", [ctypes.c_char_p, ctypes.c_long], ctypes.c_bool)
        self.agent_reset = lib.get_fun("agent_reset", [], ctypes.c_void_p)
        self.agent_act = lib.get_fun("agent_act", [ctypes.c_char_p, ctypes.c_bool], ctypes.c_int)

        # create agent

        if not self.agent_create(agent_name.encode('utf-8'), seed):
            raise ValueError(f"Could not create agent with name {agent_name}!")

        self.total_steps = 0
        self.sum_encode_time = 0.0
        self.sum_agent_act_time = 0.0

    def act(self, obs, action_space):
        act_start = time.time()

        json_obs = json.dumps(obs, cls=utility.PommermanJSONEncoder)
        act_encoded = time.time()

        move = self.agent_act(json_obs.encode('utf-8'), False)
        act_done = time.time()

        diff_encode = (act_encoded - act_start)
        self.sum_encode_time += diff_encode
        
        diff_act = (act_done - act_encoded)
        self.sum_agent_act_time += diff_act

        self.total_steps += 1

        return move

    def init_agent(self, id, game_type):
        super().init_agent(id, game_type)

        if game_type != pommerman.constants.GameType.FFA:
            raise ValueError(f"GameType {str(game_type)} is not supported!")

        self.agent_reset(id)

    def print_time_stats(self):
        print(f"Response times of agent '{self.agent_name}' over {self.total_steps} steps:")
        print(f"> Average encode time: {self.sum_encode_time / self.total_steps * 1000} ms")
        print(f"> Average act time: {self.sum_agent_act_time / self.total_steps * 1000} ms")
