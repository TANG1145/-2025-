#include "SAT.hpp"

// X数独交互界面
void Sudoku() {
  system("cls");
  PrintMenu_X();
  int num;                            // 提示数的个数
  bool isFixed[SIZE + 1][SIZE + 1];   // 记录是否为提示数字
  int board[SIZE + 1][SIZE + 1];      // 生成的初始数独
  int newBoard[SIZE + 1][SIZE + 1];   // 用来玩的数独
  int newBoard2[SIZE + 1][SIZE + 1];  // 保存答案的数独
  bool value[SIZE * SIZE * SIZE + 1]; // 记录DPLL的结果
  for (int i = 1; i <= SIZE * SIZE * SIZE; i++)
    value[i] = FALSE;
  int op = 1;   // 操作
  int flag = 0; // 是否生成数独
  while (op) {
    printf("\n|********************************************|\n");
    printf("|--------Please Choose Your Operation--------|\n");
    printf("|********************************************|\n\n");
    printf("               Your choice: ");
    scanf("%d", &op);
    system("cls");
    PrintMenu_X();
    switch (op) {
    case 1: {
      printf(" Please enter the number of prompts(>=17): ");
      scanf("%d", &num);
      while (num < 17 || num > 81) // 提示数的个数必须大于等于25小于等于81
      {
        printf(" Invalid input, please enter again: ");
        scanf("%d", &num);
      }
      if (Generate_Sudoku(board, newBoard, newBoard2, isFixed, num, value)) {
        printf(" Generate successfully.\n");
        flag = 1; // 生成成功
      } else
        printf(" Generate failed.\n");
      break;
    }
    case 2: {
      if (flag) {
        Play_Sudoku(newBoard, isFixed);
        PrintMenu_X(); // 每次玩完跳转回来重新打印菜单
      } else
        printf(" Please generate the Sudoku first.\n");
      break;
    }
    case 3: {
      if (flag) {
        printf(" Original Sudoku:\n");
        Print_Sudoku(board); // 打印原始数独
        printf("\n");
        if (Slove(newBoard2, value)) // 求解数独
        {
          printf(" Reference answer:\n");
          Print_Sudoku(newBoard2); // 打印答案
        } else
          printf(" No answer.\n"); // 无解
      } else
        printf(" Please generate the Sudoku first.\n");
      break;
    }
    case 0: {
      system("cls"); // 退出时清屏
      break;
    }
    default: {
      printf(" Invalid input.\n");
      break;
    }
    }
  }
}

// 打印X数独菜单
void PrintMenu_X() {
  printf("|**************Menu for X-Sudoku(Percent Sudoku)***************|\n");
  printf("|--------------------------------------------------------------|\n");
  printf("|              1. Generate a X-Sudoku                         |\n");
  printf("|              2. Play the X-Sudoku                           |\n");
  printf("|              3. Reference answer                            |\n");
  printf("|                    0. EXIT                                  |\n");
  printf(
      "|**************************************************************|\n\n");
}

// 生成数独
status Generate_Sudoku(int board[SIZE + 1][SIZE + 1],
                       int newBoard[SIZE + 1][SIZE + 1],
                       int newBoard2[SIZE + 1][SIZE + 1],
                       bool isFixed[SIZE + 1][SIZE + 1], int num,
                       bool value[SIZE * SIZE * SIZE + 1]) {
  char name[100] = "Sudoku.cnf"; // 文件名
START:
  srand(time(NULL));
  // 初始化棋盘
  for (int i = 1; i <= SIZE; i++)
    for (int j = 1; j <= SIZE; j++) {
      board[i][j] = 0;
      newBoard[i][j] = 0;
      newBoard2[i][j] = 0;
      isFixed[i][j] = FALSE; // 初始化为非固定数字
    }

  // 先填充两个窗口
  int windows[2][2] = {{2, 2}, {6, 6}};
  for (int w = 0; w < 2; w++) {
    int startRow = windows[w][0];
    int startCol = windows[w][1];
    Fill_Box(board, newBoard, newBoard2, startRow, startCol);
  }

  WriteToFile(board, 27, name); // 将数独约束条件写入文件
  CNF p = (CNF)malloc(sizeof(cnfNode));
  p->root = NULL;
  if (ReadFile(p, name) != OK) {
    printf("Failed to read CNF file\n");
    return ERROR;
  }
  for (int i = 1; i <= SIZE * SIZE * SIZE; i++)
    value[i] = FALSE;
  if (DPLL(p, value, 3) == ERROR) { // 求解数独
    printf("DPLL failed, regenerating...\n");
    DestroyCnf(p->root);
    free(p);
    goto START;
  }

  // 将DPLL的结果填入数独
  for (int i = 1; i <= SIZE * SIZE * SIZE; i++) {
    if (value[i] == TRUE) {
      int row = (i - 1) / (SIZE * SIZE) + 1;
      int col = (i - 1) / SIZE % SIZE + 1;
      int v = (i - 1) % SIZE + 1;
      board[row][col] = v;
      newBoard[row][col] = v;
      newBoard2[row][col] = v;
    }
  }

  // 验证数独是否满足所有约束
  if (!Validate_Sudoku(board)) {
    printf(
        "Generated Sudoku doesn't satisfy all constraints, regenerating...\n");
    DestroyCnf(p->root);
    free(p);
    goto START;
  }

  // 标记所有格子为固定（提示数）
  for (int i = 1; i <= SIZE; i++) {
    for (int j = 1; j <= SIZE; j++) {
      isFixed[i][j] = TRUE;
    }
  }

  // 挖洞,剩下num个提示数
  int remove = 81 - num;
  int attempts = 0;
  int maxAttempts = 1000; // 防止无限循环

  while (remove > 0 && attempts < maxAttempts) {
    int row = rand() % SIZE + 1;
    int col = rand() % SIZE + 1;

    // 检查是否在窗口中
    bool inWindow = false;
    for (int w = 0; w < 2; w++) {
      int startRow = windows[w][0];
      int startCol = windows[w][1];
      if (row >= startRow && row < startRow + 3 && col >= startCol &&
          col < startCol + 3) {
        inWindow = true;
        break;
      }
    }

    // 确保每个窗口至少保留3个数字
    if (inWindow) {
      int windowCount = 0;
      for (int w = 0; w < 2; w++) {
        int startRow = windows[w][0];
        int startCol = windows[w][1];
        if (row >= startRow && row < startRow + 3 && col >= startCol &&
            col < startCol + 3) {
          // 计算当前窗口中的提示数数量
          windowCount = 0;
          for (int i = startRow; i < startRow + 3; i++) {
            for (int j = startCol; j < startCol + 3; j++) {
              if (isFixed[i][j])
                windowCount++;
            }
          }
          break;
        }
      }

      // 如果窗口中的提示数已经少于3个，不再挖洞
      if (windowCount <= 3) {
        attempts++;
        continue;
      }
    }

    // 挖洞
    if (isFixed[row][col]) {
      int temp = board[row][col];
      board[row][col] = 0;
      newBoard[row][col] = 0;
      isFixed[row][col] = FALSE;

      // 检查挖洞后是否仍有唯一解
      if (!HasUniqueSolution(board, isFixed, newBoard2)) {
        // 如果没有唯一解，恢复这个洞
        board[row][col] = temp;
        newBoard[row][col] = temp;
        isFixed[row][col] = TRUE;
      } else {
        remove--;
      }
    }

    attempts++;
  }

  // 如果还有洞需要挖，但已达到最大尝试次数
  if (remove > 0) {
    printf(" Warning: Could not remove all requested holes while maintaining "
           "window constraints.\n");
  }

  DestroyCnf(p->root);
  free(p);
  return OK;
}

// 验证数独是否满足所有约束
status Validate_Sudoku(int board[SIZE + 1][SIZE + 1]) {
  // 检查行
  for (int i = 1; i <= SIZE; i++) {
    bool used[SIZE + 1] = {false};
    for (int j = 1; j <= SIZE; j++) {
      if (board[i][j] != 0) {
        if (used[board[i][j]])
          return FALSE;
        used[board[i][j]] = true;
      }
    }
  }

  // 检查列
  for (int j = 1; j <= SIZE; j++) {
    bool used[SIZE + 1] = {false};
    for (int i = 1; i <= SIZE; i++) {
      if (board[i][j] != 0) {
        if (used[board[i][j]])
          return FALSE;
        used[board[i][j]] = true;
      }
    }
  }

  // 检查3x3宫格
  for (int boxRow = 1; boxRow <= SIZE; boxRow += 3) {
    for (int boxCol = 1; boxCol <= SIZE; boxCol += 3) {
      bool used[SIZE + 1] = {false};
      for (int i = boxRow; i < boxRow + 3; i++) {
        for (int j = boxCol; j < boxCol + 3; j++) {
          if (board[i][j] != 0) {
            if (used[board[i][j]])
              return FALSE;
            used[board[i][j]] = true;
          }
        }
      }
    }
  }

  // 检查副对角线
  bool usedDiag[SIZE + 1] = {false};
  for (int i = 1; i <= SIZE; i++) {
    int j = SIZE + 1 - i;
    if (board[i][j] != 0) {
      if (usedDiag[board[i][j]])
        return FALSE;
      usedDiag[board[i][j]] = true;
    }
  }

  // 检查两个窗口
  int windows[2][2] = {{2, 2}, {6, 6}};
  for (int w = 0; w < 2; w++) {
    int startRow = windows[w][0];
    int startCol = windows[w][1];
    bool used[SIZE + 1] = {false};

    for (int i = startRow; i < startRow + 3; i++) {
      for (int j = startCol; j < startCol + 3; j++) {
        if (board[i][j] != 0) {
          if (used[board[i][j]])
            return FALSE;
          used[board[i][j]] = true;
        }
      }
    }
  }

  return TRUE;
}

// // 检查数独是否有唯一解
// status HasUniqueSolution(int board[SIZE + 1][SIZE + 1],
//                          bool isFixed[SIZE + 1][SIZE + 1],
//                          int solution[SIZE + 1][SIZE + 1]) {
//   // 创建临时数独副本
//   int tempBoard[SIZE + 1][SIZE + 1];
//   for (int i = 1; i <= SIZE; i++) {
//     for (int j = 1; j <= SIZE; j++) {
//       tempBoard[i][j] = board[i][j];
//     }
//   }

//   // 使用DPLL求解数独
//   char name[100] = "temp_Sudoku.cnf";
//   WriteToFile(tempBoard, 0, name);

//   CNF p = (CNF)malloc(sizeof(cnfNode));
//   p->root = NULL;
//   if (ReadFile(p, name) != OK) {
//     printf("Failed to read temporary CNF file\n");
//     return FALSE;
//   }

//   bool value[SIZE * SIZE * SIZE + 1];
//   for (int i = 1; i <= SIZE * SIZE * SIZE; i++)
//     value[i] = FALSE;

//   status result = DPLL(p, value, 3);

//   if (result == OK) {
//     // 将解填入solution数组
//     for (int i = 1; i <= SIZE * SIZE * SIZE; i++) {
//       if (value[i] == TRUE) {
//         int row = (i - 1) / (SIZE * SIZE) + 1;
//         int col = (i - 1) / SIZE % SIZE + 1;
//         int v = (i - 1) % SIZE + 1;
//         solution[row][col] = v;
//       }
//     }
//   }

//   DestroyCnf(p->root);
//   free(p);
//   remove(name); // 删除临时文件

//   return result == OK;
// }
// [%-Sudoku.cpp]
// 请用这个新版本替换原有的 HasUniqueSolution 函数

status HasUniqueSolution(int board[SIZE + 1][SIZE + 1],
                         bool isFixed[SIZE + 1][SIZE + 1],
                         int solution[SIZE + 1][SIZE + 1]) {
    char name[100] = "temp_Sudoku_uniqueness_check.cnf";
    bool firstSolution[SIZE * SIZE * SIZE + 1];
    status finalResult = FALSE; // 最终返回结果，默认为非唯一解

    // ==================== 阶段一：第一次求解，寻找第一个解 ====================
    CNF cnf1 = (CNF)malloc(sizeof(cnfNode));
    cnf1->root = NULL;
    bool value1[SIZE * SIZE * SIZE + 1];
    for (int i = 1; i <= SIZE * SIZE * SIZE; i++) value1[i] = FALSE;

    WriteToFile(board, 0, name); // 将当前棋盘转化为CNF
    if (ReadFile(cnf1, name) != OK) {
        printf("Error: Failed to read temp CNF for first solve.\n");
        free(cnf1);
        remove(name);
        return FALSE; // 文件读取失败，无法验证
    }

    status result1 = DPLL(cnf1, value1, 3); // 调用DPLL求解

    if (result1 == ERROR) {
        // 如果第一次求解就失败了，说明谜题根本无解，自然谈不上唯一解
        DestroyCnf(cnf1->root);
        free(cnf1);
        remove(name);
        return FALSE;
    }

    // 保存第一个解
    for (int i = 1; i <= SIZE * SIZE * SIZE; i++) {
        firstSolution[i] = value1[i];
    }
    
    // 如果找到了解，将其解码并存入solution输出参数
    for (int i = 1; i <= SIZE * SIZE * SIZE; i++) {
        if (firstSolution[i] == TRUE) {
            int row = (i - 1) / (SIZE * SIZE) + 1;
            int col = (i - 1) / SIZE % SIZE + 1;
            int v = (i - 1) % SIZE + 1;
            solution[row][col] = v;
        }
    }

    DestroyCnf(cnf1->root); // 释放第一次求解的CNF结构
    free(cnf1);


    // ==================== 阶段二：增加“阻塞子句”，尝试寻找第二个解 ====================
    // 构建一个“阻塞子句”，这个子句是第一个解的逻辑否定
    clauseList blockingClause = (clauseList)malloc(sizeof(clauseNode));
    blockingClause->head = NULL;
    blockingClause->next = NULL;
    literalList lastLiteral = NULL;

    for (int i = 1; i <= SIZE * SIZE * SIZE; i++) {
        literalList newLiteral = (literalList)malloc(sizeof(literalNode));
        // 如果第一解中变量i为真，阻塞子句中就加入-i；反之则加入i
        newLiteral->literal = (firstSolution[i] == TRUE) ? -i : i;
        newLiteral->next = NULL;
        if (blockingClause->head == NULL) {
            blockingClause->head = newLiteral;
        } else {
            lastLiteral->next = newLiteral;
        }
        lastLiteral = newLiteral;
    }

    // 重新读取原始CNF问题，并加入阻塞子句
    CNF cnf2 = (CNF)malloc(sizeof(cnfNode));
    cnf2->root = NULL;
    bool value2[SIZE * SIZE * SIZE + 1]; // 第二次求解的结果数组（虽然内容不重要）

    if (ReadFile(cnf2, name) != OK) {
        printf("Error: Failed to read temp CNF for second solve.\n");
        DestroyClause(blockingClause); // 清理创建的阻塞子句
        free(cnf2);
        remove(name);
        return FALSE; // 文件读取失败
    }

    // 将阻塞子句添加到CNF公式的头部
    blockingClause->next = cnf2->root;
    cnf2->root = blockingClause;
    cnf2->clauseCount++;

    status result2 = DPLL(cnf2, value2, 3); // 进行第二次求解

    if (result2 == ERROR) {
        // 如果在排除了第一个解之后，问题变为“不可满足”，
        // 这恰好证明了第一个解是唯一的。
        finalResult = TRUE;
    } else {
        // 如果还能找到另一个解，说明解不唯一。
        finalResult = FALSE;
    }

    DestroyCnf(cnf2->root); // 释放第二次求解的CNF结构 (这会同时释放阻塞子句)
    free(cnf2);
    remove(name); // 删除临时文件

    return finalResult;
}
// 判断board[row][col]是否可以填入v

status Is_Valid(int board[SIZE + 1][SIZE + 1], int row, int col, int v) {
  // 检查行
  for (int j = 1; j <= SIZE; j++) {
    if (j != col && board[row][j] == v) {
      return FALSE;
    }
  }

  // 检查列
  for (int i = 1; i <= SIZE; i++) {
    if (i != row && board[i][col] == v) {
      return FALSE;
    }
  }

  // 检查3x3宫格
  int startRow = ((row - 1) / 3) * 3 + 1;
  int startCol = ((col - 1) / 3) * 3 + 1;
  for (int i = startRow; i < startRow + 3; i++) {
    for (int j = startCol; j < startCol + 3; j++) {
      if ((i != row || j != col) && board[i][j] == v) {
        return FALSE;
      }
    }
  }

  // 检查副对角线
  if (row + col == SIZE + 1) {
    for (int i = 1; i <= SIZE; i++) {
      int j = SIZE + 1 - i;
      if (i != row && board[i][j] == v) {
        return FALSE;
      }
    }
  }

  // 检查两个窗口
  int windows[2][2] = {{2, 2}, {6, 6}};
  for (int w = 0; w < 2; w++) {
    int startRow = windows[w][0];
    int startCol = windows[w][1];

    // 检查当前单元格是否在这个窗口中
    if (row >= startRow && row < startRow + 3 && col >= startCol &&
        col < startCol + 3) {
      // 检查窗口内是否有重复数字
      for (int i = startRow; i < startRow + 3; i++) {
        for (int j = startCol; j < startCol + 3; j++) {
          if ((i != row || j != col) && board[i][j] == v) {
            return FALSE;
          }
        }
      }
      break; // 找到所属窗口后跳出循环
    }
  }

  return TRUE;
}

// 打印数独

void Print_Sudoku(int board[SIZE + 1][SIZE + 1]) {
  printf("   ");
  for (int j = 1; j <= SIZE; j++) {
    printf("%2d ", j);
    if (j % 3 == 0 && j != SIZE)
      printf("| ");
  }
  printf("\n");
  printf("   ");
  for (int j = 1; j <= SIZE; j++) {
    printf("---");
    if (j % 3 == 0 && j != SIZE)
      printf("+");
  }
  printf("\n");

  for (int i = 1; i <= SIZE; i++) {
    printf("%2d|", i);
    for (int j = 1; j <= SIZE; j++) {
      if (board[i][j] == 0) // 未填入
        printf(" . ");
      else // 已填入
        printf("%2d ", board[i][j]);
      if (j % 3 == 0 && j != SIZE) // 每3列打印一个竖线
        printf("| ");
    }
    printf("\n");
    if (i % 3 == 0 && i != SIZE) { // 每3行打印一个横线
      printf("   ");
      for (int j = 1; j <= SIZE; j++) {
        printf("---");
        if (j % 3 == 0 && j != SIZE)
          printf("+");
      }
      printf("\n");
    }
  }
}

// 玩数独的交互界面

void Play_Sudoku(int board[SIZE + 1][SIZE + 1],
                 bool isFixed[SIZE + 1][SIZE + 1]) {
  system("cls");       // 清屏
  Print_Sudoku(board); // 打印初始数独
  printf("\n");
  while (1) {
    int row, col, v;
    printf(" Please enter the row, col and value(0 to EXIT): ");
    scanf("%d", &row);
    if (row == 0) // 退出
    {
      system("cls");
      return;
    }
    scanf("%d%d", &col, &v);
    if (row < 1 || row > SIZE || col < 1 || col > SIZE || v < 1 ||
        v > SIZE) // 输入不合法
    {
      printf(" Invalid input.\n");
      continue;
    }
    if (isFixed[row][col]) // 是提示数
    {
      printf(" This is a fixed number.\n");
      continue;
    }
    if (!Is_Valid(board, row, col, v)) // 不符合数独规则
    {
      printf(" Wrong answer.\n");
      continue;
    } else // 符合数独规则
    {
      board[row][col] = v;
      system("cls");
      Print_Sudoku(board); // 打印新数独
      printf("\n");

      // 检查是否完成
      int complete = 1;
      for (int i = 1; i <= SIZE; i++) {
        for (int j = 1; j <= SIZE; j++) {
          if (board[i][j] == 0) {
            complete = 0;
            break;
          }
        }
        if (!complete)
          break;
      }

      if (complete) {
        if (Validate_Sudoku(board)) {
          printf(" Congratulations! You solved the Sudoku!\n");
        } else {
          printf(" Sorry, your solution is incorrect.\n");
        }
        return;
      }
    }
  }
}

// 将数独约束条件写入文件
status WriteToFile(int board[SIZE + 1][SIZE + 1], int num, char name[]) {
  FILE *fp;
  if (fopen_s(&fp, name, "w")) {
    printf(" Fail!\n");
    return ERROR;
  }

  // 计算总子句数
  int clauseCount = num; // 提示数子句

  // 每个格子必须填入一个数字: 81个子句
  clauseCount += 81;
  // 每个格子不能填入两个数字: C(9,2)*81 = 36*81 = 2916个子句
  clauseCount += 2916;
  // 行约束: 每行必须包含1-9: 9*9 = 81个子句
  clauseCount += 81;
  // 行约束: 每行不能有重复数字: C(9,2)*9*9 = 36*81 = 2916个子句
  clauseCount += 2916;
  // 列约束: 每列必须包含1-9: 9*9 = 81个子句
  clauseCount += 81;
  // 列约束: 每列不能有重复数字: C(9,2)*9*9 = 36*81 = 2916个子句
  clauseCount += 2916;
  // 3x3宫格约束: 每个宫格必须包含1-9: 9*9 = 81个子句
  clauseCount += 81;
  // 3x3宫格约束: 每个宫格不能有重复数字: C(9,2)*9*9 = 36*81 = 2916个子句
  clauseCount += 2916;
  // 副对角线约束: 必须包含1-9: 9个子句
  clauseCount += 9;
  // 副对角线约束: 不能有重复数字: C(9,2) = 36个子句
  clauseCount += 36;
  // 窗口约束: 两个窗口各必须包含1-9: 2*9 = 18个子句
  clauseCount += 18;
  // 窗口约束: 两个窗口各不能有重复数字: 2*C(9,2) = 2*36 = 72个子句
  clauseCount += 72;

  fprintf(fp, "c %s\n", name);
  fprintf(fp, "p cnf 729 %d\n", clauseCount);

  /*提示数约束(写在前面,便于单子句规则进行)*/
  for (int i = 1; i <= SIZE; i++) {
    for (int j = 1; j <= SIZE; j++) {
      if (board[i][j] != 0) {
        fprintf(fp, "%d 0\n",
                (i - 1) * SIZE * SIZE + (j - 1) * SIZE + board[i][j]);
      }
    }
  }

  /*每个格子的约束*/
  // 每个格子必须填入一个数字
  for (int i = 1; i <= SIZE; i++) {
    for (int j = 1; j <= SIZE; j++) {
      for (int k = 1; k <= SIZE; k++) {
        fprintf(fp, "%d ", (i - 1) * SIZE * SIZE + (j - 1) * SIZE + k);
      }
      fprintf(fp, "0\n");
    }
  }
  // 每个格子不能填入两个数字
  for (int i = 1; i <= SIZE; i++) {
    for (int j = 1; j <= SIZE; j++) {
      for (int k = 1; k <= SIZE; k++) {
        for (int l = k + 1; l <= SIZE; l++) {
          fprintf(fp, "%d %d 0\n",
                  -((i - 1) * SIZE * SIZE + (j - 1) * SIZE + k),
                  -((i - 1) * SIZE * SIZE + (j - 1) * SIZE + l));
        }
      }
    }
  }

  /*行约束*/
  // 每一行必须填入1-9
  for (int i = 1; i <= SIZE; i++) {
    for (int k = 1; k <= SIZE; k++) {
      for (int j = 1; j <= SIZE; j++) {
        fprintf(fp, "%d ", (i - 1) * SIZE * SIZE + (j - 1) * SIZE + k);
      }
      fprintf(fp, "0\n");
    }
  }
  // 每一行不能填入两个相同的数字
  for (int i = 1; i <= SIZE; i++) {
    for (int k = 1; k <= SIZE; k++) {
      for (int j = 1; j <= SIZE; j++) {
        for (int l = j + 1; l <= SIZE; l++) {
          fprintf(fp, "%d %d 0\n",
                  -((i - 1) * SIZE * SIZE + (j - 1) * SIZE + k),
                  -((i - 1) * SIZE * SIZE + (l - 1) * SIZE + k));
        }
      }
    }
  }

  /*列约束*/
  // 每一列必须填入1-9
  for (int j = 1; j <= SIZE; j++) {
    for (int k = 1; k <= SIZE; k++) {
      for (int i = 1; i <= SIZE; i++) {
        fprintf(fp, "%d ", (i - 1) * SIZE * SIZE + (j - 1) * SIZE + k);
      }
      fprintf(fp, "0\n");
    }
  }
  // 每一列不能填入两个相同的数字
  for (int j = 1; j <= SIZE; j++) {
    for (int k = 1; k <= SIZE; k++) {
      for (int i = 1; i <= SIZE; i++) {
        for (int l = i + 1; l <= SIZE; l++) {
          fprintf(fp, "%d %d 0\n",
                  -((i - 1) * SIZE * SIZE + (j - 1) * SIZE + k),
                  -((l - 1) * SIZE * SIZE + (j - 1) * SIZE + k));
        }
      }
    }
  }

  /*3x3宫格约束*/
  // 每个3x3宫格必须填入1-9
  for (int boxRow = 1; boxRow <= SIZE; boxRow += 3) {
    for (int boxCol = 1; boxCol <= SIZE; boxCol += 3) {
      for (int k = 1; k <= SIZE; k++) {
        for (int i = boxRow; i < boxRow + 3; i++) {
          for (int j = boxCol; j < boxCol + 3; j++) {
            fprintf(fp, "%d ", (i - 1) * SIZE * SIZE + (j - 1) * SIZE + k);
          }
        }
        fprintf(fp, "0\n");
      }
    }
  }
  // 每个3x3宫格不能填入两个相同的数字
  for (int boxRow = 1; boxRow <= SIZE; boxRow += 3) {
    for (int boxCol = 1; boxCol <= SIZE; boxCol += 3) {
      for (int k = 1; k <= SIZE; k++) {
        for (int i = boxRow; i < boxRow + 3; i++) {
          for (int j = boxCol; j < boxCol + 3; j++) {
            for (int p = i; p < boxRow + 3; p++) {
              for (int q = (p == i ? j + 1 : boxCol); q < boxCol + 3; q++) {
                fprintf(fp, "%d %d 0\n",
                        -((i - 1) * SIZE * SIZE + (j - 1) * SIZE + k),
                        -((p - 1) * SIZE * SIZE + (q - 1) * SIZE + k));
              }
            }
          }
        }
      }
    }
  }

  /*副对角线约束*/
  // 副对角线必须包含1-9
  for (int k = 1; k <= SIZE; k++) {
    for (int i = 1; i <= SIZE; i++) {
      int j = SIZE + 1 - i;
      fprintf(fp, "%d ", (i - 1) * SIZE * SIZE + (j - 1) * SIZE + k);
    }
    fprintf(fp, "0\n");
  }
  // 副对角线不能有重复数字
  for (int k = 1; k <= SIZE; k++) {
    for (int i = 1; i <= SIZE; i++) {
      int j = SIZE + 1 - i;
      for (int p = i + 1; p <= SIZE; p++) {
        int q = SIZE + 1 - p;
        fprintf(fp, "%d %d 0\n", -((i - 1) * SIZE * SIZE + (j - 1) * SIZE + k),
                -((p - 1) * SIZE * SIZE + (q - 1) * SIZE + k));
      }
    }
  }

  /*两个窗口约束*/
  int windows[2][2] = {{2, 2}, {6, 6}};
  for (int w = 0; w < 2; w++) {
    int startRow = windows[w][0];
    int startCol = windows[w][1];

    // 窗口必须包含1-9
    for (int k = 1; k <= SIZE; k++) {
      for (int i = startRow; i < startRow + 3; i++) {
        for (int j = startCol; j < startCol + 3; j++) {
          fprintf(fp, "%d ", (i - 1) * SIZE * SIZE + (j - 1) * SIZE + k);
        }
      }
      fprintf(fp, "0\n");
    }

    // 窗口内不能有重复数字
    for (int k = 1; k <= SIZE; k++) {
      for (int i = startRow; i < startRow + 3; i++) {
        for (int j = startCol; j < startCol + 3; j++) {
          for (int p = i; p < startRow + 3; p++) {
            for (int q = (p == i ? j + 1 : startCol); q < startCol + 3; q++) {
              fprintf(fp, "%d %d 0\n",
                      -((i - 1) * SIZE * SIZE + (j - 1) * SIZE + k),
                      -((p - 1) * SIZE * SIZE + (q - 1) * SIZE + k));
            }
          }
        }
      }
    }
  }

  fclose(fp);
  return OK;
}

// DPLL求解数独

status Slove(int board[SIZE + 1][SIZE + 1],
             bool value[SIZE * SIZE * SIZE + 1]) {
  for (int i = 1; i <= SIZE * SIZE * SIZE; i++) {
    if (value[i] == TRUE) {
      int row = (i - 1) / (SIZE * SIZE) + 1;
      int col = (i - 1) / SIZE % SIZE + 1;
      int v = (i - 1) % SIZE + 1;
      board[row][col] = v;
    }
  }
  return OK;
}

// 填充3x3的宫格
status Fill_Box(int board[SIZE + 1][SIZE + 1], int newBoard[SIZE + 1][SIZE + 1],
                int newBoard2[SIZE + 1][SIZE + 1], int rowStart, int colStart) {
  int numbers[SIZE];
  for (int i = 0; i < SIZE; i++) {
    numbers[i] = i + 1;
  }
  Shuffle(numbers, SIZE);

  int index = 0;
  for (int i = rowStart; i < rowStart + 3; i++) {
    for (int j = colStart; j < colStart + 3; j++) {
      if (board[i][j] == 0) {
        board[i][j] = numbers[index];
        newBoard[i][j] = numbers[index];
        newBoard2[i][j] = numbers[index];
        index++;
      }
    }
  }
  return OK;
}

// 打乱数组顺序
void Shuffle(int arr[], int n) {
  srand(time(NULL)); // 用时间做种子
  // 每次从后面的数中随机选一个数与前面的数交换
  for (int i = n - 1; i > 0; i--) {
    int j = rand() % (i + 1);
    int temp = arr[i];
    arr[i] = arr[j];
    arr[j] = temp;
  }
}