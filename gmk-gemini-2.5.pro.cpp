//created by gemini-2.5.pro
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <limits>
#include <chrono>
#include <map>

// --- Configuration ---
constexpr int BOARD_SIZE = 15;
constexpr int WIN_LENGTH = 5;
constexpr int AI_SEARCH_DEPTH = 4; // Adjust for difficulty. 4 is strong, 6 is very strong.

// --- Core Game Definitions ---

enum class Cell {
    EMPTY,
    BLACK,
    WHITE
};

struct Move {
    int row = -1, col = -1;
};

// --- Helper Functions ---

// Converts Cell enum to a printable character
char cellToChar(Cell c) {
    switch (c) {
        case Cell::BLACK: return 'X';
        case Cell::WHITE: return 'O';
        default: return '.';
    }
}

// Gets the opponent's color
Cell getOpponent(Cell player) {
    return (player == Cell::BLACK) ? Cell::WHITE : Cell::BLACK;
}


// --- Game Logic Class ---

class GomokuGame {
private:
    std::vector<std::vector<Cell>> board;
    int move_count = 0;

public:
    GomokuGame() : board(BOARD_SIZE, std::vector<Cell>(BOARD_SIZE, Cell::EMPTY)) {}

    // Prints the game board to the console
    void printBoard() const {
        std::cout << "   ";
        for (int i = 0; i < BOARD_SIZE; ++i) {
            std::cout << (char)('A' + i) << " ";
        }
        std::cout << std::endl;

        for (int i = 0; i < BOARD_SIZE; ++i) {
            printf("%2d ", i);
            for (int j = 0; j < BOARD_SIZE; ++j) {
                std::cout << cellToChar(board[i][j]) << " ";
            }
            std::cout << std::endl;
        }
    }

    bool isValidMove(int r, int c) const {
        return r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE && board[r][c] == Cell::EMPTY;
    }

    void makeMove(const Move& m, Cell player) {
        if (isValidMove(m.row, m.col)) {
            board[m.row][m.col] = player;
            move_count++;
        }
    }
    
    void undoMove(const Move& m) {
        if (m.row != -1 && m.col != -1) {
            board[m.row][m.col] = Cell::EMPTY;
            move_count--;
        }
    }

    bool isBoardFull() const {
        return move_count == BOARD_SIZE * BOARD_SIZE;
    }

    // Optimized win check focusing only on the last move
    bool checkWin(const Move& last_move, Cell player) const {
        if (last_move.row == -1) return false;

        int r = last_move.row;
        int c = last_move.col;

        // Directions: horizontal, vertical, diagonal (down-right), diagonal (up-right)
        int dr[] = {0, 1, 1, 1};
        int dc[] = {1, 0, 1, -1};

        for (int i = 0; i < 4; ++i) {
            int count = 1;
            // Check in one direction
            for (int j = 1; j < WIN_LENGTH; ++j) {
                int nr = r + j * dr[i];
                int nc = c + j * dc[i];
                if (nr >= 0 && nr < BOARD_SIZE && nc >= 0 && nc < BOARD_SIZE && board[nr][nc] == player) {
                    count++;
                } else {
                    break;
                }
            }
            // Check in the opposite direction
            for (int j = 1; j < WIN_LENGTH; ++j) {
                int nr = r - j * dr[i];
                int nc = c - j * dc[i];
                if (nr >= 0 && nr < BOARD_SIZE && nc >= 0 && nc < BOARD_SIZE && board[nr][nc] == player) {
                    count++;
                } else {
                    break;
                }
            }
            if (count >= WIN_LENGTH) {
                return true;
            }
        }
        return false;
    }

    const std::vector<std::vector<Cell>>& getBoard() const {
        return board;
    }
    
    int getMoveCount() const { return move_count; }
};


// --- AI Player Class ---

class AIPlayer {
private:
    Cell ai_player;
    Cell opponent_player;
    int search_depth;

    // Scores for different patterns. Higher is better.
    const int SCORE_FIVE = 100000000;
    const int SCORE_OPEN_FOUR = 1000000;
    const int SCORE_HALF_OPEN_FOUR = 10000;
    const int SCORE_OPEN_THREE = 5000;
    const int SCORE_HALF_OPEN_THREE = 100;
    const int SCORE_OPEN_TWO = 50;
    const int SCORE_HALF_OPEN_TWO = 10;
    const int SCORE_ONE = 1;

public:
    AIPlayer(Cell player_color, int depth) : ai_player(player_color), search_depth(depth) {
        opponent_player = getOpponent(player_color);
    }

    Move findBestMove(GomokuGame& game) {
        Move best_move;
        int best_score = std::numeric_limits<int>::min();
        auto candidate_moves = generateMoves(game.getBoard());

        // First move optimization: play in the center
        if (game.getMoveCount() == 0) {
            return {BOARD_SIZE / 2, BOARD_SIZE / 2};
        }

        for (const auto& move : candidate_moves) {
            game.makeMove(move, ai_player);
            int score = minimax(game, search_depth - 1, false, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), move);
            game.undoMove(move);

            if (score > best_score) {
                best_score = score;
                best_move = move;
            }
        }
        return best_move;
    }

private:
    int minimax(GomokuGame& game, int depth, bool is_maximizing, int alpha, int beta, const Move& last_move) {
        // Base cases: check for terminal states or max depth
        if (game.checkWin(last_move, is_maximizing ? opponent_player : ai_player)) {
            return is_maximizing ? std::numeric_limits<int>::min() + 1 : std::numeric_limits<int>::max() - 1;
        }
        if (game.isBoardFull()) {
            return 0;
        }
        if (depth == 0) {
            return evaluateBoard(game.getBoard());
        }

        auto candidate_moves = generateMoves(game.getBoard());

        if (is_maximizing) {
            int max_eval = std::numeric_limits<int>::min();
            for (const auto& move : candidate_moves) {
                game.makeMove(move, ai_player);
                int eval = minimax(game, depth - 1, false, alpha, beta, move);
                game.undoMove(move);
                max_eval = std::max(max_eval, eval);
                alpha = std::max(alpha, eval);
                if (beta <= alpha) {
                    break; // Prune
                }
            }
            return max_eval;
        } else { // Minimizing player
            int min_eval = std::numeric_limits<int>::max();
            for (const auto& move : candidate_moves) {
                game.makeMove(move, opponent_player);
                int eval = minimax(game, depth - 1, true, alpha, beta, move);
                game.undoMove(move);
                min_eval = std::min(min_eval, eval);
                beta = std::min(beta, eval);
                if (beta <= alpha) {
                    break; // Prune
                }
            }
            return min_eval;
        }
    }
    
    // Generates moves only in the vicinity of existing pieces for efficiency
    std::vector<Move> generateMoves(const std::vector<std::vector<Cell>>& board) {
        std::vector<Move> moves;
        std::vector<std::vector<bool>> visited(BOARD_SIZE, std::vector<bool>(BOARD_SIZE, false));

        for (int r = 0; r < BOARD_SIZE; ++r) {
            for (int c = 0; c < BOARD_SIZE; ++c) {
                if (board[r][c] == Cell::EMPTY) continue;

                // Check a 3x3 area around each existing piece
                for (int dr = -1; dr <= 1; ++dr) {
                    for (int dc = -1; dc <= 1; ++dc) {
                        int nr = r + dr;
                        int nc = c + dc;

                        if (nr >= 0 && nr < BOARD_SIZE && nc >= 0 && nc < BOARD_SIZE &&
                            board[nr][nc] == Cell::EMPTY && !visited[nr][nc]) {
                            moves.push_back({nr, nc});
                            visited[nr][nc] = true;
                        }
                    }
                }
            }
        }
        return moves;
    }

    // The core heuristic function: evaluates the entire board state
    int evaluateBoard(const std::vector<std::vector<Cell>>& board) {
        int score = 0;
        // Evaluate rows, columns, and diagonals
        for (int i = 0; i < BOARD_SIZE; ++i) {
            score += evaluateLine(board, i, 0, 0, 1); // Rows
            score += evaluateLine(board, 0, i, 1, 0); // Columns
        }
        // Diagonals
        for (int i = 0; i < BOARD_SIZE; ++i) {
            score += evaluateLine(board, i, 0, 1, 1); // Top-left to bottom-right
            if (i > 0) score += evaluateLine(board, 0, i, 1, 1);
            
            score += evaluateLine(board, i, 0, 1, -1); // Top-right to bottom-left
            if (i > 0) score += evaluateLine(board, 0, i, 1, -1);
        }
        return score;
    }

    // Evaluates a single line (row, col, or diagonal)
    int evaluateLine(const std::vector<std::vector<Cell>>& board, int r_start, int c_start, int dr, int dc) {
        int ai_score = 0;
        int opponent_score = 0;

        for (int i = 0; i <= BOARD_SIZE - WIN_LENGTH; ++i) {
            int r = r_start + i * dr;
            int c = c_start + i * dc;
            if (r < 0 || r >= BOARD_SIZE || c < 0 || c >= BOARD_SIZE) continue;
            
            ai_score += scoreWindow(board, r, c, dr, dc, ai_player);
            opponent_score += scoreWindow(board, r, c, dr, dc, opponent_player);
        }
        return ai_score - opponent_score;
    }
    
    // Scores a window of size WIN_LENGTH
    int scoreWindow(const std::vector<std::vector<Cell>>& board, int r_start, int c_start, int dr, int dc, Cell player) {
        int player_count = 0;
        int empty_count = 0;

        for (int k = 0; k < WIN_LENGTH; ++k) {
            int r = r_start + k * dr;
            int c = c_start + k * dc;

            if (r < 0 || r >= BOARD_SIZE || c < 0 || c >= BOARD_SIZE) return 0; // Out of bounds

            if (board[r][c] == player) player_count++;
            else if (board[r][c] == Cell::EMPTY) empty_count++;
            else return 0; // Contains opponent's piece, no threat for `player` in this window
        }

        if (player_count == 5) return SCORE_FIVE;
        if (player_count == 4 && empty_count == 1) return SCORE_HALF_OPEN_FOUR;
        if (player_count == 3 && empty_count == 2) return SCORE_HALF_OPEN_THREE;
        if (player_count == 2 && empty_count == 3) return SCORE_HALF_OPEN_TWO;
        if (player_count == 1 && empty_count == 4) return SCORE_ONE;

        // Note: Open patterns (e.g., _XXX_) are handled by overlapping windows.
        // For example, _XXX_ will be caught by two half-open three windows.
        // A more complex evaluation function could detect them explicitly.

        return 0;
    }

};

// --- Main Game Loop ---

int main() {
    GomokuGame game;
    AIPlayer ai_black(Cell::BLACK, AI_SEARCH_DEPTH);
    AIPlayer ai_white(Cell::WHITE, AI_SEARCH_DEPTH);

    Cell current_player = Cell::BLACK;
    Move last_move;

    while (true) {
        game.printBoard();
        
        std::cout << "\nPlayer " << cellToChar(current_player) << "'s turn." << std::endl;
        auto start = std::chrono::high_resolution_clock::now();

        if (current_player == Cell::BLACK) {
            last_move = ai_black.findBestMove(game);
        } else {
            last_move = ai_white.findBestMove(game);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;
        
        if (!game.isValidMove(last_move.row, last_move.col)) {
             std::cout << "AI generated an invalid move! Something is wrong." << std::endl;
             break;
        }

        std::cout << "AI moved to " << (char)('A' + last_move.col) << last_move.row
                  << " in " << diff.count() << " seconds." << std::endl;

        game.makeMove(last_move, current_player);

        if (game.checkWin(last_move, current_player)) {
            game.printBoard();
            std::cout << "\nPlayer " << cellToChar(current_player) << " wins!" << std::endl;
            break;
        }

        if (game.isBoardFull()) {
            game.printBoard();
            std::cout << "\nIt's a draw!" << std::endl;
            break;
        }

        current_player = getOpponent(current_player);
    }

    return 0;
}
