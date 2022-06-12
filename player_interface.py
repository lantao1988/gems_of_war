from abc import ABC, abstractmethod

class PlayerInterface(ABC):

    @abstractmethod
    def next_move(self, current, all_moves):
        """
        """