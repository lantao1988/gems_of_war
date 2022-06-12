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
#from sklearn.metrics import roc_auc_score


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
    predicts = []
    expect_reward = []
    org_reward = []
    lens = []

    search_rate = 0.6 * (1.03 ** (0 - epoch)) + \
        0.3 if (epoch % 3 == 2) else 0.0
    alpha = 0.7 * (1.03 ** (0 - epoch)) + 0.3
    with_random = epoch % 3
    with_random = 1

    for r in range(100):
        # use_teacher = random.randint(0, 10) > 7 if (epoch % 5 == 1) else False
        # use_teacher = (epoch % 3) == 1
        use_teacher = False
        #with_random = (epoch % 10 != 0)
        # with_random = random.randint(0, 2)
        player = strategies.ModelPlayer(my_model, use_teacher,
                                        with_random, search_rate)
        game = gow_game.GowGame()
        while not game.end():
            moves = game.all_valid_move()
            if len(moves) == 0:
                break
            move = player.next_move(game.current_state(), moves)
            game.play_move(move, True, False)

        reward = player.expect_reward
        discover_loss = player.discover_loss
        expect = [0] * len(reward)
        for i in range(len(reward) - 1, -1, -1):
            # discover_loss[i] += 0.9 * (discover_loss[i+1] if i != len(reward) - 1 else 0.0)
            actual_reward = 0.95 * \
                (expect[i+1] + discover_loss[i+1]
                 ) if i != len(reward) - 1 else -5.0
            actual_reward += r_map[game.actual_states[i]]
            # if actual_reward > reward[i]:
            #     discover_loss[i] = 0
            # elif reward[i] - actual_reward < discover_loss[i]:
            #     discover_loss[i] = reward[i] - actual_reward
            expect[i] = (1-alpha) * reward[i] + alpha * actual_reward

        expect_reward.extend(expect)
        org_reward.extend(reward)
        counts.extend(game.counts)
        states.extend(game.predict_states)
        predicts.extend(game.predicts)

        lens.append(len(game.predicts))

    return counts, predicts, states, expect_reward, org_reward, lens


def run_game():
    # writer = SummaryWriter('runs/fashion_mnist_experiment_1')
    reward_loss_fn = torch.nn.SmoothL1Loss()
    #end_loss_fn = torch.nn.BCEWithLogitsLoss()
#        torch.tensor([0.2, 1.0, 0.6, 0.4, 0.2]).cuda())
    #loss_fn = torch.nn.SmoothL1Loss()

    my_model = GowGameModel()

    if os.path.exists("./model/dict6"):
        my_model.load_state_dict(torch.load("./model/dict6"))
        print("recoverd!!")

    optimizer = torch.optim.Adam(my_model.parameters())
    #optimizer = torch.optim.SGD(my_model.parameters(), lr=0.001, momentum=0.1)

    set_start_method('spawn')
    for epoch in range(100000):
        begin = time.time()
        if True:
            counts, predicts, states, expect_reward, org_reward, lens = run_once(
                (epoch, my_model))
        else:
            with Pool(10) as p:
                examples = p.map(run_once, [(epoch, my_model)]*10)
            counts = []
            predicts = []
            states = []
            expect_reward = []
            org_reward = []
            lens = []
            for e in examples:
                counts.extend(e[0])
                predicts.extend(e[1])
                states.extend(e[2])
                expect_reward.extend(e[3])
                org_reward.extend(e[4])
                lens.extend(e[5])

        print('epoch: ', epoch, 'time: ', time.time() - begin, 'max:', max(lens),
              'game:', len(lens), 'avg:', len(counts) / len(lens))

        my_model.train(True)

        print([myfloat(x) for x in org_reward[0:lens[0]]])
        print([myfloat(x) for x in expect_reward[0:lens[0]]])
        for xx in range(3):
            step = 10000
            for i in range(0, len(counts), step):
                # writer.add_graph(my_model)
                end = min(len(counts), i + step)
                reward = my_model.forward(torch.tensor(states[i:end]),
                                          torch.tensor(counts[i:end]),
                                          torch.tensor(predicts[i:end]))

                loss = reward_loss_fn(reward, torch.tensor(
                    expect_reward[i:end]).cuda())
                print("[{:d}/{:d}], loss: {:.6f}".format(i,
                      len(counts), loss.item()))
                optimizer.zero_grad()
                loss.backward()
                optimizer.step()
        if epoch % 100 == 50:
            torch.save(my_model.state_dict(), "./model/dict6")


def main(argv):
    del argv  # Unused
    run_game()


if __name__ == '__main__':
    app.run(main)
