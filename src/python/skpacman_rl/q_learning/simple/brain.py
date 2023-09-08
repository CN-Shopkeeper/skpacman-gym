import os

import numpy as np
import pandas as pd


class QLearningTable:
    def __init__(self, actions, learning_rate=0.01, reward_decay=0.9, e_greedy=0.9):
        self.actions = actions
        self.lr = learning_rate
        self.gamma = reward_decay
        self.epsilon = e_greedy
        self.q_table = pd.DataFrame(columns=self.actions, dtype=np.float64)

    def choose_action(self, observation, deus_ex_machina=None):
        self.check_state_exist(observation)
        if deus_ex_machina is not None:
            action = deus_ex_machina
        else:
            if np.random.uniform() < self.epsilon:
                state_action = self.q_table.loc[observation, :]
                action = np.random.choice(state_action[state_action == np.max(state_action)].index)
            else:
                action = np.random.choice(self.actions)
        return action

    def learn(self, s, a, r, s_, done):
        self.check_state_exist(s_)
        q_predict = self.q_table.loc[s, a]
        if not done:
            q_target = r + self.gamma * self.q_table.loc[s_, :].max()
        else:
            q_target = r
        self.q_table.loc[s, a] += self.lr * (q_target - q_predict)

    def check_state_exist(self, state):
        # print(state)
        if state not in self.q_table.index:
            self.q_table.loc[state] = [0] * len(self.actions)
            # print(self.q_table)

    def save(self, filepath="q_table.csv"):
        self.q_table.to_csv(filepath)

    def load(self, filepath="q_table.csv"):
        if not os.path.exists(filepath):
            print("file {} not exists".format(filepath))
        else:
            self.q_table = pd.read_csv(filepath, header=0, index_col=0).astype(np.float64)

            self.q_table.columns = self.q_table.columns.astype(int)
