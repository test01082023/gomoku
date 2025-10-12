// gomoku.cpp - Production-quality Gomoku AI vs AI 
//created by claude opus-4.1-08052025
#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include <chrono>
#include <random>
#include <limits>
#include <memory>
#include <unordered_map>
#include <thread>
#include <iomanip>

constexpr int BOARD_SIZE = 15;
constexpr int WIN_LENGTH = 5;
constexpr int MAX_DEPTH = 8;
constexpr int INFINITY_SCORE = 1000000;
constexpr int WIN_SCORE = 100000;

enum class Stone { EMPTY = 0, BLACK = 1, WHITE = 2 };
enum class GameStatus { ONGOING, BLACK_WIN, WHITE_WIN, DRAW };

// Direction vectors for checking lines
const std::vector<std::pair<int, int>> DIRECTIONS = {
    {0, 1},   // Horizontal
    {1, 0},   // Vertical
    {1, 1},   // Diagonal \
    {1, -1}   // Diagonal /
};

// Pattern scores for evaluation
struct PatternScore {
    static constexpr int FIVE = 100000;
    static constexpr int OPEN_FOUR = 10000;
    static constexpr int BLOCKED_FOUR = 1000;
    static constexpr int OPEN_THREE = 1000;
    static constexpr int BLOCKED_THREE = 100;
    static constexpr int OPEN_TWO = 100;
    static constexpr int BLOCKED_TWO = 10;
    static constexpr int ONE = 1;
};

class Position {
public:
    int row, col;
    Position(int r = 0, int c = 0) : row(r), col(c) {}
    bool isValid() const { 
        return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE; 
    }
    bool operator==(const Position& other) const {
        return row == other.row && col == other.col;
    }
};

class Board {
private:
    std::array<std::array<Stone, BOARD_SIZE>, BOARD_SIZE> board;
    std::vector<Position> moveHistory;
    int moveCount;
    
public:
    Board() : moveCount(0) {
        for (auto& row : board) {
            row.fill(Stone::EMPTY);
        }
    }
    
    Stone getStone(int row, int col) const {
        if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) {
            return Stone::EMPTY;
        }
        return board[row][col];
    }
    
    bool placeStone(int row, int col, Stone stone) {
        if (!isValidMove(row, col)) return false;
        board[row][col] = stone;
        moveHistory.push_back(Position(row, col));
        moveCount++;
        return true;
    }
    
    void removeStone(int row, int col) {
        board[row][col] = Stone::EMPTY;
        if (!moveHistory.empty()) {
            moveHistory.pop_back();
            moveCount--;
        }
    }
    
    bool isValidMove(int row, int col) const {
        return row >= 0 && row < BOARD_SIZE && 
               col >= 0 && col < BOARD_SIZE && 
               board[row][col] == Stone::EMPTY;
    }
    
    bool isFull() const {
        return moveCount >= BOARD_SIZE * BOARD_SIZE;
    }
    
    std::vector<Position> getEmptyPositions() const {
        std::vector<Position> empty;
        for (int r = 0; r < BOARD_SIZE; r++) {
            for (int c = 0; c < BOARD_SIZE; c++) {
                if (board[r][c] == Stone::EMPTY) {
                    empty.emplace_back(r, c);
                }
            }
        }
        return empty;
    }
    
    std::vector<Position> getRelevantMoves(int range = 2) const {
        std::vector<Position> moves;
        std::vector<std::vector<bool>> considered(BOARD_SIZE, std::vector<bool>(BOARD_SIZE, false));
        
        // If board is empty, start from center
        if (moveCount == 0) {
            moves.emplace_back(BOARD_SIZE / 2, BOARD_SIZE / 2);
            return moves;
        }
        
        // Get positions near existing stones
        for (int r = 0; r < BOARD_SIZE; r++) {
            for (int c = 0; c < BOARD_SIZE; c++) {
                if (board[r][c] != Stone::EMPTY) {
                    for (int dr = -range; dr <= range; dr++) {
                        for (int dc = -range; dc <= range; dc++) {
                            int nr = r + dr;
                            int nc = c + dc;
                            if (isValidMove(nr, nc) && !considered[nr][nc]) {
                                moves.emplace_back(nr, nc);
                                considered[nr][nc] = true;
                            }
                        }
                    }
                }
            }
        }
        
        return moves;
    }
    
    GameStatus checkWin() const {
        // Check for five in a row
        for (int r = 0; r < BOARD_SIZE; r++) {
            for (int c = 0; c < BOARD_SIZE; c++) {
                Stone stone = board[r][c];
                if (stone == Stone::EMPTY) continue;
                
                for (const auto& [dr, dc] : DIRECTIONS) {
                    int count = 1;
                    
                    // Check positive direction
                    for (int i = 1; i < WIN_LENGTH; i++) {
                        int nr = r + dr * i;
                        int nc = c + dc * i;
                        if (getStone(nr, nc) != stone) break;
                        count++;
                    }
                    
                    if (count >= WIN_LENGTH) {
                        return (stone == Stone::BLACK) ? GameStatus::BLACK_WIN : GameStatus::WHITE_WIN;
                    }
                }
            }
        }
        
        if (isFull()) return GameStatus::DRAW;
        return GameStatus::ONGOING;
    }
    
    void display() const {
        std::cout << "\n  ";
        for (int i = 0; i < BOARD_SIZE; i++) {
            std::cout << std::setw(2) << i << " ";
        }
        std::cout << "\n";
        
        for (int r = 0; r < BOARD_SIZE; r++) {
            std::cout << std::setw(2) << r << " ";
            for (int c = 0; c < BOARD_SIZE; c++) {
                char symbol = '.';
                if (board[r][c] == Stone::BLACK) symbol = 'X';
                else if (board[r][c] == Stone::WHITE) symbol = 'O';
                std::cout << symbol << "  ";
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }
    
    const std::vector<Position>& getMoveHistory() const { return moveHistory; }
};

class PatternEvaluator {
private:
    struct LinePattern {
        int consecutive;
        int openEnds;
        int gaps;
    };
    
    static LinePattern analyzeLine(const Board& board, Position start, 
                                   int dr, int dc, Stone stone) {
        LinePattern pattern = {1, 0, 0};
        
        // Check both directions
        for (int dir = -1; dir <= 1; dir += 2) {
            int count = 0;
            bool blocked = false;
            int gapCount = 0;
            
            for (int i = 1; i < WIN_LENGTH; i++) {
                int r = start.row + dr * i * dir;
                int c = start.col + dc * i * dir;
                
                Stone current = board.getStone(r, c);
                
                if (current == stone) {
                    count++;
                    if (gapCount > 0) pattern.gaps++;
                    gapCount = 0;
                } else if (current == Stone::EMPTY) {
                    if (count > 0 && gapCount == 0) {
                        gapCount++;
                    } else {
                        if (!blocked) pattern.openEnds++;
                        break;
                    }
                } else {
                    blocked = true;
                    break;
                }
            }
            
            pattern.consecutive += count;
        }
        
        return pattern;
    }
    
public:
    static int evaluatePosition(const Board& board, Stone stone) {
        int score = 0;
        Stone opponent = (stone == Stone::BLACK) ? Stone::WHITE : Stone::BLACK;
        
        // Check all positions
        for (int r = 0; r < BOARD_SIZE; r++) {
            for (int c = 0; c < BOARD_SIZE; c++) {
                Stone current = board.getStone(r, c);
                if (current == Stone::EMPTY) continue;
                
                Position pos(r, c);
                bool isMyStone = (current == stone);
                int multiplier = isMyStone ? 1 : -1;
                
                // Analyze patterns in all directions
                for (const auto& [dr, dc] : DIRECTIONS) {
                    // Skip if we've already counted this line from another stone
                    if (dr == 0 || (dr == 1 && dc >= 0)) {
                        LinePattern pattern = analyzeLine(board, pos, dr, dc, current);
                        score += multiplier * getPatternScore(pattern);
                    }
                }
                
                // Position value (center is more valuable)
                int centerDist = std::abs(r - BOARD_SIZE/2) + std::abs(c - BOARD_SIZE/2);
                score += multiplier * (BOARD_SIZE - centerDist);
            }
        }
        
        return score;
    }
    
    static int getPatternScore(const LinePattern& pattern) {
        if (pattern.consecutive >= 5) return PatternScore::FIVE;
        if (pattern.consecutive == 4) {
            if (pattern.openEnds == 2) return PatternScore::OPEN_FOUR;
            if (pattern.openEnds == 1) return PatternScore::BLOCKED_FOUR;
        }
        if (pattern.consecutive == 3) {
            if (pattern.openEnds == 2) return PatternScore::OPEN_THREE;
            if (pattern.openEnds == 1) return PatternScore::BLOCKED_THREE;
        }
        if (pattern.consecutive == 2) {
            if (pattern.openEnds == 2) return PatternScore::OPEN_TWO;
            if (pattern.openEnds == 1) return PatternScore::BLOCKED_TWO;
        }
        if (pattern.consecutive == 1 && pattern.openEnds > 0) return PatternScore::ONE;
        return 0;
    }
    
    static bool isThreat(const Board& board, Position pos, Stone stone) {
        // Check if placing a stone at pos creates a significant threat
        Board tempBoard = board;
        tempBoard.placeStone(pos.row, pos.col, stone);
        
        for (const auto& [dr, dc] : DIRECTIONS) {
            LinePattern pattern = analyzeLine(tempBoard, pos, dr, dc, stone);
            if (pattern.consecutive >= 4 || 
                (pattern.consecutive == 3 && pattern.openEnds == 2)) {
                return true;
            }
        }
        
        return false;
    }
};

class GomokuAI {
private:
    Stone myStone;
    Stone opponentStone;
    int maxDepth;
    std::mt19937 rng;
    
    struct MoveScore {
        Position move;
        int score;
        MoveScore(Position m, int s) : move(m), score(s) {}
    };
    
    int minimax(Board& board, int depth, int alpha, int beta, bool isMaximizing) {
        GameStatus status = board.checkWin();
        
        // Terminal node evaluation
        if (status != GameStatus::ONGOING) {
            if (status == GameStatus::DRAW) return 0;
            bool iWon = (status == GameStatus::BLACK_WIN && myStone == Stone::BLACK) ||
                       (status == GameStatus::WHITE_WIN && myStone == Stone::WHITE);
            return iWon ? WIN_SCORE - depth : -WIN_SCORE + depth;
        }
        
        if (depth >= maxDepth) {
            return PatternEvaluator::evaluatePosition(board, myStone);
        }
        
        std::vector<Position> moves = board.getRelevantMoves();
        if (moves.empty()) return 0;
        
        // Move ordering for better pruning
        orderMoves(board, moves, isMaximizing ? myStone : opponentStone);
        
        if (isMaximizing) {
            int maxEval = -INFINITY_SCORE;
            for (const auto& move : moves) {
                board.placeStone(move.row, move.col, myStone);
                int eval = minimax(board, depth + 1, alpha, beta, false);
                board.removeStone(move.row, move.col);
                
                maxEval = std::max(maxEval, eval);
                alpha = std::max(alpha, eval);
                if (beta <= alpha) break; // Beta pruning
            }
            return maxEval;
        } else {
            int minEval = INFINITY_SCORE;
            for (const auto& move : moves) {
                board.placeStone(move.row, move.col, opponentStone);
                int eval = minimax(board, depth + 1, alpha, beta, true);
                board.removeStone(move.row, move.col);
                
                minEval = std::min(minEval, eval);
                beta = std::min(beta, eval);
                if (beta <= alpha) break; // Alpha pruning
            }
            return minEval;
        }
    }
    
    void orderMoves(Board& board, std::vector<Position>& moves, Stone stone) {
        std::vector<MoveScore> scoredMoves;
        
        for (const auto& move : moves) {
            int score = 0;
            
            // Check for immediate win
            board.placeStone(move.row, move.col, stone);
            if (board.checkWin() != GameStatus::ONGOING) {
                score = INFINITY_SCORE;
            } else {
                // Quick evaluation
                score = PatternEvaluator::evaluatePosition(board, stone);
                
                // Check if it blocks opponent's threat
                Stone opponent = (stone == Stone::BLACK) ? Stone::WHITE : Stone::BLACK;
                if (PatternEvaluator::isThreat(board, move, opponent)) {
                    score += 5000;
                }
            }
            board.removeStone(move.row, move.col);
            
            scoredMoves.emplace_back(move, score);
        }
        
        // Sort moves by score (descending)
        std::sort(scoredMoves.begin(), scoredMoves.end(),
                 [](const MoveScore& a, const MoveScore& b) {
                     return a.score > b.score;
                 });
        
        moves.clear();
        for (const auto& ms : scoredMoves) {
            moves.push_back(ms.move);
            if (moves.size() >= 10) break; // Limit branching factor
        }
    }
    
public:
    GomokuAI(Stone stone, int depth = MAX_DEPTH) 
        : myStone(stone), 
          opponentStone(stone == Stone::BLACK ? Stone::WHITE : Stone::BLACK),
          maxDepth(depth),
          rng(std::chrono::steady_clock::now().time_since_epoch().count()) {}
    
    Position getBestMove(Board& board) {
        auto startTime = std::chrono::steady_clock::now();
        
        std::vector<Position> moves = board.getRelevantMoves();
        if (moves.empty()) {
            return Position(BOARD_SIZE / 2, BOARD_SIZE / 2);
        }
        
        // Check for immediate win or block
        for (const auto& move : moves) {
            // Check for win
            board.placeStone(move.row, move.col, myStone);
            if (board.checkWin() != GameStatus::ONGOING) {
                board.removeStone(move.row, move.col);
                return move;
            }
            board.removeStone(move.row, move.col);
            
            // Check for blocking opponent's win
            board.placeStone(move.row, move.col, opponentStone);
            if (board.checkWin() != GameStatus::ONGOING) {
                board.removeStone(move.row, move.col);
                return move;
            }
            board.removeStone(move.row, move.col);
        }
        
        // Use minimax for best move
        Position bestMove = moves[0];
        int bestScore = -INFINITY_SCORE;
        
        orderMoves(board, moves, myStone);
        
        for (const auto& move : moves) {
            board.placeStone(move.row, move.col, myStone);
            int score = minimax(board, 1, -INFINITY_SCORE, INFINITY_SCORE, false);
            board.removeStone(move.row, move.col);
            
            // Add small random factor for variety
            score += (rng() % 10) - 5;
            
            if (score > bestScore) {
                bestScore = score;
                bestMove = move;
            }
        }
        
        auto endTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        std::cout << "AI (" << (myStone == Stone::BLACK ? "Black" : "White") 
                  << ") thinks for " << duration.count() << "ms, "
                  << "score: " << bestScore << std::endl;
        
        return bestMove;
    }
};

class Game {
private:
    Board board;
    std::unique_ptr<GomokuAI> blackAI;
    std::unique_ptr<GomokuAI> whiteAI;
    GameStatus status;
    int turnCount;
    
public:
    Game(int blackDepth = 6, int whiteDepth = 6) 
        : status(GameStatus::ONGOING), turnCount(0) {
        blackAI = std::make_unique<GomokuAI>(Stone::BLACK, blackDepth);
        whiteAI = std::make_unique<GomokuAI>(Stone::WHITE, whiteDepth);
    }
    
    void play() {
        std::cout << "=== GOMOKU AI vs AI ===" << std::endl;
        std::cout << "Black (X) vs White (O)" << std::endl;
        std::cout << "Board size: " << BOARD_SIZE << "x" << BOARD_SIZE << std::endl;
        std::cout << "First to get 5 in a row wins!\n" << std::endl;
        
        board.display();
        
        while (status == GameStatus::ONGOING) {
            turnCount++;
            Stone currentStone = (turnCount % 2 == 1) ? Stone::BLACK : Stone::WHITE;
            
            std::cout << "Turn " << turnCount << " - " 
                     << (currentStone == Stone::BLACK ? "Black (X)" : "White (O)") 
                     << " is thinking..." << std::endl;
            
            Position move;
            if (currentStone == Stone::BLACK) {
                move = blackAI->getBestMove(board);
            } else {
                move = whiteAI->getBestMove(board);
            }
            
            board.placeStone(move.row, move.col, currentStone);
            std::cout << "Placed at (" << move.row << ", " << move.col << ")" << std::endl;
            
            board.display();
            
            status = board.checkWin();
            
            // Add a small delay for visualization
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        
        // Game over
        std::cout << "\n=== GAME OVER ===" << std::endl;
        switch (status) {
            case GameStatus::BLACK_WIN:
                std::cout << "Black (X) wins!" << std::endl;
                break;
            case GameStatus::WHITE_WIN:
                std::cout << "White (O) wins!" << std::endl;
                break;
            case GameStatus::DRAW:
                std::cout << "It's a draw!" << std::endl;
                break;
            default:
                break;
        }
        std::cout << "Total moves: " << turnCount << std::endl;
        
        // Show winning sequence if there's a winner
        if (status != GameStatus::DRAW && status != GameStatus::ONGOING) {
            std::cout << "\nMove sequence:" << std::endl;
            int moveNum = 1;
            for (const auto& pos : board.getMoveHistory()) {
                std::cout << moveNum++ << ". (" << pos.row << ", " << pos.col << ") ";
                if (moveNum % 10 == 1) std::cout << "\n";
            }
            std::cout << std::endl;
        }
    }
    
    void playMultipleGames(int numGames) {
        int blackWins = 0, whiteWins = 0, draws = 0;
        
        for (int i = 0; i < numGames; i++) {
            std::cout << "\n=== Game " << (i + 1) << " of " << numGames << " ===" << std::endl;
            
            // Reset for new game
            board = Board();
            status = GameStatus::ONGOING;
            turnCount = 0;
            
            // Play silently
            while (status == GameStatus::ONGOING) {
                turnCount++;
                Stone currentStone = (turnCount % 2 == 1) ? Stone::BLACK : Stone::WHITE;
                
                Position move;
                if (currentStone == Stone::BLACK) {
                    move = blackAI->getBestMove(board);
                } else {
                    move = whiteAI->getBestMove(board);
                }
                
                board.placeStone(move.row, move.col, currentStone);
                status = board.checkWin();
            }
            
            // Record result
            switch (status) {
                case GameStatus::BLACK_WIN:
                    blackWins++;
                    std::cout << "Black wins in " << turnCount << " moves" << std::endl;
                    break;
                case GameStatus::WHITE_WIN:
                    whiteWins++;
                    std::cout << "White wins in " << turnCount << " moves" << std::endl;
                    break;
                case GameStatus::DRAW:
                    draws++;
                    std::cout << "Draw after " << turnCount << " moves" << std::endl;
                    break;
                default:
                    break;
            }
        }
        
        // Statistics
        std::cout << "\n=== STATISTICS ===" << std::endl;
        std::cout << "Black wins: " << blackWins << " (" 
                  << (100.0 * blackWins / numGames) << "%)" << std::endl;
        std::cout << "White wins: " << whiteWins << " (" 
                  << (100.0 * whiteWins / numGames) << "%)" << std::endl;
        std::cout << "Draws: " << draws << " (" 
                  << (100.0 * draws / numGames) << "%)" << std::endl;
    }
};

int main() {
    try {
        // Single game with visualization
        Game game(6, 6);  // Both AIs use depth 6
        game.play();
        
        // Uncomment for multiple games statistics
        // Game tournament(5, 5);
        // tournament.playMultipleGames(10);
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
