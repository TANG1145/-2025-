/*----------------------------solver----------------------------*/

#include "SAT.hpp"

long long cnt = 0;

// 判断是否为单子句
status IsUnitClause(literalList l) {
  if (l != NULL && l->next == NULL) // 只有一个文字
    return TRUE;
  else
    return FALSE;
}

// 找到单子句并返回该文字
int FindUnitClause(clauseList cL) {
  clauseList p = cL;
  while (p) {
    if (IsUnitClause(p->head)) // 是单子句
      return p->head->literal; // 返回该文字
    p = p->next;
  }
  return 0;
}

// 销毁子句
status DestroyClause(clauseList &cL) {
  literalList p = cL->head;
  while (p) {
    literalList temp = p;
    p = p->next; // 指向下一个文字
    free(temp);  // 释放文字
  }
  free(cL);  // 释放子句
  cL = NULL; // cL指向NULL
  return OK;
}

// 根据选择的文字化简
void Simplify(clauseList &cL, int literal) {
  clauseList pre = NULL, p = cL; // pre指向前一个子句
  while (p != NULL) {
    bool clauseDeleted = false;           // 是否删除子句
    literalList lpre = NULL, q = p->head; // lpre指向前一个文字
    while (q != NULL) {
      if (q->literal == literal) // 删除该子句
      {
        if (pre == NULL) // 删除的是第一个子句
          cL = p->next;
        else // 删除的不是第一个子句
          pre->next = p->next;
        DestroyClause(p);                   // 销毁该子句
        p = (pre == NULL) ? cL : pre->next; // 指向下一个子句
        clauseDeleted = true;               // 子句已删除
        break;
      } else if (q->literal == -literal) // 删除该文字
      {
        if (lpre == NULL) // 删除的是第一个文字
          p->head = q->next;
        else // 删除的不是第一个文字
          lpre->next = q->next;
        free(q);                                   // 释放该文字
        q = (lpre == NULL) ? p->head : lpre->next; // 指向下一个文字
      } else                                       // 未删除
      {
        lpre = q;
        q = q->next;
      }
    }
    if (!clauseDeleted) // 子句未删除
    {
      pre = p;
      p = p->next;
    }
  }
}

// 复制cnf

clauseList CopyCnf(clauseList cL) {
  // 初始化新的CNF
  clauseList newCnf = (clauseList)malloc(sizeof(clauseNode));
  clauseList lpa, lpb;  // lpa指向新的子句,lpb指向旧的子句
  literalList tpa, tpb; // tpa指向新的文字,tpb指向旧的文字
  newCnf->head = (literalList)malloc(sizeof(literalNode));
  newCnf->next = NULL;
  newCnf->head->next = NULL;
  for (lpb = cL, lpa = newCnf; lpb != NULL; lpb = lpb->next, lpa = lpa->next) {
    for (tpb = lpb->head, tpa = lpa->head; tpb != NULL;
         tpb = tpb->next, tpa = tpa->next) {
      tpa->literal = tpb->literal;
      tpa->next = (literalList)malloc(sizeof(literalNode));
      tpa->next->next = NULL;
      if (tpb->next == NULL) // 旧的子句中的文字已经复制完
      {
        free(tpa->next);
        tpa->next = NULL;
      }
    }
    lpa->next = (clauseList)malloc(sizeof(clauseNode));
    lpa->next->head = (literalList)malloc(sizeof(literalNode));
    lpa->next->next = NULL;
    lpa->next->head->next = NULL;
    if (lpb->next == NULL) // 旧的CNF中的子句已经复制完
    {
      free(lpa->next->head);
      free(lpa->next);
      lpa->next = NULL;
    }
  }
  return newCnf;
}

// 选择文字(第一个文字)
int ChooseLiteral_1(CNF cnf) { return cnf->root->head->literal; }

//(没有单子句时的策略)选择文字(出现次数最多的文字)
int ChooseLiteral_2(CNF cnf) {
  clauseList lp = cnf->root;
  literalList dp;
  int *count, MaxWord,
      max; // count记录每个文字出现次数,MaxWord记录出现最多次数的文字
  count = (int *)malloc(sizeof(int) * (cnf->boolCount * 2 + 1));
  for (int i = 0; i <= cnf->boolCount * 2; i++)
    count[i] = 0; // 初始化
  // 计算子句中各文字出现次数
  for (lp = cnf->root; lp != NULL; lp = lp->next) {
    for (dp = lp->head; dp != NULL; dp = dp->next) {
      if (dp->literal > 0) // 正文字
        count[dp->literal]++;
      else
        count[cnf->boolCount - dp->literal]++; // 负文字
    }
  }
  max = 0;
  // 找到出现次数最多的正文字
  for (int i = 1; i <= cnf->boolCount; i++) {
    if (max < count[i]) {
      max = count[i];
      MaxWord = i;
    }
  }
  if (max == 0) {
    // 若没有出现正文字,找到出现次数最多的负文字
    for (int i = cnf->boolCount + 1; i <= cnf->boolCount * 2; i++) {
      if (max < count[i]) {
        max = count[i];
        MaxWord = cnf->boolCount - i;
      }
    }
  }
  free(count);
  return MaxWord;
}

// 选择最短子句中出现次数最多的文字
int ChooseLiteral_3(CNF cnf) {
  clauseList p = cnf->root;
  int *count = (int *)calloc(cnf->boolCount * 2 + 1, sizeof(int));
  int minSize = INT_MAX; // 初始化为大于可能的最大子句长度
  int literal = 0;
  clauseList temp = NULL;
  // 遍历子句，找到最小子句并统计其文字
  while (p != NULL) {
    literalList q = p->head;
    int clauseSize = 0;
    while (q != NULL) {
      clauseSize++;
      q = q->next;
    }
    if (clauseSize < minSize) {
      minSize = clauseSize; // 更新最小子句大小
      temp = p;
    }
    p = p->next;
  }
  // 遍历子句，统计最小子句中各文字出现次数
  literalList q = temp->head;
  while (q != NULL) {
    count[q->literal + cnf->boolCount]++;
    q = q->next;
  }
  // 找到最频繁的文字
  int maxCount = 0;
  for (int i = 0; i < cnf->boolCount * 2 + 1; i++) {
    if (count[i] > maxCount) {
      maxCount = count[i];
      literal = i - cnf->boolCount;
    }
  }
  free(count);
  return literal;
}

// 是否满足
status Satisfy(clauseList cL) {
  if (cL == NULL)
    return OK;
  else
    return ERROR;
}

// 是否有空子句
status EmptyClause(clauseList cL) {
  clauseList p = cL;
  while (p) {
    if (p->head == NULL) // 空子句，返回UNSAT
      return TRUE;
    p = p->next;
  }
  return FALSE;
}

// DPLL算法求解SAT问题
status DPLL(CNF cnf, bool value[], int flag) {
  /*1.单子句规则*/
  int unitLiteral = FindUnitClause(cnf->root); // 找单子句
  while (unitLiteral != 0) {
    cnt++;
    if (cnt % 10000 == 0 && cnt != 0)
      printf("Round:%d\n", cnt);
    value[abs(unitLiteral)] = (unitLiteral > 0) ? TRUE : FALSE;
    Simplify(cnf->root, unitLiteral); // 删句子(true)或者删文字(false)
    // 终止条件
    if (Satisfy(cnf->root) == OK)
      return OK;
    if (EmptyClause(cnf->root) == TRUE)
      return ERROR;
    unitLiteral = FindUnitClause(cnf->root);
  }
  /*2.选择一个未赋值的文字*/
  int literal;
  if (flag == 1)
    literal = ChooseLiteral_1(cnf); // 未优化
  else if (flag == 2)
    literal = ChooseLiteral_2(cnf); // 优化
  else
    literal = ChooseLiteral_3(cnf);
  /*3.将该文字赋值为真，递归求解*/
  CNF newCnf = (CNF)malloc(sizeof(cnfNode));
  newCnf->root = CopyCnf(cnf->root); // 复制CNF
  newCnf->boolCount = cnf->boolCount;
  newCnf->clauseCount = cnf->clauseCount;
  clauseList p = (clauseList)malloc(sizeof(clauseNode));
  p->head = (literalList)malloc(sizeof(literalNode));
  p->head->literal = literal;
  p->head->next = NULL;
  p->next = newCnf->root;
  newCnf->root = p; // 插入到表头
  if (DPLL(newCnf, value, flag) == 1)
    return 1; // 在第一分支中搜索
  DestroyCnf(newCnf->root);
  /*4.将该文字赋值为假，递归求解*/
  clauseList q = (clauseList)malloc(sizeof(clauseNode));
  q->head = (literalList)malloc(sizeof(literalNode));
  q->head->literal = -literal;
  q->head->next = NULL;
  q->next = cnf->root;
  cnf->root = q;                      // 插入到表头
  status re = DPLL(cnf, value, flag); // 回溯到执行分支策略的初态进入另一分支
  // DestroyCnf(cL);
  return re;
}

// 保存求解结果
// status SaveResult(int result, double time, double time_, bool value[],
//                   char fileName[], int boolCount) {
//   FILE *fp;
//   char name[100];
//   for (int i = 0; fileName[i] != '\0'; i++) {
//     // 修改拓展名.res
//     if (fileName[i] == '.' && fileName[i + 4] == '\0') {
//       name[i] = '.';
//       name[i + 1] = 'r';
//       name[i + 2] = 'e';
//       name[i + 3] = 's';
//       name[i + 4] = '\0';
//       break;
//     }
//     name[i] = fileName[i];
//   }
//   if (fopen_s(&fp, name, "w")) {
//     printf(" Fail!\n");
//     return ERROR;
//   }

//   // 按照要求格式输出
//   fprintf(fp, "s %d\n", result); // 求解结果

//   if (result == 1) {
//     fprintf(fp, "v");
//     // 确保输出所有变元的取值
//     for (int i = 1; i <= boolCount; i++) {
//       // 检查变元是否被赋值
//       if (value[i] == TRUE || value[i] == FALSE) {
//         if (value[i] == TRUE)
//           fprintf(fp, " %d", i);
//         else
//           fprintf(fp, " %d", -i);
//       } else {
//         // 如果变元没有被赋值，输出一个默认值（例如 TRUE）
//         fprintf(fp, " %d", i);
//       }
//     }
//     fprintf(fp, "\n");
//   }

//   // 输出优化后的执行时间（毫秒）
//   fprintf(fp, "t %.0f\n", time_ * 1000);

//   // 保留未优化时间和优化率信息（作为注释）
//   if (time != 0) {
//     fprintf(fp, "c 未优化时间: %.0fms\n", time * 1000);
//     double optimization_rate = ((time - time_) / time) * 100;
//     fprintf(fp, "c 优化率: %.2lf%%\n", optimization_rate);
//   }

//   fclose(fp);
//   return OK;
// }
status SaveResult(int result, double time_unoptimized, double time_optimized,
                  bool value[], char fileName[], int boolCount) {
    FILE *fp;
    char name[100];
    int len = strlen(fileName);

    // 健壮地生成.res文件名，替换.cnf后缀
    strcpy_s(name, sizeof(name), fileName);
    if (len > 4 && strcmp(name + len - 4, ".cnf") == 0) {
        name[len - 3] = 'r';
        name[len - 2] = 'e';
        name[len - 1] = 's';
    } else {
        // 如果原始文件名不以.cnf结尾，则直接追加.res
        strcat_s(name, sizeof(name), ".res");
    }

    // 使用fopen_s安全地打开文件
    if (fopen_s(&fp, name, "w")) {
        printf(" Fail to open result file!\n");
        return ERROR;
    }

    // 1. 写入求解结果 (s 1 or s 0)，1代表可满足，0代表不可满足 [cite: 151, 152]
    fprintf(fp, "s %d\n", result);

    // 2. 如果可满足，写入每个变元的赋值序列 (v ...) [cite: 151, 153]
    if (result == OK) {
        fprintf(fp, "v");
        for (int i = 1; i <= boolCount; i++) {
            // 根据value数组的真值，输出正整数或负整数
            if (value[i] == TRUE) {
                fprintf(fp, " %d", i);
            } else {
                fprintf(fp, " %d", -i);
            }
        }
        fprintf(fp, " 0\n"); // 按照惯例，以0作为赋值行的结束标志
    }

    // 3. 写入优化后的求解时间，以毫秒为单位 (t ...) [cite: 151, 154]
    fprintf(fp, "t %.0f\n", time_optimized * 1000);

    // 4. 以注释形式写入性能对比的详细信息，便于分析
    fprintf(fp, "c === Performance Analysis ===\n");
    fprintf(fp, "c Unoptimized Time: %.3f ms\n", time_unoptimized * 1000);
    fprintf(fp, "c Optimized Time:   %.3f ms\n", time_optimized * 1000);
    
    // 增加一个检查，防止未优化时间为0时出现除零错误
    if (time_unoptimized > 1e-9) { 
        double optimization_rate = ((time_unoptimized - time_optimized) / time_unoptimized) * 100;
        fprintf(fp, "c Optimization Rate: %.2f%%\n", optimization_rate);
    } else {
        fprintf(fp, "c Optimization Rate: N/A (Unoptimized time is zero)\n");
    }

    fclose(fp);
    return OK;
}
