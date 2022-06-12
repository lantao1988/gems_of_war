import player_interface
import pybind
import random

import torch

class ModelPlayer(player_interface.PlayerInterface):
    def __init__(self, model, use_teacher = False, with_random = 0, search_rate = 0.0):
        self.model = model
        self.use_teacher = use_teacher
        self.with_random = with_random
        self.search_rate = search_rate
        # self.current_convert = torch.arange(0.0, 3.0).cuda()
        # self.future_convert = torch.arange(0.0, 9.0).cuda()
        self.expect_reward = []
        self.discover_loss = []

    def next_move(self, current_state, valid_moves):
        if self.use_teacher:
            move = DefaultPlayer(self.with_random).next_move(
                current_state, valid_moves)
            for m in valid_moves:
                if move.movement == m.movement:
                    valid_moves = [m]
                    break

        moves = []
        states = []
        for m in valid_moves:
            moves.append(m.c33)
            states.append(int(m.moveResult.state))

        reward = self.model(
            torch.tensor(states),
            torch.tensor([current_state[0]] * len(valid_moves)),
            torch.tensor(moves))

        s = reward
        self.expect_reward.append(s.max().item())
        if not self.use_teacher and random.random() < self.search_rate:
            s = torch.softmax(s, dim=0)
            idx = random.choices(range(s.shape[0]), s)[0]
            self.discover_loss.append(s.max().item() - s[idx].item())
            return valid_moves[idx]
        self.discover_loss.append(0.0)
        return valid_moves[torch.argmax(s).item()]


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