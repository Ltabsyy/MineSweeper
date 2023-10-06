#include<stdio.h>
#include<stdlib.h>
#include<time.h>//时间戳作种子生成随机数

/*
* 扫雷 by-Jws
* 
* 游戏思路：
* 开始界面——游戏界面——调用游戏后台生成雷及雷周围的数字——游戏界面——游戏机制（判定胜负）——（胜利/失败）/继续游戏
*/

void ForeGround();//游戏界面
void BackGround(int x, int y);//游戏后台
void Mechanism(int mode, int x, int y);//游戏机制
void Set();//游戏设置
void End();//游戏结束

char Board[64][64];//游戏后台
char ForeBoard[64][64];//游戏界面

int Process;//只执行一次游戏初始化
int iWhile = 0;//对生成随机数的种子的更改
int Mine[128][2];//储蓄雷，行下标为雷数的最大上限
int Number[65][65];//用来存储生成的雷的坐标（比界面数组多一个元素防止数据溢出）

int mineNumer = 10;//雷数量的初始化
int boardHeighthSize = 10;//界面高度初始化
int boardWideSize = 10;//界面宽度初始化

int main(void) {

	Process = 0;//初始化Process的值
	int choiceMode;//游戏功能的选择

	while (1) {

		system("cls");//清屏

		printf("*******************************\n");
		printf("**新游戏(1)**设置(2)**退出(3)**\n");
		printf("*******************************\n");
		scanf("%d", &choiceMode);
		switch (choiceMode)
		{
		case 1:
			ForeGround();//调用游戏前台，开始游戏
			break;
		case 2:
			Set();//调用游戏设置，设置游戏内容
			break;
		case 3:
			exit(0);//退出游戏
			break;
		default:
			break;
		}
	}//开始菜单

	return 0;

}//游戏开始

void ForeGround() {

	system("cls");//清屏

	if (Process == 0) {

		int a = 48;
		for (int Row = 0; Row < boardHeighthSize; Row++) {
			if (a == 58) a = 97;
			if (a == 122) a = 65;
			ForeBoard[Row][0] = (char)a;
			a++;
		}
		a = 48;
		for (int Column = 0; Column < boardWideSize; Column++) {
			if (a == 58) a = 97;
			if (a == 122) a = 65;
			ForeBoard[0][Column] = (char)a;
			a++;
		}//前台界面横纵坐标的生成

		for (int Row = 1; Row < boardHeighthSize; Row++) {
			for (int Column = 1; Column < boardWideSize; Column++) {
				ForeBoard[Row][Column] = '%';
			}
		}//前台界面的生成

		for (int Row = 0; Row < boardHeighthSize; Row++) {
			for (int Column = 0; Column < boardWideSize; Column++) {
				printf("%c ", ForeBoard[Row][Column]);
			}
			printf("\n");
		}//第一次打印界面

		char x1, y1;//选择的坐标
		int x, y;//转换后的坐标
		printf("选择坐标[x][y]\n");
		//通过键盘获取第一次选择的坐标
		getchar();
		x1 = getchar();
		getchar();
		y1 = getchar();
		//将char数据转为int数据
		if ((int)x1 > 96) x = (int)x1 - 87;
		else if ((int)x1 > 64) x = (int)x1 - 29;
		else if ((int)x1 > 48) x = (int)x1 - 48;
		if ((int)y1 > 96) y = (int)y1 - 87;
		else if ((int)y1 > 64) y = (int)y1 - 29;
		else if ((int)y1 > 48) y = (int)y1 - 48;

		Process++;

		BackGround(x, y);//调用游戏后台生成雷

	}//游戏的初始化

	for (int Row = 0; Row < boardHeighthSize; Row++) {
		for (int Column = 0; Column < boardWideSize; Column++) {
			printf("%c ", ForeBoard[Row][Column]);
		}
		printf("\n");
	}//打印游戏界面
	printf("剩余雷数: %d\n", mineNumer);//打印剩余雷数

	int determine = 0;
	for (int Row = 0; Row < boardHeighthSize; Row++) {
		for (int Column = 0; Column < boardWideSize; Column++) {
			if (ForeBoard[Row][Column] == '%') determine++;
		}
	}//判定是否胜利
	if (determine == mineNumer) Mechanism(1, 0, 0);//如果剩余格与雷数一致，则胜利

	char choosemode;//选择的坐标
	char x1, y1;//选择的坐标
	int x, y;//转换后的坐标
	printf("选择模式与坐标(@:翻开/#:标记)\n[@/#] [x] [y]\n");
	//通过键盘获取选择的坐标
	getchar();
	choosemode = getchar();
	getchar();
	x1 = getchar();
	getchar();
	y1 = getchar();
	//将char数据转为int数据
	if ((int)x1 > 96) x = ((int)x1) - 87;
	else if ((int)x1 > 64) x = ((int)x1) - 29;
	else if ((int)x1 > 48) x = ((int)x1) - 48;
	if ((int)y1 > 96) y = ((int)y1) - 87;
	else if ((int)y1 > 64) y = ((int)y1) - 29;
	else if ((int)y1 > 48) y = ((int)y1) - 48;

	if (choosemode == '@') {
		if (Board[x][y] == '*') Mechanism(2, 0, 0); //如果踩到雷，则失败
		if (Board[x][y] == '1'
			|| Board[x][y] == '2'
			|| Board[x][y] == '3'
			|| Board[x][y] == '4'
			|| Board[x][y] == '5'
			|| Board[x][y] == '6'
			|| Board[x][y] == '7'
			|| Board[x][y] == '8') Mechanism(3, x, y);//如果踩到数字
		if (Board[x][y] == ' ') Mechanism(4, x, y);//如果踩到空白
	}
	if (choosemode == '#') {
		if (ForeBoard[x][y] == '%') {
			ForeBoard[x][y] = '#';
			mineNumer--;//剩余雷数-1
			ForeGround();
		}
		if (ForeBoard[x][y] == '#') {
			ForeBoard[x][y] = '%';
			mineNumer++;//剩余雷数+1
			ForeGround();
		}
		ForeGround();
	}

}//游戏界面

void BackGround(int x, int y) {

	for (int Row = 1; Row < boardHeighthSize; Row++) {
		for (int Column = 1; Column < boardWideSize; Column++) {
			Board[Row][Column] = ' ';
		}
	}//后台初始界面的生成

	srand((unsigned)time(NULL) + iWhile);//当前时间戳作种子生成随机数
	for (int Site = 0; Site < mineNumer; Site++) {
		Mine[Site][0] = rand() % (boardHeighthSize - 1) + 1;//雷的行下标
		Mine[Site][1] = rand() % (boardWideSize - 1) + 1;//雷的列下标
		if (Board[Mine[Site][0]][Mine[Site][1]] != '*') {
			Board[Mine[Site][0]][Mine[Site][1]] = '*';
		}//检测新雷位置是否与旧雷重复并为Board数组赋值
		else Site--;
	}//雷的生成，定义“Mine”数组，行下标表示雷的个数，列下标的两个值表示雷的行下标与列下标。
	iWhile++;//防止下一次调用程序时生成随机数的种子（时间戳）重复

	for (int Row = 1; Row < boardHeighthSize; Row++) {
		for (int Column = 1; Column < boardWideSize; Column++) {
			Number[Row][Column] = 48;//初始数字为48（0的Ascii码），方便类型转换
		}
	}//雷周围数字的生成
	for (int Row = 0; Row < boardHeighthSize; Row++) {
		for (int Column = 0; Column < boardWideSize; Column++) {
			if (Board[Row][Column] == '*') {
				Number[Row - 1][Column - 1]++;
				Number[Row - 1][Column]++;
				Number[Row - 1][Column + 1]++;
				Number[Row][Column - 1]++;
				Number[Row][Column + 1]++;
				Number[Row + 1][Column - 1]++;
				Number[Row + 1][Column]++;
				Number[Row + 1][Column + 1]++;//循环遍历雷，在雷周围生成数字
				if (Board[Row - 1][Column - 1] != '*')
					Board[Row - 1][Column - 1] = (char)Number[Row - 1][Column - 1];
				if (Board[Row - 1][Column] != '*')
					Board[Row - 1][Column] = (char)Number[Row - 1][Column];
				if (Board[Row - 1][Column + 1] != '*')
					Board[Row - 1][Column + 1] = (char)Number[Row - 1][Column + 1];
				if (Board[Row][Column - 1] != '*')
					Board[Row][Column - 1] = (char)Number[Row][Column - 1];
				if (Board[Row][Column + 1] != '*')
					Board[Row][Column + 1] = (char)Number[Row][Column + 1];
				if (Board[Row + 1][Column - 1] != '*')
					Board[Row + 1][Column - 1] = (char)Number[Row + 1][Column - 1];
				if (Board[Row + 1][Column] != '*')
					Board[Row + 1][Column] = (char)Number[Row + 1][Column];
				if (Board[Row + 1][Column + 1] != '*')
					Board[Row + 1][Column + 1] = (char)Number[Row + 1][Column + 1];
			}//循环遍历雷，在雷周围生成数字，并解决数字覆盖掉雷的情况
		}
	}//雷周围数字的生成

	if (Board[x][y] != ' ') BackGround(x, y);//如果选中坐标不为空格，则重新生成雷

	int a = 48;
	for (int Row = 0; Row < boardHeighthSize; Row++) {
		if (a == 58) a = 97;
		if (a == 123) a = 65;
		Board[Row][0] = (char)a;
		a++;
	}
	a = 48;
	for (int Column = 0; Column < boardWideSize; Column++) {
		if (a == 58) a = 97;
		if (a == 123) a = 65;
		Board[0][Column] = (char)a;
		a++;
	}//游戏后台界面横纵坐标的生成

	Mechanism(4, x, y);//展开

}//游戏后台

void Mechanism(int mode, int x, int y) {

	if (mode == 1) {
		printf("You Win!\n");
		system("pause");
		End();
	}//胜利

	if (mode == 2) {
		printf("Game Over!\n");
		system("pause");
		End();
	}//踩到雷的情况

	if (mode == 3) {
		ForeBoard[x][y] = Board[x][y];
		int markMine = 0;
		if (ForeBoard[x - 1][y - 1] == '#') markMine++;
		if (ForeBoard[x - 1][y] == '#')markMine++;
		if (ForeBoard[x - 1][y + 1] == '#') markMine++;
		if (ForeBoard[x][y - 1] == '#')  markMine++;
		if (ForeBoard[x][y + 1] == '#') markMine++;
		if (ForeBoard[x + 1][y - 1] == '#') markMine++;
		if (ForeBoard[x + 1][y] == '#') markMine++;
		if (ForeBoard[x + 1][y + 1] == '#') markMine++;

		if (markMine == int(ForeBoard[x][y] - 48)) {
			if ((ForeBoard[x - 1][y - 1] != '#' && Board[x - 1][y - 1] == '*')
				|| (ForeBoard[x - 1][y] != '#' && Board[x - 1][y] == '*')
				|| (ForeBoard[x - 1][y + 1] != '#' && Board[x - 1][y + 1] == '*')
				|| (ForeBoard[x][y - 1] != '#' && Board[x][y - 1] == '*')
				|| (ForeBoard[x][y + 1] != '#' && Board[x][y + 1] == '*')
				|| (ForeBoard[x + 1][y - 1] != '#' && Board[x + 1][y - 1] == '*')
				|| (ForeBoard[x + 1][y] != '#' && Board[x + 1][y] == '*')
				|| (ForeBoard[x + 1][y + 1] != '#' && Board[x + 1][y + 1] == '*'))Mechanism(2, 0, 0); 
			//如果周围有未打开的雷则失败
			
			if (ForeBoard[x - 1][y - 1] != '#')
				ForeBoard[x - 1][y - 1] = Board[x - 1][y - 1];
			if (ForeBoard[x - 1][y] != '#')
				ForeBoard[x - 1][y] = Board[x - 1][y];
			if (ForeBoard[x - 1][y + 1] != '#')
				ForeBoard[x - 1][y + 1] = Board[x - 1][y + 1];
			if (ForeBoard[x][y - 1] != '#')
				ForeBoard[x][y - 1] = Board[x][y - 1];
			if (ForeBoard[x][y + 1] != '#')
				ForeBoard[x][y + 1] = Board[x][y + 1];
			if (ForeBoard[x + 1][y - 1] != '#')
				ForeBoard[x + 1][y - 1] = Board[x + 1][y - 1];
			if (ForeBoard[x + 1][y] != '#')
				ForeBoard[x + 1][y] = Board[x + 1][y];
			if (ForeBoard[x + 1][y + 1] != '#')
				ForeBoard[x + 1][y + 1] = Board[x + 1][y + 1];
			//打开数字周围未打开的区域

			for (int round = 0; round < 100; round++) {
				for (int Row = 0; Row < boardHeighthSize; Row++) {
					for (int Column = 0; Column < boardWideSize; Column++) {
						if (ForeBoard[Row][Column] == ' ') {
							ForeBoard[Row][Column] = Board[Row][Column];
							if (Board[Row - 1][Column + 1] != '*' && Board[Row - 1][Column + 1] != ' ')
								ForeBoard[Row - 1][Column + 1] = Board[Row - 1][Column + 1];
							if (Board[Row - 1][Column - 1] != '*' && Board[Row - 1][Column - 1] != ' ')
								ForeBoard[Row - 1][Column - 1] = Board[Row - 1][Column - 1];
							if (Board[Row - 1][Column] != '*') ForeBoard[Row - 1][Column] = Board[Row - 1][Column];
							if (Board[Row + 1][Column + 1] != '*' && Board[Row + 1][Column + 1] != ' ')
								ForeBoard[Row + 1][Column + 1] = Board[Row + 1][Column + 1];
							if (Board[Row + 1][Column - 1] != '*' && Board[Row + 1][Column - 1] != ' ')
								ForeBoard[Row + 1][Column - 1] = Board[Row + 1][Column - 1];
							if (Board[Row + 1][Column] != '*') ForeBoard[Row + 1][Column] = Board[Row + 1][Column];
							if (Board[Row][Column + 1] != '*') ForeBoard[Row][Column + 1] = Board[Row][Column + 1];
							if (Board[Row][Column - 1] != '*') ForeBoard[Row][Column - 1] = Board[Row][Column - 1];
						}
					}
				}
			}//通过循环遍历足够次数的ForeBoard数组（游戏前台）实现展开（详见笔记）
		}

		ForeGround();
	}//踩到数字的情况

	if (mode == 4) {

		if (Board[x][y] != '*') ForeBoard[x][y] = Board[x][y];

		for (int round = 0; round < 100; round++) {
			for (int Row = 0; Row < boardHeighthSize; Row++) {
				for (int Column = 0; Column < boardWideSize; Column++) {
					if (ForeBoard[Row][Column] == ' ') {
						ForeBoard[Row][Column] = Board[Row][Column];
						if (Board[Row - 1][Column + 1] != '*' && Board[Row - 1][Column + 1] != ' ') 
							ForeBoard[Row - 1][Column + 1] = Board[Row - 1][Column + 1];
						if (Board[Row - 1][Column - 1] != '*' && Board[Row - 1][Column - 1] != ' ') 
							ForeBoard[Row - 1][Column - 1] = Board[Row - 1][Column - 1];
						if (Board[Row - 1][Column] != '*') ForeBoard[Row - 1][Column] = Board[Row - 1][Column];
						if (Board[Row + 1][Column + 1] != '*' && Board[Row + 1][Column + 1] != ' ') 
							ForeBoard[Row + 1][Column + 1] = Board[Row + 1][Column + 1];
						if (Board[Row + 1][Column - 1] != '*' && Board[Row + 1][Column - 1] != ' ') 
							ForeBoard[Row + 1][Column - 1] = Board[Row + 1][Column - 1];
						if (Board[Row + 1][Column] != '*') ForeBoard[Row + 1][Column] = Board[Row + 1][Column];
						if (Board[Row][Column + 1] != '*') ForeBoard[Row][Column + 1] = Board[Row][Column + 1];
						if (Board[Row][Column - 1] != '*') ForeBoard[Row][Column - 1] = Board[Row][Column - 1];
					}
				}
			}
		}//通过循环遍历足够次数的ForeBoard数组（游戏前台）实现展开（详见笔记）

		ForeGround();

	}//踩到空白的情况
}//游戏机制

void Set() {

	int set;

	while (1) {

		system("cls");

		printf("*******************************\n");
		printf("********设置游戏难度(1)********\n");
		printf("*******自定义设置雷数(2)*******\n");
		printf("*****自定义设置界面大小(3)*****\n");
		printf("**********返回菜单(4)**********\n");
		printf("*******************************\n");
		printf("当前雷数:%d|当前界面大小:%d*%d\n", mineNumer, (boardWideSize - 1), (boardHeighthSize - 1));
		scanf("%d", &set);

		if (set == 1) {
			int difficuly;
			system("cls");
			printf("*******************************\n");
			printf("************初级(1)************\n");
			printf("************中级(2)************\n");
			printf("************高级(3)************\n");
			printf("*******************************\n");
			scanf("%d", &difficuly);
			if (difficuly == 1) {
				mineNumer = 10;
				boardWideSize = 9 + 1;
				boardHeighthSize = 9 + 1;
			}
			if (difficuly == 2) {
				mineNumer = 40;
				boardWideSize = 16 + 1;
				boardHeighthSize = 16 + 1;
			}
			if (difficuly == 3) {
				mineNumer = 99;
				boardWideSize = 30 + 1;
				boardHeighthSize = 24 + 1;
			}
		}

		if (set == 2) {
			printf("请输入雷数:\n");
			scanf("%d", &mineNumer);
		}

		if (set == 3) {
			int Wide, Heighth;
			printf("请输入界面宽度:\n");
			scanf("%d", &Wide);
			boardWideSize = ++Wide;
			printf("请输入界面高度:\n");
			scanf("%d", &Heighth);
			boardHeighthSize = ++Heighth;
		}

		if (set == 4) main();//返回主界面

	}

}//游戏设置

void End() {

	int next;

	system("cls");

	printf("********************************\n");
	printf("*********输入(1)继续游戏********\n");
	printf("******输入任意数字退出游戏******\n");
	printf("********************************\n");
	scanf("%d", &next);

	switch (next)
	{
	case 1: {
		mineNumer = 10;
		boardWideSize = 9 + 1;
		boardHeighthSize = 9 + 1;
		main();
	}
	default:
		exit(0);
	}

}//游戏结束
