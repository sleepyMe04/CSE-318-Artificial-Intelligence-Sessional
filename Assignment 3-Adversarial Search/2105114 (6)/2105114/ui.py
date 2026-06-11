import pygame
import numpy as np
from engine import ChainReactionGame
from minimax import ChainReactionAI
import os
import sys
import random
from pygame.locals import *

# Initialize Pygame
pygame.init()
pygame.font.init()

# Constants
SCREEN_WIDTH = 900
SCREEN_HEIGHT = 700
BOARD_OFFSET_X = 150
BOARD_OFFSET_Y = 100
CELL_SIZE = 60
COLORS = {
    'background_start': (44, 62, 80),  # Dark blue for gradient
    'background_end': (76, 161, 175),  # Light teal for gradient
    'board': (236, 239, 241, 200),     # Semi-transparent light gray
    'red': (233, 30, 99),             # Vibrant pink-red
    'blue': (33, 150, 243),           # Bright blue
    'text': (255, 255, 255),          # White for text
    'text_shadow': (50, 50, 50),      # Shadow for text
    'highlight': (255, 215, 0),       # Gold for highlight
    'highlight_fill': (255, 215, 0, 100),  # Semi-transparent gold
    'grid': (120, 144, 156),         # Soft gray for grid
    'status_bg': (33, 33, 33, 220),   # Semi-transparent dark status bar
    'button_normal': (60, 60, 60, 220),  # Dark button background
    'button_hover': (80, 80, 80, 220),   # Lighter on hover
}

class MinimaxAgent:
    def __init__(self, depth, evaluate_func, player):
        self.depth = depth
        self.evaluate_func = evaluate_func
        self.player = player  # 1 for Red, 2 for Blue

    def get_move(self, game):
        if self.player == game.current_player + 1:
            maximizing = (self.player == 2)  # Blue maximizes
            _, move = ChainReactionAI.minimax(game, self.depth, float('-inf'), float('inf'), maximizing, self.evaluate_func)
            return move
        return None

class RandomAgent:
    def __init__(self, player):
        self.player = player  # 1 for Red

    def get_move(self, game):
        if self.player == game.current_player + 1:
            valid_moves = []
            for i in range(game.rows):
                for j in range(game.cols):
                    if game.is_valid_move(i, j):
                        valid_moves.append((i, j))
            if valid_moves:
                return random.choice(valid_moves)
        return None

class ChainReactionUI:
    def __init__(self):
        self.screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
        pygame.display.set_caption('Chain Reaction')
        self.clock = pygame.time.Clock()
        self.font = pygame.font.SysFont('Verdana', 28, bold=True)
        self.big_font = pygame.font.SysFont('Verdana', 48, bold=True)
        self.title_font = pygame.font.SysFont('Verdana', 64, bold=True)
        self.game = None
        self.selected_cell = None
        self.ai_thinking = False
        self.game_over = False
        self.game_mode = None  # 'human_vs_ai', 'ai_vs_ai', 'random_vs_ai'
        self.ai1 = None
        self.ai2 = None
        self.random_agent = None
        self.animations = []
        self.pulse_time = 0
        self.click_lock = False
        # UI state
        self.state = 'title'  # 'title', 'heuristic_selection', 'game'
        self.mode_buttons = [
            pygame.Rect(SCREEN_WIDTH // 2 - 150, SCREEN_HEIGHT // 2 - 90, 300, 50),  # Human vs. AI
            pygame.Rect(SCREEN_WIDTH // 2 - 150, SCREEN_HEIGHT // 2 - 20, 300, 50),  # AI vs. AI
            pygame.Rect(SCREEN_WIDTH // 2 - 150, SCREEN_HEIGHT // 2 + 50, 300, 50),  # Random Agent vs. AI
        ]
        self.heuristic_options = [
            ('Combined', ChainReactionAI.evaluate_combined),
            ('H1', ChainReactionAI.evaluate_h1),
            ('H2', ChainReactionAI.evaluate_h2),
            ('H3', ChainReactionAI.evaluate_h3),
            ('H4', ChainReactionAI.evaluate_h4),
            ('H5', ChainReactionAI.evaluate_h5)
        ]
        self.ai1_heuristic = None
        self.ai2_heuristic = ChainReactionAI.evaluate_h1
        self.ai1_depth = 4
        self.ai2_depth = 2
        self.heuristic_buttons = [
            pygame.Rect(SCREEN_WIDTH // 2 - 150, SCREEN_HEIGHT // 2 - 120 + i * 60, 300, 50)
            for i in range(len(self.heuristic_options))
        ]
        self.start_button = pygame.Rect(SCREEN_WIDTH // 2 - 150, SCREEN_HEIGHT // 2 + 180, 300, 50)
        self.selecting_for = None  # 'ai1' or 'ai2'
        # Back button (bottom center)
        self.back_button = pygame.Rect(SCREEN_WIDTH // 2 - 50, SCREEN_HEIGHT - 60, 100, 40)

    def initialize_game(self):
        self.game = ChainReactionGame()
        if self.game_mode == 'ai_vs_ai':
            self.ai1 = MinimaxAgent(self.ai1_depth, self.ai1_heuristic or ChainReactionAI.evaluate_combined, player=1)
            self.ai2 = MinimaxAgent(self.ai2_depth, self.ai2_heuristic, player=2)
        elif self.game_mode == 'random_vs_ai':
            self.random_agent = RandomAgent(player=1)
            self.ai2 = MinimaxAgent(self.ai2_depth, self.ai2_heuristic, player=2)
        else:  # human_vs_ai
            self.ai2 = MinimaxAgent(self.ai2_depth, self.ai2_heuristic, player=2)
        self.initialize_state_file()

    def initialize_state_file(self):
        if os.path.exists(self.game.state_file):
            os.remove(self.game.state_file)
        self.game.to_file("Human" if self.game_mode == 'human_vs_ai' else "AI")

    def draw_gradient_background(self):
        for y in range(SCREEN_HEIGHT):
            ratio = y / SCREEN_HEIGHT
            color = (
                int(COLORS['background_start'][0] * (1 - ratio) + COLORS['background_end'][0] * ratio),
                int(COLORS['background_start'][1] * (1 - ratio) + COLORS['background_end'][1] * ratio),
                int(COLORS['background_start'][2] * (1 - ratio) + COLORS['background_end'][2] * ratio)
            )
            pygame.draw.line(self.screen, color, (0, y), (SCREEN_WIDTH, y))

    def draw_back_button(self, mouse_pos):
        color = COLORS['button_hover'] if self.back_button.collidepoint(mouse_pos) else COLORS['button_normal']
        pygame.draw.rect(self.screen, color, self.back_button, border_radius=10)
        pygame.draw.rect(self.screen, COLORS['highlight'], self.back_button, 2, border_radius=10)
        text = self.font.render("Back", True, COLORS['text'])
        text_rect = text.get_rect(center=self.back_button.center)
        self.screen.blit(text, text_rect)

    def draw_title_screen(self):
        self.draw_gradient_background()
        title = self.title_font.render("Chain Reaction", True, COLORS['text'])
        shadow = self.title_font.render("Chain Reaction", True, COLORS['text_shadow'])
        title_rect = title.get_rect(center=(SCREEN_WIDTH // 2, SCREEN_HEIGHT // 2 - 150))
        self.screen.blit(shadow, (title_rect.x + 4, title_rect.y + 4))
        self.screen.blit(title, title_rect)

        mouse_pos = pygame.mouse.get_pos()
        for i, button in enumerate(self.mode_buttons):
            color = COLORS['button_hover'] if button.collidepoint(mouse_pos) else COLORS['button_normal']
            pygame.draw.rect(self.screen, color, button, border_radius=10)
            pygame.draw.rect(self.screen, COLORS['highlight'], button, 2, border_radius=10)
            text = self.font.render(
                "Human vs. AI" if i == 0 else "AI vs. AI" if i == 1 else "Random Agent vs. AI",
                True, COLORS['text']
            )
            text_rect = text.get_rect(center=button.center)
            self.screen.blit(text, text_rect)

        self.draw_back_button(mouse_pos)  # No action on title

    def draw_heuristic_selection(self):
        self.draw_gradient_background()
        title = self.font.render(f"Select Heuristic for {'AI1 (Red)' if self.selecting_for == 'ai1' else 'AI2 (Blue)'}", 
                                True, COLORS['text'])
        shadow = self.font.render(f"Select Heuristic for {'AI1 (Red)' if self.selecting_for == 'ai1' else 'AI2 (Blue)'}", 
                                 True, COLORS['text_shadow'])
        title_rect = title.get_rect(center=(SCREEN_WIDTH // 2, SCREEN_HEIGHT // 2 - 180))
        self.screen.blit(shadow, (title_rect.x + 2, title_rect.y + 2))
        self.screen.blit(title, title_rect)

        mouse_pos = pygame.mouse.get_pos()
        for i, button in enumerate(self.heuristic_buttons):
            color = COLORS['button_hover'] if button.collidepoint(mouse_pos) else COLORS['button_normal']
            pygame.draw.rect(self.screen, color, button, border_radius=10)
            pygame.draw.rect(self.screen, COLORS['highlight'], button, 2, border_radius=10)
            text = self.font.render(self.heuristic_options[i][0], True, COLORS['text'])
            text_rect = text.get_rect(center=button.center)
            self.screen.blit(text, text_rect)

        if self.selecting_for is None:
            start_color = COLORS['button_hover'] if self.start_button.collidepoint(mouse_pos) else COLORS['button_normal']
            pygame.draw.rect(self.screen, start_color, self.start_button, border_radius=10)
            pygame.draw.rect(self.screen, COLORS['highlight'], self.start_button, 2, border_radius=10)
            start_text = self.font.render("Start Game", True, COLORS['text'])
            start_rect = start_text.get_rect(center=self.start_button.center)
            self.screen.blit(start_text, start_rect)

        self.draw_back_button(mouse_pos)

    def draw_board(self):
        board_width = self.game.cols * CELL_SIZE
        board_height = self.game.rows * CELL_SIZE
        board_surface = pygame.Surface((board_width, board_height), pygame.SRCALPHA)
        board_surface.fill(COLORS['board'])

        for i in range(self.game.rows + 1):
            pygame.draw.line(board_surface, COLORS['grid'], (0, i * CELL_SIZE), (board_width, i * CELL_SIZE), 2)
        for j in range(self.game.cols + 1):
            pygame.draw.line(board_surface, COLORS['grid'], (j * CELL_SIZE, 0), (j * CELL_SIZE, board_height), 2)

        self.screen.blit(board_surface, (BOARD_OFFSET_X, BOARD_OFFSET_Y))

        current_time = pygame.time.get_ticks()
        for i in range(self.game.rows):
            for j in range(self.game.cols):
                count, player = self.game.board[i, j]
                if count > 0:
                    color = COLORS['red'] if player == 1 else COLORS['blue']
                    x_center = BOARD_OFFSET_X + j * CELL_SIZE + CELL_SIZE // 2
                    y_center = BOARD_OFFSET_Y + i * CELL_SIZE + CELL_SIZE // 2
                    ball_radius = CELL_SIZE // 6  # Small radius to fit multiple balls

                    scale = 1.0
                    for anim in self.animations[:]:
                        if anim[0] == i and anim[1] == j:
                            elapsed = current_time - anim[3]
                            if elapsed < 300:
                                scale = 1.0 + 0.5 * (1 - elapsed / 300)
                            else:
                                self.animations.remove(anim)
                            break

                    # Arrange balls based on count
                    scaled_radius = int(ball_radius * scale)
                    if count == 1:
                        positions = [(x_center, y_center)]
                    elif count == 2:
                        positions = [(x_center - ball_radius, y_center), (x_center + ball_radius, y_center)]
                    elif count == 3:
                        positions = [
                            (x_center, y_center - ball_radius),
                            (x_center - ball_radius, y_center + ball_radius // 2),
                            (x_center + ball_radius, y_center + ball_radius // 2)
                        ]
                    elif count == 4:
                        positions = [
                            (x_center - ball_radius, y_center - ball_radius),
                            (x_center + ball_radius, y_center - ball_radius),
                            (x_center - ball_radius, y_center + ball_radius),
                            (x_center + ball_radius, y_center + ball_radius)
                        ]
                    else:
                        # For count >= 5, use a grid layout (up to 3x3)
                        grid_size = int(np.ceil(np.sqrt(count)))
                        offset = ball_radius * 2
                        positions = []
                        for n in range(count):
                            row = n // grid_size
                            col = n % grid_size
                            x = x_center - offset * (grid_size - 1) / 2 + col * offset
                            y = y_center - offset * (grid_size - 1) / 2 + row * offset
                            positions.append((x, y))

                    # Draw each 3D ball
                    for x, y in positions:
                        # Gradient for 3D effect
                        for r in range(scaled_radius, 0, -1):
                            ratio = r / scaled_radius
                            shade = (
                                int(color[0] * (0.6 + 0.4 * ratio)),
                                int(color[1] * (0.6 + 0.4 * ratio)),
                                int(color[2] * (0.6 + 0.4 * ratio)),
                                max(50, int(255 * (0.7 + 0.3 * ratio)))
                            )
                            pygame.draw.circle(self.screen, shade, (int(x), int(y)), r)

                        # Highlight for 3D effect
                        highlight_pos = (int(x - scaled_radius // 3), int(y - scaled_radius // 3))
                        pygame.draw.circle(self.screen, (255, 255, 255, 100), highlight_pos, scaled_radius // 4)

        if self.selected_cell:
            i, j = self.selected_cell
            rect = pygame.Rect(BOARD_OFFSET_X + j * CELL_SIZE + 2, BOARD_OFFSET_Y + i * CELL_SIZE + 2, 
                              CELL_SIZE - 4, CELL_SIZE - 4)
            pulse = 0.8 + 0.2 * np.sin(self.pulse_time / 200)
            fill_surface = pygame.Surface((CELL_SIZE - 4, CELL_SIZE - 4), pygame.SRCALPHA)
            fill_surface.fill((*COLORS['highlight'][:3], int(100 * pulse)))
            self.screen.blit(fill_surface, (BOARD_OFFSET_X + j * CELL_SIZE + 2, BOARD_OFFSET_Y + i * CELL_SIZE + 2))
            pygame.draw.rect(self.screen, COLORS['highlight'], rect, int(3 + pulse))

    def draw_status(self):
        status_surface = pygame.Surface((SCREEN_WIDTH, 60), pygame.SRCALPHA)
        status_surface.fill(COLORS['status_bg'])
        self.screen.blit(status_surface, (0, 0))
        pygame.draw.line(self.screen, COLORS['grid'], (0, 60), (SCREEN_WIDTH, 60), 2)

        player = "Red" if self.game.current_player == 0 else "Blue"
        color = COLORS['red'] if self.game.current_player == 0 else COLORS['blue']
        turn_text = self.font.render(f"Turn: {player}", True, COLORS['text'])
        shadow = self.font.render(f"Turn: {player}", True, COLORS['text_shadow'])
        self.screen.blit(shadow, (22, 17))
        self.screen.blit(turn_text, (20, 15))

        if self.ai_thinking and self.game_mode == 'human_vs_ai':
            ai_text = self.font.render("AI Thinking...", True, COLORS['blue'])
            shadow = self.font.render("AI Thinking...", True, COLORS['text_shadow'])
            self.screen.blit(shadow, (SCREEN_WIDTH - 178, 17))
            self.screen.blit(ai_text, (SCREEN_WIDTH - 180, 15))

        if not self.game_over and self.game.current_player == 0 and not self.ai_thinking and self.game_mode == 'human_vs_ai':
            instr_text = self.font.render("Click to place orb", True, COLORS['text'])
            shadow = self.font.render("Click to place orb", True, COLORS['text_shadow'])
            rotated = pygame.transform.rotate(instr_text, 90)
            rotated_shadow = pygame.transform.rotate(shadow, 90)
            self.screen.blit(rotated_shadow, (32, BOARD_OFFSET_Y + 102))
            self.screen.blit(rotated, (30, BOARD_OFFSET_Y + 100))
            arrow_points = [
                (50, BOARD_OFFSET_Y + 150),
                (40, BOARD_OFFSET_Y + 160),
                (60, BOARD_OFFSET_Y + 160)
            ]
            pygame.draw.polygon(self.screen, COLORS['red'], arrow_points)

    def handle_click(self, pos):
        if self.game_over or self.ai_thinking or self.game_mode != 'human_vs_ai' or self.click_lock:
            return
        self.click_lock = True
        x, y = pos
        if self.back_button.collidepoint(pos):
            self.state = 'heuristic_selection'
            self.selecting_for = 'ai2'
            self.game_over = False
            self.game = None
            self.click_lock = False
            return
        j = (x - BOARD_OFFSET_X) // CELL_SIZE
        i = (y - BOARD_OFFSET_Y) // CELL_SIZE
        if 0 <= i < self.game.rows and 0 <= j < self.game.cols and self.game.is_valid_move(i, j):
            self.selected_cell = (i, j)
            self.animations.append((i, j, 1.5, pygame.time.get_ticks()))
            self.draw_board()
            self.draw_status()
            pygame.display.flip()
            pygame.time.delay(400)
            success, explosion_player = self.game.make_move(i, j)
            if success == False:
                self.game_over = True
                self.show_game_over(explosion_winner=explosion_player)
            else:
                self.game.to_file("Human")
                self.draw_board()
                self.draw_status()
                pygame.display.flip()
                pygame.time.delay(400)
                self.selected_cell = None
                if self.game.check_win():
                    self.game_over = True
                    self.show_game_over()
                elif not self.game_over:
                    self.ai_turn()
        pygame.event.clear(MOUSEBUTTONDOWN)
        self.click_lock = False

    def ai_turn(self):
        if self.game_over:
            return
        self.ai_thinking = True
        self.draw_board()
        self.draw_status()
        pygame.display.flip()
        current_player = self.game.current_player
        move = None
        if self.game_mode == 'ai_vs_ai':
            move = self.ai1.get_move(self.game) if current_player == 0 else self.ai2.get_move(self.game)
        elif self.game_mode == 'random_vs_ai' and current_player == 0:
            move = self.random_agent.get_move(self.game)
        elif self.game_mode in ['human_vs_ai', 'random_vs_ai'] and current_player == 1:
            move = self.ai2.get_move(self.game)
        if move:
            self.selected_cell = move
            self.animations.append((*move, 1.5, pygame.time.get_ticks()))
            self.draw_board()
            self.draw_status()
            pygame.display.flip()
            pygame.time.delay(500)
            success, explosion_player = self.game.make_move(*move)
            if success == False:
                self.game_over = True
                self.show_game_over(explosion_winner=explosion_player)
            else:
                self.game.to_file("AI")
                self.draw_board()
                self.draw_status()
                pygame.display.flip()
                pygame.time.delay(500)
                self.selected_cell = None
                if self.game.check_win():
                    self.game_over = True
                    self.show_game_over()
        self.ai_thinking = False

    def show_game_over(self, explosion_winner=None):
        if explosion_winner is not None:
            winner = "Red" if explosion_winner == 0 else "Blue"
            color = COLORS['red'] if explosion_winner == 0 else COLORS['blue']
        else:
            player1_orbs = np.sum((self.game.board[:, :, 1] == 1) & (self.game.board[:, :, 0] > 0))
            player2_orbs = np.sum((self.game.board[:, :, 1] == 2) & (self.game.board[:, :, 0] > 0))
            winner = "Blue" if player2_orbs > 0 and player1_orbs == 0 else "Red" if player1_orbs > 0 and player2_orbs == 0 else "Unknown"
            color = COLORS['blue'] if winner == "Blue" else COLORS['red'] if winner == "Red" else COLORS['text']
        
        overlay = pygame.Surface((SCREEN_WIDTH, SCREEN_HEIGHT), pygame.SRCALPHA)
        overlay.fill((0, 0, 0, 180))
        self.screen.blit(overlay, (0, 0))
        text1 = self.big_font.render("Game Over!", True, COLORS['text'])
        text2 = self.big_font.render(f"{winner} Wins!", True, color)
        shadow1 = self.big_font.render("Game Over!", True, COLORS['text_shadow'])
        shadow2 = self.big_font.render(f"{winner} Wins!", True, COLORS['text_shadow'])
        rect1 = text1.get_rect(center=(SCREEN_WIDTH // 2, SCREEN_HEIGHT // 2 - 30))
        rect2 = text2.get_rect(center=(SCREEN_WIDTH // 2, SCREEN_HEIGHT // 2 + 30))
        self.screen.blit(shadow1, (rect1.x + 3, rect1.y + 3))
        self.screen.blit(shadow2, (rect2.x + 3, rect2.y + 3))
        self.screen.blit(text1, rect1)
        self.screen.blit(text2, rect2)
        
        button_rect = pygame.Rect(SCREEN_WIDTH // 2 - 100, SCREEN_HEIGHT // 2 + 80, 200, 50)
        pygame.draw.rect(self.screen, COLORS['status_bg'], button_rect, border_radius=10)
        pygame.draw.rect(self.screen, COLORS['highlight'], button_rect, 2, border_radius=10)
        restart_text = self.font.render("Restart (R)", True, COLORS['text'])
        restart_rect = restart_text.get_rect(center=button_rect.center)
        self.screen.blit(restart_text, restart_rect)
        self.restart_button_rect = button_rect

        if os.path.exists(self.game.state_file):
            os.remove(self.game.state_file)

    def restart_game(self):
        self.game = ChainReactionGame()
        self.selected_cell = None
        self.ai_thinking = False
        self.game_over = False
        self.click_lock = False
        self.animations = []
        self.game.to_file("Human" if self.game_mode == 'human_vs_ai' else "AI")

    def run(self):
        running = True
        while running:
            self.pulse_time = pygame.time.get_ticks()
            mouse_pos = pygame.mouse.get_pos()
            for event in pygame.event.get():
                if event.type == QUIT:
                    running = False
                elif event.type == MOUSEBUTTONDOWN:
                    if self.state == 'title':
                        if self.mode_buttons[0].collidepoint(event.pos):
                            self.game_mode = 'human_vs_ai'
                            self.state = 'heuristic_selection'
                            self.selecting_for = 'ai2'
                        elif self.mode_buttons[1].collidepoint(event.pos):
                            self.game_mode = 'ai_vs_ai'
                            self.state = 'heuristic_selection'
                            self.selecting_for = 'ai1'
                        elif self.mode_buttons[2].collidepoint(event.pos):
                            self.game_mode = 'random_vs_ai'
                            self.state = 'heuristic_selection'
                            self.selecting_for = 'ai2'
                    elif self.state == 'heuristic_selection':
                        if self.back_button.collidepoint(event.pos):
                            self.state = 'title'
                            self.ai1_heuristic = None
                            self.ai2_heuristic = ChainReactionAI.evaluate_h1
                            self.selecting_for = None
                        else:
                            for i, button in enumerate(self.heuristic_buttons):
                                if button.collidepoint(event.pos):
                                    if self.selecting_for == 'ai1':
                                        self.ai1_heuristic = self.heuristic_options[i][1]
                                        self.selecting_for = 'ai2'
                                    else:
                                        self.ai2_heuristic = self.heuristic_options[i][1]
                                        self.selecting_for = None
                            if self.start_button.collidepoint(event.pos) and self.selecting_for is None:
                                self.initialize_game()
                                self.state = 'game'
                    elif self.state == 'game':
                        if self.back_button.collidepoint(event.pos):
                            self.state = 'heuristic_selection'
                            self.selecting_for = 'ai2' if self.game_mode in ['human_vs_ai', 'random_vs_ai'] else 'ai1'
                            self.game_over = False
                            self.game = None
                        elif self.game_mode == 'human_vs_ai':
                            self.handle_click(event.pos)
                        if self.game_over and self.restart_button_rect.collidepoint(event.pos):
                            self.restart_game()
                elif event.type == KEYDOWN:
                    if event.key == K_r and self.game_over and self.state == 'game':
                        self.restart_game()

            if self.state == 'title':
                self.draw_title_screen()
            elif self.state == 'heuristic_selection':
                self.draw_heuristic_selection()
            elif self.state == 'game':
                self.draw_gradient_background()
                self.game.from_file()
                self.draw_board()
                self.draw_status()
                self.draw_back_button(mouse_pos)
                if self.game_over:
                    self.show_game_over()
                elif self.game_mode in ['ai_vs_ai', 'random_vs_ai'] and not self.ai_thinking:
                    self.ai_turn()

            pygame.display.flip()
            self.clock.tick(60)
        pygame.quit()
        sys.exit()

if __name__ == "__main__":
    ui = ChainReactionUI()
    ui.run()