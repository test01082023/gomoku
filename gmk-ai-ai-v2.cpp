#include <iostream>
#include <vector>
#include <iomanip>
#include <limits>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <chrono>
#include <thread>
#include <array>
#include <unordered_set>

class Gomoku {
private:
    static constexpr int BOARD_SIZE = 15;
    static constexpr int WIN_COUNT = 5;
    static constexpr int MAX_MOVES = BOARD_SIZE * BOARD_SIZE;
    static constexpr int WIN_SCORE = 1000000;
    static constexpr int MUST_BLOCK_SCORE = 999999;
    
    // Direction vectors for optimization
    static constexpr int DIRECTIONS[4][2] = {{0,1}, {1,0}, {1,1}, {1,-1}};
    
    // Use array instead of vector for better cache locality
    std::array<std::array<int, BOARD_SIZE>, BOARD_SIZE> board;
    
    // Precomputed neighbor positions for faster move generation
    std::vector<std::pair<int, int>> occupiedPositions;
    std::unordered_set<int> consideredMoves; // Using hash for O(1) lookup
    
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
    
    // Cache for evaluation scores
    mutable std::array<std::array<int, BOARD_SIZE>, BOARD_SIZE> scoreCache;
    mutable bool cacheValid;

    struct Move {
        int row, col, score;
        Move(int r = -1, int c = -1, int s = 0) : row(r), col(c), score(s) {}
        
        bool operator<(const Move& other) const {
            return score > other.score; // For sorting in descending order
        }
    };
    
    // Thread pool for parallel evaluation (optional)
    static constexpr int THREAD_THRESHOLD = 50; // Use threads only for many moves

public:
    Gomoku() : currentPlayer(1), gameOver(false), winner(0), moveCount(0),
               vsAI(false), aiVsAI(false), aiDifficulty(2), 
               ai1Difficulty(2), ai2Difficulty(2), lastMove(-1, -1),
               cacheValid(false) {
        std::srand(std::time(nullptr));
        resetBoard();
    }
    
    void resetBoard() {
        for (auto& row : board) {
            row.fill(0);
        }
        occupiedPositions.clear();
        consideredMoves.clear();
        scoreCache = {};
        cacheValid = false;
    }

    void displayBoard() const {
        std::cout << "\n   ";
        for (int i = 0; i < BOARD_SIZE; i++) {
            std::cout << std::setw(2) << i << " ";
        }
        std::cout << "\n   " << std::string(BOARD_SIZE * 3, '-') << "\n";

        for (int i = 0; i < BOARD_SIZE; i++) {
            std::cout << std::setw(2) << i << "|";
            for (int j = 0; j < BOARD_SIZE; j++) {
                char symbol = '.';
                if (board[i][j] == 1) symbol = 'X';
                else if (board[i][j] == 2) symbol = 'O';
                
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

    inline bool isValidMove(int row, int col) const {
        return row >= 0 && row < BOARD_SIZE && 
               col >= 0 && col < BOARD_SIZE && 
               board[row][col] == 0;
    }
    
    inline int posToHash(int row, int col) const {
        return row * BOARD_SIZE + col;
    }
    
    inline std::pair<int, int> hashToPos(int hash) const {
        return {hash / BOARD_SIZE, hash % BOARD_SIZE};
    }

    bool makeMove(int row, int col) {
        if (!isValidMove(row, col) || gameOver) {
            return false;
        }

        board[row][col] = currentPlayer;
        lastMove = {row, col};
        occupiedPositions.push_back({row, col});
        moveCount++;
        cacheValid = false; // Invalidate cache

        if (checkWinFast(row, col)) {
            gameOver = true;
            winner = currentPlayer;
        } else if (moveCount == MAX_MOVES) {
            gameOver = true;
            winner = 0;
        } else {
            currentPlayer = 3 - currentPlayer; // Faster than ternary
        }

        return true;
    }

    // Optimized consecutive count using inline and avoiding repeated calculations
    inline int countLine(int row, int col, int dRow, int dCol, int player) const {
        int count = 1; // Count the current position
        
        // Count forward
        int r = row + dRow, c = col + dCol;
        while (r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE && 
               board[r][c] == player) {
            count++;
            r += dRow;
            c += dCol;
        }
        
        // Count backward
        r = row - dRow;
        c = col - dCol;
        while (r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE && 
               board[r][c] == player) {
            count++;
            r -= dRow;
            c -= dCol;
        }
        
        return count;
    }

    // Fast evaluation using pattern matching
    int evaluatePositionFast(int row, int col, int player) const {
        if (!isValidMove(row, col)) return -1;
        
        int score = 0;
        const int opponent = 3 - player;
        
        for (const auto& dir : DIRECTIONS) {
            int playerCount = 0, opponentCount = 0;
            int openEnds = 0;
            
            // Check line in both directions from the position
            for (int sign = -1; sign <= 1; sign += 2) {
                int r = row + sign * dir[0];
                int c = col + sign * dir[1];
                int consecutive = 0;
                
                while (r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE) {
                    if (board[r][c] == player) {
                        playerCount++;
                        consecutive++;
                    } else if (board[r][c] == opponent) {
                        opponentCount++;
                        break;
                    } else {
                        if (consecutive > 0) openEnds++;
                        break;
                    }
                    r += sign * dir[0];
                    c += sign * dir[1];
                }
                
                // Check if line extends to edge
                if (!(r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE)) {
                    if (consecutive == 0) openEnds++;
                }
            }
            
            // Score based on pattern
            int lineLength = playerCount + 1; // Include the position itself
            
            if (lineLength >= WIN_COUNT) {
                return WIN_SCORE; // Winning move
            }
            
            if (opponentCount == 0) { // No blocks
                if (lineLength == 4) {
                    score += (openEnds == 2) ? 50000 : 10000;
                } else if (lineLength == 3) {
                    score += (openEnds == 2) ? 5000 : 1000;
                } else if (lineLength == 2) {
                    score += (openEnds == 2) ? 500 : 100;
                }
            }
        }
        
        // Center preference with manhattan distance
        int centerDist = std::abs(row - BOARD_SIZE/2) + std::abs(col - BOARD_SIZE/2);
        score += (BOARD_SIZE - centerDist) * 5;
        
        return score;
    }

    // Optimized move generation
    std::vector<Move> generateCandidateMoves(int player) const {
        std::vector<Move> moves;
        moves.reserve(50); // Pre-allocate typical number of moves
        
        if (moveCount == 0) {
            moves.emplace_back(BOARD_SIZE/2, BOARD_SIZE/2, 0);
            return moves;
        }
        
        // Use hash set for O(1) lookup of already considered positions
        std::unordered_set<int> visited;
        
        // Generate moves around occupied positions
        int searchRadius = (aiDifficulty == 3) ? 2 : 1;
        
        for (const auto& [occRow, occCol] : occupiedPositions) {
            for (int di = -searchRadius; di <= searchRadius; di++) {
                for (int dj = -searchRadius; dj <= searchRadius; dj++) {
                    if (di == 0 && dj == 0) continue;
                    
                    int ni = occRow + di;
                    int nj = occCol + dj;
                    
                    if (isValidMove(ni, nj)) {
                        int hash = posToHash(ni, nj);
                        if (visited.insert(hash).second) { // Returns true if newly inserted
                            int opponent = 3 - player;
                            int attackScore = evaluatePositionFast(ni, nj, player);
                            int defenseScore = evaluatePositionFast(ni, nj, opponent);
                            
                            // Priority calculation based on difficulty
                            double defenseWeight = (aiDifficulty == 1) ? 0.5 : 
                                                 (aiDifficulty == 2) ? 0.9 : 1.1;
                            int totalScore = attackScore + static_cast<int>(defenseScore * defenseWeight);
                            
                            // Immediate threats handling
                            if (attackScore >= WIN_SCORE) {
                                totalScore = WIN_SCORE + 1000;
                            } else if (defenseScore >= WIN_SCORE) {
                                totalScore = MUST_BLOCK_SCORE;
                            }
                            
                            moves.emplace_back(ni, nj, totalScore);
                        }
                    }
                }
            }
        }
        
        return moves;
    }

    Move findBestMove(int difficulty, int player) {
        auto moves = generateCandidateMoves(player);
        
        if (moves.empty()) {
            // Fallback to center area
            for (int i = BOARD_SIZE/2 - 1; i <= BOARD_SIZE/2 + 1; i++) {
                for (int j = BOARD_SIZE/2 - 1; j <= BOARD_SIZE/2 + 1; j++) {
                    if (isValidMove(i, j)) {
                        return Move(i, j, 0);
                    }
                }
            }
        }
        
        // Use partial_sort for efficiency when we only need top moves
        int topN = (difficulty == 1) ? moves.size() / 2 : 
                   (difficulty == 2) ? std::min(3, (int)moves.size()) : 1;
        
        std::partial_sort(moves.begin(), moves.begin() + topN, moves.end());
        
        // Select move based on difficulty
        if (difficulty == 1) {
            return moves[std::rand() % topN];
        } else if (difficulty == 2) {
            return (std::rand() % 100 < 70) ? moves[0] : moves[std::rand() % topN];
        } else {
            return (moves.size() > 1 && std::rand() % 100 < 10) ? moves[1] : moves[0];
        }
    }

    // Optimized win checking
    bool checkWinFast(int row, int col) const {
        int player = board[row][col];
        
        for (const auto& dir : DIRECTIONS) {
            if (countLine(row, col, dir[0], dir[1], player) >= WIN_COUNT) {
                return true;
            }
        }
        return false;
    }

    void playAIvsAI() {
        std::cout << "\n=== AI vs AI Mode ===\n";
        
        // Get AI difficulties
        auto getDifficulty = [](const std::string& aiName) {
            std::cout << "\nSelect " << aiName << " Difficulty:\n";
            std::cout << "1. Easy\n2. Medium\n3. Hard\n";
            std::cout << "Choose difficulty (1-3): ";
            int diff;
            std::cin >> diff;
            return std::clamp(diff, 1, 3);
        };
        
        ai1Difficulty = getDifficulty("AI 1 (X)");
        ai2Difficulty = getDifficulty("AI 2 (O)");
        
        std::cout << "\nGame speed:\n";
        std::cout << "1. Fast (0.5 sec/move)\n2. Normal (1 sec/move)\n3. Slow (2 sec/move)\n";
        std::cout << "Choose speed (1-3): ";
        int speed;
        std::cin >> speed;
        
        int delay = (speed == 2) ? 1000 : (speed == 3) ? 2000 : 500;
        
        const char* levels[] = {"", "Easy", "Medium", "Hard"};
        
        std::cout << "\n=== Starting Game ===\n";
        std::cout << "AI 1 (X): " << levels[ai1Difficulty] 
                  << " vs AI 2 (O): " << levels[ai2Difficulty] << "\n\n";
        
        int moveNumber = 1;
        
        while (!gameOver) {
            displayBoard();
            
            int currentDifficulty = (currentPlayer == 1) ? ai1Difficulty : ai2Difficulty;
            std::string currentAI = (currentPlayer == 1) ? "AI 1 (X)" : "AI 2 (O)";
            
            std::cout << "Move " << moveNumber << ": " << currentAI 
                      << " [" << levels[currentDifficulty] << "] is thinking...\n";
            
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            
            Move aiMove = findBestMove(currentDifficulty, currentPlayer);
            makeMove(aiMove.row, aiMove.col);
            
            std::cout << currentAI << " played at: " << aiMove.row << " " << aiMove.col << "\n";
            std::cout << std::string(40, '-') << "\n";
            
            moveNumber++;
        }
        
        displayBoard();
        displayGameResult(levels[ai1Difficulty], levels[ai2Difficulty]);
    }
    
    void displayGameResult(const std::string& ai1Level, const std::string& ai2Level) const {
        std::cout << "\n=== GAME OVER ===\n";
        
        if (winner == 0) {
            std::cout << "It's a DRAW after " << moveCount << " moves!\n";
        } else {
            std::string winnerAI = (winner == 1) ? "AI 1 (X)" : "AI 2 (O)";
            std::string winnerLevel = (winner == 1) ? ai1Level : ai2Level;
            std::cout << winnerAI << " [" << winnerLevel << "] WINS in " << moveCount << " moves!\n";
        }
        
        std::cout << "\n=== Game Statistics ===\n";
        std::cout << "Total moves: " << moveCount << "\n";
        std::cout << "AI 1 (X) Difficulty: " << ai1Level << "\n";
        std::cout << "AI 2 (O) Difficulty: " << ai2Level << "\n";
    }

    void playGame() {
        std::cout << "\nWelcome to Gomoku (Five in a Row)!\n";
        std::cout << std::string(35, '=') << "\n";
        std::cout << "1. Player vs Player\n2. Player vs AI\n3. AI vs AI (Watch Mode)\n";
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
            std::cout << "\nAI Difficulty:\n1. Easy\n2. Medium\n3. Hard\n";
            std::cout << "Choose difficulty (1-3): ";
            std::cin >> aiDifficulty;
            aiDifficulty = std::clamp(aiDifficulty, 1, 3);
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
        resetBoard();
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
