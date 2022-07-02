import player_interface
import pybind
import random

import torch

class ModelPlayer(player_interface.PlayerInterface):
    def __init__(self, model, search_rate = 0.0):
        self.model = model
        self.search_rate = search_rate
        self.current_convert = torch.arange(-1.0, 2.0).cuda()
        self.expect_reward = []
        self.discover_loss = []
        self.maps = []
        self.states = []
        self.counts = []

    def next_move(self, current_state, valid_moves):
        maps = []
        counts = [current_state[0]] * len(valid_moves) 
        for m in valid_moves:
            maps.append(m.map)

        predict, reward = self.model(torch.tensor(maps, dtype=int).cuda(),
            torch.tensor(counts, dtype=int).cuda())

        s = reward + torch.matmul(torch.softmax(predict, dim=1), self.current_convert)
        self.expect_reward.append(s.max().item())

        if random.random() < self.search_rate:
            idx = random.choices(range(s.shape[0]), torch.softmax(s, dim=0))[0]
        else:
            idx = torch.argmax(s).item()
        self.discover_loss.append(s[idx].item() - s.max().item())
        self.maps.append(valid_moves[idx].map)
        return valid_moves[idx]

    def collect_states(self, state, count):
        self.states.append(int(state) - 1)
        self.counts.append(count)


class DefaultPlayer(player_interface.PlayerInterface):
    def __init__(self, with_random):
        self.with_random = with_random

    def next_move(self, current_state, valid_moves):
        assert(len(valid_moves) > 0)
        mp = {
            x: [] for x in [
                pybind.MS_OK,
                pybind.MS_FOUR,
                pybind.MS_EXTERTURN]
        }
        for result in valid_moves:
            mp[result.moveResult.state].append(result)
        c = []
        if len(mp[pybind.MS_EXTERTURN]) > 0:
            c = mp[pybind.MS_EXTERTURN]
        elif len(mp[pybind.MS_FOUR]) > 0:
            c = mp[pybind.MS_FOUR]
        else:
            c = mp[pybind.MS_OK]

        if self.with_random == 0:
            return c[random.randint(0, len(c)-1)]
        if self.with_random == 1:
            c.sort(reverse=True, key=lambda x: x.movement[0][0])
            return c[0]
        else:
            c.sort(key=lambda x: x.movement[0][0])
            return c[0]