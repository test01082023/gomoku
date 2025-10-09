#!/usr/bin/env python3
"""
Gomoku AI vs AI - Five in a Row
Complete implementation with multiple AI difficulty levels
"""

import random
import time
import os
from typing import List, Tuple, Optional
from copy import deepcopy

# Evaluation patterns for AI
# [pattern, score, name]
PATTERNS = [
    # Five in a row (win)
    ([1, 1, 1, 1, 1], 100000, "Five"),
    
    # Open four (guaranteed win next move)
    ([0, 1, 1, 1, 1, 0], 50000, "Open Four"),
    
    # Four (one end blocked)
    ([1, 1, 1, 1, 0], 10000, "Four"),
    ([0, 1, 1, 1, 1], 10000, "Four"),
    
    # Open three
    ([0, 1, 1, 1, 0], 5000, "Open Three"),
    ([0, 1, 0, 1, 1, 0], 5000, "Open Three"),
    ([0, 1, 1, 0, 1, 0], 5000, "Open Three"),
    
    # Three (one end blocked)
    ([1, 1, 1, 0, 0], 1000, "Three"),
    ([0, 0, 1, 1, 1], 1000, "Three"),
    
    # Open two
    ([0, 1, 1, 0], 500, "Open Two"),
    ([0, 1, 0, 1, 0], 500, "Open Two"),
    
    # Two (one end blocked)
    ([1, 1, 0, 0], 100, "Two"),
    ([0, 0, 1, 1], 100, "Two"),
    
    # One
    ([0, 1, 0], 10, "One"),
]


class Gomoku:
    def __init__(self, board_size: int = 15):
        self.board_size = board_size
        self.board = [[0 for _ in range(board_size)] for _ in range(board_size)]
        self.current_player = 1  # 1 = Black (goes first), 2 = White
        self.move_history = []
        self.winner = None
        self.difficulty_ai1 = 2  # Black AI (Player 1)
        self.difficulty_ai2 = 2  # White AI (Player 2)
        
        # Statistics
        self.black_wins = 0
        self.white_wins = 0
        self.draws = 0
        self.total_games = 0
        
        # Game settings
        self.show_last_move = True
        self.animation_speed = 0.3
    
    def reset_game(self):
        """Reset the game to initial state."""
        self.board = [[0 for _ in range(self.board_size)] for _ in range(self.board_size)]
        self.current_player = 1
        self.move_history = []
        self.winner = None
    
    def display_board(self):
        """Display the Gomoku board."""
        os.system('cls' if os.name == 'nt' else 'clear')
        
        current_name = "Black ●" if self.current_player == 1 else "White ○"
        
        print("\n" + "=" * 60)
        print(f"   GOMOKU (Five in a Row) - {current_name}'s Turn")
        print("=" * 60)
        
        # Symbols
        symbols = {
            0: '·',   # Empty
            1: '●',   # Black
            2: '○'    # White
        }
        
        # Column headers
        print("\n    ", end="")
        for i in range(self.board_size):
            if i < 10:
                print(f" {i} ", end="")
            else:
                print(f"{i} ", end="")
        print()
        
        # Top border
        print("   ┌" + "─" * (self.board_size * 3) + "┐")
        
        # Board rows
        for i in range(self.board_size):
            if i < 10:
                print(f" {i} │", end="")
            else:
                print(f"{i} │", end="")
            
            for j in range(self.board_size):
                stone = self.board[i][j]
                symbol = symbols[stone]
                
                # Highlight last move
                if self.show_last_move and self.move_history and (i, j) == self.move_history[-1]:
                    print(f" \033[92m{symbol}\033[0m ", end="")
                else:
                    print(f" {symbol} ", end="")
            
            if i < 10:
                print(f"│ {i}")
            else:
                print(f"│{i}")
        
        # Bottom border
        print("   └" + "─" * (self.board_size * 3) + "┘")
        
        # Column headers (bottom)
        print("    ", end="")
        for i in range(self.board_size):
            if i < 10:
                print(f" {i} ", end="")
            else:
                print(f"{i} ", end="")
        print("\n")
        
        # Show game info
        print(f"Move: {len(self.move_history)}")
        if self.move_history:
            last_row, last_col = self.move_history[-1]
            last_player = "Black ●" if len(self.move_history) % 2 == 1 else "White ○"
            print(f"Last move: {last_player} at ({last_row}, {last_col})")
        print()
    
    def is_valid_move(self, row: int, col: int) -> bool:
        """Check if a move is valid."""
        if row < 0 or row >= self.board_size or col < 0 or col >= self.board_size:
            return False
        return self.board[row][col] == 0
    
    def make_move(self, row: int, col: int) -> bool:
        """Make a move on the board."""
        if not self.is_valid_move(row, col):
            return False
        
        self.board[row][col] = self.current_player
        self.move_history.append((row, col))
        return True
    
    def check_winner(self, row: int, col: int) -> bool:
        """Check if the last move resulted in a win."""
        player = self.board[row][col]
        if player == 0:
            return False
        
        # Directions: horizontal, vertical, diagonal /, diagonal \
        directions = [
            [(0, 1), (0, -1)],    # Horizontal
            [(1, 0), (-1, 0)],    # Vertical
            [(1, 1), (-1, -1)],   # Diagonal \
            [(1, -1), (-1, 1)]    # Diagonal /
        ]
        
        for direction_pair in directions:
            count = 1  # Count the stone we just placed
            
            # Check both directions
            for dr, dc in direction_pair:
                r, c = row + dr, col + dc
                while (0 <= r < self.board_size and 
                       0 <= c < self.board_size and 
                       self.board[r][c] == player):
                    count += 1
                    r += dr
                    c += dc
            
            if count >= 5:
                return True
        
        return False
    
    def is_board_full(self) -> bool:
        """Check if the board is full."""
        for row in self.board:
            if 0 in row:
                return False
        return True
    
    def get_line(self, row: int, col: int, dr: int, dc: int, length: int) -> List[int]:
        """Get a line of stones in a direction."""
        line = []
        for i in range(length):
            r = row + dr * i
            c = col + dc * i
            if 0 <= r < self.board_size and 0 <= c < self.board_size:
                line.append(self.board[r][c])
            else:
                line.append(-1)  # Out of bounds
        return line
    
    def evaluate_position(self, player: int) -> int:
        """Evaluate the board position for a player."""
        score = 0
        opponent = 3 - player  # If player is 1, opponent is 2, and vice versa
        
        # All directions to check
        directions = [(0, 1), (1, 0), (1, 1), (1, -1)]
        
        # Check all positions and directions
        for row in range(self.board_size):
            for col in range(self.board_size):
                for dr, dc in directions:
                    # Get line of stones
                    line = self.get_line(row, col, dr, dc, 6)
                    
                    # Convert to player perspective (1 for player, -1 for opponent, 0 for empty)
                    pattern = []
                    for stone in line:
                        if stone == player:
                            pattern.append(1)
                        elif stone == opponent:
                            pattern.append(-1)
                        elif stone == 0:
                            pattern.append(0)
                        else:
                            pattern.append(-2)  # Out of bounds
                    
                    # Check against known patterns
                    for pat, pat_score, name in PATTERNS:
                        if self.pattern_matches(pattern, pat):
                            score += pat_score
                        
                        # Check for opponent patterns (defensive)
                        opponent_pat = [-p if p in [1, -1] else p for p in pat]
                        if self.pattern_matches(pattern, opponent_pat):
                            score -= pat_score * 0.9  # Slightly less weight on defense
        
        return score
    
    def pattern_matches(self, line: List[int], pattern: List[int]) -> bool:
        """Check if a pattern matches a line."""
        if len(line) < len(pattern):
            return False
        
        for i in range(len(line) - len(pattern) + 1):
            match = True
            for j in range(len(pattern)):
                if pattern[j] != line[i + j]:
                    match = False
                    break
            if match:
                return True
        return False
    
    def get_valid_moves(self) -> List[Tuple[int, int]]:
        """Get all valid moves."""
        moves = []
        
        # If board is empty, return center
        if not self.move_history:
            center = self.board_size // 2
            return [(center, center)]
        
        # Get moves near existing stones (within 2 squares)
        checked = set()
        for move_row, move_col in self.move_history:
            for dr in range(-2, 3):
                for dc in range(-2, 3):
                    r, c = move_row + dr, move_col + dc
                    if (r, c) not in checked and self.is_valid_move(r, c):
                        moves.append((r, c))
                        checked.add((r, c))
        
        return moves if moves else [(i, j) for i in range(self.board_size) 
                                     for j in range(self.board_size) 
                                     if self.board[i][j] == 0]
    
    def get_ai_move(self, difficulty: int) -> Optional[Tuple[int, int]]:
        """Get AI move based on difficulty level."""
        valid_moves = self.get_valid_moves()
        
        if not valid_moves:
            return None
        
        if difficulty == 1:  # Easy - Random move
            return random.choice(valid_moves)
        
        # Evaluate all moves
        move_scores = []
        
        for row, col in valid_moves:
            # Make move temporarily
            self.board[row][col] = self.current_player
            
            # Check if this move wins
            if self.check_winner(row, col):
                self.board[row][col] = 0
                return (row, col)  # Take winning move immediately
            
            # Check if opponent can win next move (must block)
            opponent = 3 - self.current_player
            must_block = False
            
            for opp_row, opp_col in valid_moves:
                if (opp_row, opp_col) == (row, col):
                    continue
                self.board[opp_row][opp_col] = opponent
                if self.check_winner(opp_row, opp_col):
                    must_block = True
                    self.board[opp_row][opp_col] = 0
                    break
                self.board[opp_row][opp_col] = 0
            
            if must_block and (row, col) != (opp_row, opp_col):
                self.board[row][col] = 0
                continue
            
            # Evaluate position
            score = self.evaluate_position(self.current_player)
            
            # Add some randomness based on difficulty
            if difficulty == 2:  # Medium
                score += random.randint(-1000, 1000)
            elif difficulty == 3:  # Hard
                score += random.randint(-100, 100)
            
            move_scores.append((row, col, score))
            
            # Restore board
            self.board[row][col] = 0
        
        if not move_scores:
            return random.choice(valid_moves) if valid_moves else None
        
        # Sort by score
        move_scores.sort(key=lambda x: x[2], reverse=True)
        
        if difficulty == 2:  # Medium - sometimes pick from top 5
            if random.random() < 0.7:
                return (move_scores[0][0], move_scores[0][1])
            else:
                top_moves = move_scores[:min(5, len(move_scores))]
                selected = random.choice(top_moves)
                return (selected[0], selected[1])
        else:  # Hard - mostly pick best, sometimes from top 3
            if random.random() < 0.95:
                return (move_scores[0][0], move_scores[0][1])
            else:
                top_moves = move_scores[:min(3, len(move_scores))]
                selected = random.choice(top_moves)
                return (selected[0], selected[1])
    
    def play_ai_turn(self, ai_name: str, difficulty: int) -> bool:
        """Play AI turn."""
        print(f"{ai_name} is thinking", end="", flush=True)
        
        # Thinking animation
        delay = 0.2 if difficulty == 1 else (0.4 if difficulty == 2 else 0.6)
        
        for _ in range(3):
            print(".", end="", flush=True)
            time.sleep(delay / 3)
        print()
        
        # Get AI move
        move = self.get_ai_move(difficulty)
        
        if move:
            row, col = move
            if self.make_move(row, col):
                symbol = "●" if self.current_player == 1 else "○"
                print(f"{ai_name} plays {symbol} at ({row}, {col})")
                time.sleep(self.animation_speed)
                return True
        
        return False
    
    def get_difficulty_name(self, diff: int) -> str:
        """Get difficulty name."""
        names = {1: "Easy", 2: "Medium", 3: "Hard"}
        return names.get(diff, "Medium")
    
    def play_ai_vs_ai(self):
        """Play AI vs AI game."""
        self.reset_game()
        
        print("\n=== AI vs AI Gomoku Match ===")
        print(f"Black AI (●): {self.get_difficulty_name(self.difficulty_ai1)}")
        print(f"White AI (○): {self.get_difficulty_name(self.difficulty_ai2)}")
        print("Starting in 2 seconds...\n")
        time.sleep(2)
        
        max_moves = self.board_size * self.board_size
        
        while len(self.move_history) < max_moves:
            self.display_board()
            
            # Get AI name and difficulty
            if self.current_player == 1:
                ai_name = f"Black AI (●) [{self.get_difficulty_name(self.difficulty_ai1)}]"
                ai_diff = self.difficulty_ai1
            else:
                ai_name = f"White AI (○) [{self.get_difficulty_name(self.difficulty_ai2)}]"
                ai_diff = self.difficulty_ai2
            
            # Play AI turn
            if not self.play_ai_turn(ai_name, ai_diff):
                self.winner = 'draw'
                break
            
            # Check for winner
            last_move = self.move_history[-1]
            if self.check_winner(last_move[0], last_move[1]):
                self.winner = 'black' if self.current_player == 1 else 'white'
                break
            
            # Check for draw
            if self.is_board_full():
                self.winner = 'draw'
                break
            
            # Switch player
            self.current_player = 3 - self.current_player
        
        if len(self.move_history) >= max_moves:
            self.winner = 'draw'
        
        # Display final position
        self.display_board()
        
        # Show result
        print("\n" + "=" * 60)
        if self.winner == 'black':
            print("   🏆 FIVE IN A ROW! Black AI (●) Wins! 🏆")
            self.black_wins += 1
        elif self.winner == 'white':
            print("   🏆 FIVE IN A ROW! White AI (○) Wins! 🏆")
            self.white_wins += 1
        else:
            print("   DRAW - Board is Full!")
            self.draws += 1
        print("=" * 60 + "\n")
        
        self.total_games += 1
        self.show_statistics()
    
    def show_statistics(self):
        """Display game statistics."""
        print("\n=== Game Statistics ===")
        print(f"Black AI (●) Wins: {self.black_wins}")
        print(f"White AI (○) Wins: {self.white_wins}")
        print(f"Draws: {self.draws}")
        print(f"Total Games: {self.total_games}")
        
        if self.total_games > 0:
            print(f"\nBlack Win Rate: {self.black_wins / self.total_games * 100:.1f}%")
            print(f"White Win Rate: {self.white_wins / self.total_games * 100:.1f}%")
            print(f"Draw Rate: {self.draws / self.total_games * 100:.1f}%")
        print("=" * 23 + "\n")
    
    def show_menu(self):
        """Display main menu."""
        print("\n" + "=" * 60)
        print("              GOMOKU AI vs AI (Five in a Row)")
        print("=" * 60)
        print("1. Watch AI vs AI")
        print(f"2. Set Black AI (●) Difficulty (Current: {self.get_difficulty_name(self.difficulty_ai1)})")
        print(f"3. Set White AI (○) Difficulty (Current: {self.get_difficulty_name(self.difficulty_ai2)})")
        print(f"4. Set Board Size (Current: {self.board_size}x{self.board_size})")
        print(f"5. Toggle Last Move Highlight (Current: {'ON' if self.show_last_move else 'OFF'})")
        print("6. Show Statistics")
        print("7. Reset Statistics")
        print("8. Play Multiple Games")
        print("9. Exit")
        print("=" * 60)
        print("Enter your choice: ", end="")
    
    def play_multiple_games(self):
        """Play multiple games in a row."""
        try:
            num_games = int(input("How many games to play? "))
            if num_games <= 0:
                print("Invalid number.")
                time.sleep(1)
                return
            
            auto_continue = input("Auto-continue without pausing? (y/n): ").lower() == 'y'
            
            for i in range(num_games):
                print(f"\n=== Game {i+1}/{num_games} ===")
                time.sleep(1)
                
                # Temporarily disable animation for speed
                old_speed = self.animation_speed
                if auto_continue:
                    self.animation_speed = 0.1
                
                self.play_ai_vs_ai()
                
                self.animation_speed = old_speed
                
                if not auto_continue and i < num_games - 1:
                    input("\nPress Enter for next game...")
            
            print(f"\n=== Completed {num_games} games! ===")
            self.show_statistics()
            input("\nPress Enter to continue...")
            
        except ValueError:
            print("Invalid input.")
            time.sleep(1)
    
    def run(self):
        """Main game loop."""
        running = True
        
        while running:
            self.show_menu()
            try:
                choice = int(input())
            except ValueError:
                print("Invalid choice. Please try again.")
                time.sleep(1)
                continue
            
            if choice == 1:
                self.play_ai_vs_ai()
                input("\nPress Enter to continue...")
            
            elif choice == 2:
                try:
                    diff = int(input("Enter difficulty (1=Easy, 2=Medium, 3=Hard): "))
                    if 1 <= diff <= 3:
                        self.difficulty_ai1 = diff
                        print(f"Black AI difficulty set to {self.get_difficulty_name(diff)}")
                    else:
                        print("Invalid difficulty. Use 1, 2, or 3.")
                    time.sleep(1)
                except ValueError:
                    print("Invalid input.")
                    time.sleep(1)
            
            elif choice == 3:
                try:
                    diff = int(input("Enter difficulty (1=Easy, 2=Medium, 3=Hard): "))
                    if 1 <= diff <= 3:
                        self.difficulty_ai2 = diff
                        print(f"White AI difficulty set to {self.get_difficulty_name(diff)}")
                    else:
                        print("Invalid difficulty. Use 1, 2, or 3.")
                    time.sleep(1)
                except ValueError:
                    print("Invalid input.")
                    time.sleep(1)
            
            elif choice == 4:
                try:
                    size = int(input("Enter board size (9-19, recommended 15): "))
                    if 9 <= size <= 19:
                        self.board_size = size
                        self.reset_game()
                        print(f"Board size set to {size}x{size}")
                    else:
                        print("Invalid size. Use 9-19.")
                    time.sleep(1)
                except ValueError:
                    print("Invalid input.")
                    time.sleep(1)
            
            elif choice == 5:
                self.show_last_move = not self.show_last_move
                print(f"Last move highlight: {'ON' if self.show_last_move else 'OFF'}")
                time.sleep(1)
            
            elif choice == 6:
                self.show_statistics()
                input("\nPress Enter to continue...")
            
            elif choice == 7:
                confirm = input("Reset all statistics? (y/n): ")
                if confirm.lower() == 'y':
                    self.black_wins = 0
                    self.white_wins = 0
                    self.draws = 0
                    self.total_games = 0
                    print("Statistics reset!")
                time.sleep(1)
            
            elif choice == 8:
                self.play_multiple_games()
            
            elif choice == 9:
                running = False
                print("\nThanks for watching! Goodbye! 👋")
            
            else:
                print("Invalid choice. Please try again.")
                time.sleep(1)


if __name__ == "__main__":
    print("\n" + "=" * 60)
    print("   Welcome to GOMOKU AI vs AI")
    print("   (Five in a Row)")
    print("=" * 60)
    print("\nRules:")
    print("- Players alternate placing stones on the board")
    print("- First to get 5 stones in a row (any direction) wins")
    print("- Black (●) goes first")
    print("\nLoading...")
    time.sleep(1)
    
    game = Gomoku(board_size=15)
    game.run()
