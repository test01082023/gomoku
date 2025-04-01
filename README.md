# Gomoku Game

Gomoku, hay còn gọi là cờ Caro, là một trò chơi hấp dẫn dành cho 2 người chơi. Dự án này cung cấp một phiên bản Gomoku với khả năng chơi với AI và có tính năng lưu và tải lại trận đấu.

---

## 🧠 Pseudocode

1. **Khởi tạo trò chơi**:
    - Tạo bàn cờ kích thước 15x15.
    - Đặt người chơi hiện tại là 'X'.
    - Xác định đối thủ là AI hay con người.
    - Đặt `game_over = False`.

2. **Trong khi trò chơi chưa kết thúc**:
    - Hiển thị bàn cờ.
    - Nếu người chơi hiện tại là AI:
        - Tự động tạo nước đi.
      Ngược lại:
        - Nhập nước đi từ người dùng.
    - Xác minh và thực hiện nước đi.
    - Kiểm tra thắng/thua.
    - Kiểm tra hòa.
    - Nếu trò chơi tiếp tục, chuyển lượt người chơi.
    - Tùy chọn lưu lại trận đấu.

3. **Khi trò chơi kết thúc**:
    - Hiển thị người chiến thắng hoặc hòa.

---

## 🐍 Python Code

Dự án được xây dựng hoàn toàn bằng Python với các tính năng chơi vòng lặp, AI cơ bản, và lưu/tải trận đấu. Xem mã nguồn đầy đủ trong tệp chính `gomoku.py`.

---

## 🔁 Flowchart

Dưới đây là sơ đồ luồng hoạt động của trò chơi:

```mermaid
graph TD
  Start[Start Game] --> Init[Initialize Game State]
  Init --> Opponent[Choose Opponent Type]
  Opponent --> Loop[Main Game Loop]
  Loop --> ShowBoard[Display Board]
  ShowBoard --> PlayerType{Is Current Player AI?}
  PlayerType -- Yes --> AIMove[Generate AI Move]
  PlayerType -- No --> GetMove[Prompt Human Move]
  GetMove --> SaveCheck{Save Game?}
  SaveCheck -- Yes --> SaveGame[Save to File] --> Loop
  SaveCheck -- No --> Validate
  AIMove --> Validate[Validate and Apply Move]
  Validate -- Invalid --> Loop
  Validate -- Valid --> CheckWin[Check Win]
  CheckWin -- Yes --> EndWin[Declare Winner]
  CheckWin -- No --> CheckDraw[Check Draw]
  CheckDraw -- Yes --> EndDraw[Declare Draw]
  CheckDraw -- No --> Switch[Switch Player] --> Loop
  EndWin & EndDraw --> End[Game Over]
