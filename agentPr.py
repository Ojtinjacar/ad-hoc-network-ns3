#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import time
import gym
import os
import numpy as np
from ns3gym import ns3env
import matplotlib.pyplot as plt

# Environment initialization
port = 5555
simTime = 100
startSim = True
stepTime = 0.1
seed = 0
simArgs = {"--distance": 500}
debug = False



env = ns3env.Ns3Env(port=port, stepTime=stepTime, startSim=startSim, simSeed=seed, simArgs=simArgs, debug=debug)

ob_space = env.observation_space.shape[0]
ac_space = env.action_space.shape[0]
print("Observation space: ", ob_space)
print("Action space: ", ac_space)

# Q and rewards
##Q = np.zeros(shape=(5, 11, 10), dtype=np.float)
##Q = np.zeros((ob_space, ac_space))
Q = np.zeros((2000, 2000))

##action = np.zeros(shape=(5), dtype=np.uint)

rewards = []
iterations = []
epsilons = []

# Parameters
alpha = 0.75
discount = 0.95
episodes = 10
epsilon = 1                  
max_epsilon = 1
min_epsilon = 0.01         
decay = 0.01

# Episodes
for episode in range(episodes):
    # Refresh state
    state = env.reset()
    ##state = np.uint(np.array(state, dtype=np.uint32) / 10)
    done = False
    t_reward = 0

    i = 0
    # Run episode
    while True:
        if done:
            break
        exp_exp_tradeoff = np.random.uniform(0, 1) 
        ##i += 1
        if exp_exp_tradeoff > epsilon:
            action = np.argmax(Q[state,:]) 
        else:
            action = env.action_space.sample()
        new_state, reward, done, info = env.step(action)
        Q[state, action] = Q[state, action]+alpha*(reward+discount*
        np.max(Q[new_state, :])-Q[state, action]) 
        #Increasing our total reward and updating the state
        t_reward += reward      
        state = new_state         
        
        #Ending the episode
        if done == True:
            #print ("Total reward for episode {}: {}".format(episode, 
            #total_training_rewards))
            break
    
    #Cutting down on exploration by reducing the epsilon 
    epsilon = min_epsilon+(max_epsilon-min_epsilon)*np.exp(-decay*episode)
    
    #Adding the total reward and reduced epsilon values
    epsilons.append(epsilon)
    """current = state
        for n in range(5):
            # Action that maximize the simulation
            action[n] = np.argmax(Q[n, current[n], :] + np.random.randn(1, 10) * (1 / float(episode + 1)))

        # Send action to the simulation
        saction = np.uint(action * 100) + 1
        state, reward, done, info = env.step(saction)
        state = np.uint(np.array(state) / 10 )

        # Save reward and action on Q-Table
        t_reward += reward
        for n in range(5):
            Q[n, current[n], action[n]] += alpha * (reward + discount * np.max(Q[n, state[n], :]) - Q[n, current[n], action[n]])"""
print("Total reward:", t_reward)
rewards.append(t_reward)
iterations.append(i)

# Close environment
env.close()

# Plot results
def chunks_func(l, n):
    n = max(1, n)
    return (l[i:i+n] for i in xrange(0, len(l), n))

size = episodes
#chunks = list(chunk_list(rewards, size))
rewards = np.array(rewards)
chunks = np.array_split(rewards, size)
#chunks = chunks_func(rewards, size)
averages = [sum(chunk) / len(chunk) for chunk in chunks]

plt.plot(averages)
plt.xlabel('Episode')
plt.ylabel('Average Reward')
plt.show()
