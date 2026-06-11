import numpy as np
from engine import ChainReactionGame
import copy

class ChainReactionAI:
    @staticmethod
    def get_adjacent_cells(game, i, j):
        directions = [(-1, 0), (1, 0), (0, -1), (0, 1)]  # Up, down, left, right
        adj = []
        for di, dj in directions:
            ni, nj = i + di, j + dj
            if 0 <= ni < game.rows and 0 <= nj < game.cols:
                adj.append((ni, nj))
        return adj

    @staticmethod
    def h1(game):
        # Orb Count Difference
        blue_orbs = np.sum(game.board[:, :, 0] * (game.board[:, :, 1] == 2))
        red_orbs = np.sum(game.board[:, :, 0] * (game.board[:, :, 1] == 1))
        return blue_orbs - red_orbs

    @staticmethod
    def h2(game):
        # Critical Mass Proximity
        h2_val = 0
        for i in range(game.rows):
            for j in range(game.cols):
                if game.board[i, j, 1] == 2 and game.critical_mass[i, j] > 0:
                    h2_val += game.board[i, j, 0] / game.critical_mass[i, j]
                elif game.board[i, j, 1] == 1 and game.critical_mass[i, j] > 0:
                    h2_val -= game.board[i, j, 0] / game.critical_mass[i, j]
        return h2_val

    @staticmethod
    def h3(game):
        # Board Control
        blue_controlled = np.sum(game.board[:, :, 1] == 2)
        red_controlled = np.sum(game.board[:, :, 1] == 1)
        return blue_controlled - red_controlled

    @staticmethod
    def h4(game):
        # Explosion Potential
        score = 0
        for i in range(game.rows):
            for j in range(game.cols):
                if game.board[i, j, 1] == 2:
                    adjacent_red = sum(1 for ni, nj in ChainReactionAI.get_adjacent_cells(game, i, j) if game.board[ni, nj, 1] == 1)
                    if game.critical_mass[i, j] > 0:
                        score += adjacent_red * (game.board[i, j, 0] / game.critical_mass[i, j])
                elif game.board[i, j, 1] == 1:
                    adjacent_blue = sum(1 for ni, nj in ChainReactionAI.get_adjacent_cells(game, i, j) if game.board[ni, nj, 1] == 2)
                    if game.critical_mass[i, j] > 0:
                        score -= adjacent_blue * (game.board[i, j, 0] / game.critical_mass[i, j])
        return score

    @staticmethod
    def h5(game):
        # Stability and Vulnerability
        score = 0
        for i in range(game.rows):
            for j in range(game.cols):
                if game.board[i, j, 1] == 2 and game.critical_mass[i, j] > 0:
                    score += 1 - (game.board[i, j, 0] / game.critical_mass[i, j])
                elif game.board[i, j, 1] == 1:
                    adjacent_blue = sum(1 for ni, nj in ChainReactionAI.get_adjacent_cells(game, i, j) if game.board[ni, nj, 1] == 2)
                    if game.critical_mass[i, j] > 0:
                        score += (game.board[i, j, 0] / game.critical_mass[i, j]) * adjacent_blue
        return score

    @staticmethod
    def evaluate_h1(game):
        return ChainReactionAI.h1(game)

    @staticmethod
    def evaluate_h2(game):
        return ChainReactionAI.h2(game)

    @staticmethod
    def evaluate_h3(game):
        return ChainReactionAI.h3(game)

    @staticmethod
    def evaluate_h4(game):
        return ChainReactionAI.h4(game)

    @staticmethod
    def evaluate_h5(game):
        return ChainReactionAI.h5(game)

    @staticmethod
    def evaluate_combined(game):
        w1, w2, w3, w4, w5 = 1.0, 1.0, 1.0, 2.0, 2.0  # Increased weights for H4, H5
        return (w1 * ChainReactionAI.h1(game) +
                w2 * ChainReactionAI.h2(game) +
                w3 * ChainReactionAI.h3(game) +
                w4 * ChainReactionAI.h4(game) +
                w5 * ChainReactionAI.h5(game))

    @staticmethod
    def minimax(game, depth, alpha, beta, maximizing_player, evaluate_func=None):
        if depth == 0 or game.check_win():
            eval_func = evaluate_func or ChainReactionAI.evaluate_combined
            return eval_func(game), None
        
        valid_moves = ChainReactionAI.get_valid_moves(game, maximizing_player)
        
        best_move = None
        if maximizing_player:
            max_eval = float('-inf')
            for r, c in valid_moves:
                new_game = ChainReactionAI.simulate_move(game, r, c)
                eval, _ = ChainReactionAI.minimax(new_game, depth-1, alpha, beta, False, evaluate_func)
                if eval > max_eval:
                    max_eval = eval
                    best_move = (r, c)
                alpha = max(alpha, eval)
                if beta <= alpha:
                    break
            return max_eval, best_move
        else:
            min_eval = float('inf')
            for r, c in valid_moves:
                new_game = ChainReactionAI.simulate_move(game, r, c)
                eval, _ = ChainReactionAI.minimax(new_game, depth-1, alpha, beta, True, evaluate_func)
                if eval < min_eval:
                    min_eval = eval
                    best_move = (r, c)
                beta = min(beta, eval)
                if beta <= alpha:
                    break
            return min_eval, best_move

    @staticmethod
    def get_valid_moves(game, maximizing_player):
        player = 2 if maximizing_player else 1
        return [(i, j) for i in range(game.rows) for j in range(game.cols)
                if game.board[i, j, 1] in [0, player]]

    @staticmethod
    def order_moves(game, moves, maximizing_player):
        player = 2 if maximizing_player else 1
        move_scores = []
        for r, c in moves:
            score = 0
            if game.board[r, c, 1] == player:
                score += (game.critical_mass[r, c] - game.board[r, c, 0]) * 3
            if (r, c) in [(0, 0), (0, game.cols-1), (game.rows-1, 0), (game.rows-1, game.cols-1)]:
                score += 3
            elif r in [0, game.rows-1] or c in [0, game.cols-1]:
                score += 1
            move_scores.append(score)
        return [move for _, move in sorted(zip(move_scores, moves), reverse=maximizing_player)]

    @staticmethod
    def simulate_move(game, r, c):
        new_game = copy.deepcopy(game)
        new_game.make_move(r, c)
        return new_game