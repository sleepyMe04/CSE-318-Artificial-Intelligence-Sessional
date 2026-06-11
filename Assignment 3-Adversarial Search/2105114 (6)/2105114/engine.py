import numpy as np
import copy
import os
from collections import deque

class ChainReactionGame:
    def __init__(self, rows=9, cols=6):
        self.rows = rows
        self.cols = cols
        self.board = np.zeros((rows, cols, 2), dtype=int)  # [count, player]
        self.players = ['R', 'B']
        self.current_player = 0  # R starts
        self.critical_mass = self.init_critical_mass()
        self.state_file = 'game_state.txt'

    def init_critical_mass(self):
        mass = np.full((self.rows, self.cols), 4)
        mass[0, :] = mass[-1, :] = 3
        mass[:, 0] = mass[:, -1] = 3
        mass[0, 0] = mass[0, -1] = mass[-1, 0] = mass[-1, -1] = 2
        return mass

    def is_valid_move(self, r, c):
        cell = self.board[r, c]
        return cell[0] == 0 or cell[1] == self.current_player + 1

    def make_move(self, r, c):
        if not self.is_valid_move(r, c):
            return False, None
        
        self.board[r, c][0] += 1
        self.board[r, c][1] = self.current_player + 1
        
        # Process all explosions iteratively
        success, explosion_player = self.process_explosions(r, c)
        if not success:
            return False, explosion_player  # Infinite explosion, return triggering player
        
        if self.check_win():
            return True, None
        
        self.current_player = 1 - self.current_player
        return True, None

    def process_explosions(self, r, c):
        queue = deque()
        seen_states = set()
        max_iterations = 1000  # Prevent infinite loops
        iteration = 0
        
        queue.append((r, c))
        
        while queue:
            if iteration >= max_iterations:

                return False, self.current_player  # Infinite loop detected
            # Serialize board state
            # state = tuple(self.board.flatten())
            # if state in seen_states:
            #     print("Cycle detected in explosions")
            #     print("Current player:", self.current_player)
            #     return False, self.current_player  # Cycle detected
            # seen_states.add(state)
            
            r, c = queue.popleft()
            if self.board[r, c][0] < self.critical_mass[r, c]:
                continue

            player = self.board[r, c][1]
            self.board[r, c][0] = 0  # Clear orb count
            self.board[r, c][1] = 0  # Clear player ownership

            # Distribute orbs to adjacent cells
            for dr, dc in [(0, 1), (1, 0), (0, -1), (-1, 0)]:
                nr, nc = r + dr, c + dc
                if 0 <= nr < self.rows and 0 <= nc < self.cols:
                    self.board[nr, nc][0] += 1  # Add orb
                    # Always update ownership to the exploding player
                    self.board[nr, nc][1] = player
                    if self.board[nr, nc][0] >= self.critical_mass[nr, nc]:
                        queue.append((nr, nc))
            
            iteration += 1
        
        return True, None

    def check_win(self):
        player1_orbs = np.sum((self.board[:, :, 1] == 1) & (self.board[:, :, 0] > 0))  # Red orbs
        player2_orbs = np.sum((self.board[:, :, 1] == 2) & (self.board[:, :, 0] > 0))  # Blue orbs

        has_red = player1_orbs > 0
        has_blue = player2_orbs > 0

        both_played = self.board[:, :, 0].sum() > 1  # Total orbs > 1 implies both have played

        if both_played:
            if has_blue and not has_red:
                return True  # Blue wins
            if has_red and not has_blue:
                return True  # Red wins
        return False  # No winner yet

    def to_file(self, move_type):
        with open(self.state_file, 'w') as f:
            f.write(f"{move_type} Move:\n")
            for row in self.board:
                line = []
                for cell in row:
                    if cell[1] == 0:
                        line.append('0')
                    else:
                        line.append(f"{cell[0]}{self.players[cell[1]-1]}")
                f.write(" ".join(line) + "\n")
    
    def from_file(self):
        if os.path.exists(self.state_file):
            with open(self.state_file, 'r') as f:
                lines = [line.strip() for line in f.readlines() if line.strip()]
                if len(lines) >= 10:
                    move_type = lines[0]
                    for i in range(9):
                        cells = lines[i+1].split()
                        for j in range(6):
                            if cells[j] == '0':
                                self.board[i, j] = [0, 0]
                            else:
                                count = int(cells[j][:-1])
                                player = 1 if cells[j][-1] == 'R' else 2
                                self.board[i, j] = [count, player]
                    return move_type
        return None