import numpy as np
import time
from ui import ChainReactionUI
from engine import ChainReactionGame
from minimax import ChainReactionAI

def experiment(heuristic1, heuristic2, depth1=4, depth2=2, games=10, mode='ai_vs_ai'):
    """
    Run experiments for Chain Reaction game matchups.
    
    Parameters:
    - heuristic1: Heuristic function for AI1 (Red) or None for Random.
    - heuristic2: Heuristic function for AI2 (Blue).
    - depth1: Search depth for AI1 (default: 4).
    - depth2: Search depth for AI2 (default: 2).
    - games: Number of games to run (default: 10).
    - mode: Game mode ('ai_vs_ai', 'random_vs_ai').
    
    Returns:
    - Dictionary with metrics: wins1, wins2, infinite_wins1, infinite_wins2, moves, times.
    """
    stats = {
        "wins1": 0,           # Red standard wins
        "wins2": 0,           # Blue standard wins
        "infinite_wins1": 0,  # Red infinite explosion wins
        "infinite_wins2": 0,  # Blue infinite explosion wins
        "moves": [],          # Moves per game
        "times": []           # Time per game (seconds)
    }
    
    for game_num in range(games):
        try:
            # Initialize UI and game
            ui = ChainReactionUI()
            ui.game_mode = mode
            ui.ai1_heuristic = heuristic1 if mode == 'ai_vs_ai' else None
            ui.ai2_heuristic = heuristic2
            ui.ai1_depth = depth1
            ui.ai2_depth = depth2
            ui.initialize_game()
            
            start_time = time.time()
            move_count = 0
            
            # Run game until completion
            while not ui.game_over:
                ui.ai_turn()
                move_count += 1
            
            # Record game duration
            stats["times"].append(time.time() - start_time)
            stats["moves"].append(move_count)
            
            # Determine winner
            blue_orbs = np.sum(ui.game.board[:, :, 1] == 2)
            red_orbs = np.sum(ui.game.board[:, :, 1] == 1)
            
            if blue_orbs > 0 and red_orbs == 0:
                stats["wins2"] += 1
            elif red_orbs > 0 and blue_orbs == 0:
                stats["wins1"] += 1
            elif ui.game.state_file and not ui.game.check_win():
                # Check for infinite explosion win
                with open(ui.game.state_file, 'r') as f:
                    last_move = f.readline().strip()
                    if last_move.startswith("AI") and ui.game.current_player == 0:
                        stats["infinite_wins2"] += 1
                    elif last_move.startswith("AI") and ui.game.current_player == 1:
                        stats["infinite_wins1"] += 1
                        
        except Exception as e:
            print(f"Error in game {game_num + 1}: {str(e)}")
            continue
    
    return stats

def run_experiments():
    """
    Run experiments for specified matchups and print results.
    """
    matchups = [
        ("H1 vs. Combined", ChainReactionAI.evaluate_h1, ChainReactionAI.evaluate_combined, 'ai_vs_ai'),
        ("Random vs. Combined", None, ChainReactionAI.evaluate_combined, 'random_vs_ai'),
        ("H4 vs. H4", ChainReactionAI.evaluate_h4, ChainReactionAI.evaluate_h4, 'ai_vs_ai')
    ]
    
    results = []
    
    for name, h1, h2, mode in matchups:
        print(f"Running matchup: {name}")
        stats = experiment(h1, h2, depth1=4, depth2=2, games=10, mode=mode)
        
        # Calculate percentages
        total_games = stats["wins1"] + stats["wins2"] + stats["infinite_wins1"] + stats["infinite_wins2"]
        if total_games == 0:
            total_games = 1  # Avoid division by zero
        standard_wins = (stats["wins2"] / total_games) * 100
        infinite_wins = (stats["infinite_wins2"] / total_games) * 100
        avg_moves = np.mean(stats["moves"]) if stats["moves"] else 0
        avg_time = np.mean(stats["times"]) if stats["times"] else 0
        
        results.append({
            "Matchup": name,
            "Standard Wins (%)": round(standard_wins, 1),
            "Infinite Wins (%)": round(infinite_wins, 1),
            "Avg. Moves": round(avg_moves, 1),
            "Avg. Time (s)": round(avg_time, 1)
        })
    
    # Print results table
    print("\nExperimental Results for Blue (Player 2):")
    print("-" * 60)
    print(f"{'Matchup':<20} {'Standard Wins (%)':<18} {'Infinite Wins (%)':<18} {'Avg. Moves':<12} {'Avg. Time (s)':<12}")
    print("-" * 60)
    for r in results:
        print(f"{r['Matchup']:<20} {r['Standard Wins (%)']:<18} {r['Infinite Wins (%)']:<18} {r['Avg. Moves']:<12} {r['Avg. Time (s)']:<12}")
    
    return results

if __name__ == "__main__":
    results = run_experiments()