#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <thread>
#include <iomanip>
#include <cmath>
#include <tuple>
#include <string>
#include <cctype>

const int BOARD_SIZE = 15;
const int WIN_LENGTH = 5;

class Gomoku {
private:
    std::vector<std::vector<int>> board;
    int currentPlayer;
    int lastMoveRow, lastMoveCol;
    int winner;
    int difficulty; // 1: Easy, 2: Medium, 3: Hard
    int difficulty_ai1; // For AI vs AI mode
    int difficulty_ai2; // For AI vs AI mode
    int totalMoves;
    std::mt19937 rng;
    
    // Statistics
    int playerWins;
    int aiWins;
    int draws;
    int ai1Wins; // For AI vs AI mode
    int ai2Wins; // For AI vs AI mode
    int aiDraws; // For AI vs AI mode
    
public:
    Gomoku() : board(BOARD_SIZE, std::vector<int>(BOARD_SIZE, 0)), 
               currentPlayer(1), 
               lastMoveRow(-1), 
               lastMoveCol(-1),
               winner(0),
               difficulty(2),
               difficulty_ai1(2),
               difficulty_ai2(2),
               totalMoves(0),
               rng(std::chrono::steady_clock::now().time_since_epoch().count()),
               playerWins(0),
               aiWins(0),
               draws(0),
               ai1Wins(0),
               ai2Wins(0),
               aiDraws(0) {}
    
    void displayBoard() {
        // Clear screen
        #ifdef _WIN32
            system("cls");
        #else
            system("clear");
        #endif
        
        std::cout << "\n   ";
        for (int i = 0; i < BOARD_SIZE; i++) {
            std::cout << std::setw(2) << (char)('A' + i) << " ";
        }
        std::cout << "\n";
        
        for (int i = 0; i < BOARD_SIZE; i++) {
            std::cout << std::setw(2) << i + 1 << " ";
            for (int j = 0; j < BOARD_SIZE; j++) {
                // Highlight last move
                if (i == lastMoveRow && j == lastMoveCol) {
                    std::cout << "[";
                } else {
                    std::cout << " ";
                }
                
                if (board[i][j] == 0) {
                    std::cout << ".";
                } else if (board[i][j] == 1) {
                    std::cout << "X";
                } else {
                    std::cout << "O";
                }
                
                if (i == lastMoveRow && j == lastMoveCol) {
                    std::cout << "]";
                } else {
                    std::cout << " ";
                }
            }
            std::cout << " " << i + 1 << "\n";
        }
        
        std::cout << "   ";
        for (int i = 0; i < BOARD_SIZE; i++) {
            std::cout << std::setw(2) << (char)('A' + i) << " ";
        }
        std::cout << "\n\n";
    }
    
    bool isValidMove(int row, int col) {
        return row >= 0 && row < BOARD_SIZE && 
               col >= 0 && col < BOARD_SIZE && 
               board[row][col] == 0;
    }
    
    bool makeMove(int row, int col, int player) {
        if (!isValidMove(row, col)) {
            return false;
        }
        
        board[row][col] = player;
        lastMoveRow = row;
        lastMoveCol = col;
        totalMoves++;
        
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
    
    bool checkDirection(int row, int col, int dRow, int dCol) {
        int player = board[row][col];
        if (player == 0) return false;
        
        int count = 1;
        count += countConsecutive(row, col, dRow, dCol, player);
        count += countConsecutive(row, col, -dRow, -dCol, player);
        
        return count >= WIN_LENGTH;
    }
    
    bool checkWin(int row, int col) {
        return checkDirection(row, col, 0, 1) ||  // Horizontal
               checkDirection(row, col, 1, 0) ||  // Vertical
               checkDirection(row, col, 1, 1) ||  // Diagonal down-right
               checkDirection(row, col, 1, -1);   // Diagonal down-left
    }
    
    bool isBoardFull() {
        return totalMoves >= BOARD_SIZE * BOARD_SIZE;
    }
    
    int evaluatePosition(int row, int col, int player) {
        int score = 0;
        
        board[row][col] = player; // Temporarily place piece
        
        // Check all four directions
        int directions[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}};
        
        for (auto& dir : directions) {
            int count = 1;
            int openEnds = 0;
            
            // Count in positive direction
            int r = row + dir[0];
            int c = col + dir[1];
            while (r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE && 
                   board[r][c] == player) {
                count++;
                r += dir[0];
                c += dir[1];
            }
            if (r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE && 
                board[r][c] == 0) {
                openEnds++;
            }
            
            // Count in negative direction
            r = row - dir[0];
            c = col - dir[1];
            while (r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE && 
                   board[r][c] == player) {
                count++;
                r -= dir[0];
                c -= dir[1];
            }
            if (r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE && 
                board[r][c] == 0) {
                openEnds++;
            }
            
            // Score based on count and openness
            if (count >= 5) {
                score += 100000; // Winning move
            } else if (count == 4) {
                // Check if open on both ends
                if (openEnds == 2) {
                    score += 10000; // Open four
                } else if (openEnds == 1) {
                    score += 5000; // Semi-open four
                }
            } else if (count == 3) {
                if (openEnds == 2) {
                    score += 1000; // Open three
                } else if (openEnds == 1) {
                    score += 500; // Semi-open three
                }
            } else if (count == 2) {
                if (openEnds == 2) {
                    score += 100; // Open two
                } else if (openEnds == 1) {
                    score += 50; // Semi-open two
                }
            }
        }
        
        // Add positional bonus (center is better)
        int centerDistance = std::abs(row - BOARD_SIZE/2) + std::abs(col - BOARD_SIZE/2);
        score += (BOARD_SIZE - centerDistance);
        
        board[row][col] = 0; // Remove temporary piece
        
        return score;
    }
    
    std::pair<int, int> getAIMove(int aiDifficulty = -1) {
        int player = currentPlayer;
        int opponent = (player == 1) ? 2 : 1;
        int useDifficulty = (aiDifficulty == -1) ? difficulty : aiDifficulty;
        
        std::vector<std::tuple<int, int, int>> moves; // row, col, score
        
        // Special case for first move
        if (totalMoves == 0) {
            return {BOARD_SIZE/2, BOARD_SIZE/2};
        }
        
        // Consider moves near existing pieces
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (board[i][j] != 0) continue;
                
                // Check if there's a piece nearby
                bool hasNeighbor = false;
                int searchRadius = (useDifficulty == 3) ? 2 : 1; // Harder AI looks further
                
                for (int di = -searchRadius; di <= searchRadius && !hasNeighbor; di++) {
                    for (int dj = -searchRadius; dj <= searchRadius && !hasNeighbor; dj++) {
                        if (di == 0 && dj == 0) continue;
                        int ni = i + di;
                        int nj = j + dj;
                        if (ni >= 0 && ni < BOARD_SIZE && nj >= 0 && nj < BOARD_SIZE && 
                            board[ni][nj] != 0) {
                            hasNeighbor = true;
                        }
                    }
                }
                
                if (hasNeighbor) {
                    int attackScore = evaluatePosition(i, j, player); // AI's move
                    int defenseScore = evaluatePosition(i, j, opponent); // Block opponent
                    
                    // Adjust weights based on difficulty
                    float attackWeight = 1.0;
                    float defenseWeight = 0.9;
                    
                    if (useDifficulty == 1) { // Easy
                        attackWeight = 0.7;
                        defenseWeight = 0.5;
                    } else if (useDifficulty == 2) { // Medium
                        attackWeight = 0.9;
                        defenseWeight = 0.8;
                    }
                    
                    int totalScore = static_cast<int>(attackScore * attackWeight + 
                                                      defenseScore * defenseWeight);
                    
                    // Check for immediate threats or wins
                    if (attackScore >= 100000) {
                        totalScore = 1000000; // Prioritize winning moves
                    } else if (defenseScore >= 100000) {
                        totalScore = 999999; // Must block opponent's win
                    }
                    
                    moves.push_back({i, j, totalScore});
                }
            }
        }
        
        // No nearby pieces - play near center
        if (moves.empty()) {
            int center = BOARD_SIZE / 2;
            for (int di = -1; di <= 1; di++) {
                for (int dj = -1; dj <= 1; dj++) {
                    int i = center + di;
                    int j = center + dj;
                    if (isValidMove(i, j)) {
                        int score = evaluatePosition(i, j, player);
                        moves.push_back({i, j, score});
                    }
                }
            }
        }
        
        // Sort moves by score
        std::sort(moves.begin(), moves.end(), 
                  [](const auto& a, const auto& b) {
                      return std::get<2>(a) > std::get<2>(b);
                  });
        
        // Based on difficulty, choose move
        int selectedIndex = 0;
        
        if (useDifficulty == 1 && moves.size() > 1) {
            // Easy: Random from top 50%
            int range = moves.size() / 2;
            std::uniform_int_distribution<int> dist(0, range);
            selectedIndex = dist(rng);
        } else if (useDifficulty == 2 && moves.size() > 1) {
            // Medium: Random from top 3 with some randomness
            std::uniform_real_distribution<double> prob(0.0, 1.0);
            // 70% chance to pick best move, 30% to pick from top 3
            if (prob(rng) < 0.7) {
                selectedIndex = 0;
            } else {
                int range = std::min(3, static_cast<int>(moves.size()));
                std::uniform_int_distribution<int> dist(0, range - 1);
                selectedIndex = dist(rng);
            }
        } else if (useDifficulty == 3 && moves.size() > 1) {
            // Hard: Always best move with occasional variation
            std::uniform_real_distribution<double> prob(0.0, 1.0);
            // 90% best move, 10% second best (if exists)
            if (prob(rng) < 0.9 || moves.size() == 1) {
                selectedIndex = 0;
            } else {
                selectedIndex = 1;
            }
        }
        
        if (!moves.empty()) {
            return {std::get<0>(moves[selectedIndex]), 
                    std::get<1>(moves[selectedIndex])};
        }
        
        // Fallback to center
        return {BOARD_SIZE/2, BOARD_SIZE/2};
    }
    
    void playAITurn(const std::string& aiName = "AI", int aiDifficulty = -1) {
        std::cout << aiName << " is thinking";
        
        int delay = 500; // Default fast
        int useDifficulty = (aiDifficulty == -1) ? difficulty : aiDifficulty;
        
        if (useDifficulty == 1) {
            delay = 300;
        } else if (useDifficulty == 2) {
            delay = 500;
        } else {
            delay = 700;
        }
        
        // Add delay for visualization
        for (int i = 0; i < 3; i++) {
            std::cout << ".";
            std::cout.flush();
            std::this_thread::sleep_for(std::chrono::milliseconds(delay / 3));
        }
        std::cout << "\n";
        
        auto [row, col] = getAIMove(aiDifficulty);
        makeMove(row, col, currentPlayer);
        
        std::cout << aiName << " plays: " << static_cast<char>('A' + col) << (row + 1) << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    void playHumanTurn() {
        std::string input;
        bool validMove = false;
        
        while (!validMove) {
            std::cout << "Player " << currentPlayer << " (";
            std::cout << (currentPlayer == 1 ? "X" : "O");
            std::cout << "), enter your move (e.g., H8): ";
            std::cin >> input;
            
            if (input.length() >= 2) {
                char colChar = std::toupper(input[0]);
                int col = colChar - 'A';
                int row;
                
                try {
                    row = std::stoi(input.substr(1)) - 1;
                    
                    if (isValidMove(row, col)) {
                        makeMove(row, col, currentPlayer);
                        validMove = true;
                    } else {
                        std::cout << "Invalid move! That position is occupied or out of bounds.\n";
                    }
                } catch (...) {
                    std::cout << "Invalid input format! Use format like 'H8'.\n";
                }
            } else {
                std::cout << "Invalid input format! Use format like 'H8'.\n";
            }
        }
    }
    
    void showStatistics() {
        // Show statistics
        std::cout << "\n=== Game Statistics ===\n";
        std::cout << "\n-- Human vs AI --\n";
        std::cout << "Player Wins: " << playerWins << "\n";
        std::cout << "AI Wins: " << aiWins << "\n";
        std::cout << "Draws: " << draws << "\n";
        std::cout << "Total: " << (playerWins + aiWins + draws) << "\n";
        
        std::cout << "\n-- AI vs AI --\n";
        std::cout << "AI 1 (X) Wins: " << ai1Wins << "\n";
        std::cout << "AI 2 (O) Wins: " << ai2Wins << "\n";
        std::cout << "Draws: " << aiDraws << "\n";
        std::cout << "Total: " << (ai1Wins + ai2Wins + aiDraws) << "\n";
        std::cout << "======================\n\n";
    }
    
    void setDifficulty(int diff) {
        difficulty = std::max(1, std::min(3, diff));
    }
    
    void setAIDifficulties(int diff1, int diff2) {
        difficulty_ai1 = std::max(1, std::min(3, diff1));
        difficulty_ai2 = std::max(1, std::min(3, diff2));
    }
    
    std::string getDifficultyName(int diff) {
        if (diff == 1) return "Easy";
        if (diff == 2) return "Medium";
        return "Hard";
    }
    
    void resetBoard() {
        board = std::vector<std::vector<int>>(BOARD_SIZE, std::vector<int>(BOARD_SIZE, 0));
        currentPlayer = 1;
        lastMoveRow = -1;
        lastMoveCol = -1;
        winner = 0; // Draw
        totalMoves = 0;
    }
    
    void playAIvsAI() {
        resetBoard();
        bool gameOver = false;
        
        std::cout << "\n=== AI vs AI Match ===\n";
        std::cout << "AI 1 (X): " << getDifficultyName(difficulty_ai1) << "\n";
        std::cout << "AI 2 (O): " << getDifficultyName(difficulty_ai2) << "\n";
        std::cout << "Starting in 2 seconds...\n\n";
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        while (!gameOver) {
            displayBoard();
            
            std::string aiName = (currentPlayer == 1) ? "AI 1 (X)" : "AI 2 (O)";
            int aiDiff = (currentPlayer == 1) ? difficulty_ai1 : difficulty_ai2;
            
            playAITurn(aiName, aiDiff);
            
            if (checkWin(lastMoveRow, lastMoveCol)) {
                winner = currentPlayer;
                gameOver = true;
            } else if (isBoardFull()) {
                winner = 0; // Draw
                gameOver = true;
            } else {
                currentPlayer = (currentPlayer == 1) ? 2 : 1;
            }
        }
        
        displayBoard();
        
        if (winner == 0) {
            std::cout << "\n=== Game Over: Draw! ===\n";
            aiDraws++;
        } else {
            std::cout << "\n=== Game Over: AI " << winner;
            std::cout << " (" << (winner == 1 ? "X" : "O") << ") Wins! ===\n";
            if (winner == 1) {
                ai1Wins++;
            } else {
                ai2Wins++;
            }
        }
        
        showStatistics();
    }
    
    void playGame(bool vsAI = true) {
        resetBoard();
        bool gameOver = false;
        
        while (!gameOver) {
            displayBoard();
            
            if (vsAI && currentPlayer == 2) {
                playAITurn();
            } else {
                playHumanTurn();
            }
            
            if (checkWin(lastMoveRow, lastMoveCol)) {
                winner = currentPlayer;
                gameOver = true;
            } else if (isBoardFull()) {
                winner = 0; // Draw
                gameOver = true;
            } else {
                currentPlayer = (currentPlayer == 1) ? 2 : 1;
            }
        }
        
        displayBoard();
        
        if (winner == 0) {
            std::cout << "\n=== Game Over: Draw! ===\n";
            draws++;
        } else {
            std::cout << "\n=== Game Over: Player " << winner;
            std::cout << " (" << (winner == 1 ? "X" : "O") << ") Wins! ===\n";
            if (vsAI) {
                if (winner == 1) {
                    playerWins++;
                } else {
                    aiWins++;
                }
            }
        }
        
        showStatistics();
    }
    
    void showMenu() {
        std::cout << "\n===== GOMOKU (Five in a Row) =====\n";
        std::cout << "1. Play vs AI\n";
        std::cout << "2. Play vs Human\n";
        std::cout << "3. Watch AI vs AI\n";
        std::cout << "4. Set AI Difficulty (Current: " << getDifficultyName(difficulty) << ")\n";
        std::cout << "5. Set AI vs AI Difficulties (AI1: " << getDifficultyName(difficulty_ai1) 
                  << ", AI2: " << getDifficultyName(difficulty_ai2) << ")\n";
        std::cout << "6. Show Statistics\n";
        std::cout << "7. Exit\n";
        std::cout << "==================================\n";
        std::cout << "Enter your choice: ";
    }
    
    void run() {
        int choice;
        bool running = true;
        
        while (running) {
            showMenu();
            std::cin >> choice;
            
            switch (choice) {
                case 1:
                    playGame(true);
                    break;
                case 2:
                    playGame(false);
                    break;
                case 3:
                    playAIvsAI();
                    break;
                case 4: {
                    int diff;
                    std::cout << "Enter difficulty (1=Easy, 2=Medium, 3=Hard): ";
                    std::cin >> diff;
                    setDifficulty(diff);
                    std::cout << "AI difficulty set to " << getDifficultyName(difficulty) << "\n";
                    break;
                }
                case 5: {
                    int diff1, diff2;
                    std::cout << "Enter AI 1 difficulty (1=Easy, 2=Medium, 3=Hard): ";
                    std::cin >> diff1;
                    std::cout << "Enter AI 2 difficulty (1=Easy, 2=Medium, 3=Hard): ";
                    std::cin >> diff2;
                    setAIDifficulties(diff1, diff2);
                    std::cout << "AI 1 set to " << getDifficultyName(difficulty_ai1) 
                              << ", AI 2 set to " << getDifficultyName(difficulty_ai2) << "\n";
                    break;
                }
                case 6:
                    showStatistics();
                    break;
                case 7:
                    running = false;
                    std::cout << "Thanks for playing!\n";
                    break;
                default:
                    std::cout << "Invalid choice. Please try again.\n";
            }
            
            if (running && (choice >= 1 && choice <= 3)) {
                char playAgain;
                std::cout << "Play again? (y/n): ";
                std::cin >> playAgain;
                if (std::tolower(playAgain) != 'y') {
                    running = false;
                    std::cout << "Thanks for playing!\n";
                }
            }
        }
    }
};

int main() {
    Gomoku game;
    game.run();
    return 0;
}
