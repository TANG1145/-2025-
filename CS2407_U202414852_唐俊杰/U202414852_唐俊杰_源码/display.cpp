/*----------------------------display----------------------------*/

#include "SAT.hpp"

/*
 @ 函数名称: DisPlay
 @ 函数功能: 交互界面
 @ 返回值: void
 */
void DisPlay() {
  bool *value = NULL; // 存储文字的真值
  CNF cnf = (CNF)malloc(sizeof(cnfNode));
  cnf->root = NULL;
  char fileName[100]; // 文件名
  PrintMenu();        // 打印菜单
  int op = 1;
  while (op) {
    printf("\n|--------------------------------------------|\n");
    printf("|--------Please Choose Your Operation--------|\n");
    printf("|--------------------------------------------|\n\n");
    printf("               Your choice: ");
    scanf("%d", &op);
    system("cls"); // 每次进来清屏
    PrintMenu();   // 打印菜单
    switch (op) {
    case 1: {
      if (cnf->root != NULL) // 如果已经打开了CNF文件
      {
        printf(" The CNF has been read.\n");
        printf(" Do you want to read another? (1/0): ");
        int choice;
        scanf("%d", &choice);
        if (choice == 0)
          break;
        else // 重新读取
        {
          cnt = 0;
          DestroyCnf(cnf->root); // 销毁当前解析的CNF
        }
      }
      printf(" Please input the file name: ");
      scanf("%s", fileName);
      if (ReadFile(cnf, fileName) == OK) // 读取文件并解析CNF
        printf(" Read successfully.\n");
      else
        printf(" Read failed.\n");
      break;
    }
    case 2: {
      if (cnf->root == NULL) // 如果没有打开CNF文件
        printf(" You haven't open the CNF file.\n");
      else
        PrintCnf(cnf); // 打印CNF文件
      break;
    }
    // case 3: {
    //   if (cnf->root == NULL) // 如果没有打开CNF文件
    //   {
    //     printf(" You haven't open the CNF file.\n");
    //     break;
    //   } else {
    //     CNF newCnf = (CNF)malloc(sizeof(cnfNode));
    //     newCnf->root = CopyCnf(cnf->root); // 复制CNF
    //     newCnf->boolCount = cnf->boolCount;
    //     newCnf->clauseCount = cnf->clauseCount;
    //     value = (bool *)malloc(sizeof(bool) * (cnf->boolCount + 1));
    //     for (int i = 1; i <= cnf->boolCount; i++)
    //       value[i] = FALSE;                     // 初始化为FALSE
    //     LARGE_INTEGER frequency, frequency_;    // 计时器频率
    //     LARGE_INTEGER start, start_, end, end_; // 设置时间变量
    //     double time = 0, time_;
    //     int result;
    //     /*
    //     // 未优化的时间
    //     QueryPerformanceFrequency(&frequency);
    //     QueryPerformanceCounter(&start); // 计时开始;
    //     result = DPLL(newCnf, value, 1);
    //     QueryPerformanceCounter(&end); //
    //     time = (double)(end.QuadPart - start.QuadPart) /
    //            frequency.QuadPart; // 计算运行时间
    //     // 输出SAT结果
    //     if (result == OK) // SAT
    //     {
    //       printf(" SAT\n\n");
    //       // 输出文字的真值
    //       for (int i = 1; i <= cnf->boolCount; i++) {
    //         if (value[i] == TRUE)
    //           printf(" %-4d: TRUE\n", i);
    //         else
    //           printf(" %-4d: FALSE\n", i);
    //       }
    //     } else // UNSAT
    //       printf(" UNSAT\n");
    //     // 输出优化前的时间
    //     printf("\n Time: %lf ms(not optimized)\n", time * 1000);
    //     */
    //     // 直接使用优化后的DPLL
    //     int ch = 1; // 直接设置为1，表示使用优化

    //     // 是否优化
    //     // int ch;
    //     printf("\n Do you want to optimize the algorithm? (1/0): ");
    //     scanf("%d", &ch);

    //     if (ch == 0) {
    //       time_ = 0;
    //     } else {
    //       // 重置value数组
    //       for (int i = 1; i <= cnf->boolCount; i++)
    //         value[i] = FALSE;

    //       QueryPerformanceFrequency(&frequency_);
    //       QueryPerformanceCounter(&start_); // 计时开始;
    //       result = DPLL(newCnf, value, 3);  // 使用优化后的DPLL
    //       QueryPerformanceCounter(&end_);   // 结束
    //       time_ = (double)(end_.QuadPart - start_.QuadPart) /
    //               frequency_.QuadPart; // 计算运行时间

    //       // 输出SAT结果
    //       if (result == OK) // SAT
    //       {
    //         printf(" SAT\n\n");
    //         // 输出文字的真值
    //         for (int i = 1; i <= cnf->boolCount; i++) {
    //           if (value[i] == TRUE)
    //             printf(" %-4d: TRUE\n", i);
    //           else
    //             printf(" %-4d: FALSE\n", i);
    //         }
    //       } else // UNSAT
    //         printf(" UNSAT\n");

    //       printf("\n Time: %lf ms(optimized)\n", time_ * 1000);
    //     }

    //     // 是否保存
    //     printf("\n Save the result to file? (1/0): ");
    //     int choice;
    //     scanf("%d", &choice);
    //     printf("\n");
    //     if (choice == 1) {
    //       // 保存求解结果，将未优化时间设为0（因为我们只运行了优化版本）
    //       if (SaveResult(result, 1, time_, value, fileName, cnf->boolCount))
    //         printf(" Save successfully.\n");
    //       else
    //         printf(" Save failed.\n");
    //     }

    //     // 释放内存
    //     DestroyCnf(newCnf->root);
    //     free(newCnf);
    //   }
    //   break;
    // }
    // display.cpp -> DisPlay() -> switch(op)
    case 3: {
      if (cnf->root == NULL) // 如果没有打开CNF文件
      {
        printf(" You haven't open the CNF file.\n");
        break;
      } else {
        // --- 初始化 ---
        value = (bool *)malloc(sizeof(bool) * (cnf->boolCount + 1));
        LARGE_INTEGER frequency;    // 计时器频率
        LARGE_INTEGER start, end;   // 时间变量
        double time_unoptimized = 0, time_optimized = 0;
        int result_unoptimized, result_optimized;

        printf("\n--- Running Unoptimized DPLL (Strategy 1) ---\n");
        
        // --- 1. 进行未优化的求解 ---
        // 为未优化求解创建独立的CNF副本
        CNF cnf_unoptimized = (CNF)malloc(sizeof(cnfNode));
        cnf_unoptimized->root = CopyCnf(cnf->root);
        cnf_unoptimized->boolCount = cnf->boolCount;
        cnf_unoptimized->clauseCount = cnf->clauseCount;
        
        // 初始化value数组
        for (int i = 1; i <= cnf->boolCount; i++) value[i] = FALSE;

        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&start); // 计时开始
        result_unoptimized = DPLL(cnf_unoptimized, value, 1); // 使用 flag = 1
        QueryPerformanceCounter(&end); // 计时结束
        time_unoptimized = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart; // 计算运行时间
        
        if (result_unoptimized == OK) printf(" Result: SAT\n");
        else printf(" Result: UNSAT\n");
        printf(" Time: %lf ms\n", time_unoptimized * 1000);
        
        DestroyCnf(cnf_unoptimized->root); // 销毁副本
        free(cnf_unoptimized);

        printf("\n--- Running Optimized DPLL (Strategy 3) ---\n");

        // --- 2. 进行优化后的求解 ---
        // 为优化求解创建独立的CNF副本
        CNF cnf_optimized = (CNF)malloc(sizeof(cnfNode));
        cnf_optimized->root = CopyCnf(cnf->root);
        cnf_optimized->boolCount = cnf->boolCount;
        cnf_optimized->clauseCount = cnf->clauseCount;

        // 重新初始化value数组
        for (int i = 1; i <= cnf->boolCount; i++) value[i] = FALSE;

        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&start); // 计时开始
        result_optimized = DPLL(cnf_optimized, value, 3); // 使用 flag = 3
        QueryPerformanceCounter(&end); // 计时结束
        time_optimized = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart; // 计算运行时间

        if (result_optimized == OK) {
            printf(" Result: SAT\n\n");
            // 打印优化后的解
            for (int i = 1; i <= cnf->boolCount; i++) {
                if (value[i] == TRUE) printf(" %-4d: TRUE\n", i);
                else printf(" %-4d: FALSE\n", i);
            }
        } else {
            printf(" Result: UNSAT\n");
        }
        printf("\n Time: %lf ms\n", time_optimized * 1000);

        DestroyCnf(cnf_optimized->root); // 销毁副本
        free(cnf_optimized);
        
        // --- 3. 计算并输出优化率 ---
        printf("\n--- Comparison ---\n");
        printf(" Unoptimized Time: %lf ms\n", time_unoptimized * 1000);
        printf(" Optimized Time:   %lf ms\n", time_optimized * 1000);
        if (time_unoptimized > 0) {
            double optimization_rate = ((time_unoptimized - time_optimized) / time_unoptimized) * 100;
            printf(" Optimization Rate: %.2lf%%\n", optimization_rate);
        } else {
            printf(" Optimization Rate: N/A (unoptimized time is zero)\n");
        }

        // --- 4. 保存结果 ---
        printf("\n Save the result to file? (1/0): ");
        int choice;
        scanf("%d", &choice);
        printf("\n");
        if (choice == 1) {
            // 保存优化后的求解结果，并传入两个时间用于记录
            if (SaveResult(result_optimized, time_unoptimized, time_optimized, value, fileName, cnf->boolCount))
                printf(" Save successfully.\n");
            else
                printf(" Save failed.\n");
        }

        // 释放value数组
        free(value);
        value = NULL;
      }
      break;
    }
    case 4: {
      Sudoku();    // X数独界面
      PrintMenu(); // 跳转回来时重新打印菜单
      break;
    }
    case 0: {
      printf(" Exit successfully.\n");
      return; // 退出
    }
    default: {
      printf(" Invalid input.\n"); // 无效输入
      break;
    }
    }
  }
  if (cnf->root != NULL)
    DestroyCnf(cnf->root); // 退出时销毁CNF
  free(cnf);
  return;
}

// 打印菜单
void PrintMenu() {
  printf("|================Menu for SAT================|\n");
  printf("|--------------------------------------------|\n");
  printf("|            1. Open the CNF file            |\n");
  printf("|     2. Traverse and output each clause     |\n");
  printf("|   3. Solve using DPLL and save the result  |\n");
  printf("|              4. Sudoku game              |\n");
  printf("|                 0.  EXIT                   |\n");
  printf("|============================================|\n\n");
}