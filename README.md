# Gomoku Game

This project implements a complete **Gomoku (Five in a Row)** game in Python, featuring:
- A playable Gomoku game with an AI opponent.
- Save/load functionality for game state persistence.
- Undo functionality to revert the last move.
- Visual representation of the game flow using a flowchart.
- Well-documented code and a JSON example for game state serialization.

---

## Features

### 🎮 Gameplay
- **Two modes**: Play against another human or challenge the AI.
- **Win conditions**: Connect 5 consecutive stones in a row, column, or diagonal.
- **Draw detection**: Game ends if no valid moves remain.

### 🔁 Key Functionalities
- **Undo Moves**: Go back to the previous game state.
- **Save Game**: Save your current progress to a JSON file.
- **Load Game**: Resume gameplay from a previously saved state.

### 🤖 AI Opponent
The AI makes random moves during its turn.

### 📄 Game Flow
The following flowchart illustrates the game's logic:
```mermaid
graph TD
  Start[Start Game] --> Init[Initialize Game]
  Init --> Opponent[Choose Opponent Type]
  Opponent --> Loop[Main Game Loop]
  Loop --> Board[Display Board]
  Board --> Action{Choose Action}
  Action -- Move --> GetMove[Get Move Input]
  Action -- Undo --> UndoMove[Undo Last Move] --> Loop
  Action -- Save --> SaveGame[Save Game to File] --> Loop
  Action -- Load --> LoadGame[Load Game from File] --> Loop
  GetMove --> Validate[Validate Move]
  Validate -- Invalid --> Loop
  Validate -- Valid --> ApplyMove[Apply Move]
  ApplyMove --> CheckWin[Check Win]
  CheckWin -- Yes --> EndWin[Declare Winner]
  CheckWin -- No --> CheckDraw[Check Draw]
  CheckDraw -- Yes --> EndDraw[Declare Draw]
  CheckDraw -- No --> Switch[Switch Player] --> Loop
  EndWin --> End[Game Over]
  EndDraw --> End
