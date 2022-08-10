#include<stdio.h>
#include<stdlib.h>//提供 malloc() 跟 realloc()

void OutputTest(int a[8][8]){
    for(int i=0;i<8;i++){
        for(int j=0;j<8;j++){
            printf("%d ",a[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}
//【已注释的代码用于输出测试】
void draw();
int ifTraversed(int r, int c);
//判断当前格子是否已经被下了
long hash(int i, int j);
//简单散列算法：将当前状态的棋子数、各棋子的位置顺序编码（1-64）简单相乘
//所谓「不同的状态」的定义就取决于上述散列算法的各个因数：各棋子的位置以及棋子数，所以散列值不可能碰撞
//为"ifTraversed"函数服务
void ClearChessboard(int a[8][8]);
//清空棋盘
int BACKTRACKING(int chessboard[8][8]);
//核心代码：回溯算法
void PlaceChessman(int a[8][8], int row, int column);
//放置棋子
void BackToPreviousState(int a[8][8]);
//返回上一状态
void HashNode();
//计算当前状态的散列值并保存至列表 List_HashedNodes

int CurrentNumber_Queens = 0;//棋盘上棋子的个数
int HistoryRow[9] = {9, 9, 9, 9, 9, 9, 9, 9, 9};
int HistoryColumn[9] = {9, 9, 9, 9, 9, 9, 9, 9, 9};
//假设当前棋盘上有 N 个棋子，那么上述两数组的第 1 到第 N 元素就是第 N 个放置的棋子所在的行跟列
//定义9个元素是为了便于散列值计算，所以上述两数组的首元素永远不会用到
//棋子的行跟列的取值从 0 到 7，为不影响散列计算，全部元素均初始化为"9"
//CurrentNumber_Queens 就相当于在上述两数组中，指示当前棋盘上最近一次下的棋子的位置

//整个回溯过程只使用一个棋盘，以上三个全局变量用于恢复棋盘状态。以下全局变量用于保存散列表

long *List_HashedNodes;//保存每个状态散列值的散列表
int Size_Long = sizeof(long);//前述列表使用长整型
int Size_List_HashedNodes = 0;//散列值列表的容量，用作重新分配内存空间函数 realloc 的参数
int Number_HashedNodes = 0;//已保存的状态的散列值数量

int SOLUTIONS = 0;//解的数量
int main() {
    int chessboard[8][8] = {0};//定义空棋盘
    BACKTRACKING(chessboard);//开始回溯
    free(List_HashedNodes);//释放散列表所占的内存空间
    return 0;
}

int BACKTRACKING(int chessboard[8][8]) {
    for (int row = 0; row < 8; row++) {//从第1个格子遍历到第64个格子
        for (int column = 0; column < 8; column++) {//每次遍历有四次判断
            for (int k = 1; k <= CurrentNumber_Queens; k++) {
                if (row == HistoryRow[k]) {//加速运行：对照当前棋盘上各个棋子所在的行
                    column = 7;//如果正在遍历已经有棋子的行，那么将正在遍历的列号调为"7"，下次循环就是下一行
                    break;//加速运行
                }
            }
            if (!chessboard[row][column]) {//逻辑判断、如果遍历到的格子是空的（分支 0）
                if (ifTraversed(row, column)) {//再判断此格子是否已经遍历过，通过散列表（分支 1）
                    if (row == 7 && column == 7) {//如果不仅遍历过而且还是最后一个格子（分支 2）
                        if (CurrentNumber_Queens == 0)//而且棋盘上没有棋子了（分支 3）
                            return 0;//分支 3 为真：那么回溯完毕，结束
                        BackToPreviousState(chessboard);//分支 3 为假：那么返回上一状态
                        // printf("Have been back:\n");//（测试代码）
                        // OutputTest(chessboard);//（测试代码）
                        BACKTRACKING(chessboard);//继续回溯
                    }
                    continue;//分支 2 为假：此格子虽然是空的但是落子之后的状态已经遍历过，所以跳过该格子
                }
//                 printf("HashAfterPlacing: %lu\n",hash(row,column));//（测试代码）
                PlaceChessman(chessboard, row, column);//分支 1 为假：格子不是空的，也没有遍历过，那么在这里放置棋子及其「领土」
                CurrentNumber_Queens++;//棋子数量增加一个
                HistoryRow[CurrentNumber_Queens] = row;
                HistoryColumn[CurrentNumber_Queens] = column;//记录落子的行跟列
                HashNode();//计算当前状态的散列值并保存到散列表
                //  printf("Dropped: (%d, %d)\n",row,column);//（测试代码）
                //  OutputTest(chessboard);//（测试代码）
                if (CurrentNumber_Queens == 8) {
                    SOLUTIONS++;//如果下了八枚棋子，那么得到解
                    draw();//画出解
                    BackToPreviousState(chessboard);//返回上一状态
                    BACKTRACKING(chessboard);//继续回溯
                }
                BACKTRACKING(chessboard);//放置完棋子后，继续回溯
            }
            else if (row == 7 && column == 7) {
                BackToPreviousState(chessboard);//分支 0 为假：虽然有棋子，但如果正在遍历最后一个格子，那么返回上一状态
                //  printf("Have been back:\n");//（测试代码）
                //  OutputTest(chessboard);//（测试代码）
                BACKTRACKING(chessboard);//继续回溯
            }
        }//每次遍历中遇到的所有判断都为假，那么继续下次遍历
    }
}

int ifTraversed(int r, int c) {
    long CompareNodeHashValue = hash(r, c);//计算假设在(r, c)落子后的散列值
    for (int i = 0; i < Number_HashedNodes; i++) {//跟散列表中的元素逐个对比
        if (CompareNodeHashValue == List_HashedNodes[i])
            return 1;//一旦有相同的散列值，则表示已经遍历过该状态
    }
    return 0;//否则表示没有遍历过该状态
}

void HashNode() {
    List_HashedNodes = realloc(List_HashedNodes, Size_List_HashedNodes + Size_Long);//给散列表多分配一个元素的位置
    Size_List_HashedNodes += Size_Long;//散列表大小增加一个元素的大小
    List_HashedNodes[Number_HashedNodes++] = hash(-1, -1);//计算散列值并添加到散列表，散列值数量自增一
}

long hash(int i, int j) {//散列算法（如果传入行列号参数表示本函数服务于 ifTraversed 函数，用于对比
    //正常情况下只传入「(-1, -1)」，表示服务于 HashNode 函数，记录散列值
    int a[8][8] = {0};//空棋盘
    for (int x = 1; x <= CurrentNumber_Queens; x++) {
        a[HistoryRow[x]][HistoryColumn[x]] = 1;
    }//计算散列值只需简单摆上棋子，不需要考虑棋子能直接移动到的格子
    long HashValue_Node = CurrentNumber_Queens;//计算散列值的乘积的因式中，包含「当前状态棋盘上的棋子数量」（其余因式是各个棋子的位置号）
    if (i != -1) {//如果传入行列号，那么为服务 ifTraversed 函数
        a[i][j] = 1;//在传入的行列号上多摆上一个棋子
        HashValue_Node++;//将「棋子数量」因式增加一个
//         OutputTest(a);//（测试代码）
    }
    int location = 1;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (a[i][j]) {
                HashValue_Node *= location;
            }//遍历整个棋盘的64个格子，如果遇到棋子，则该棋子的位置号跟散列值 HashValue_Node 相乘
            location++;
        }
    }
//     if(i != -1)//（测试代码）
//         printf("^^^ComparingHash: %lu\n",HashValue_Node);//（测试代码）
    return HashValue_Node;//返回散列值
}

void BackToPreviousState(int a[8][8]) {
    ClearChessboard(a);//先清空棋盘
    for (int i = 1; i < CurrentNumber_Queens/*遍历到最后下的棋子的前一枚棋子，所以不用「等于」*/; i++) {
        PlaceChessman(a, HistoryRow[i], HistoryColumn[i]);
    }//逐个放置棋子及其「领土」
    CurrentNumber_Queens--;//当前棋盘上棋子的数量减一
}

void PlaceChessman(int a[8][8], int row, int column) {//放置棋子及其「领土」
    for (int i = 0; i < 8; i++) {
        a[row][i] = 1;
    }
    for (int i = 0; i < 8; i++) {
        a[i][column] = 1;
    }
    int i = row, j = column;
    while (i != 8 && j != 8) {
        a[i][j] = 1;
        i++;
        j++;
    }
    i = row;
    j = column;
    while (i != 8 && j != -1) {
        a[i][j] = 1;
        i++;
        j--;
    }
    i = row;
    j = column;
    while (i != -1 && j != 8) {
        a[i][j] = 1;
        i--;
        j++;
    }
    i = row;
    j = column;
    while (i != -1 && j != -1) {
        a[i][j] = 1;
        i--;
        j--;
    }
}

void ClearChessboard(int a[8][8]) {//清理棋盘，拿走所有棋子
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (a[i][j])
                a[i][j] = 0;
        }
    }
}
void draw(){//画出解
    printf("第 %d 个解：\n",SOLUTIONS);
    int a[8][8]={0};
    for (int i = 1; i <= CurrentNumber_Queens; i++) {
        a[HistoryRow[i]][HistoryColumn[i]]=1;
    }
    for(int r=0; r<8; r++){
        for(int c=0; c<8; c++){
            printf("%d ",a[r][c]);
        }
        printf("\n");
    }
}