#include <iostream>
#include <vector>
#include <iomanip>
#include <limits>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <chrono>
#include <thread>

class Gomoku {
private:
    static const int BOARD_SIZE = 15;
    static const int WIN_COUNT = 5;
    std::vector<std::vector<int>> board;
    int currentPlayer;
    bool gameOver;
    int winner;
    int moveCount;
    bool vsAI;
    bool aiVsAI;
    int aiDifficulty;
    int ai1Difficulty;
    int ai2Difficulty;
    std::pair<int, int> lastMove;

    struct Move {
        int row, col, score;
        Move(int r = -1, int c = -1, int s = 0) : row(r), col(c), score(s) {}
    };

public:
    Gomoku() : board(BOARD_SIZE, std::vector<int>(BOARD_SIZE, 0)), 
               currentPlayer(1), gameOver(false), winner(0), moveCount(0),
               vsAI(false), aiVsAI(false), aiDifficulty(2), 
               ai1Difficulty(2), ai2Difficulty(2), lastMove(-1, -1) {
        std::srand(std::time(nullptr));
    }

    void displayBoard() {
        std::cout << "\n   ";
        for (int i = 0; i < BOARD_SIZE; i++) {
            std::cout << std::setw(2) << i << " ";
        }
        std::cout << "\n   ";
        for (int i = 0; i < BOARD_SIZE; i++) {
            std::cout << "---";
        }
        std::cout << "\n";

        for (int i = 0; i < BOARD_SIZE; i++) {
            std::cout << std::setw(2) << i << "|";
            for (int j = 0; j < BOARD_SIZE; j++) {
                char symbol = '.';
                if (board[i][j] == 1) symbol = 'X';
                else if (board[i][j] == 2) symbol = 'O';
                
                // Highlight last move
                if (i == lastMove.first && j == lastMove.second && moveCount > 0) {
                    std::cout << "[" << symbol << "]";
                } else {
                    std::cout << " " << symbol << " ";
                }
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }

    bool isValidMove(int row, int col) {
        return row >= 0 && row < BOARD_SIZE && 
               col >= 0 && col < BOARD_SIZE && 
               board[row][col] == 0;
    }

    bool makeMove(int row, int col) {
        if (!isValidMove(row, col) || gameOver) {
            return false;
        }

        board[row][col] = currentPlayer;
        lastMove = {row, col};
        moveCount++;

        if (checkWin(row, col)) {
            gameOver = true;
            winner = currentPlayer;
        } else if (moveCount == BOARD_SIZE * BOARD_SIZE) {
            gameOver = true;
            winner = 0; // Draw
        } else {
            currentPlayer = (currentPlayer == 1) ? 2 : 1;
        }

        return true;
    }

    int countConsecutive(int row, int col, int dRow, int dCol, int player) {
        int count = 0;
        int r = row + dRow;
        int c = col + dCol;
        
        while (r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE && 
               board[r][c] == player) {
            count++;
            r += dRow;
            c += dCol;
        }
        
        return count;
    }

    int evaluatePosition(int row, int col, int player) {
        if (!isValidMove(row, col)) return -1;
        
        int score = 0;
        board[row][col] = player; // Temporarily place piece
        
        // Check all four directions
        int directions[4][2] = {{0,1}, {1,0}, {1,1}, {1,-1}};
        
        for (auto& dir : directions) {
            int count = 1;
            count += countConsecutive(row, col, dir[0], dir[1], player);
            count += countConsecutive(row, col, -dir[0], -dir[1], player);
            
            if (count >= WIN_COUNT) {
                score += 100000; // Winning move
            } else if (count == 4) {
                // Check if open on both ends
                int r1 = row + (count - countConsecutive(row, col, -dir[0], -dir[1], player)) * dir[0];
                int c1 = col + (count - countConsecutive(row, col, -dir[0], -dir[1], player)) * dir[1];
                int r2 = row - (countConsecutive(row, col, -dir[0], -dir[1], player) + 1) * dir[0];
                int c2 = col - (countConsecutive(row, col, -dir[0], -dir[1], player) + 1) * dir[1];
                
                bool open1 = (r1 >= 0 && r1 < BOARD_SIZE && c1 >= 0 && c1 < BOARD_SIZE && board[r1][c1] == 0);
                bool open2 = (r2 >= 0 && r2 < BOARD_SIZE && c2 >= 0 && c2 < BOARD_SIZE && board[r2][c2] == 0);
                
                if (open1 && open2) {
                    score += 10000; // Open four
                } else {
                    score += 5000; // Semi-open four
                }
            } else if (count == 3) {
                score += 1000;
            } else if (count == 2) {
                score += 100;
            }
        }
        
        // Add positional bonus (center is better)
        int centerDist = std::abs(row - BOARD_SIZE/2) + std::abs(col - BOARD_SIZE/2);
        score += (BOARD_SIZE - centerDist) * 10;
        
        board[row][col] = 0; // Remove temporary piece
        return score;
    }

    Move findBestMove(int difficulty, int player) {
        std::vector<Move> moves;
        
        // Special case for first move
        if (moveCount == 0) {
            return Move(BOARD_SIZE/2, BOARD_SIZE/2, 0);
        }
        
        // Consider moves near existing pieces
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (board[i][j] != 0) continue;
                
                // Check if there's a piece nearby
                bool hasNeighbor = false;
                int searchRadius = (difficulty == 3) ? 2 : 1; // Harder AI looks further
                
                for (int di = -searchRadius; di <= searchRadius && !hasNeighbor; di++) {
                    for (int dj = -searchRadius; dj <= searchRadius; dj++) {
                        if (di == 0 && dj == 0) continue;
                        int ni = i + di, nj = j + dj;
                        if (ni >= 0 && ni < BOARD_SIZE && nj >= 0 && nj < BOARD_SIZE && 
                            board[ni][nj] != 0) {
                            hasNeighbor = true;
                            break;
                        }
                    }
                }
                
                if (hasNeighbor) {
                    int opponent = (player == 1) ? 2 : 1;
                    int attackScore = evaluatePosition(i, j, player); // AI's move
                    int defenseScore = evaluatePosition(i, j, opponent); // Block opponent
                    
                    // Adjust weights based on difficulty
                    double defenseWeight = (difficulty == 1) ? 0.5 : (difficulty == 2) ? 0.9 : 1.1;
                    int totalScore = attackScore + defenseScore * defenseWeight;
                    
                    // Check for immediate threats or wins
                    if (attackScore >= 100000) {
                        totalScore = 1000000; // Prioritize winning moves
                    } else if (defenseScore >= 100000) {
                        totalScore = 999999; // Must block opponent's win
                    }
                    
                    moves.push_back(Move(i, j, totalScore));
                }
            }
        }
        
        if (moves.empty()) {
            // No nearby pieces - play near center
            for (int i = BOARD_SIZE/2 - 1; i <= BOARD_SIZE/2 + 1; i++) {
                for (int j = BOARD_SIZE/2 - 1; j <= BOARD_SIZE/2 + 1; j++) {
                    if (isValidMove(i, j)) {
                        return Move(i, j, 0);
                    }
                }
            }
        }
        
        // Sort moves by score
        std::sort(moves.begin(), moves.end(), 
                  [](const Move& a, const Move& b) { return a.score > b.score; });
        
        // Based on difficulty, choose move
        if (difficulty == 1) {
            // Easy: Random from top 50%
            int range = std::max(1, (int)moves.size() / 2);
            return moves[std::rand() % range];
        } else if (difficulty == 2) {
            // Medium: Random from top 3 with some randomness
            int range = std::min(3, (int)moves.size());
            // 70% chance to pick best move, 30% to pick from top 3
            if (std::rand() % 100 < 70) {
                return moves[0];
            } else {
                return moves[std::rand() % range];
            }
        } else {
            // Hard: Always best move with occasional variation
            // 90% best move, 10% second best (if exists)
            if (moves.size() > 1 && std::rand() % 100 < 10) {
                return moves[1];
            }
            return moves[0];
        }
    }

    bool checkDirection(int row, int col, int dRow, int dCol) {
        int count = 1;
        int player = board[row][col];

        count += countConsecutive(row, col, dRow, dCol, player);
        count += countConsecutive(row, col, -dRow, -dCol, player);

        return count >= WIN_COUNT;
    }

    bool checkWin(int row, int col) {
        return checkDirection(row, col, 0, 1) ||  // Horizontal
               checkDirection(row, col, 1, 0) ||  // Vertical
               checkDirection(row, col, 1, 1) ||  // Diagonal
               checkDirection(row, col, 1, -1);   // Diagonal /
    }

    void playAIvsAI() {
        std::cout << "\n=== AI vs AI Mode ===\n";
        std::cout << "\nSelect AI 1 (X) Difficulty:\n";
        std::cout << "1. Easy\n";
        std::cout << "2. Medium\n";
        std::cout << "3. Hard\n";
        std::cout << "Choose difficulty (1-3): ";
        std::cin >> ai1Difficulty;
        ai1Difficulty = std::max(1, std::min(3, ai1Difficulty));
        
        std::cout << "\nSelect AI 2 (O) Difficulty:\n";
        std::cout << "1. Easy\n";
        std::cout << "2. Medium\n";
        std::cout << "3. Hard\n";
        std::cout << "Choose difficulty (1-3): ";
        std::cin >> ai2Difficulty;
        ai2Difficulty = std::max(1, std::min(3, ai2Difficulty));
        
        std::cout << "\nGame speed:\n";
        std::cout << "1. Fast (0.5 sec/move)\n";
        std::cout << "2. Normal (1 sec/move)\n";
        std::cout << "3. Slow (2 sec/move)\n";
        std::cout << "Choose speed (1-3): ";
        int speed;
        std::cin >> speed;
        
        int delay = 500; // Default fast
        if (speed == 2) delay = 1000;
        else if (speed == 3) delay = 2000;
        
        std::string ai1Level = (ai1Difficulty == 1) ? "Easy" : (ai1Difficulty == 2) ? "Medium" : "Hard";
        std::string ai2Level = (ai2Difficulty == 1) ? "Easy" : (ai2Difficulty == 2) ? "Medium" : "Hard";
        
        std::cout << "\n=== Starting Game ===\n";
        std::cout << "AI 1 (X): " << ai1Level << " vs AI 2 (O): " << ai2Level << "\n\n";
        
        int moveNumber = 1;
        
        while (!gameOver) {
            displayBoard();
            
            int currentDifficulty = (currentPlayer == 1) ? ai1Difficulty : ai2Difficulty;
            std::string currentAI = (currentPlayer == 1) ? "AI 1 (X)" : "AI 2 (O)";
            std::string currentLevel = (currentPlayer == 1) ? ai1Level : ai2Level;
            
            std::cout << "Move " << moveNumber << ": " << currentAI << " [" << currentLevel << "] is thinking...\n";
            
            // Add delay for visualization
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            
            Move aiMove = findBestMove(currentDifficulty, currentPlayer);
            makeMove(aiMove.row, aiMove.col);
            
            std::cout << currentAI << " played at: " << aiMove.row << " " << aiMove.col << "\n";
            std::cout << "----------------------------------------\n";
            
            moveNumber++;
        }
        
        displayBoard();
        std::cout << "\n=== GAME OVER ===\n";
        
        if (winner == 0) {
            std::cout << "It's a DRAW after " << moveCount << " moves!\n";
        } else {
            std::string winnerAI = (winner == 1) ? "AI 1 (X)" : "AI 2 (O)";
            std::string winnerLevel = (winner == 1) ? ai1Level : ai2Level;
            std::cout << winnerAI << " [" << winnerLevel << "] WINS in " << moveCount << " moves!\n";
        }
        
        // Show statistics
        std::cout << "\n=== Game Statistics ===\n";
        std::cout << "Total moves: " << moveCount << "\n";
        std::cout << "AI 1 (X) Difficulty: " << ai1Level << "\n";
        std::cout << "AI 2 (O) Difficulty: " << ai2Level << "\n";
    }

    void playGame() {
        std::cout << "\nWelcome to Gomoku (Five in a Row)!\n";
        std::cout << "==================================\n";
        std::cout << "1. Player vs Player\n";
        std::cout << "2. Player vs AI\n";
        std::cout << "3. AI vs AI (Watch Mode)\n";
        std::cout << "Choose game mode (1-3): ";
        
        int mode;
        std::cin >> mode;
        
        if (mode == 3) {
            aiVsAI = true;
            playAIvsAI();
            return;
        }
        
        vsAI = (mode == 2);
        
        if (vsAI) {
            std::cout << "\nAI Difficulty:\n";
            std::cout << "1. Easy\n";
            std::cout << "2. Medium\n";
            std::cout << "3. Hard\n";
            std::cout << "Choose difficulty (1-3): ";
            std::cin >> aiDifficulty;
            aiDifficulty = std::max(1, std::min(3, aiDifficulty));
        }
        
        std::cout << "\nPlayer 1: X" << (vsAI ? ", AI: O" : ", Player 2: O") << "\n";
        std::cout << "Enter moves as 'row col' (e.g., '7 7' for center)\n\n";

        while (!gameOver) {
            displayBoard();
            
            if (vsAI && currentPlayer == 2) {
                std::cout << "AI is thinking...\n";
                Move aiMove = findBestMove(aiDifficulty, 2);
                makeMove(aiMove.row, aiMove.col);
                std::cout << "AI played at: " << aiMove.row << " " << aiMove.col << "\n";
            } else {
                std::cout << "Player " << currentPlayer << "'s turn (" 
                          << (currentPlayer == 1 ? 'X' : 'O') << ")\n";
                std::cout << "Enter row and column: ";

                int row, col;
                if (!(std::cin >> row >> col)) {
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "Invalid input! Please enter two numbers.\n";
                    continue;
                }

                if (!makeMove(row, col)) {
                    std::cout << "Invalid move! Try again.\n";
                    continue;
                }
            }
        }

        displayBoard();
        if (winner == 0) {
            std::cout << "Game Over! It's a draw!\n";
        } else {
            if (vsAI) {
                std::cout << "Game Over! " << (winner == 1 ? "You win!" : "AI wins!") << "\n";
            } else {
                std::cout << "Game Over! Player " << winner << " (" 
                          << (winner == 1 ? 'X' : 'O') << ") wins!\n";
            }
        }
    }

    void reset() {
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                board[i][j] = 0;
            }
        }
        currentPlayer = 1;
        gameOver = false;
        winner = 0;
        moveCount = 0;
        lastMove = {-1, -1};
        aiVsAI = false;
    }
};

int main() {
    Gomoku game;
    char playAgain;

    do {
        game.playGame();
        std::cout << "\nPlay again? (y/n): ";
        std::cin >> playAgain;
        if (playAgain == 'y' || playAgain == 'Y') {
            game.reset();
            std::cout << "\n--- New Game ---\n";
        }
    } while (playAgain == 'y' || playAgain == 'Y');

    std::cout << "Thanks for playing!\n";
    return 0;
}
