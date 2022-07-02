from absl import app, flags
import os.path
import pybind
import strategies
import gow_game
import torch
import random
import numpy as np
import time
from multiprocessing import Pool, set_start_method
from model import GowGameModel
#from torch.utils.tensorboard import SummaryWriter
from sklearn.metrics import roc_auc_score


class myfloat(float):
    def __str__(self):
        return "%0.2f" % self.real

    def __repr__(self):
        return "%0.2f" % self.real


r_map = {
    pybind.MS_EXTERTURN: 1,
    pybind.MS_OK: -1,
    pybind.MS_FOUR: 0
}

def run_once(param):
    epoch = param[0]
    if isinstance(param[1], GowGameModel):
        my_model = param[1]
    else:
        my_model = GowGameModel()
        my_model.load_state_dict(param[1])
    my_model.train(False)

    counts = []
    states = []
    maps = []
    expect_reward = []
    org_reward = []
    lens = []

    search_rate = 0.6 * (1.03 ** (0 - epoch)) + \
        0.3 if (epoch % 3 == 2) else 0.0
    alpha = 0.7 * (1.03 ** (0 - epoch)) + 0.3
    lbd = 0.95

    for r in range(1000):
        player = strategies.ModelPlayer(my_model, search_rate)
        game = gow_game.GowGame()
        while not game.end():
            moves = game.all_valid_move()
            if len(moves) == 0:
                break
            count = game.count
            move = player.next_move(game.current_state(), moves)
            state = game.play_move(move, False)
            player.collect_states(state, count)

        reward = player.expect_reward
        discover_loss = player.discover_loss
        expect = [0] * len(reward)
        for i in range(len(reward) - 1, -1, -1):
            actual_reward = lbd * (expect[i+1] + discover_loss[i+1]) if i != len(reward) - 1 else (0-1/(1-lbd))
            actual_reward += player.states[i] - 1
            expect[i] = (1-alpha) * reward[i] + alpha * actual_reward

        expect_reward.extend(expect)
        org_reward.extend(reward)
        counts.extend(player.counts)
        states.extend(player.states)
        maps.extend(player.maps)

        lens.append(len(player.maps))

    return counts, maps, states, expect_reward, org_reward, lens


def run_game():
    # writer = SummaryWriter('runs/fashion_mnist_experiment_1')
    reward_loss_fn = torch.nn.SmoothL1Loss()
    state_loss_fn = torch.nn.CrossEntropyLoss(reduction='none')

    my_model = GowGameModel().cuda()

    model_name = './model/cnn_model'
    if os.path.exists(model_name):
        my_model.load_state_dict(torch.load(model_name))
        print("recoverd!!")

    optimizer = torch.optim.Adam(my_model.parameters())

    set_start_method('spawn')
    for epoch in range(100000):
        begin = time.time()
        if True:
            counts, maps, states, expect_reward, org_reward, lens = run_once(
                (epoch, my_model))
        else:
            with Pool(10) as p:
                examples = p.map(run_once, [(epoch, my_model)]*10)
            counts = []
            maps = []
            states = []
            expect_reward = []
            org_reward = []
            lens = []
            for e in examples:
                counts.extend(e[0])
                maps.extend(e[1])
                states.extend(e[2])
                expect_reward.extend(e[3])
                org_reward.extend(e[4])
                lens.extend(e[5])

        total = len(counts)

        print('epoch: ', epoch, 'time: ', time.time() - begin, 'max:', max(lens),
              'game:', len(lens), 'avg:', total / len(lens))

        my_model.train(False)

        state_count = [0] * 3
        for s in states:
            state_count[s] += 1

        loss_weight_mp = [total / c / 3 for c in state_count]
        print("weights:", loss_weight_mp)
        loss_weights = [0.0] * total
        for i in range(total):
            loss_weights[i] = loss_weight_mp[states[i]]

        auc_size = 1000
        predict, _ = my_model.forward(torch.tensor(maps[0:auc_size], dtype=int).cuda(),
                                      torch.tensor(counts[0:auc_size], dtype=int).cuda())
        eye3 = np.eye(3)
        actual = [eye3[i] for i in states[0:auc_size]]
        print('auc:', roc_auc_score(actual, predict.cpu().detach().numpy(), multi_class='ovr', sample_weight=loss_weights[0:auc_size]))
        my_model.train(True)

        print([myfloat(x) for x in org_reward[0:lens[0]]])
        print([myfloat(x) for x in expect_reward[0:lens[0]]])
        for xx in range(10):
            step = 1000
            for i in range(0, total, step):
                # writer.add_graph(my_model)
                end = min(total, i + step)
                if end - i < 0.5 * step:
                    continue
                predict, reward = my_model.forward(torch.tensor(maps[i:end], dtype=int).cuda(),
                                                    torch.tensor(counts[i:end], dtype=int).cuda())
                loss = state_loss_fn(predict, torch.tensor(states[i:end], dtype=int).cuda())
                loss = (loss * torch.tensor(loss_weights[i:end]).cuda()).mean()
                loss = loss + reward_loss_fn(reward, torch.tensor(
                    expect_reward[i:end]).cuda())
                print(time.strftime('%y-%m-%d %H:%M:%S', time.localtime()), "[{:d}/{:d}], loss: {:.6f}".format(i,
                      total, loss.item()))
                optimizer.zero_grad()
                loss.backward()
                optimizer.step()
        if epoch % 10 == 9:
            torch.save(my_model.state_dict(), model_name)


def main(argv):
    del argv  # Unused
    run_game()


if __name__ == '__main__':
    app.run(main)
