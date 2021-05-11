import pommerman
import pommerman.agents as agents
from pommerman.agents.simple_agent import SimpleAgent
from cppagent import CppAgent
from util import ffa_evaluate

agent_list = [
    agents.RandomAgent(),
    agents.RandomAgent(),
    agents.RandomAgent(),
    CppAgent("./Release/libpomcpp.so", "SimpleAgent")
]

# Make the "Free-For-All" environment using the agent list
env = pommerman.make('PommeFFACompetition-v0', agent_list)

ffa_evaluate(env, 20, True, False)