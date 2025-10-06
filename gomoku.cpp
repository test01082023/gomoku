#include <iostream>
#include <vector>
#include <iomanip>
#include <limits>
#include <cstdlib>
#include <ctime>
#include <algorithm>

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
    int aiDifficulty;

    struct Move {
        int row, col, score;
        Move(int r = -1, int c = -1, int s = 0) : row(r), col(c), score(s) {}
    };

public:
    Gomoku() : board(BOARD_SIZE, std::vector<int>(BOARD_SIZE, 0)), 
               currentPlayer(1), gameOver(false), winner(0), moveCount(0),
               vsAI(false), aiDifficulty(2) {
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
                std::cout << " " << symbol << " ";
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
                score += 10000; // Winning move
            } else if (count == 4) {
                score += 1000;
            } else if (count == 3) {
                score += 100;
            } else if (count == 2) {
                score += 10;
            }
        }
        
        board[row][col] = 0; // Remove temporary piece
        return score;
    }

    Move findBestMove() {
        std::vector<Move> moves;
        
        // Consider moves near existing pieces
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (board[i][j] != 0) continue;
                
                // Check if there's a piece nearby
                bool hasNeighbor = false;
                for (int di = -2; di <= 2 && !hasNeighbor; di++) {
                    for (int dj = -2; dj <= 2; dj++) {
                        int ni = i + di, nj = j + dj;
                        if (ni >= 0 && ni < BOARD_SIZE && nj >= 0 && nj < BOARD_SIZE && 
                            board[ni][nj] != 0) {
                            hasNeighbor = true;
                            break;
                        }
                    }
                }
                
                if (hasNeighbor || moveCount == 0) {
                    int attackScore = evaluatePosition(i, j, 2); // AI's move
                    int defenseScore = evaluatePosition(i, j, 1); // Block opponent
                    int totalScore = attackScore + defenseScore * 0.9; // Slightly favor attack
                    
                    moves.push_back(Move(i, j, totalScore));
                }
            }
        }
        
        if (moves.empty()) {
            // First move or no nearby pieces - play center
            return Move(BOARD_SIZE/2, BOARD_SIZE/2, 0);
        }
        
        // Sort moves by score
        std::sort(moves.begin(), moves.end(), 
                  [](const Move& a, const Move& b) { return a.score > b.score; });
        
        // Based on difficulty, choose move
        if (aiDifficulty == 1) {
            // Easy: Random from top 50%
            int range = moves.size() / 2;
            return moves[std::rand() % range];
        } else if (aiDifficulty == 2) {
            // Medium: Random from top 3
            int range = std::min(3, (int)moves.size());
            return moves[std::rand() % range];
        } else {
            // Hard: Always best move
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

    void playGame() {
        std::cout << "Welcome to Gomoku (Five in a Row)!\n";
        std::cout << "1. Player vs Player\n";
        std::cout << "2. Player vs AI\n";
        std::cout << "Choose game mode (1 or 2): ";
        
        int mode;
        std::cin >> mode;
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
                Move aiMove = findBestMove();
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
