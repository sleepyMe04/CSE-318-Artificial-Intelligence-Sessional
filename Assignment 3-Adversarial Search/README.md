# Assignment 3 – Adversarial Search: Chain Reaction

A fully playable **Chain Reaction** game with a Pygame GUI, an AI opponent powered by **Minimax with Alpha-Beta Pruning**, five custom heuristic evaluation functions, and an automated experiment runner for benchmarking.

---

## Table of Contents

- [Game Overview](#game-overview)
- [Project Structure](#project-structure)
- [Installation & Running](#installation--running)
- [Game Modes](#game-modes)
- [UI & Controls](#ui--controls)
- [Algorithm: Minimax with Alpha-Beta Pruning](#algorithm-minimax-with-alpha-beta-pruning)
- [Heuristic Functions](#heuristic-functions)
- [Experiments](#experiments)
- [Implementation Details](#implementation-details)
  - [engine.py](#enginepy)
  - [minimax.py](#minimaxpy)
  - [ui.py](#uipy)
  - [experiments.py](#experimentspy)

---

## Game Overview

**Chain Reaction** is a two-player strategy game played on a **9×6 grid**.

### Core Rules

Players take turns — **Red goes first**. On your turn, place one orb on:
- Any **empty cell**, or
- Any cell you **already own**

Each cell has a **critical mass** — the number of its orthogonal neighbors:

| Cell Type | Critical Mass |
|-----------|--------------|
| Corner (4 cells) | 2 |
| Edge (non-corner) | 3 |
| Interior | 4 |

When a cell's orb count reaches its critical mass, it **explodes**: all its orbs scatter one each to every adjacent cell. Those neighbors gain an orb and are **captured** by the exploding player, regardless of who owned them before. If any neighbor also reaches its critical mass, it explodes too — creating **chain reactions**.

### Winning

A player wins when **all orbs on the board belong to them** (the opponent has zero orbs). The win condition is only active after both players have placed at least one orb, preventing an instant win on the very first move.

### Strategy Notes

- **Corners** are powerful — they only need 2 orbs to explode and can convert adjacent cells.
- **Chain reactions** can cascade across the entire board in a single move, turning a losing position into a win.
- **Overloading enemy cells** — placing orbs adjacent to a nearly-full enemy cell — is a key offensive tactic.
- **Stability** matters: a cell at `count = critical_mass - 1` is one move from exploding and is a liability if surrounded by enemies.

---

## Project Structure

```
2105114/
├── engine.py        — Board representation, move validation, explosion logic, win detection
├── minimax.py       — Minimax + alpha-beta pruning, all 5 heuristics, move ordering
├── ui.py            — Full Pygame GUI: title screen, heuristic selector, game view, animations
└── experiments.py   — Headless AI-vs-AI experiment runner with stats reporting
```

---

## Installation & Running

### Requirements

```bash
pip install numpy pygame
```

Python 3.8+ recommended.

### Launch the GUI

```bash
cd 2105114
python ui.py
```

### Run Headless Experiments

```bash
python experiments.py
```

---

## Game Modes

Three modes are available from the title screen:

| Mode | Red (Player 1) | Blue (Player 2) |
|------|---------------|----------------|
| **Human vs. AI** | You (mouse clicks) | Minimax AI |
| **AI vs. AI** | Minimax AI | Minimax AI |
| **Random Agent vs. AI** | Random move picker | Minimax AI |

In **AI vs. AI** and **Random vs. AI** modes, the game plays itself automatically — you watch the match unfold in real time.

### Heuristic Selection

Before any AI game starts, a **heuristic selection screen** appears. You pick from:
- Combined, H1, H2, H3, H4, H5

In **AI vs. AI**, you select a heuristic for each player independently, then press **Start Game**.

---

## UI & Controls

The game runs at **900×700** with a dark blue-to-teal gradient background.

### During Human vs. AI

| Action | Control |
|--------|---------|
| Place orb | Left-click a valid cell |
| Restart after game over | Press `R` or click **Restart** button |
| Return to menu | Click **Back** button |

### Visual Feedback

- **Gold pulsing highlight** — shows the currently selected/last-played cell, with a sine-wave pulse animation.
- **3D orbs** — each orb is drawn with a radial gradient shading and a specular highlight dot to give a spherical look.
- **Orb layouts** — orbs within a cell are arranged spatially (side-by-side for 2, triangle for 3, square for 4, grid for 5+).
- **"AI Thinking…"** — status bar indicator shown while the AI is computing its move.
- **Game Over overlay** — semi-transparent black overlay with winner text and a restart button.

### Status Bar

The top status bar always shows whose turn it is, with the player name colored in their team color (red/blue).

---

## Algorithm: Minimax with Alpha-Beta Pruning

### Minimax

The AI searches the game tree to a configurable depth, alternating between a **maximizing player** (Blue) and a **minimizing player** (Red):

```
minimax(game, depth, α, β, maximizing):
    if depth == 0 or game.check_win():
        return evaluate(game), None

    best_move = None
    if maximizing:
        max_eval = -∞
        for each valid move (r, c):
            child = simulate_move(game, r, c)
            eval, _ = minimax(child, depth-1, α, β, False)
            if eval > max_eval:
                max_eval = eval
                best_move = (r, c)
            α = max(α, eval)
            if β ≤ α: break          ← alpha cutoff
        return max_eval, best_move
    else:
        min_eval = +∞
        for each valid move (r, c):
            child = simulate_move(game, r, c)
            eval, _ = minimax(child, depth-1, α, β, True)
            if eval < min_eval:
                min_eval = eval
                best_move = (r, c)
            β = min(β, eval)
            if β ≤ α: break          ← beta cutoff
        return min_eval, best_move
```

### Alpha-Beta Pruning

Alpha (`α`) tracks the best score the maximizing player is guaranteed so far. Beta (`β`) tracks the best the minimizing player is guaranteed. Whenever `β ≤ α`, the remaining siblings are irrelevant and pruned. This dramatically reduces the number of nodes evaluated, allowing deeper search in the same time.

### Move Ordering

Before the recursive calls, moves are **sorted by a priority score** to increase the chance of early cutoffs:

| Move Property | Score Bonus |
|--------------|-------------|
| Own cell close to critical mass | `+3 × (critical_mass - current_count)` |
| Corner cell | `+3` |
| Edge cell | `+1` |

Maximizing player sees highest-scoring moves first; minimizing player sees lowest-scoring moves first.

### Search Depths (Defaults)

| Agent | Depth |
|-------|-------|
| AI1 (Red, in AI vs AI) | 4 |
| AI2 (Blue) | 2 |

Deeper search = stronger play but longer think time. Depth 4 on a 9×6 board is already significant given the high branching factor.

### Simulation

Each move is simulated on a **deep copy** of the game state (`copy.deepcopy`) so the original board is never modified during search.

---

## Heuristic Functions

All heuristics output a **signed score from Blue's perspective**: positive favors Blue, negative favors Red.

---

### H1 — Orb Count Difference

```
H1 = (total Blue orbs) - (total Red orbs)
```

The simplest baseline. Counts raw orb ownership across all cells. Does not account for board position or explosion potential.

---

### H2 — Critical Mass Proximity

```
H2 = Σ (Blue cell: orbs/critical_mass) - Σ (Red cell: orbs/critical_mass)
```

Rewards cells that are close to exploding. A cell at `3/4` capacity scores 0.75 for its owner. Captures the intuition that near-critical cells are more valuable because they can soon chain-react.

---

### H3 — Board Control

```
H3 = (cells owned by Blue) - (cells owned by Red)
```

Counts territory — the number of distinct cells controlled. Rewards spreading across the board rather than stacking orbs in fewer cells.

---

### H4 — Explosion Potential

```
for each Blue cell near critical mass:
    score += (adjacent enemy cells) × (orbs / critical_mass)

for each Red cell near critical mass:
    score -= (adjacent enemy cells) × (orbs / critical_mass)
```

Rewards cells that are nearly full **and** surrounded by enemy cells — the most threatening attacking positions. A near-critical Blue cell adjacent to 2 Red cells scores twice as much as one adjacent to 1.

---

### H5 — Stability and Vulnerability

```
for each Blue cell:
    score += 1 - (orbs / critical_mass)          ← stability reward

for each Red cell near critical mass:
    score += (orbs / critical_mass) × (adjacent Blue cells)   ← vulnerability exploit
```

Rewards Blue cells that are far from their critical mass (stable and hard to trigger). Also rewards situations where Red cells are near exploding and surrounded by Blue cells — i.e., Red is about to chain into Blue's territory, which is favorable.

---

### Combined — Weighted Sum

```
Combined = 1·H1 + 1·H2 + 1·H3 + 2·H4 + 2·H5
```

All five heuristics blended together. **H4 and H5 are doubled** because explosion potential and stability are strategically more decisive in the mid-to-late game than raw orb count or board coverage.

---

## Experiments

`experiments.py` runs automated AI-vs-AI matchups and reports statistics. No GUI — fully headless.

### Default Matchups

| Matchup | Red Agent | Red Depth | Blue Agent | Blue Depth |
|---------|-----------|-----------|------------|------------|
| H1 vs. Combined | H1 | 4 | Combined | 2 |
| Random vs. Combined | Random | — | Combined | 2 |
| H4 vs. H4 | H4 | 4 | H4 | 2 |

Each matchup runs **10 games** by default.

### Output

```
Running matchup: H1 vs. Combined
Running matchup: Random vs. Combined
Running matchup: H4 vs. H4

Experimental Results for Blue (Player 2):
------------------------------------------------------------
Matchup              Standard Wins (%)  Infinite Wins (%)  Avg. Moves   Avg. Time (s)
------------------------------------------------------------
H1 vs. Combined      ...                ...                ...          ...
Random vs. Combined  ...                ...                ...          ...
H4 vs. H4            ...                ...                ...          ...
```

- **Standard Wins** — Blue wins by eliminating all Red orbs normally.
- **Infinite Wins** — Blue wins because Red triggered an infinite explosion loop (exceeding the 1000-iteration guard), which is counted as a loss for the triggering player.
- **Avg. Moves** — Average total moves per game (both players combined).
- **Avg. Time** — Average wall-clock time per game in seconds.

---

## Implementation Details

### `engine.py`

The `ChainReactionGame` class holds the full game state.

**Board representation:**
```python
self.board = np.zeros((rows, cols, 2), dtype=int)
# board[i][j][0] = orb count at (i, j)
# board[i][j][1] = player id (0 = empty, 1 = Red, 2 = Blue)
```

**Critical mass initialization:**
```python
mass = np.full((9, 6), 4)   # interior
mass[0, :] = mass[-1, :] = 3   # top/bottom edges
mass[:, 0] = mass[:, -1] = 3   # left/right edges
mass[0,0] = mass[0,-1] = mass[-1,0] = mass[-1,-1] = 2  # corners
```

**Explosion processing** uses an **iterative BFS** (`collections.deque`) rather than recursion, which avoids Python's recursion limit on long chain reactions:
1. Enqueue the initially placed cell.
2. Pop a cell — if it's below critical mass, skip.
3. Clear its orbs, scatter one orb to each neighbor, set each neighbor's owner to the exploding player.
4. If any neighbor now meets or exceeds its critical mass, enqueue it.
5. A hard cap of **1000 iterations** detects pathological infinite loops and terminates with the triggering player marked as the loser.

**State persistence** (`to_file` / `from_file`): The board state is written to `game_state.txt` after every move in a human-readable format (`2R`, `3B`, `0` for empty), allowing the game loop to re-read state cleanly each frame.

---

### `minimax.py`

The `ChainReactionAI` class is a stateless collection of `@staticmethod` methods.

Key methods:

| Method | Purpose |
|--------|---------|
| `minimax(game, depth, α, β, maximizing, evaluate_func)` | Core recursive search |
| `get_valid_moves(game, maximizing_player)` | Returns all legal moves for the current player |
| `order_moves(game, moves, maximizing_player)` | Sorts moves to improve pruning efficiency |
| `simulate_move(game, r, c)` | Deep-copies game, applies move, returns new state |
| `h1` – `h5` | Individual heuristic computations |
| `evaluate_combined` | Weighted blend of all five heuristics |

---

### `ui.py`

The `ChainReactionUI` class manages the entire Pygame application through a **3-state machine**:

```
'title'  →  'heuristic_selection'  →  'game'
                     ↑                    |
                     └────────────────────┘  (Back button)
```

| State | What renders |
|-------|-------------|
| `title` | Title card + 3 mode buttons |
| `heuristic_selection` | Heuristic picker for AI1 (if needed) then AI2, then Start button |
| `game` | Gradient background + board + status bar + Back button |

**Animation system:** An `animations` list stores `(row, col, scale, start_time)` tuples. During `draw_board`, if the current cell has an active animation entry, the orb radius is scaled up proportionally to `1.0 + 0.5 × (1 - elapsed/300ms)`, creating a pop-in effect when orbs are placed.

**3D orb rendering:** Each orb is drawn as a stack of concentric circles with decreasing radius, each shaded progressively darker toward the edges (simulating a diffuse sphere). A small white circle is offset toward the top-left to simulate a specular highlight.

**`MinimaxAgent` and `RandomAgent`** are thin wrappers inside `ui.py` that hold the player index and delegate to `ChainReactionAI.minimax` or `random.choice` respectively.

---

### `experiments.py`

The `experiment()` function accepts heuristic functions, depths, game count, and mode as parameters, runs the specified number of games by driving `ChainReactionUI.ai_turn()` in a headless loop, and returns a stats dictionary. `run_experiments()` defines the matchup table and formats the final printed report.