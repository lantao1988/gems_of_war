from abc import ABC, abstractmethod

class GameInterface(ABC):
    @abstractmethod
    def all_valid_move(self):
        """
        """

    
    @abstractmethod
    def play_move(self, move):
        """
        """

    @abstractmethod
    def current_state(self):
        """
        """

    @abstractmethod
    def clone(self):
        """
        """
    