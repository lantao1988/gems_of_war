import game_interface
import pybind
import copy
import random
import shutil
import math
#from colorama import Fore, Style

class GowGame(game_interface.GameInterface):
    def __init__(self):
        self.game = pybind.Game(True, 9, 9, random.randint(0,1000000))
        self.count = 15
        self.total_step = 0
        self.predicts = []
        self.predict_states = []
        self.counts = []
        self.actual_states = []

    def all_valid_move(self, show=False):
        moves = self.game.AllValidMove()
        if not show:
            return moves
        mp = self.game.CurrentState()

        games = []
        for i in range(len(moves)):
            m = moves[i]
            exter_info = '{:d}:({:d},{:d}),{:d},\x1b[6;30;{:d}m{:d}\x1b[0m'.format(i, m.movement[0][0], m.movement[0][1], int(m.movement[1]), 40+int(m.moveResult.state), int(m.moveResult.state))
            games.append((mp, {m.movement[0], self.game.NewPos(m.movement[0], m.movement[1])}, exter_info))
        self._print_games(games)

    def play_move(self, move, train=False, show=True):
        assert(self.count > 0)
        self.total_step = self.total_step + 1
        if isinstance(move, pybind.PredictResult):
            movement = move.movement
        elif isinstance(move, int):
            move = self.all_valid_move()[move]
            movement = move.movement
        else:
            movement = move
            if not isinstance(movement[1], pybind.MoveType):
                m = [pybind.MT_UP,
                     pybind.MT_DOWN,
                     pybind.MT_LEFT,
                     pybind.MT_RIGHT]
                movement[1] = m[movement[1]]
        result = self.game.Move(movement, show)
        state = result.state
        if state == pybind.MS_EXTERTURN:
            self.count = self.count + 1
        elif state == pybind.MS_OK:
            self.count = self.count - 1
        if train:
            self.counts.append(self.count)
            self.predict_states.append(int(move.moveResult.state))
            self.predicts.append(move.c33)
            self.actual_states.append(state)
        if show:
            self._print_games(result.snapshots)

    def _print_games(self, games):
        snapshots = games
        highligths = []
        for i in range(len(snapshots)):
            highligths.append(set(snapshots[i][1]))

        screen_size = shutil.get_terminal_size(fallback=(80, 60))
        wide = screen_size[0]
        max_game_one_line = math.floor((wide - 9) / (9 + 8) + 1)
        for game_start in range(0, len(snapshots), max_game_one_line):
            game_end = min(game_start + max_game_one_line, len(snapshots))
            for row in range(len(snapshots[0][0])):
                s = ''
                for game in range(game_start, game_end):
                    for col in range(len(snapshots[0][0][0])):
                        c = snapshots[game][0][row][col]
                        if len(highligths[game]) == 0:
                            s += '\x1b[6;30;'+str(40+int(c))+'m' + str(int(c)) + '\x1b[0m'
                        elif (row, col) not in highligths[game]:
                            s += str(int(c))
                        else:
                            s += '\x1b[6;30;42m' + str(int(c)) + '\x1b[0m'
                    s += '\t'
                print(s)

            if len(snapshots[game]) == 3:
                s = ''
                for game in range(game_start, game_end):
                    s += snapshots[game][2] + '\t'
                print(s)
            print('-'*(wide-1))

    def current_state(self):
        return (self.count, self.game.CurrentState())

    def show(self):
        print('total: ', self.total_step, 'left count:', self.count)
        mp = self.game.CurrentState()
        ratio = 4
        new_mp = [[0]*(ratio*len(mp[0])) for i in range(len(mp)*ratio)]
        for i in range(len(new_mp)):
            for j in range(len(new_mp[0])):
                new_mp[i][j] = mp[math.floor(i/ratio)][math.floor(j/ratio)]
        self._print_games([(new_mp, ())])

    def clone(self):
        return copy.deepcopy(self)

    def end(self):
        return self.count <= 0


    def hint(self):
        self.all_valid_move(True)

    def move(self, m):
        self.play_move(m)