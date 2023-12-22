//#define _CRT_SECURE_NO_WARNINGS//vs专属(doge)
#include <stdio.h>
#include <stdlib.h>
#include <time.h>//时间戳作种子生成随机数，用于生成地图和无确定解时随机翻开
#include <conio.h>//非阻塞式输入
#include <windows.h>//面向Windows输出彩色字符
//#include <stdbool.h>
//#include <graphics.h>//Easy Graphics Engine
//#include <math.h>
/**
 * 扫雷 MineSweeper Run
 * 
 * by Ltabsyy & Jws
 * 
 * https://github.com/Ltabsyy/MineSweeper
 **/
#define LimHeight 128//最大高度，限制行数
#define LimWidth 93//最大宽度，限制列数
#define LimDictionary 1024//字典大小
#define LimName 32//限制自制地图名称长度
//内存占用至少为 59*H*W+128*D Byte，默认约0.8MB

// 外部窗口形态
/*int sideLength = 32;//外部窗口方块边长
#define heightOfBlock sideLength
#define widthOfBlock sideLength//锁定纵横比
#define heightOfBar 2*sideLength
#define xOfChar widthOfBlock/3
#define yOfChar heightOfBlock/6//略低于居中的heightOfBlock/8
#define heightOfChar heightOfBlock*3/4//字符高度为方块的3/4*/

// 控制栏
int IsPosInRectangle(int x, int y, int x1, int y1, int x2, int y2);
void DrawControlBar(int select);
int ChoiceControlBar(COORD mousePos, DWORD dwButtonState);
void DrawProgressBar(int progress);

// 鼠标操作栏
void DrawSettingsBar(int select);
int ChoiceSettingsBar(COORD mousePos, DWORD dwButtonState);
void DrawSetDifficulty(int select);
int ChoiceSetDifficulty(COORD mousePos, DWORD dwButtonState);
void DrawSetSummonCheckMode(int select);
int ChoiceSetSummonCheckMode(COORD mousePos, DWORD dwButtonState);
void DrawSetOperateMode(int select);
int ChoiceSetOperateMode(COORD mousePos, DWORD dwButtonState);
void DrawSetCursor(int select);
int ChoiceSetCursor(COORD mousePos, DWORD dwButtonState);

// 地图生成和显示
void SummonBoard(int seed, int r0, int c0);//生成雷，数字和后台总板
int Place(int n);//计算某数所占位数，用于对齐坐标轴
void PrintCell(int board, int isShown, int bkcolor);//打印地图方块内字符
void ShowBlock(int r, int c, int mode);//根据背景颜色和模式显示地图方块
void ShowBoard(int mode);//在控制台显示地图，0前台，1后台
//void ShowPartBoard(int r1, int c1, int r2, int c2, int mode);//在控制台显示部分地图
void ReShowBoard(int r, int c, int mode, int yOfMap);//地图重绘
void FreshCursor(int r, int c, int yOfMap);//刷新光标

// 外部窗口显示
/*void DrawMine(int r, int c);//绘制地图地雷
void DrawMineA(int x0, int y0, int r);//绘制地雷图形
void DrawFlag(int r, int c);//绘制地图旗帜
void DrawBlock(int r, int c, int board, int isShown);//绘制方块
void DrawLineA(int x0, int y0, int r, int angle);//绘制时钟指针
void DrawClock(int x0, int y0, int r, int time);//绘制时钟
void DrawBoard(int mode, int remainder, int t);//绘制总外部窗口*/

// 后台计算
int Difficulty(int height, int width, int mine);//根据地图信息判断难度
int IsAroundZeroChain(int r0, int c0);
void OpenZeroChain(int r0, int c0);//翻开0连锁翻开
int RealRemainder();//真实的剩余雷数，非显示的剩余雷数
int NumberOfNotShown();//%的数量
int NumberOfSign();//#的数量
int NumberOfNotShownAround(int r0, int c0);//某坐标附近%的数量
int NumberOfSignAround(int r0, int c0);//某坐标附近#的数量
int WASDMove(int* r, int* c, WORD wVirtualKeyCode);

// 方案思考
void LookMap();//从实时游戏获取地图
void TranslateMap(int rs0, int cs0);//从键盘输入获取地图
int Solve();
int Think();
int DeepThink();
//int WholeThink();
void SummonMineRateForNotShown();
int NumberOfNumberAround(int r0, int c0);
void ShowAnswer(int isFoundOpen, int uncertainOpen);
void ShownModeBak(int mode);//备份显示方式矩阵，1备份，0恢复
int IsSolvableMap(int seed, int r0, int c0);
void ShowSolution(int yOfMap);//在原地图显示方案矩阵

// 历史记录
struct Record
{
	//存储数据
	int numberOfMine;
	int heightOfBoard, widthOfBoard;
	int summonCheckMode;
	int seed, r0, c0;
	int time;
	int solved3BV, total3BV;
	int isHelped;
	//计算数据
	float speed;
	int difficulty;
};
struct Records
{
	//存储数据
	struct Record* record;
	int numberOfRecords;
	//计算数据
	int minimumTime[5];//难度0-4时间纪录
	float fastestSpeed[5];//3BV/s纪录
	int totalTime;//所有记录总时间
	int totalSolved3BV;//总翻开数
	int maxOpenNumber;//最大翻开数字
	int gamerLevel;//玩家等级
};
int IsEffectiveRecord(struct Record record);
void PrintRecords(struct Records records);
struct Records ReadRecords();
void WriteRecords(struct Records records);
struct Records AddRecord(struct Records records, struct Record newRecord);
struct Records DeleteRecord(struct Records records, int deleteNumber);
int MaxOpenNumber(struct Records records);
int GamerLevel(struct Records records);

// 自制地图
struct Map
{
	//地图标识(含回车，可含空格)
	char* name;//地图名称
	char* author;//作者名称
	//存储数据
	int height;
	int width;
	int** isMine;
	//计算数据
	int numberOfMine;
	int** board;
};
struct Maps
{
	struct Map* map;
	int numberOfMaps;
};
void PrintMap(struct Map map, int showNumber);
void PrintMaps(struct Maps maps);
struct Maps ReadMaps();
void WriteMaps(struct Maps maps);
struct Maps AddMaps(struct Maps maps, struct Map newMap);
struct Maps DeleteMap(struct Maps maps, int deleteNumber);
struct Maps DeleteMaps(struct Maps maps);
struct Map EditMap(struct Map map);

// 功能模块
void RCScan(char* operation, int* r, int* c, int yOfCommand, struct Record information);//@#rc指令输入模块
void Solution();//地图求解模块
void Bench(int seedMin, int seedMax, int r0, int c0, int showStep, int showSolution);//Bench模块
int BBBV(int seed, int r0, int c0, int mode);//Bechtel'sBoardBenchmarkValue，计算地图3BV
void MapSearch(int seedMin, int seedMax, int r0, int c0);//地图搜索模块
void** MatrixMemory(void** matrix, int rm, int cm, int sizeOfElements, int mode);//矩阵内存操作
void ReallocMemory(int height, int width, int mode);//全局矩阵内存重分配，1申请，0释放
struct Records RecordsEditer(struct Records records);//RecordsEditer模块
int CustomMapsEditer();
void Error();//错误

// 全局矩阵
int** isMine = 0;//雷
int** numberOfMineAround = 0;//数字
int** board = 0;//后台总板，数字0~8雷9
int** isShown = 0;//显示方式矩阵，0未翻开，1已翻开，2已标记
int** zeroChain = 0;
int** isShownBak = 0;//显示方式矩阵备份
char** mapShown = 0;//键盘输入的地图
char** map = 0;//获取的地图
int** numberShown = 0;//获取的数字，非数字为9
int** solution = 0;//方案，1翻开，2标记
int** isThought = 0;//1未知方块，2附近数字，3枚举后放弃，4过长放弃
int** numberTeam = 0;//数对，1数对之一独占区，2数对之二独占区，3共享区
int** thinkChain = 0;//未知链
//int* possibility = 0;
int possibility[32]={0};
int** numberCheckBase = 0;//校验基底
int** numberCheck = 0;
int** dictionary = 0;
float** isMineRate = 0;

// 全局变量及初始化
int numberOfMine = 10;//雷数量
int heightOfBoard = 10;//界面高度
int widthOfBoard = 10;//界面宽度
int summonCheckMode = 2;//0不校验，1非雷，2必空，3可解
int showTime = 1;//显示用时，0不显示，1显示
int show3BV = 0;//显示3BV和3BV/s
int backgroundColor = 0x07;//背景颜色，深色模式0x07，浅色模式0xf0
int operateMode = 2;//操作模式，0@#rc，1wasd23，2鼠标点击，3Window
int solveMode = 1;//1游戏模式，2分析模式
int lengthOfThinkChain = 19;//未知链长度
int refreshCycle = 50;//刷新周期，默认50ms，一般鼠标8ms，游戏鼠标1ms
int fastSign = 0;//#数字快速标记周围
int newCursor = 2;//1><光标，2淡黄色高亮光标，3淡黄色高亮行列
int visibleCursor = 0;//1显示控制台光标，0隐藏控制台光标
int dynamicMemory = 0;//启用动态内存分配
int heightOfMapShown, widthOfMapShown, rs0, cs0;//控制台地图显示
int numberOfAbandonedThinkChain = 0;//Bench统计
char cursorLeft[2] = ">";//><光标左
char cursorRight[2] = "<";//><光标右

// 是否调试(0:关 1:开，显示部分 2:开，显示全部)
int debug = 0;

// 控制台显示
//void ColorStr(const char* content, int color)
void ColorStr(char* content, int color)//输出彩色字符
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
	printf("%s", content);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), backgroundColor);
}
//void gotoxy(short int x, short int y)
void gotoxy(int x, int y)//以覆写代替清屏，加速Bench
{
	COORD coord = {x, y};
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}
void clrscr()//清空屏幕
{
	HANDLE hdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(hdout, &csbi);//获取标准输出设备的屏幕缓冲区属性
	DWORD size = csbi.dwSize.X * csbi.dwSize.Y, num = 0;//定义双字节变量
	COORD pos = {0, 0};
	//把窗口缓冲区全部填充为空格并填充为默认颜色
	FillConsoleOutputCharacter(hdout, ' ', size, pos, &num);
	FillConsoleOutputAttribute(hdout, csbi.wAttributes, size, pos, &num);
	SetConsoleCursorPosition(hdout, pos);//光标定位到窗口左上角
}
void setbgcolor(int color)//设置背景颜色
{
	HANDLE hdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(hdout, &csbi);
	DWORD size = csbi.dwSize.X * csbi.dwSize.Y, num = 0;
	COORD pos = {0, 0};
	FillConsoleOutputAttribute(hdout, color, size, pos, &num);
	SetConsoleTextAttribute(hdout, color);
}
void showCursor(int visible)//显示或隐藏光标
{
	CONSOLE_CURSOR_INFO cursor_info = {20, visible};
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
}
void ColorNumber(int number, int color)//输出彩色数字
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
	printf("%d", number);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), backgroundColor);
}
void SetConsoleMouseMode(int mode)//键鼠操作切换
{
	if(mode == 1)//切换到鼠标
	{
		SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS);
	}
	else if(mode == 0)//切换到键盘
	{
		//SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS);
		//system("pause");//system指令使SetConsoleMode失效
		SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), ENABLE_PROCESSED_INPUT | ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT
			| ENABLE_MOUSE_INPUT | ENABLE_INSERT_MODE | ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS
			| ENABLE_AUTO_POSITION);
	}
}
void GetSizeOfMap(int* heightOfWindow, int* widthOfWindow, int* heightOfFont, int* maxHeightOfMap, int* maxWidthOfMap)
{
	HANDLE hdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	CONSOLE_FONT_INFO cfi;
	//获取控制台窗口大小和字体大小
	GetConsoleScreenBufferInfo(hdout, &csbi);
	GetCurrentConsoleFont(hdout, 0, &cfi);
	*heightOfWindow = csbi.srWindow.Bottom-csbi.srWindow.Top+1;//30 50
	*widthOfWindow = csbi.dwSize.X;//120 189
	*heightOfFont = cfi.dwFontSize.Y;//16
	//计算最大地图大小
	*maxHeightOfMap = *heightOfWindow-3-4-1-2;//30-3-4-1-2=20 40，去除控制栏3，游戏信息栏4，留白1，列坐标轴2
	*maxWidthOfMap = (*widthOfWindow-1-3)/2;//(120-1-3)/2=58 92(.5)，去除回车1，行坐标3，空白一半
	if(*maxHeightOfMap > 37) *maxHeightOfMap += 3;//大于40时不显示控制栏
	//if(*heightOfFont > 16) *maxHeightOfMap -= 3*(*heightOfFont-16);
}

int main()
{
	FILE* file;//文件读写
	int choiceMode = 0, choiceTemp = 0, choiceSet = 0;//游戏功能的选择
	char operation, key;//游戏操作和设置的字符读取，用户按键
	int temp;//数据暂存
	int seed, r0, c0, seedMin, seedMax;//地图生成
	int r, c;//通用坐标
	int remainder, isOpenMine;//游戏胜利判断
	int ra, ca;//连锁反应判断
	int t0, t1, t2;//显示用时
	int yOfMapEnd;//输出定位
	int ro, co;//上一个操作坐标
	int bbbv;//地图3BV
	int difficulty;//当前难度
	float density;//雷密度，实际没有必要定义(doge)
	struct Records records;//历史记录
	struct Record newRecord;
	// 控制台鼠标操作
	HANDLE hdin = GetStdHandle(STD_INPUT_HANDLE);
	COORD mousePos = {0, 0};//鼠标坐标
	COORD mouseOperatedPos = {0, 0};//鼠标已操作坐标，屏蔽双击
	INPUT_RECORD rcd;
	DWORD rcdnum;
	int isReadyRefreshMouseOperatedPos = 0;
	// main()局部变量及初始化
	int lastMap = 0;//有上一次地图
	int clickSpace = 1;//0鼠标点击屏蔽空格
	int touchMode = 0;//鼠标点击模式左右键功能切换，用于触屏标记
	int isHelped = 0;//记录一局游戏是否被实时求解指令帮助
	int benchSeedMin = 0;//Bench种子范围
	int benchSeedMax = 99;
	int benchShowStep = 1;//0结束帧，1中间帧，2帧暂停，3全调试
	int benchShowSolution = 1;//Bench在原地图显示方案矩阵
	// Window
	//int xm, ym;
	//mouse_msg mouseMsg;
	//很好，我已经看不清我定义些啥了(doge)
	if(debug == 2) printf("<Debug>\nHello! Administrator Ltabsyy or Jws!\n");
	/*读取缓存*/
	records = ReadRecords();//读取历史记录
	if(debug == 2) PrintRecords(records);
	if((file = fopen("minesweeper-settings.txt", "r")))//读取设置
	{//内嵌括号减少warning，其实可以不加(doge)
		fscanf(file, "Map:%d*%d-%d\n", &heightOfBoard, &widthOfBoard, &numberOfMine);
		fscanf(file, "summonCheckMode=%d\n", &summonCheckMode);
		fscanf(file, "showTime=%d\n", &showTime);
		fscanf(file, "show3BV=%d\n", &show3BV);
		fscanf(file, "backgroundColor=0x%x\n", &backgroundColor);
		fscanf(file, "operateMode=%d\n", &operateMode);
		fscanf(file, "solveMode=%d\n", &solveMode);
		fscanf(file, "lengthOfThinkChain=%d\n", &lengthOfThinkChain);
		fscanf(file, "refreshCycle=%d\n", &refreshCycle);
		fscanf(file, "fastSign=%d\n", &fastSign);
		fscanf(file, "newCursor=%d\n", &newCursor);
		fscanf(file, "visibleCursor=%d\n", &visibleCursor);
		fscanf(file, "clickSpace=%d\n", &clickSpace);
		fscanf(file, "dynamicMemory=%d\n", &dynamicMemory);
		fscanf(file, "benchSeedMin=%d\n", &benchSeedMin);
		fscanf(file, "benchSeedMax=%d\n", &benchSeedMax);
		fscanf(file, "benchShowStep=%d\n", &benchShowStep);
		fscanf(file, "benchShowSolution=%d\n", &benchShowSolution);
		fscanf(file, "cursorLeft=%c\n", &cursorLeft[0]);
		fscanf(file, "cursorRight=%c\n", &cursorRight[0]);
		fclose(file);
		if(benchShowStep == 3 && debug != 2) benchShowStep = 2;
		if(debug == 2)
		{
			printf("[Debug]已获取设置：\n");
			printf("Map:%d*%d-%d\n", heightOfBoard, widthOfBoard, numberOfMine);
			printf("summonCheckMode=%d\n", summonCheckMode);
			printf("showTime=%d\n", showTime);
			printf("show3BV=%d\n", show3BV);
			printf("backgroundColor=0x%x\n", backgroundColor);
			printf("operateMode=%d\n", operateMode);
			printf("solveMode=%d\n", solveMode);
			printf("lengthOfThinkChain=%d\n", lengthOfThinkChain);
			printf("refreshCycle=%d\n", refreshCycle);
			printf("fastSign=%d\n", fastSign);
			printf("newCursor=%d\n", newCursor);
			printf("visibleCursor=%d\n", visibleCursor);
			printf("clickSpace=%d\n", clickSpace);
			printf("dynamicMemory=%d\n", dynamicMemory);
			printf("benchSeedMin=%d\n", benchSeedMin);
			printf("benchSeedMax=%d\n", benchSeedMax);
			printf("benchShowStep=%d\n", benchShowStep);
			printf("benchShowSolution=%d\n", benchShowSolution);
			printf("cursorLeft=%c\n", cursorLeft[0]);
			printf("cursorRight=%c\n", cursorRight[0]);
		}
	}
	if((file = fopen("minesweeper-lastmap.txt", "r")))//读取上一次地图
	{
		lastMap = 1;
		choiceMode = 1;
		fscanf(file, "Map:%d*%d-%d\n", &heightOfBoard, &widthOfBoard, &numberOfMine);
		fscanf(file, "seed=%d,%d,%d\n", &seed, &r0, &c0);
		fscanf(file, "time=%d\n", &t2);
		fscanf(file, "pos=(%d,%d)\n", &ro, &co);
		fscanf(file, "isHelped=%d\n", &isHelped);
		if(dynamicMemory == 1)//提前申请内存空间读入显示方式矩阵
		{
			ReallocMemory(heightOfBoard, widthOfBoard, 1);
		}
		else
		{
			ReallocMemory(LimHeight, LimWidth, 1);
		}
		for(r=0; r<heightOfBoard; r++)
		{
			fscanf(file, "  ");
			for(c=0; c<widthOfBoard; c++)
			{
				fscanf(file, "%d ", &isShown[r][c]);
			}
			fscanf(file, "\n");
		}
		fclose(file);
		if(debug == 2)
		{
			printf("[Debug]已获取上一次地图：\n");
			printf("Map:%d*%d-%d\n", heightOfBoard, widthOfBoard, numberOfMine);
			printf("seed=%d,%d,%d\n", seed, r0, c0);
			printf("time=%d\n", t2);
			printf("pos=(%d,%d)\n", ro, co);
			printf("isHelped=%d\n", isHelped);
			printf("[Debug]显示方式矩阵：\n");
			for(r=0; r<heightOfBoard; r++)
			{
				printf("  ");
				for(c=0; c<widthOfBoard; c++)
				{
					printf("%d ", isShown[r][c]);
				}
				printf("\n");
			}
		}
	}
	/*初始设置*/
	SetConsoleTitle("MineSweeper Console");
	if(backgroundColor != 0x07) setbgcolor(backgroundColor);//默认浅色模式
	if(visibleCursor == 0) showCursor(0);
	difficulty = Difficulty(heightOfBoard, widthOfBoard, numberOfMine);
	if(debug == 2)
	{
		printf("[Debug]当前难度：%d\n", difficulty);
		system("pause");
	}
	if(lastMap == 0)
	{
		if(dynamicMemory == 1)//申请内存空间
		{
			ReallocMemory(heightOfBoard, widthOfBoard, 1);
		}
		else
		{
			ReallocMemory(LimHeight, LimWidth, 1);
		}
	}
	SetConsoleMouseMode(1);
	/*进入主循环*/
	while(choiceMode != 5)
	{
		//clrscr();//清屏
		/*printf("*******************************\n"
			   "**新游戏(1)**设置(2)**退出(3)**\n"
			   "*******************************\n");*/
		/*printf("*******************************\n");//宽31
		if(lastMap == 1) printf("(0)继续上一次游戏\n");
		printf("(1)新游戏\n"
			   "(2)地图求解\n"
			   "(3)设置\n"
			   "(4)Bench\n"
			   "(5)退出\n"
			   "*******************************\n");*/
		DrawControlBar(0);
		rs0 = Place(widthOfBoard-1);//行起始指针，为列坐标显示占用行数
		cs0 = Place(heightOfBoard-1) + 1;//列起始指针，为行坐标显示占用列数，含空格
		heightOfMapShown = heightOfBoard + rs0;//计算包含坐标的行数
		widthOfMapShown = 2*widthOfBoard + cs0 + 1;//计算包含坐标的每行字符数加回车
		//printf(">");
		FlushConsoleInputBuffer(hdin);//Bench抗双击
		while(choiceMode == 0)
		{
			ReadConsoleInput(hdin, &rcd, 1, &rcdnum);
			if(rcd.EventType == MOUSE_EVENT)
			{
				mousePos = rcd.Event.MouseEvent.dwMousePosition;
				choiceTemp = ChoiceControlBar(mousePos, rcd.Event.MouseEvent.dwButtonState);
				if(choiceTemp != 0) choiceMode = choiceTemp;
			}
			else if(rcd.EventType == KEY_EVENT && rcd.Event.KeyEvent.bKeyDown == 1)
			{
				if(rcd.Event.KeyEvent.wVirtualKeyCode >= '1' && rcd.Event.KeyEvent.wVirtualKeyCode <= '9')
				{
					choiceMode = rcd.Event.KeyEvent.wVirtualKeyCode-'1'+1;//主键盘1-9
				}
				else if(rcd.Event.KeyEvent.wVirtualKeyCode >= 'a' && rcd.Event.KeyEvent.wVirtualKeyCode <= 'i')
				{
					choiceMode = rcd.Event.KeyEvent.wVirtualKeyCode-'a'+1;//小键盘1-9
				}
			}
			Sleep(refreshCycle);
		}
		if(choiceMode == 1)//游戏
		{
			/*重置*/
			clrscr();
			DrawControlBar(0);
			remainder = numberOfMine;
			if(lastMap == 1)
			{
				for(r=0; r<heightOfBoard; r++)
				{
					for(c=0; c<widthOfBoard; c++)
					{
						if(isShown[r][c] == 2) remainder--;
					}
				}
			}
			else//无上一次地图或自制地图
			{
				t2 = 0;
				isOpenMine = 0;
				isHelped = 0;
				if(lastMap == 2) isHelped = 1;
				for(r=0; r<heightOfBoard; r++)//确保第一次显示为全未翻开
				{
					for(c=0; c<widthOfBoard; c++)
					{
						isShown[r][c] = 0;
					}
				}
			}
			/*获取种子和生成位置*/
			/*if(operateMode == 3)
			{
				//创建窗口
				setcaption("MineSweeper Window");
				initgraph(widthOfBlock*widthOfBoard, heightOfBar+heightOfBlock*heightOfBoard, INIT_RENDERMANUAL);
				setbkcolor(LIGHTGRAY);
				setfont(heightOfChar, 0, "Consolas");
				//ege_enable_aa(true);
			}*/
			if(lastMap == 0)
			{
				ShowBoard(0);
				printf("选择坐标[r:行][c:列]\n");
				printf(">@ ");
				if(operateMode != 2) SetConsoleMouseMode(0);
				if(operateMode == 0)
				{
					//scanf("%d%d", &r0, &c0);//通过键盘获取第一次选择的坐标
					operation = 0;
					r0 = 0;//该坐标会陆续穿过RCScan,FreshCursor,ReShowBoard,ShowBlock造成越界访问，6
					c0 = 0;
					while(operation == 0)
					{
						operation = '@';//防止@被删除
						newRecord.summonCheckMode = 0;//不刷新游戏信息
						RCScan(&operation, &r0, &c0, heightOfMapShown-1+3+2, newRecord);
						//RCScan(&operation, &r0, &c0, heightOfMapShown-1+3+2, 0);
					}
				}
				else if(operateMode == 1 || operateMode == 2)
				{
					r0 = heightOfBoard/2;
					c0 = widthOfBoard/2;
					while(choiceMode == 1)
					{
						ReadConsoleInput(hdin, &rcd, 1, &rcdnum);
						if(rcd.EventType == MOUSE_EVENT && operateMode == 2)
						{
							mousePos = rcd.Event.MouseEvent.dwMousePosition;
							choiceTemp = ChoiceControlBar(mousePos, rcd.Event.MouseEvent.dwButtonState);
							if(choiceTemp != 0) choiceMode = choiceTemp;
							if(IsPosInRectangle((mousePos.X-cs0)/2, mousePos.Y-rs0-3, 0, 0, widthOfBoard-1, heightOfBoard-1))
							{
								r0 = mousePos.Y-rs0-3;
								c0 = (mousePos.X-cs0)/2;
								if(rcd.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
								{
									break;
								}
							}
						}
						else if(rcd.EventType == KEY_EVENT && rcd.Event.KeyEvent.bKeyDown == 1)
						{
							if(WASDMove(&r0, &c0, rcd.Event.KeyEvent.wVirtualKeyCode));
							else if(rcd.Event.KeyEvent.wVirtualKeyCode >= '3' && rcd.Event.KeyEvent.wVirtualKeyCode <= '5')
							{
								choiceMode = rcd.Event.KeyEvent.wVirtualKeyCode-'1'+1;//主键盘3-5
							}
							else if(rcd.Event.KeyEvent.wVirtualKeyCode >= 'c' && rcd.Event.KeyEvent.wVirtualKeyCode <= 'e')
							{
								choiceMode = rcd.Event.KeyEvent.wVirtualKeyCode-'a'+1;//小键盘3-5
							}
							else
							{
								break;
							}
						}
						gotoxy(0, heightOfMapShown+4);
						printf(">@ %d %d ", r0, c0);//覆写尾部
						FreshCursor(r0, c0, 3);
						//gotoxy(cs0+2*c0, heightOfMapShown+2-heightOfBoard+1+r0);
						Sleep(refreshCycle);
					}
					gotoxy(0, heightOfMapShown+5);
				}
				/*else if(operateMode == 3)
				{
					r0 = -1;
					while(r0 == -1)
					{
						DrawBoard(0, numberOfMine, 0);
						while(mousemsg())
						{
							mouseMsg = getmouse();
							if(mouseMsg.is_left() && mouseMsg.is_up())//鼠标左键抬起
							{
								mousepos(&xm, &ym);
								r0 = (ym-heightOfBar)/heightOfBlock;
								c0 = xm/widthOfBlock;
							}
						}
						delay_ms(refreshCycle);
					}
				}*/
				seed = time(0);//当前时间戳作种子生成随机数
				if(debug == 2)
				{
					temp = 0;
					printf("[Debug]seed=%d,%d,%d\n", seed, r0, c0);
					SetConsoleMouseMode(0);
					printf("[0:生成/1:更改]\n>");
					scanf("%d", &temp);
					if(temp == 1)
					{
						printf("[seed] [r0] [c0]\n>");
						scanf("%d%d%d", &seed, &r0, &c0);
					}
				}
			}
			/*生成地图*/
			if(lastMap == 0 && summonCheckMode > 2)//可解地图生成
			{
				if(summonCheckMode == 4)//生成可解速通地图
				{
					isOpenMine = 1;
					while(isOpenMine == 1)
					{
						bbbv = BBBV(seed, r0, c0, 1);
						if((difficulty == 0 && bbbv < 9)//默认和初级3BV为2，仅需1次点击
							|| (difficulty == 1 && bbbv < 10)
							|| (difficulty == 2 && bbbv < 50)//中级3BV<50
							|| (difficulty == 3 && bbbv < 150)//高级3BV<150
							|| (difficulty == 4 && bbbv < 1240))//顶级3BV<1240
						{
							isOpenMine = 1-IsSolvableMap(seed, r0, c0);
						}
						if(isOpenMine == 1) seed++;
					}
				}
				else
				{
					while(IsSolvableMap(seed, r0, c0) == 0) seed++;//可解地图保持种子不变
				}
				if(debug == 1 || debug == 2) system("pause");
			}
			else if(lastMap != 2)
			{
				ShownModeBak(1);//有上一次地图时显示方式矩阵不清零
				SummonBoard(seed, r0, c0);
				ShownModeBak(0);
			}
			/*准备开始游戏*/
			if(lastMap == 0)
			{
				ro = r0;
				co = c0;
			}
			if(lastMap == 2)
			{
				bbbv = BBBV(seed, r0, c0, 0);//自制地图防止BBBV生成地图
			}
			else
			{
				bbbv = BBBV(seed, r0, c0, 1);//地图3BV仅计算一次
				isShown[r0][c0] = 1;
				r = r0;
				c = c0;//翻开第一个0
			}
			if(operateMode == 2) SetConsoleMouseMode(1);//确保win10控制台接收鼠标信息
			else SetConsoleMouseMode(0);
			FlushConsoleInputBuffer(hdin);//清除打开地图前的多次点击
			showCursor(visibleCursor);//防止调整窗口大小导致控制台光标显示
			t0 = time(0);
			clrscr();
			DrawControlBar(0);
			/*游戏循环*/
			while(choiceMode == 1)
			{
				/*计算*/
				if(isShown[r][c] == 1)//对翻开操作做出反应
				{
					if(board[r][c] == 0)//翻开0连锁翻开
					{
						OpenZeroChain(r, c);
					}
					else if(board[r][c] == 9)//寄
					{
						clrscr();
						DrawControlBar(0);
						ShowBoard(1);
						printf(":(\nGame Over!\n");
						t1 = time(0);
						temp = BBBV(seed, r0, c0, 0);//temp暂存未解3BV
						if(showTime == 1) printf("用时：%d ", t1-t0+t2);
						if(show3BV == 1) printf("3BV：%d/%d 3BV/s：%.2f ", bbbv-temp, bbbv, (float)(bbbv-temp)/(t1-t0+t2));
						printf("\n");
						remove("minesweeper-lastmap.txt");
						lastMap = 0;
						SetConsoleMouseMode(1);
						choiceMode = 0;
						break;
					}
				}
				if(RealRemainder() == 0//正确标记所有雷则胜利
					|| NumberOfNotShown() == remainder)//未翻开的都是雷则胜利
				{
					if(NumberOfSign() > numberOfMine)//标记量不能超过雷数
					{
						gotoxy(0, yOfMapEnd+5);
						printf(":(\n标记过多！请重新标记。\n");
						if(operateMode == 2 && operation == '#') Sleep(refreshCycle);//防止鼠标出现右键菜单
						//system("pause");
						/*for(r=0; r<heightOfBoard; r++)
						{
							for(c=0; c<widthOfBoard; c++)
							{
								if(isShown[r][c] == 2)
								{
									isShown[r][c] = 0;
									remainder++;
								}
							}
						}*/
						if(isShown[r][c] == 2)
						{
							isShown[r][c] = 0;
							remainder++;
						}
						continue;
					}
					clrscr();
					DrawControlBar(0);
					ShowBoard(1);
					printf(":)\nYou Win!\n");
					remainder = 0;
					t1 = time(0);
					temp = 0;
					if(showTime == 1) printf("用时：%d ", t1-t0+t2);
					if(show3BV == 1) printf("3BV：%d 3BV/s：%.2f ", bbbv, (float)bbbv/(t1-t0+t2));
					printf("\n");
					remove("minesweeper-lastmap.txt");
					lastMap = 0;
					SetConsoleMouseMode(1);
					choiceMode = 0;
					break;
				}
				/*显示*/
				if(heightOfBoard > 40)
				{
					gotoxy(0, 0);//地图高度大于40时不显示控制栏
					yOfMapEnd = heightOfMapShown-1;
				}
				else
				{
					DrawControlBar(0);
					//gotoxy(0, 3);
					yOfMapEnd = heightOfMapShown-1+3;
				}
				if(debug == 1 || debug == 2)
				{
					//clrscr();
					//if(heightOfBoard > 10) show3BV = 0;
					//DrawControlBar(0);
					printf("<Debug>\n");
					printf("Map:%d*%d-%d\n", heightOfBoard, widthOfBoard, numberOfMine);
					printf("seed=%d,%d,%d\n", seed, r0, c0);
					ShowBoard(1);
					printf("RealRemainder=%d \n", RealRemainder());
					printf("NumberOfNotShown=%d \n", NumberOfNotShown());
					printf("\n");
					yOfMapEnd += heightOfMapShown+6;
				}
				ShowBoard(0);
				//打印剩余雷数，用时，3BV
				//gotoxy(0, yOfMapEnd+1);
				printf("剩余雷数: %d ", remainder);
				t1 = time(0);
				temp = BBBV(seed, r0, c0, 0);
				if(showTime == 1) printf("用时：%d ", t1-t0+t2);
				if(show3BV == 1) printf("3BV：%d/%d 3BV/s：%.2f ", bbbv-temp, bbbv, (float)(bbbv-temp)/(t1-t0+t2));
				printf("\n");
				//显示当前操作模式提示
				printf("选择模式与坐标\n");
				if(operateMode == 0) printf("[@:翻开/#:标记] [r] [c]  \n");//宽22+2
				if(operateMode == 1) printf("[wasd:移动/2:翻开/3:标记]\n");//宽24
				if(operateMode == 2)
				{
					if(touchMode == 1)
					{
						printf("[鼠标左键:标记/右键:翻开]\n");//宽24
					}
					else
					{
						printf("[鼠标左键:翻开/右键:标记]\n");//宽24
					}
				}
				if(operateMode == 3) printf("[请在MineSweeper Window进行游戏]\n");//宽31
				/*备份至文件*/
				if(lastMap != 2)//自制地图不备份
				{
					file = fopen("minesweeper-lastmap.txt", "w");
					fprintf(file, "Map:%d*%d-%d\n", heightOfBoard, widthOfBoard, numberOfMine);
					fprintf(file, "seed=%d,%d,%d\n", seed, r0, c0);
					fprintf(file, "time=%d\n", t1-t0+t2);
					fprintf(file, "pos=(%d,%d)\n", ro, co);
					fprintf(file, "isHelped=%d\n", isHelped);
					for(r=0; r<heightOfBoard; r++)
					{
						fprintf(file, "  ");
						for(c=0; c<widthOfBoard; c++)
						{
							fprintf(file, "%d ", isShown[r][c]);
						}
						fprintf(file, "\n");
					}
					fclose(file);
				}
				/*输入和显示用时*/
				printf(">");
				operation = 0;
				r = ro;
				c = co;
				while(choiceMode == 1)
				{
					/*if(operateMode == 3)
					{
						DrawBoard(0, remainder, t1-t0+t2);
						while(mousemsg())
						{
							mouseMsg = getmouse();
							if(mouseMsg.is_left() && mouseMsg.is_down())//鼠标左键按下
							{
								mousepos(&xm, &ym);
								operation = '@';
								r = (ym-heightOfBar)/heightOfBlock;
								c = xm/widthOfBlock;
							}
							if(mouseMsg.is_right() && mouseMsg.is_down())//鼠标右键按下
							{
								mousepos(&xm, &ym);
								operation = '#';
								r = (ym-heightOfBar)/heightOfBlock;
								c = xm/widthOfBlock;
							}
						}
						delay_ms(refreshCycle);
					}*/
					if(operateMode == 2)
					{
						isReadyRefreshMouseOperatedPos = 1;
						GetNumberOfConsoleInputEvents(hdin, &rcdnum);//检索在控制台输入缓冲区中未读取的输入事件记录个数
						if(rcdnum == 0)//无输入时刷新
						{
							gotoxy(0, yOfMapEnd+1);
							printf("剩余雷数: %d ", remainder);
							t1 = time(0);
							//temp = BBBV(seed, r0, c0, 0);
							if(showTime == 1) printf("用时：%d ", t1-t0+t2);
							if(show3BV == 1) printf("3BV：%d/%d 3BV/s：%.2f ", bbbv-temp, bbbv, (float)(bbbv-temp)/(t1-t0+t2));
							//printf("\n");
							//gotoxy(1, yOfMapEnd+4);//操作指令位置
							gotoxy(cs0+2*c, yOfMapEnd-heightOfBoard+1+r);//对应点位置
							Sleep(refreshCycle);
							continue;
						}
						ReadConsoleInput(hdin, &rcd, 1, &rcdnum);
						if(rcd.EventType == MOUSE_EVENT)
						{
							mousePos = rcd.Event.MouseEvent.dwMousePosition;
							if(heightOfBoard > 40)
							{
								//地图高度大于40时不显示控制栏
							}
							else
							{
								choiceTemp = ChoiceControlBar(mousePos, rcd.Event.MouseEvent.dwButtonState);
							}
							if(choiceTemp != 0)
							{
								if(choiceTemp == 1)//停止当前游戏，再次点击进入新游戏
								{
									gotoxy(0, yOfMapEnd+3);
									printf("[已停止游戏，按1进入新游戏]\n");//宽26
									remove("minesweeper-lastmap.txt");
									lastMap = 0;
									choiceMode = 0;
								}
								else if(choiceTemp == 2)//执行实时求解指令!
								{
									operation = '!';
								}
								else if(choiceTemp == 4)//执行实时求解指令Tab
								{
									operation = '\t';
								}
								else if(choiceTemp == 5)
								{
									gotoxy(0, yOfMapEnd+3);
									printf("[已暂停游戏，按1继续游戏]\n");//宽24
									lastMap = 1;
									choiceMode = 5;
								}
								else//choiceTemp == 3，进入设置
								{
									choiceMode = choiceTemp;
								}
							}
							//X=cs0+2*c, Y=yOfMapEnd-heightOfBoard+1+r
							//r=Y-yOfMapEnd+heightOfBoard-1, c=(X-cs0)/2
							if(IsPosInRectangle((mousePos.X-cs0)/2, mousePos.Y-yOfMapEnd+heightOfBoard-1, 0, 0, widthOfBoard-1, heightOfBoard-1))
							{
								if(clickSpace == 0 && (mousePos.X-cs0)%2 == 1) continue;//屏蔽空格
								r = mousePos.Y-yOfMapEnd+heightOfBoard-1;
								c = (mousePos.X-cs0)/2;
								if((rcd.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED && touchMode == 0)
									|| (rcd.Event.MouseEvent.dwButtonState == RIGHTMOST_BUTTON_PRESSED && touchMode == 1))
								{
									//if(mousePos.X == mouseOperatedPos.X && mousePos.Y == mouseOperatedPos.Y)
									if(IsPosInRectangle(mousePos.X, mousePos.Y, mouseOperatedPos.X-1, mouseOperatedPos.Y, mouseOperatedPos.X+1, mouseOperatedPos.Y))
									{
										//屏蔽翻开时鼠标轻微移动产生的双击
										//存在标记错误时可一定程度防止用户手快翻开周围
										//刷新间隔足够仍可标记后立即翻开
									}
									else
									{
										operation = '@';
										mouseOperatedPos = mousePos;
										break;
									}
									isReadyRefreshMouseOperatedPos = 0;
								}
								if((rcd.Event.MouseEvent.dwButtonState == RIGHTMOST_BUTTON_PRESSED && touchMode == 0)
									|| (rcd.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED && touchMode == 1))
								{
									//if(mousePos.X == mouseOperatedPos.X && mousePos.Y == mouseOperatedPos.Y)
									if(IsPosInRectangle(mousePos.X, mousePos.Y, mouseOperatedPos.X-1, mouseOperatedPos.Y, mouseOperatedPos.X+1, mouseOperatedPos.Y))
									{
										//屏蔽标记时鼠标轻微移动产生的双击
									}
									else
									{
										operation = '#';
										mouseOperatedPos = mousePos;
										break;
									}
									isReadyRefreshMouseOperatedPos = 0;
								}
							}
							else if(IsPosInRectangle(mousePos.X, mousePos.Y, 0, yOfMapEnd+3, 24, yOfMapEnd+3))
							{
								if(rcd.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
								{
									//点击[鼠标左键:翻开/右键:标记]进行左右键切换
									if(touchMode == 0) touchMode = 1;
									else touchMode = 0;
									break;//更新按钮[鼠标左键:标记/右键:翻开]
								}
							}
						}
						else if(rcd.EventType == KEY_EVENT && rcd.Event.KeyEvent.bKeyDown == 1)//按下而非松开键盘时
						{
							if(rcd.Event.KeyEvent.wVirtualKeyCode == '1')//!
							{
								//operation = '!';//按1代替!(Shift+1)
								if(GetKeyState(VK_SHIFT) < 0)
								{
									operation = '!';
								}
							}
							if(rcd.Event.KeyEvent.wVirtualKeyCode == '\t')//VK_TAB
							{
								operation = '\t';
							}
							if(WASDMove(&r, &c, rcd.Event.KeyEvent.wVirtualKeyCode));
							else if(rcd.Event.KeyEvent.wVirtualKeyCode == '2'//主键盘2
								|| rcd.Event.KeyEvent.wVirtualKeyCode == 'b'//小键盘2
								|| rcd.Event.KeyEvent.wVirtualKeyCode == 'H')//主键盘H/h
							{
								operation = '@';
							}
							else if(rcd.Event.KeyEvent.wVirtualKeyCode == '3'//主键盘3
								|| rcd.Event.KeyEvent.wVirtualKeyCode == 'c'//小键盘3
								|| rcd.Event.KeyEvent.wVirtualKeyCode == 'U')//主键盘U/u
							{
								operation = '#';
							}
							else if(rcd.Event.KeyEvent.wVirtualKeyCode == 'K'
								|| rcd.Event.KeyEvent.wVirtualKeyCode == 'C')
							{
								gotoxy(0, yOfMapEnd+5);
								if(debug == 1 || debug == 2)
								{
									printf("正在切换到键盘操作...\n");
									system("pause");
									clrscr();
								}
								SetConsoleMouseMode(0);
								if(rcd.Event.KeyEvent.wVirtualKeyCode == 'C') operateMode = 0;
								else operateMode = 1;
								break;
							}
						}
						//刷新防双击坐标
						if(isReadyRefreshMouseOperatedPos == 1)
						{
							mouseOperatedPos.X = 0;
							mouseOperatedPos.Y = 0;
						}
					}
					if(operateMode < 2 && kbhit())//按键
					{
						key = getch();//接收字符但不显示
						if(key == '`')//清空已输入操作
						{
							operation = 0;
							r = ro;
							c = co;
						}
						if(key == '!' || key == '\t')
						{
							operation = key;
						}
						//操作模式切换
						if((key == '@' || key == '#' || key == '$') && operateMode == 1)
						{
							gotoxy(0, yOfMapEnd+4);
							printf(">        ");//覆写@ 128 93
							if(debug == 1 || debug == 2)
							{
								printf("\n:(\n检测到@#rc操作指令！正在识别...\n");
								system("pause");
								clrscr();
							}
							gotoxy(1, yOfMapEnd+4);
							operateMode = 0;
							break;//立即更新操作提示
						}
						if((key == 'w' || key == 'W' || key == 'a' || key == 'A' || key == 's' || key == 'S'
							|| key == 'd' || key == 'D') && operateMode == 0)
						{
							if(debug == 1 || debug == 2)
							{
								gotoxy(0, yOfMapEnd+4);
								printf("\n:(\n检测到wasd移动操作！正在识别...\n");
								system("pause");
								clrscr();
							}
							operateMode = 1;
							break;//立即更新操作提示
						}
						if((key == 'm' || key == 'M') && operateMode != 2)
						{
							gotoxy(0, yOfMapEnd+5);
							if(debug == 1 || debug == 2)
							{
								printf("正在切换到鼠标操作...\n");
								system("pause");
								clrscr();
							}
							SetConsoleMouseMode(1);
							operateMode = 2;
							break;
						}
						if((key == 'c' || key == 'C') && operateMode != 0)
						{
							operateMode = 0;
							break;
						}
						//操作识别
						if(operateMode == 0)
						{
							if(key == '@' || key == '#' || key == '$')
							{
								operation = key;
								//gotoxy(0, yOfMapEnd+4);
								//printf(">%c ", operation);
								//scanf("%d%d", &r, &c);
								newRecord.numberOfMine = remainder;
								newRecord.time = t0-t2;//t1-t0+t2
								newRecord.solved3BV = bbbv-temp;
								newRecord.total3BV = bbbv;//打包信息至RCScan
								newRecord.summonCheckMode = 1;
								RCScan(&operation, &r, &c, yOfMapEnd+4, newRecord);
								//RCScan(&operation, &r, &c, yOfMapEnd+4, 1);
								if(operation == 'm')
								{
									gotoxy(0, yOfMapEnd+5);
									if(debug == 1 || debug == 2)
									{
										printf("正在切换到鼠标操作...\n");
										system("pause");
										clrscr();
									}
									SetConsoleMouseMode(1);
									operateMode = 2;
								}
								break;
							}
						}
						if(operateMode == 1)//wasd操作设计
						{
							if((key == 'w' || key == 'W') && r > 0) r--;//上移
							if((key == 'a' || key == 'A') && c > 0) c--;//左移
							if((key == 's' || key == 'S') && r < heightOfBoard-1) r++;//下移
							if((key == 'd' || key == 'D') && c < widthOfBoard-1) c++;//下移
							if(key == '2' || key == 'h' || key == 'H') operation = '@';//2翻开
							if(key == '3' || key == 'u' || key == 'U') operation = '#';//3标记
							//if(operation != 0) break;
						}
					}
					//实时求解指令
					if(operation == '!' || operation == '\t')
					{
						ShownModeBak(1);
						LookMap();
						gotoxy(0, yOfMapEnd+5);//输出debug信息
						Solve();//执行当前可知所有Solution
						printf("                     ");//覆写[Loading...]8192/8192，21空格
						//yOfMap=yOfMapEnd-heightOfMapShown+1
						ShowSolution(yOfMapEnd-heightOfMapShown+1);
						isHelped = 1;//记录使用过实时求解指令，不参与纪录计算
						if(operation == '!')
						{
							ShownModeBak(0);
							operation = 0;
							r = ro;
							c = co;
						}
						else if(operation == '\t')
						{
							Sleep(refreshCycle);
							for(r=0; r<heightOfBoard; r++)
							{
								for(c=0; c<widthOfBoard; c++)
								{
									if(isShown[r][c] == 1)
									{
										if(board[r][c] == 0)//翻开0连锁翻开
										{
											OpenZeroChain(r,c);
										}
										//!指令的Solve()不会翻开雷
									}
									if(solution[r][c] == 2)
									{
										remainder--;
									}
								}
							}
							r = ro;
							c = co;
							break;
						}
					}
					//刷新
					gotoxy(0, yOfMapEnd+1);
					printf("剩余雷数: %d ", remainder);//从行首写防止文字重影
					t1 = time(0);//刷新用时
					//temp = BBBV(seed, r0, c0, 0);//刷新3BV
					if(showTime == 1) printf("用时：%d ", t1-t0+t2);
					if(show3BV == 1) printf("3BV：%d/%d 3BV/s：%.2f ", bbbv-temp, bbbv, (float)(bbbv-temp)/(t1-t0+t2));
					//printf("\n");
					if(operateMode > 0)
					{
						//显示操作对应指令
						gotoxy(0, yOfMapEnd+4);
						if(operation == 0) printf(">? %d %d  ", r, c);//覆写尾部，(10,10)到(9,9)需2个空格
						else printf(">%c %d %d", operation, r, c);
						//刷新光标
						FreshCursor(r, c, yOfMapEnd-heightOfMapShown+1);
						if(operation != 0)
						{
							Sleep(refreshCycle);
							gotoxy(0, yOfMapEnd+5);
							break;
						}
					}
					Sleep(refreshCycle);//每50ms刷新一次
				}
				ro = r;
				co = c;
				/*执行操作*/
				if(operation == '@')
				{
					if(isShown[r][c] == 1)//翻开数字则尝试翻开周围
					{
						if(NumberOfSignAround(r, c) == numberOfMineAround[r][c])
						{
							for(ra=r-1; ra<=r+1; ra++)
							{
								for(ca=c-1; ca<=c+1; ca++)
								{
									if(ra>=0 && ra<heightOfBoard && ca>=0 && ca<widthOfBoard)//确认在范围内
									{
										if(isShown[ra][ca] == 0)//翻开所有%，如果标错则%中有雷
										{
											isShown[ra][ca] = 1;
											if(board[ra][ca] == 0)//翻开0连锁翻开
											{
												OpenZeroChain(ra, ca);
											}
											else if(board[ra][ca] == 9)//寄
											{
												isOpenMine = 1;
												r = ra;
												c = ca;
												break;//维持坐标退出
											}
										}
									}
								}
								if(isOpenMine == 1) break;
							}
						}
					}
					else if(isShown[r][c] == 2)
					{
						remainder++;//取消标记，剩余雷数+1
						isShown[r][c] = 1;
					}
					else
					{
						isShown[r][c] = 1;//翻开
					}
				}
				else if(operation == '#')
				{
					if(isShown[r][c] == 0)//标记
					{
						isShown[r][c] = 2;
						remainder--;
					}
					else if(isShown[r][c] == 2)//取消标记
					{
						isShown[r][c] = 0;
						remainder++;
					}
					else if(isShown[r][c] == 1)//#数字时快速标记周围
					{
						if(fastSign == 1)
						{
							if(numberOfMineAround[r][c] == NumberOfNotShownAround(r, c) + NumberOfSignAround(r, c))
							{
								for(ra=r-1; ra<=r+1; ra++)//周围(%>0且)%+#=数字时
								{
									for(ca=c-1; ca<=c+1; ca++)
									{
										if(ra>=0 && ra<heightOfBoard && ca>=0 && ca<widthOfBoard)//确认在范围内
										{
											if(isShown[ra][ca] == 0)//标记所有%
											{
												isShown[ra][ca] = 2;
												remainder--;
											}
										}
									}
								}
							}
							else if(NumberOfNotShownAround(r, c) == 0 || NumberOfSignAround(r, c) > numberOfMineAround[r][c])
							{
								for(ra=r-1; ra<=r+1; ra++)//周围方块全被标记时，%=0(且#>0)，周围标记数大于数字时
								{
									for(ca=c-1; ca<=c+1; ca++)
									{
										if(ra>=0 && ra<heightOfBoard && ca>=0 && ca<widthOfBoard)//确认在范围内
										{
											if(isShown[ra][ca] == 2)//一键取消标记
											{
												isShown[ra][ca] = 0;
												remainder++;
											}
										}
									}
								}
							}
						}
						else
						{
							//printf(":(\n该坐标已翻开！\n");
							//system("pause");
						}
					}
				}
				else if(operation == '$')
				{
					if(debug == r)
					{
						debug = c;
						if(debug == 2)
						{
							printf("<Debug>\nWelcome! Administrator Ltabsyy or Jws!\n");
						}
						system("pause");
						clrscr();
					}
				}
				/*else
				{
					printf(":(\n未选择操作模式！\n");
				}*/
			}
			/*游戏结束*/
			if(choiceMode == 5)//暂停
			{
				t2 = t1-t0+t2;//更新时间
				choiceMode = 0;//返回主页
			}
			else if(choiceMode == 0)//完成地图退出
			{
				newRecord.numberOfMine = numberOfMine;
				newRecord.heightOfBoard = heightOfBoard;
				newRecord.widthOfBoard = widthOfBoard;
				newRecord.summonCheckMode = summonCheckMode;
				newRecord.seed = seed;
				newRecord.r0 = r0;
				newRecord.c0 = c0;
				newRecord.time = t1-t0+t2;
				newRecord.solved3BV = bbbv-temp;
				newRecord.total3BV = bbbv;
				newRecord.isHelped = isHelped;
				newRecord.speed =(float) newRecord.solved3BV / newRecord.time;
				newRecord.difficulty = difficulty;
				records = AddRecord(records, newRecord);
				WriteRecords(records);
				/*if(operateMode == 3)
				{
					DrawBoard(1, remainder, t1-t0+t2);
					setcolor(RED);
					setfontbkcolor(LIGHTGRAY);
					xyprintf(widthOfBlock, heightOfBar, "请按键盘任意键关闭窗口");
					xyprintf(widthOfBlock, heightOfBar+heightOfBlock, "请勿按右上角退出！");
					getch();
					//delay_ms(2000);
					closegraph();
				}*/
			}
		}
		else if(choiceMode == 2)//地图求解
		{
			clrscr();
			/*DrawControlBar(0);
			gotoxy(0, 1);
			printf("** 键盘模式鼠标全选含坐标地图，复制粘贴并回车|按 ` 回车退出 **\n");
			gotoxy(0, 3);*/
			SetConsoleMouseMode(0);
			Solution();//Solution模块
			clrscr();
			DrawControlBar(0);
			SetConsoleMouseMode(1);
			choiceMode = 0;
		}
		else if(choiceMode == 3)//设置
		{
			clrscr();
			DrawControlBar(0);
			DrawSettingsBar(0);
			//FlushConsoleInputBuffer(hdin);//设置游戏难度返回抗双击
			choiceSet = 0;
			while(choiceMode == 3 && choiceSet != 9)
			{
				ReadConsoleInput(hdin, &rcd, 1, &rcdnum);
				if(rcd.EventType == MOUSE_EVENT)
				{
					mousePos = rcd.Event.MouseEvent.dwMousePosition;
					choiceTemp = ChoiceControlBar(mousePos, rcd.Event.MouseEvent.dwButtonState);
					if(choiceTemp != 0) choiceMode = choiceTemp;
					choiceTemp = ChoiceSettingsBar(mousePos, rcd.Event.MouseEvent.dwButtonState);
					if(choiceTemp != 0) choiceSet = choiceTemp;
				}
				else if(rcd.EventType == KEY_EVENT && rcd.Event.KeyEvent.bKeyDown == 1)
				{
					if(rcd.Event.KeyEvent.wVirtualKeyCode >= '1' && rcd.Event.KeyEvent.wVirtualKeyCode <= '9')
					{
						choiceSet = rcd.Event.KeyEvent.wVirtualKeyCode-'1'+1;//主键盘1-9
					}
					else if(rcd.Event.KeyEvent.wVirtualKeyCode >= 'a' && rcd.Event.KeyEvent.wVirtualKeyCode <= 'i')
					{
						choiceSet = rcd.Event.KeyEvent.wVirtualKeyCode-'a'+1;//小键盘1-9
					}
				}
				if(choiceSet == 1)//设置游戏难度
				{
					clrscr();
					DrawSetDifficulty(-1);
					//FlushConsoleInputBuffer(hdin);//设置游戏难度抗双击
					if(dynamicMemory == 1)//申请顶级难度内存空间
					{
						ReallocMemory(heightOfBoard, widthOfBoard, 0);
						ReallocMemory(42, 88, 1);
					}
					choiceTemp = -1;
					while(choiceTemp == -1)
					{
						ReadConsoleInput(hdin, &rcd, 1, &rcdnum);
						if(rcd.EventType == MOUSE_EVENT)
						{
							mousePos = rcd.Event.MouseEvent.dwMousePosition;
							choiceTemp = ChoiceSetDifficulty(mousePos, rcd.Event.MouseEvent.dwButtonState);
							if(choiceTemp != -1) difficulty = choiceTemp;
						}
						else if(rcd.EventType == KEY_EVENT && rcd.Event.KeyEvent.bKeyDown == 1)
						{
							if(rcd.Event.KeyEvent.wVirtualKeyCode >= '0' && rcd.Event.KeyEvent.wVirtualKeyCode <= '5')
							{
								choiceTemp = rcd.Event.KeyEvent.wVirtualKeyCode-'0'+0;//主键盘0-5
								difficulty = choiceTemp;
							}
							else if(rcd.Event.KeyEvent.wVirtualKeyCode == '`')//小键盘0
							{
								choiceTemp = 0;
								difficulty = choiceTemp;
							}
							else if(rcd.Event.KeyEvent.wVirtualKeyCode >= 'a' && rcd.Event.KeyEvent.wVirtualKeyCode <= 'e')
							{
								choiceTemp = rcd.Event.KeyEvent.wVirtualKeyCode-'a'+1;//小键盘1-5
								difficulty = choiceTemp;
							}
						}
						Sleep(refreshCycle);
					}
					if(difficulty == 0)//默认为10*10-10，比初级更简单(doge)
					{
						numberOfMine = 10;//密度0.1
						heightOfBoard = 10;
						widthOfBoard = 10;
						if(dynamicMemory == 1)
						{
							ReallocMemory(42, 88, 0);//释放顶级难度内存空间
							ReallocMemory(10, 10, 1);//申请新难度内存空间
						}
					}
					else if(difficulty == 1)//初级，胜率96%
					{
						numberOfMine = 10;//密度0.12345679
						heightOfBoard = 9;
						widthOfBoard = 9;
						if(dynamicMemory == 1)
						{
							ReallocMemory(42, 88, 0);
							ReallocMemory(9, 9, 1);
						}
					}
					else if(difficulty == 2)//中级，胜率84%
					{
						numberOfMine = 40;//密度0.15625
						heightOfBoard = 16;
						widthOfBoard = 16;
						if(dynamicMemory == 1)
						{
							ReallocMemory(42, 88, 0);
							ReallocMemory(16, 16, 1);
						}
					}
					else if(difficulty == 3)//高级，胜率48%
					{
						numberOfMine = 99;//密度0.20625
						heightOfBoard = 16;
						widthOfBoard = 30;
						if(dynamicMemory == 1)
						{
							ReallocMemory(42, 88, 0);
							ReallocMemory(16, 30, 1);
						}
					}
					else if(difficulty == 4)//顶级，胜率26%
					{
						numberOfMine = 715;//密度0.193452
						heightOfBoard = 42;
						widthOfBoard = 88;//实测较为合适的全屏地图，雷数为胡桃生日
						/*if(dynamicMemory == 1)
						{
							ReallocMemory(42, 88, 0);
							ReallocMemory(42, 88, 1);
						}*/
						//show3BV = 0;//自动隐藏3BV
					}
					else//自定义
					{
						DrawSetDifficulty(7);
						SetConsoleMouseMode(0);
						//设置界面高度
						gotoxy(0, 8);
						printf("[行数]>");
						scanf("%d", &heightOfBoard);
						if(heightOfBoard < 1) heightOfBoard = 1;
						if(heightOfBoard > LimHeight) heightOfBoard = LimHeight;
						gotoxy(13, 6);
						printf("%d", heightOfBoard);
						//设置界面宽度
						gotoxy(0, 9);
						printf("[列数]>");
						scanf("%d", &widthOfBoard);
						if(widthOfBoard < 1) widthOfBoard = 1;
						if(widthOfBoard > LimWidth) widthOfBoard = LimWidth;
						gotoxy(13, 6);
						printf("%d*%d", heightOfBoard, widthOfBoard);
						//界面校正
						/*if(widthOfBoard == LimWidth && heightOfBoard > 100)
						{
							printf(":(\n界面不足以容纳行坐标轴！\n");
							printf("(1)维持当前界面高度，界面宽度-1\n");
							printf("(2)维持当前界面宽度，界面高度设为100\n");
							printf("(3)坚持设置！\n");
							printf(">");
							scanf("%d", &temp);
							if(temp == 1) widthOfBoard--;
							else if(temp == 2) heightOfBoard = 100;
						}*/
						//设置雷数
						gotoxy(0, 10);
						printf("[雷数]>");
						scanf("%d", &numberOfMine);
						if(numberOfMine < 0) numberOfMine = 0;
						if(numberOfMine >= heightOfBoard * widthOfBoard)//调整地图生成校验
						{
							numberOfMine = heightOfBoard * widthOfBoard;//雷数校正
							summonCheckMode = 0;//起始点必为雷
						}
						else if(numberOfMine > heightOfBoard * widthOfBoard * 715/1000
							|| numberOfMine > heightOfBoard * widthOfBoard - 9)
						{
							summonCheckMode = 1;//起始点难以为空
						}
						//更新密度
						/*density =(float) numberOfMine / heightOfBoard / widthOfBoard;
						if(density > 0.72) printf(":(\n当前密度%.2f，难以生成地图\n", density);
						if(density > 0.24)
						{
							printf(":(\n当前密度%.2f，难度可能过高！\n", density);
							printf("(1)更改雷数\n");
							printf("(2)更改密度\n");
							printf("(3)坚持挑战！\n");//坚持访问！(doge)
							printf(">");
							scanf("%d", &temp);
							if(temp == 1)
							{
								printf("/set numberOfMine ");
								scanf("%d", &numberOfMine);
							}
							else if(temp == 2)
							{
								printf(">");
								scanf("%f", &density);
								numberOfMine = density * heightOfBoard * widthOfBoard;
								printf("/set numberOfMine %d\n", numberOfMine);
							}
						}*/
						//重新申请矩阵内存空间
						if(dynamicMemory == 1)
						{
							ReallocMemory(42, 88, 0);
							ReallocMemory(heightOfBoard, widthOfBoard, 1);
						}
						//显示预览
						clrscr();
						DrawSetDifficulty(-1);
						gotoxy(13, 6);
						printf("%d*%d-%d", heightOfBoard, widthOfBoard, numberOfMine);
						SummonBoard(0, heightOfBoard/2, widthOfBoard/2);
						gotoxy(0, 8);
						ShowBoard(1);
						//gotoxy(0, 0);//防止地图过高时界面下滑
						//gotoxy(62, 7);
						system("pause");
						SetConsoleMouseMode(1);
					}
					clrscr();
					DrawControlBar(0);
					DrawSettingsBar(0);
					choiceSet = 0;
				}
				else if(choiceSet == 2)//设置地图生成校验
				{
					choiceTemp = -1;
					DrawSetSummonCheckMode(-1);
					while(choiceMode == 3 && choiceSet == 2 && choiceTemp == -1)
					{
						ReadConsoleInput(hdin, &rcd, 1, &rcdnum);
						if(rcd.EventType == MOUSE_EVENT)
						{
							mousePos = rcd.Event.MouseEvent.dwMousePosition;
							choiceTemp = ChoiceControlBar(mousePos, rcd.Event.MouseEvent.dwButtonState);
							if(choiceTemp != 0) choiceMode = choiceTemp;
							choiceTemp = ChoiceSettingsBar(mousePos, rcd.Event.MouseEvent.dwButtonState);
							if(choiceTemp != 0) choiceSet = choiceTemp;
							choiceTemp = ChoiceSetSummonCheckMode(mousePos, rcd.Event.MouseEvent.dwButtonState);
							if(choiceTemp != -1) summonCheckMode = choiceTemp;
						}
						else if(rcd.EventType == KEY_EVENT && rcd.Event.KeyEvent.bKeyDown == 1)
						{
							if(rcd.Event.KeyEvent.wVirtualKeyCode >= '0' && rcd.Event.KeyEvent.wVirtualKeyCode <= '3')
							{
								choiceTemp = rcd.Event.KeyEvent.wVirtualKeyCode-'0'+0;//主键盘0-3
								summonCheckMode = choiceTemp;
							}
							else if(rcd.Event.KeyEvent.wVirtualKeyCode == '`')//小键盘0
							{
								choiceTemp = 0;
								summonCheckMode = choiceTemp;
							}
							else if(rcd.Event.KeyEvent.wVirtualKeyCode >= 'a' && rcd.Event.KeyEvent.wVirtualKeyCode <= 'c')
							{
								choiceTemp = rcd.Event.KeyEvent.wVirtualKeyCode-'a'+1;//小键盘1-3
								summonCheckMode = choiceTemp;
							}
						}
						Sleep(refreshCycle);
					}
					clrscr();
					DrawControlBar(0);
					DrawSettingsBar(0);
					choiceSet = 0;
				}
				else if(choiceSet == 3)//更改显示用时和3BV
				{
					if(showTime == 0 && show3BV == 0) showTime = 1;
					else if(showTime == 1 && show3BV == 0) show3BV = 1;
					else if(showTime == 1 && show3BV == 1) showTime = 0;
					else if(showTime == 0 && show3BV == 1) show3BV = 0;
					else//默认为showTime == 1 && show3BV == 0
					{
						showTime = 1;
						show3BV = 1;
					}
					clrscr();
					DrawControlBar(0);
					DrawSettingsBar(0);//立即刷新页面
					choiceSet = 0;
				}
				else if(choiceSet == 4)//浅色模式/深色模式
				{
					if(backgroundColor == 0x07)
					{
						//system("color f0");
						backgroundColor = 0xf0;
					}
					else
					{
						//system("color 07");
						backgroundColor = 0x07;
					}
					setbgcolor(backgroundColor);
					clrscr();
					DrawControlBar(0);
					DrawSettingsBar(0);
					choiceSet = 0;
				}
				else if(choiceSet == 5)//设置默认操作模式
				{
					choiceTemp = -1;
					DrawSetOperateMode(-1);
					while(choiceMode == 3 && choiceSet == 5 && choiceTemp == -1)
					{
						ReadConsoleInput(hdin, &rcd, 1, &rcdnum);
						if(rcd.EventType == MOUSE_EVENT)
						{
							mousePos = rcd.Event.MouseEvent.dwMousePosition;
							choiceTemp = ChoiceControlBar(mousePos, rcd.Event.MouseEvent.dwButtonState);
							if(choiceTemp != 0) choiceMode = choiceTemp;
							choiceTemp = ChoiceSettingsBar(mousePos, rcd.Event.MouseEvent.dwButtonState);
							if(choiceTemp != 0) choiceSet = choiceTemp;
							choiceTemp = ChoiceSetOperateMode(mousePos, rcd.Event.MouseEvent.dwButtonState);
							if(choiceTemp != -1) operateMode = choiceTemp;
						}
						else if(rcd.EventType == KEY_EVENT && rcd.Event.KeyEvent.bKeyDown == 1)
						{
							if(rcd.Event.KeyEvent.wVirtualKeyCode >= '0' && rcd.Event.KeyEvent.wVirtualKeyCode <= '3')
							{
								choiceTemp = rcd.Event.KeyEvent.wVirtualKeyCode-'0'+0;//主键盘0-3
								operateMode = choiceTemp;
							}
							else if(rcd.Event.KeyEvent.wVirtualKeyCode == '`')//小键盘0
							{
								choiceTemp = 0;
								operateMode = choiceTemp;
							}
							else if(rcd.Event.KeyEvent.wVirtualKeyCode >= 'a' && rcd.Event.KeyEvent.wVirtualKeyCode <= 'c')
							{
								choiceTemp = rcd.Event.KeyEvent.wVirtualKeyCode-'a'+1;//小键盘1-3
								operateMode = choiceTemp;
							}
						}
						Sleep(refreshCycle);
					}
					/*if(operateMode == 1) visibleCursor = 1;//仅wasd23显示光标 
					else visibleCursor = 0;
					showCursor(visibleCursor);*/
					clrscr();
					DrawControlBar(0);
					DrawSettingsBar(0);
					choiceSet = 0;
				}
				else if(choiceSet == 6)//快速标记（#数字快速标记周围）
				{
					if(fastSign == 0) fastSign = 1;
					else fastSign = 0;
					clrscr();
					DrawControlBar(0);
					DrawSettingsBar(0);
					choiceSet = 0;
				}
				else if(choiceSet == 7)//设置光标
				{
					choiceTemp = 0;
					temp = 0;
					DrawSetCursor(0);
					while(choiceMode == 3 && choiceSet == 7 && choiceTemp == 0)
					{
						ReadConsoleInput(hdin, &rcd, 1, &rcdnum);
						if(rcd.EventType == MOUSE_EVENT)
						{
							mousePos = rcd.Event.MouseEvent.dwMousePosition;
							choiceTemp = ChoiceControlBar(mousePos, rcd.Event.MouseEvent.dwButtonState);
							if(choiceTemp != 0) choiceMode = choiceTemp;
							choiceTemp = ChoiceSettingsBar(mousePos, rcd.Event.MouseEvent.dwButtonState);
							if(choiceTemp != 0) choiceSet = choiceTemp;
							choiceTemp = ChoiceSetCursor(mousePos, rcd.Event.MouseEvent.dwButtonState);
							if(choiceTemp != 0) temp = choiceTemp;
						}
						else if(rcd.EventType == KEY_EVENT && rcd.Event.KeyEvent.bKeyDown == 1)
						{
							if(rcd.Event.KeyEvent.wVirtualKeyCode >= '1' && rcd.Event.KeyEvent.wVirtualKeyCode <= '3')
							{
								choiceMode = rcd.Event.KeyEvent.wVirtualKeyCode-'1'+1;//主键盘1-3
							}
							else if(rcd.Event.KeyEvent.wVirtualKeyCode >= 'a' && rcd.Event.KeyEvent.wVirtualKeyCode <= 'c')
							{
								choiceMode = rcd.Event.KeyEvent.wVirtualKeyCode-'a'+1;//小键盘1-3
							}
						}
						if(temp == 1)//控制台光标
						{
							if(visibleCursor == 1) visibleCursor = 0;
							else visibleCursor = 1;
							showCursor(visibleCursor);
							clrscr();
							DrawControlBar(0);
							DrawSettingsBar(0);
							DrawSetCursor(0);
							choiceTemp = 0;
							temp = 0;
						}
						else if(temp == 2)//自选光标
						{
							if(newCursor < 3) newCursor++;
							else newCursor = 0;
							//自动隐藏乱动的控制台光标
							if(newCursor == 0 && visibleCursor == 0)
							{
								visibleCursor = 1;
								showCursor(1);
								//printf("已自动显示控制台光标\n");
								//system("pause");
							}
							else if(newCursor > 0 && visibleCursor == 1)
							{
								visibleCursor = 0;
								showCursor(0);
								//printf("已自动隐藏控制台光标\n");
								//system("pause");
							}
							clrscr();
							DrawControlBar(0);
							DrawSettingsBar(0);
							DrawSetCursor(0);
							choiceTemp = 0;
							temp = 0;
						}
						Sleep(refreshCycle);
					}
					clrscr();
					DrawControlBar(0);
					DrawSettingsBar(0);
					choiceSet = 0;
				}
				else if(choiceSet == 8)//调试选项
				{
					printf("*******************************\n"//宽31
						   "调试选项仅提供给专业玩家和开发者使用。\n"
						   "除非你知道你在做什么，否则请退出设置。\n"
						   "(1)退出\n"
						   "(2)设置求解模式\n"
						   "(3)以密度设置雷数\n"
						   "(4)设置背景颜色\n"
						   "(5)设置刷新周期\n");
					if(clickSpace == 1) printf("(6)启用鼠标点击屏蔽空格\n");
					if(clickSpace == 0) printf("(6)关闭鼠标点击屏蔽空格\n");
					if(dynamicMemory == 0) printf("(7)启用动态内存分配\n");
					if(dynamicMemory == 1) printf("(7)关闭动态内存分配\n");
					printf("(8)设置Bench\n");
					printf("(9)自定义><光标字符\n");
					printf("*******************************\n");
					SetConsoleMouseMode(0);
					//getchar();
					fflush(stdin);//清空不稳定的键盘缓冲
					printf(">");
					operation = getchar();
					if(operation == '$')//调试
					{
						scanf("%d", &temp);
						if(temp == debug)
						{
							scanf("%d", &debug);
							if(debug == 2)
							{
								printf("<Debug>\nWelcome! Administrator Ltabsyy or Jws!\n");
							}
							system("pause");
							printf("胡桃当妻，枸杞难医！\n");//doge
						}
					}
					else if(operation == '2')//设置求解模式
					{
						printf("*******************************\n"//宽31
							   "(1)设置模式：游戏模式/分析模式\n"
							   "(2)自定义设置枚举限制\n"
							   "(3)退出\n"
							   "*******************************\n");
						printf("当前模式：%d|当前枚举限制：%d\n", solveMode, lengthOfThinkChain);
						printf(">");
						scanf("%d", &temp);
						if(temp == 1)
						{
							printf("*******************************\n"//宽31
								   "(1)游戏模式：先不枚举，无解则枚举\n"
								   "(2)分析模式：始终以设置的限制枚举\n"
								   "*******************************\n");
							printf("/set solveMode ");
							scanf("%d", &temp);
							if(temp == 1 || temp == 2) solveMode = temp;
						}
						else if(temp == 2)
						{
							if(solveMode == 2)
							{
								printf("*******************************\n"//宽31
									   "** 0 ---- 17 ------------ 30 **\n"
									   "** 关    效率           最大 **\n"
									   "*******************************\n");
							}
							if(solveMode == 1)
							{
								printf("*******************************\n"//宽31
									   "** 0 ------ 19 ---------- 30 **\n"
									   "** 关      效率         最大 **\n"
									   "*******************************\n");
							}
							printf("/set lengthOfThinkChain ");
							scanf("%d", &temp);
							if(temp >= 0 && temp <= 30) lengthOfThinkChain = temp;
						}
					}
					else if(operation == '3')//以密度设置雷数
					{
						printf(">");
						scanf("%f", &density);
						numberOfMine = density * heightOfBoard * widthOfBoard;
						printf("/set numberOfMine %d\n", numberOfMine);
						system("pause");
					}
					else if(operation == '4')//设置背景颜色
					{
						printf("*******************************\n"//宽31
							   "[背景颜色][字体颜色]\n"
							   "0 = 黑色       8 = 灰色\n"
							   "1 = 蓝色       9 = 淡蓝色\n"
							   "2 = 绿色       A = 淡绿色\n"
							   "3 = 浅绿色     B = 淡浅绿色\n"
							   "4 = 红色       C = 淡红色\n"
							   "5 = 紫色       D = 淡紫色\n"
							   "6 = 黄色       E = 淡黄色\n"
							   "7 = 白色       F = 亮白色\n");
						ColorStr("0 ", 0x00);
						ColorStr("1 ", 0x10);
						ColorStr("2 ", 0x20);
						ColorStr("3 ", 0x30);
						ColorStr("4 ", 0x40);
						ColorStr("5 ", 0x50);
						ColorStr("6 ", 0x60);
						ColorStr("7 ", 0x70);
						ColorStr("8 ", 0x80);
						ColorStr("9 ", 0x90);
						ColorStr("a ", 0xa0);
						ColorStr("b ", 0xb0);
						ColorStr("c ", 0xc0);
						ColorStr("d ", 0xd0);
						ColorStr("e ", 0xe0);
						ColorStr("f ", 0xf0);
						printf("\n");
						printf("默认深色模式为07，浅色模式为f0\n"
							   "*******************************\n");
						printf("当前颜色：0x%x\n", backgroundColor);
						printf("/set backgroundColor 0x");
						scanf("%x", &backgroundColor);
						setbgcolor(backgroundColor);
					}
					else if(operation == '5')//设置刷新周期
					{
						printf("当前刷新周期：%dms|当前刷新率：%dHz\n", refreshCycle, 1000/refreshCycle);
						printf("/set refreshCycle ");
						scanf("%d", &temp);
						if(temp < 1) refreshCycle = 1;
						else refreshCycle = temp;
					}
					else if(operation == '6')//鼠标点击屏蔽空格
					{
						if(clickSpace == 1) clickSpace = 0;
						else clickSpace = 1;
					}
					else if(operation == '7')//动态内存分配
					{
						if(dynamicMemory == 0)
						{
							dynamicMemory = 1;
							//开启时重新分配为当前空间
							ReallocMemory(LimHeight, LimWidth, 0);
							ReallocMemory(heightOfBoard, widthOfBoard, 1);
						}
						else
						{
							dynamicMemory = 0;
							//关闭时重新分配为最大空间
							ReallocMemory(heightOfBoard, widthOfBoard, 0);
							ReallocMemory(LimHeight, LimWidth, 1);
						}
					}
					else if(operation == '8')//设置Bench
					{
						printf("*******************************\n");//宽31
						printf("(1)设置种子范围\n");
						printf("(2)设置帧显示\n");
						printf("(3)设置方案显示\n");
						printf("(4)退出\n");
						printf("*******************************\n");
						printf("当前种子范围:%d-%d|帧:", benchSeedMin, benchSeedMax);
						if(benchShowStep == 0) printf("结束帧");
						else if(benchShowStep == 1) printf("中间帧");
						else if(benchShowStep == 2) printf("帧暂停");
						else if(benchShowStep == 3) printf("全调试");
						printf("|方案:");
						if(benchShowSolution == 0) printf("不显示");
						else if(benchShowSolution == 1) printf("显示");
						printf("\n");
						printf(">");
						scanf("%d", &temp);
						if(temp == 1)
						{
							printf("[seedMin] [seedMax]>");
							scanf("%d%d", &benchSeedMin, &benchSeedMax);
							if(benchSeedMin > benchSeedMax)
							{
								temp = benchSeedMin;
								benchSeedMin = benchSeedMax;
								benchSeedMax = temp;
							}
						}
						else if(temp == 2)
						{
							printf("[0:结束帧/1:中间帧/2:帧暂停]>");
							scanf("%d", &benchShowStep);
							if(benchShowStep < 0) benchShowStep = 0;
							if(benchShowStep > 3) benchShowStep = 3;
							if(benchShowStep == 3 && debug != 2) benchShowStep = 2;
						}
						else if(temp == 3)
						{
							printf("[0:不显示/1:显示]>");
							scanf("%d", &benchShowSolution);
							if(benchShowStep != 0) benchShowStep = 1;
						}
					}
					else if(operation == '9')//自定义><光标字符
					{
						getchar();
						printf("[Left]>");
						cursorLeft[0] = getchar();
						getchar();
						printf("[Right]>");
						cursorRight[0] = getchar();
					}
					SetConsoleMouseMode(1);
					clrscr();
					DrawControlBar(0);
					DrawSettingsBar(0);
					choiceSet = 0;
				}
				Sleep(refreshCycle);
			}
			if(choiceSet == 9)//返回并保存
			{
				file = fopen("minesweeper-settings.txt", "w");
				fprintf(file, "Map:%d*%d-%d\n", heightOfBoard, widthOfBoard, numberOfMine);
				fprintf(file, "summonCheckMode=%d\n", summonCheckMode);
				fprintf(file, "showTime=%d\n", showTime);
				fprintf(file, "show3BV=%d\n", show3BV);
				fprintf(file, "backgroundColor=0x%x\n", backgroundColor);
				fprintf(file, "operateMode=%d\n", operateMode);
				fprintf(file, "solveMode=%d\n", solveMode);
				fprintf(file, "lengthOfThinkChain=%d\n", lengthOfThinkChain);
				fprintf(file, "refreshCycle=%d\n", refreshCycle);
				fprintf(file, "fastSign=%d\n", fastSign);
				fprintf(file, "newCursor=%d\n", newCursor);
				fprintf(file, "visibleCursor=%d\n", visibleCursor);
				fprintf(file, "clickSpace=%d\n", clickSpace);
				fprintf(file, "dynamicMemory=%d\n", dynamicMemory);
				fprintf(file, "benchSeedMin=%d\n", benchSeedMin);
				fprintf(file, "benchSeedMax=%d\n", benchSeedMax);
				fprintf(file, "benchShowStep=%d\n", benchShowStep);
				fprintf(file, "benchShowSolution=%d\n", benchShowSolution);
				fprintf(file, "cursorLeft=%c\n", cursorLeft[0]);
				fprintf(file, "cursorRight=%c\n", cursorRight[0]);
				fclose(file);
				clrscr();
				choiceMode = 0;
			}
		}
		else if(choiceMode == 4)//Bench
		{
			if(debug == 2)
			{
				SetConsoleMouseMode(0);
				clrscr();//清除设置转Bench的界面残留
				DrawControlBar(0);
				printf("[seedMin] [seedMax] [0:结束帧/1:中间帧/2:帧暂停/3:全调试]\n>");
				scanf("%d%d%d", &seedMin, &seedMax, &temp);
				clrscr();//清除输入信息
				DrawControlBar(0);
				gotoxy(0, 1);
				printf("**                   请等待Bench结束. . .                   **\n");
				Bench(seedMin, seedMax, heightOfBoard/2, widthOfBoard/2, temp, benchShowSolution);//Bench模块
				system("pause");//暂停观察
				SetConsoleMouseMode(1);//Bench可左键暂停，右键继续
			}
			else
			{
				clrscr();
				DrawControlBar(0);
				gotoxy(0, 1);
				printf("**                   请等待Bench结束. . .                   **\n");
				Bench(benchSeedMin, benchSeedMax, heightOfBoard/2, widthOfBoard/2, benchShowStep, benchShowSolution);//Bench模块
			}
			choiceMode = 0;
		}
		else if(choiceMode == 6)//地图搜索
		{
			SetConsoleMouseMode(0);
			printf("当前时间戳：%d\n", time(0));
			printf("[seedMin] [seedMax] [r0] [c0]\n>");
			scanf("%d%d%d%d", &seedMin, &seedMax, &r0, &c0);
			MapSearch(seedMin, seedMax, r0, c0);//MapSearch模块
			system("pause");
			SetConsoleMouseMode(1);
			clrscr();
			DrawControlBar(0);
			choiceMode = 0;
		}
		else if(choiceMode == 7)//记录编辑器
		{
			SetConsoleMouseMode(0);
			records = RecordsEditer(records);//RecordsEditer模块
			SetConsoleMouseMode(1);
			clrscr();
			DrawControlBar(0);
			choiceMode = 0;
		}
		else if(choiceMode == 8)//自定义种子游戏
		{
			SetConsoleMouseMode(0);
			printf("当前时间戳：%d\n", time(0));
			printf("[seed] [r0] [c0]\n>");
			scanf("%d%d%d", &seed, &r0, &c0);
			lastMap = 1;
			t2 = 0;
			ro = r0;
			co = c0;
			isHelped = 1;//无效记录
			for(r=0; r<heightOfBoard; r++)
			{
				for(c=0; c<widthOfBoard; c++)
				{
					isShown[r][c] = 0;
				}
			}
			//isShown[r0][c0] = 1;
			SetConsoleMouseMode(1);
			clrscr();
			DrawControlBar(0);
			choiceMode = 1;//进入游戏
		}
		else if(choiceMode == 9)//自制地图编辑器
		{
			SetConsoleMouseMode(0);
			temp = CustomMapsEditer();
			if(temp == 1)//进入游戏
			{
				seed = -1;
				r0 = 0;
				c0 = 0;
				lastMap = 2;
				ro = heightOfBoard/2;
				co = widthOfBoard/2;
			}
			SetConsoleMouseMode(1);
			clrscr();
			DrawControlBar(0);
			choiceMode = temp;
		}
	}
	/*退出*/
	if(dynamicMemory == 1)//释放内存空间，可能并不必要(doge)
	{
		ReallocMemory(heightOfBoard, widthOfBoard, 0);
	}
	else
	{
		ReallocMemory(LimHeight, LimWidth, 0);
	}
	return 0;
}

int IsPosInRectangle(int x, int y, int x1, int y1, int x2, int y2)
{
	if(y >= y1 && y <= y2 && x >= x1 && x <= x2)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void DrawControlBar(int select)
{
	gotoxy(0, 0);
	printf("**************************************************************\n");//宽62
	printf("** [(1)新游戏] [(2)地图求解] [(3)设置] [(4)Bench] [(5)退出] **\n");
	printf("**************************************************************\n");
	//printf("** [(6)地图搜索] [(7)历史记录]  [(8)种子游戏] [(9)自制地图] **\n");
	//printf("**************************************************************\n");
	if(select == 1)
	{
		gotoxy(3, 1);
		ColorStr("[(1)新游戏]", 0xff-backgroundColor);
	}
	else if(select == 2)
	{
		gotoxy(15, 1);
		ColorStr("[(2)地图求解]", 0xff-backgroundColor);
	}
	else if(select == 3)
	{
		gotoxy(29, 1);
		ColorStr("[(3)设置]", 0xff-backgroundColor);
	}
	else if(select == 4)
	{
		gotoxy(39, 1);
		ColorStr("[(4)Bench]", 0xff-backgroundColor);
	}
	else if(select == 5)
	{
		gotoxy(50, 1);
		ColorStr("[(5)退出]", 0xff-backgroundColor);
	}
	gotoxy(0, 3);
}

int ChoiceControlBar(COORD mousePos, DWORD dwButtonState)
{
	int choiceMode = 0;
	static int lastChoice = 0;//仅在悬浮选择变化时刷新，消除闪烁
	if(IsPosInRectangle(mousePos.X, mousePos.Y, 3, 1, 13, 1))
	{
		if(lastChoice != 1)
		{
			DrawControlBar(1);
			lastChoice = 1;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceMode = 1;
			lastChoice = 0;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 15, 1, 27, 1))
	{
		if(lastChoice != 2)
		{
			DrawControlBar(2);
			lastChoice = 2;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceMode = 2;
			lastChoice = 0;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 29, 1, 37, 1))
	{
		if(lastChoice != 3)
		{
			DrawControlBar(3);
			lastChoice = 3;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceMode = 3;
			lastChoice = 0;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 39, 1, 48, 1))
	{
		if(lastChoice != 4)
		{
			DrawControlBar(4);
			lastChoice = 4;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceMode = 4;
			lastChoice = 0;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 50, 1, 58, 1))
	{
		if(lastChoice != 5)
		{
			DrawControlBar(5);
			lastChoice = 5;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceMode = 5;
			lastChoice = 0;
		}
	}
	else if(lastChoice != 0)
	{
		DrawControlBar(0);
		lastChoice = 0;
	}
	return choiceMode;
}

void DrawProgressBar(int progress)//在控制栏显示进度条
{
	int i;
	gotoxy(0, 0);
	printf("**************************************************************\n");//宽62
	//printf("** |====================================--------------| 72% **\n");
	printf("** |");
	for(i=0; i<progress/2; i++)
	{
		printf("=");
	}
	if(i!=0 && i!=50)
	{
		printf("\b>");
	}
	for(; i<100/2; i++)
	{
		printf("-");
	}
	printf("|%3d%% **\n", progress);
	printf("**************************************************************\n");
}

void DrawSettingsBar(int select)
{
	gotoxy(0, 3);
	printf("**************************************************************\n");//宽62
	printf("* [(1)设置游戏难度]      当前雷数:%d|界面大小:%d*%d|密度:%.2f\n", numberOfMine, heightOfBoard, widthOfBoard, (float)numberOfMine/heightOfBoard/widthOfBoard);
	
	if(summonCheckMode == 0) printf("* [(2)设置地图生成校验]  当前:关闭校验\n");
	else if(summonCheckMode == 1) printf("* [(2)设置地图生成校验]  当前:起始点必非雷\n");
	else if(summonCheckMode == 2) printf("* [(2)设置地图生成校验]  当前:起始点必为空\n");
	else if(summonCheckMode == 3) printf("* [(2)设置地图生成校验]  当前:地图可解\n");
	else if(summonCheckMode == 4) printf("* [(2)设置地图生成校验]  当前:地图可速解\n");
	else printf("* [(2)设置地图生成校验]  当前:起始点必为空?\n");
	
	if(showTime == 0 && show3BV == 0) printf("* [(3)设置显示用时和3BV] 当前:不显示\n");
	else if(showTime == 1 && show3BV == 0) printf("* [(3)设置显示用时和3BV] 当前:仅显示用时\n");
	else if(showTime == 1 && show3BV == 1) printf("* [(3)设置显示用时和3BV] 当前:显示用时和3BV\n");
	else if(showTime == 0 && show3BV == 1) printf("* [(3)设置显示用时和3BV] 当前:仅显示3BV\n");
	else printf("* [(3)设置显示用时和3BV] 当前:仅显示用时?\n");
	
	if(backgroundColor == 0x07) printf("* [(4)浅色模式]          当前:深色模式\n");
	else if(backgroundColor == 0xf0) printf("* [(4)深色模式]          当前:浅色模式\n");
	else printf("* [(4)深色模式]          当前:自定义模式\n");
	
	if(operateMode == 0) printf("* [(5)设置默认操作模式]  当前:键盘@#rc\n");
	else if(operateMode == 1) printf("* [(5)设置默认操作模式]  当前:键盘wasd23\n");
	else if(operateMode == 2) printf("* [(5)设置默认操作模式]  当前:鼠标点击\n");
	else if(operateMode == 3) printf("* [(5)设置默认操作模式]  当前:Window\n");
	else printf("* [(5)设置默认操作模式]  当前:鼠标点击?\n");
	
	if(fastSign == 0) printf("* [(6)启用快速标记]      当前:禁用\n");
	else if(fastSign == 1) printf("* [(6)禁用快速标记]      当前:启用(标记数字快速标记周围)\n");
	else printf("* [(6)启用快速标记]      当前:禁用?\n");
	
	printf("* [(7)设置光标]\n");
	printf("* [(8)调试选项]\n");
	printf("* [(9)返回并保存]\n");
	printf("**************************************************************\n");
	if(select == 1)
	{
		gotoxy(2, 4);
		ColorStr("[(1)设置游戏难度]", 0xff-backgroundColor);
	}
	else if(select == 2)
	{
		gotoxy(2, 5);
		ColorStr("[(2)设置地图生成校验]", 0xff-backgroundColor);
	}
	else if(select == 3)
	{
		gotoxy(2, 6);
		if(showTime == 0 && show3BV == 0) ColorStr("[(3)启用实时显示用时 ]", 0xff-backgroundColor);
		else if(showTime == 1 && show3BV == 0) ColorStr("[(3)启用实时显示3BV  ]", 0xff-backgroundColor);
		else if(showTime == 1 && show3BV == 1) ColorStr("[(3)关闭实时显示用时 ]", 0xff-backgroundColor);
		else if(showTime == 0 && show3BV == 1) ColorStr("[(3)关闭实时显示3BV  ]", 0xff-backgroundColor);
		else ColorStr("[(3)设置显示用时和3BV]", 0xff-backgroundColor);
	}
	else if(select == 4)
	{
		gotoxy(2, 7);
		if(backgroundColor == 0x07) ColorStr("[(4)浅色模式]", 0xff-backgroundColor);
		else ColorStr("[(4)深色模式]", 0xff-backgroundColor);
	}
	else if(select == 5)
	{
		gotoxy(2, 8);
		ColorStr("[(5)设置默认操作模式]", 0xff-backgroundColor);
	}
	else if(select == 6)
	{
		gotoxy(2, 9);
		if(fastSign == 0) ColorStr("[(6)启用快速标记]", 0xff-backgroundColor);
		else ColorStr("[(6)禁用快速标记]", 0xff-backgroundColor);
	}
	else if(select == 7)
	{
		gotoxy(2, 10);
		ColorStr("[(7)设置光标]", 0xff-backgroundColor);
	}
	else if(select == 8)
	{
		gotoxy(2, 11);
		ColorStr("[(8)调试选项]", 0xff-backgroundColor);
	}
	else if(select == 9)
	{
		gotoxy(2, 12);
		ColorStr("[(9)返回并保存]", 0xff-backgroundColor);
	}
	gotoxy(0, 14);
}

int ChoiceSettingsBar(COORD mousePos, DWORD dwButtonState)
{
	int choiceSet = 0;
	static int lastChoice = 0;
	if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 4, 18, 4))
	{
		if(lastChoice != 1)
		{
			DrawSettingsBar(1);
			lastChoice = 1;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSet = 1;
			lastChoice = 0;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 5, 22, 5))
	{
		if(lastChoice != 2)
		{
			DrawSettingsBar(2);
			lastChoice = 2;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSet = 2;
			lastChoice = 0;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 6, 23, 6))
	{
		if(lastChoice != 3)
		{
			DrawSettingsBar(3);
			lastChoice = 3;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSet = 3;
			lastChoice = 0;//按下后刷新按钮
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 7, 14, 7))
	{
		if(lastChoice != 4)
		{
			DrawSettingsBar(4);
			lastChoice = 4;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSet = 4;
			lastChoice = 0;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 8, 22, 8))
	{
		if(lastChoice != 5)
		{
			DrawSettingsBar(5);
			lastChoice = 5;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSet = 5;
			lastChoice = 0;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 9, 18, 9))
	{
		if(lastChoice != 6)
		{
			DrawSettingsBar(6);
			lastChoice = 6;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSet = 6;
			lastChoice = 0;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 10, 14, 10))
	{
		if(lastChoice != 7)
		{
			DrawSettingsBar(7);
			lastChoice = 7;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSet = 7;
			lastChoice = 0;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 11, 14, 11))
	{
		if(lastChoice != 8)
		{
			DrawSettingsBar(8);
			lastChoice = 8;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSet = 8;
			lastChoice = 0;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 12, 16, 12))
	{
		if(lastChoice != 9)
		{
			DrawSettingsBar(9);
			lastChoice = 9;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSet = 9;
			lastChoice = 0;
		}
	}
	else if(lastChoice != 0)
	{
		DrawSettingsBar(0);
		lastChoice = 0;
	}
	return choiceSet;
}

void DrawSetDifficulty(int select)
{
	int r, c;
	int heightOfWindow, widthOfWindow, heightOfFont, maxHeightOfMap, maxWidthOfMap;
	GetSizeOfMap(&heightOfWindow, &widthOfWindow, &heightOfFont, &maxHeightOfMap, &maxWidthOfMap);
	gotoxy(0, 0);
	/*printf("**************************************************************\n");//宽62
	printf("** [(0)默认：10*10 - 10] ** 空间占用示意 ** 可使用全屏，    **\n");
	printf("** [(1)初级： 9*9  - 10] ****************** 可使用Ctrl+滚轮 **\n");
	printf("** [(2)中级：16*16 - 40] **              ** 调整字体大小。  **\n");
	printf("** [(3)高级：16*30 - 99] **              *********************\n");
	printf("** [(4)顶级：42*88 -715] **    [清屏]    ** 如界面下滑，    **\n");
	printf("** [(5)自定义***** - **] **              ** 不建议选择！    **\n");
	printf("**************************************************************\n");*/
	printf("**************************************************************\n");//宽62
	printf("** [(0)默认：10*10 - 10] ** 窗口大小：%3d*%2d 字体大小：%2d   **\n", widthOfWindow, heightOfWindow, heightOfFont);
	printf("** [(1)初级： 9*9  - 10] ** 最大地图：%2d*%3d 当前密度：%.2f **\n", maxHeightOfMap, maxWidthOfMap, (float)numberOfMine/heightOfBoard/widthOfBoard);
	printf("** [(2)中级：16*16 - 40] *************************************\n");
	printf("** [(3)高级：16*30 - 99] **              ** 可使用全屏，    **\n");
	printf("** [(4)顶级：42*88 -715] **    [清屏]    ** 可使用Ctrl+滚轮 **\n");
	printf("** [(5)自定义***** - **] **              ** 调整字体大小。  **\n");
	printf("**************************************************************\n");
	if(select == 0)
	{
		gotoxy(3, 1);
		ColorStr("[(0)默认：10*10 - 10]", 0xff-backgroundColor);
	}
	else if(select == 1)
	{
		gotoxy(3, 2);
		ColorStr("[(1)初级： 9*9  - 10]", 0xff-backgroundColor);
	}
	else if(select == 2)
	{
		gotoxy(3, 3);
		ColorStr("[(2)中级：16*16 - 40]", 0xff-backgroundColor);
	}
	else if(select == 3)
	{
		gotoxy(3, 4);
		ColorStr("[(3)高级：16*30 - 99]", 0xff-backgroundColor);
	}
	else if(select == 4)
	{
		gotoxy(3, 5);
		ColorStr("[(4)顶级：42*88 -715]", 0xff-backgroundColor);
	}
	else if(select == 5)
	{
		gotoxy(3, 6);
		ColorStr("[(5)自定义***** - **]", 0xff-backgroundColor);
	}
	else if(select == 6)
	{
		gotoxy(31, 5);
		ColorStr("[清屏]", 0xff-backgroundColor);
	}
	gotoxy(0, 8);
	if(select == 7)//清除预览地图
	{
		for(r=0; r<heightOfBoard+2; r++)
		{
			for(c=0; c<widthOfBoard+2; c++)
			{
				printf("  ");
			}
			printf("\n");
		}
		gotoxy(0, 0);
	}
}

int ChoiceSetDifficulty(COORD mousePos, DWORD dwButtonState)
{
	int choiceDifficulty = -1;
	static int lastChoice = -1;
	if(IsPosInRectangle(mousePos.X, mousePos.Y, 3, 1, 23, 1))
	{
		if(lastChoice != 0)
		{
			if(heightOfBoard > 10 || widthOfBoard > 10)
			{
				DrawSetDifficulty(7);
			}
			//预览默认地图
			numberOfMine = 10;
			heightOfBoard = 10;
			widthOfBoard = 10;
			DrawSetDifficulty(0);
			SummonBoard(0, 5, 5);
			ShowBoard(1);
			lastChoice = 0;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceDifficulty = 0;
			lastChoice = -1;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 3, 2, 23, 2))
	{
		if(lastChoice != 1)
		{
			if(heightOfBoard > 9 || widthOfBoard > 9)
			{
				DrawSetDifficulty(7);
			}
			//预览初级地图
			numberOfMine = 10;
			heightOfBoard = 9;
			widthOfBoard = 9;
			DrawSetDifficulty(1);
			SummonBoard(0, 4, 4);
			ShowBoard(1);
			lastChoice = 1;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceDifficulty = 1;
			lastChoice = -1;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 3, 3, 23, 3))
	{
		if(lastChoice != 2)
		{
			if(heightOfBoard > 16 || widthOfBoard > 16)
			{
				DrawSetDifficulty(7);
			}
			//预览中级地图
			numberOfMine = 40;
			heightOfBoard = 16;
			widthOfBoard = 16;
			DrawSetDifficulty(2);
			SummonBoard(0, 8, 8);
			ShowBoard(1);
			lastChoice = 2;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceDifficulty = 2;
			lastChoice = -1;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 3, 4, 23, 4))
	{
		if(lastChoice != 3)
		{
			if(heightOfBoard > 16 || widthOfBoard > 30)
			{
				DrawSetDifficulty(7);
			}
			//预览高级地图
			numberOfMine = 99;
			heightOfBoard = 16;
			widthOfBoard = 30;
			DrawSetDifficulty(3);
			SummonBoard(0, 8, 15);
			ShowBoard(1);
			lastChoice = 3;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceDifficulty = 3;
			lastChoice = -1;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 3, 5, 23, 5))
	{
		if(lastChoice != 4)
		{
			//预览顶级地图
			numberOfMine = 715;
			heightOfBoard = 42;
			widthOfBoard = 88;
			DrawSetDifficulty(4);
			SummonBoard(0, 21, 44);
			ShowBoard(1);
			gotoxy(0, 0);//防止界面下滑
			lastChoice = 4;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceDifficulty = 4;
			lastChoice = -1;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 3, 6, 23, 6))
	{
		if(lastChoice != 5)
		{
			DrawSetDifficulty(5);
			lastChoice = 5;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)//自定义
		{
			choiceDifficulty = 5;
			lastChoice = -1;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 31, 5, 36, 5))
	{
		if(lastChoice != 6)
		{
			DrawSetDifficulty(6);
			lastChoice = 6;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			//choiceDifficulty = -1;
			clrscr();//清屏
			lastChoice = -1;//按下后刷新防止空屏
		}
	}
	else// if(lastChoice != -1)//始终刷新
	{
		DrawSetDifficulty(-1);//刷新窗口大小
		lastChoice = -1;//防止清屏移开后回来按钮不反色
	}
	return choiceDifficulty;
}

void DrawSetSummonCheckMode(int select)
{
	gotoxy(0, 14);
	printf("*******************************\n"//宽31
		   "* [(0)关闭校验]\n"
		   "* [(1)起始点必非雷]\n"
		   "* [(2)起始点必为空]\n"
		   "* [(3)地图可解]（生成时间可能较长）\n"
		   "* [(4)地图可速解]（易生成重复地图）\n"
		   "*******************************\n");
	if(select == 0)
	{
		gotoxy(2, 15);
		ColorStr("[(0)关闭校验]", 0xff-backgroundColor);
	}
	else if(select == 1)
	{
		gotoxy(2, 16);
		ColorStr("[(1)起始点必非雷]", 0xff-backgroundColor);
	}
	else if(select == 2)
	{
		gotoxy(2, 17);
		ColorStr("[(2)起始点必为空]", 0xff-backgroundColor);
	}
	else if(select == 3)
	{
		gotoxy(2, 18);
		ColorStr("[(3)地图可解]", 0xff-backgroundColor);
	}
	else if(select == 4)
	{
		gotoxy(2, 19);
		ColorStr("[(4)地图可速解]", 0xff-backgroundColor);
	}
	gotoxy(0, 21);
}

int ChoiceSetSummonCheckMode(COORD mousePos, DWORD dwButtonState)
{
	int choiceSummonCheckMode = -1;
	static int lastChoice = -1;
	if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 15, 14, 15))
	{
		if(lastChoice != 0)
		{
			DrawSetSummonCheckMode(0);
			lastChoice = 0;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSummonCheckMode = 0;
			//lastChoice = -1;//按下后操作栏立即消失
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 16, 18, 16))
	{
		if(lastChoice != 1)
		{
			DrawSetSummonCheckMode(1);
			lastChoice = 1;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSummonCheckMode = 1;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 17, 18, 17))
	{
		if(lastChoice != 2)
		{
			DrawSetSummonCheckMode(2);
			lastChoice = 2;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSummonCheckMode = 2;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 18, 14, 18))
	{
		if(lastChoice != 3)
		{
			DrawSetSummonCheckMode(3);
			lastChoice = 3;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSummonCheckMode = 3;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 19, 16, 19))
	{
		if(lastChoice != 4)
		{
			DrawSetSummonCheckMode(4);
			lastChoice = 4;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSummonCheckMode = 4;
		}
	}
	else if(lastChoice != -1)
	{
		DrawSetSummonCheckMode(-1);
		lastChoice = -1;
	}
	return choiceSummonCheckMode;
}

void DrawSetOperateMode(int select)
{
	gotoxy(0, 14);
	printf("**************************************************************\n");
	printf("* 键盘操作时可根据按键自动切换@#rc/wasd23操作模式！\n");
	printf("* 键盘操作可按M切换到鼠标操作，鼠标操作可按K切换到键盘操作。\n");
	printf("* [(0)@#rc操作模式]\n");
	printf("* 输入[@:翻开/#:标记] [r:行坐标] [c:列坐标]，\n");
	printf("* 并按回车后执行，如@ 0 0指翻开(0,0)。\n");
	printf("* [(1)wasd23操作模式]\n");
	printf("* 通过wasd移动光标，按2翻开，按3标记，\n");
	printf("* 无小键盘可用hu代替23。\n");
	printf("* [(2)鼠标点击操作模式]\n");
	printf("* 可通过鼠标，wasd，方向键移动光标。\n");
	printf("* 兼容wasd23，但无法使用@#rc和鼠标全选复制地图。\n");
	//printf("* [(3)Window]\n");
	//printf("* 在外部窗口进行游戏，控制台内操作均屏蔽，\n");
	//printf("* 游戏结束后按键盘任意键返回控制台。\n");
	printf("**************************************************************\n");
	if(select == 0)
	{
		gotoxy(2, 17);
		ColorStr("[(0)@#rc操作模式]", 0xff-backgroundColor);
	}
	else if(select == 1)
	{
		gotoxy(2, 20);
		ColorStr("[(1)wasd23操作模式]", 0xff-backgroundColor);
	}
	else if(select == 2)
	{
		gotoxy(2, 23);
		ColorStr("[(2)鼠标点击操作模式]", 0xff-backgroundColor);
	}
	else if(select == 3)
	{
		gotoxy(2, 26);
		//ColorStr("[(3)Window]", 0xff-backgroundColor);
	}
	gotoxy(0, 27);
}

int ChoiceSetOperateMode(COORD mousePos, DWORD dwButtonState)
{
	int choiceOperateMode = -1;
	static int lastChoice = -1;
	if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 17, 18, 17))
	{
		if(lastChoice != 0)
		{
			DrawSetOperateMode(0);
			lastChoice = 0;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceOperateMode = 0;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 20, 20, 20))
	{
		if(lastChoice != 1)
		{
			DrawSetOperateMode(1);
			lastChoice = 1;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceOperateMode = 1;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 23, 22, 23))
	{
		if(lastChoice != 2)
		{
			DrawSetOperateMode(2);
			lastChoice = 2;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceOperateMode = 2;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 26, 12, 26))
	{
		if(lastChoice != 3)
		{
			DrawSetOperateMode(3);
			lastChoice = 3;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceOperateMode = 3;
		}
	}
	else if(lastChoice != -1)
	{
		DrawSetOperateMode(-1);
		lastChoice = -1;
	}
	return choiceOperateMode;
}

void DrawSetCursor(int select)
{
	gotoxy(0, 14);
	printf("*******************************\n");//宽31
	if(visibleCursor == 1) printf("* [(1)隐藏控制台光标]   当前:显示\n");
	else if(visibleCursor == 0) printf("* [(1)显示控制台光标]   当前:隐藏\n");
	else printf("* [(1)隐藏控制台光标]   当前:显示?\n");
	
	if(newCursor == 0) printf("* [(2)设置自选光标形态] 当前:未启用\n");
	else if(newCursor == 1) printf("* [(2)设置自选光标形态] 当前:><光标\n");
	else if(newCursor == 2) printf("* [(2)设置自选光标形态] 当前:淡黄色高亮光标\n");
	else if(newCursor == 3) printf("* [(2)设置自选光标形态] 当前:淡黄色高亮行列\n");
	else printf("* [(2)设置自选光标形态] 当前:未启用?\n");
	printf("* [(3)退出]\n");
	printf("*******************************\n");
	if(select == 1)
	{
		gotoxy(2, 15);
		if(visibleCursor == 0) ColorStr("[(1)显示控制台光标]", 0xff-backgroundColor);
		else ColorStr("[(1)隐藏控制台光标]", 0xff-backgroundColor);
	}
	else if(select == 2)
	{
		gotoxy(2, 16);
		ColorStr("[(2)设置自选光标形态]", 0xff-backgroundColor);
	}
	else if(select == 3)
	{
		gotoxy(2, 17);
		ColorStr("[(3)退出]", 0xff-backgroundColor);
	}
	gotoxy(0, 19);
}

int ChoiceSetCursor(COORD mousePos, DWORD dwButtonState)
{
	int choiceCursor = 0;
	static int lastChoice = 0;
	if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 15, 20, 15))
	{
		if(lastChoice != 1)
		{
			DrawSetCursor(1);
			lastChoice = 1;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceCursor = 1;
			lastChoice = 0;//按下后立即刷新信息
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 16, 22, 16))
	{
		if(lastChoice != 2)
		{
			DrawSetCursor(2);
			lastChoice = 2;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceCursor = 2;
			lastChoice = 0;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 17, 10, 17))
	{
		if(lastChoice != 3)
		{
			DrawSetCursor(3);
			lastChoice = 3;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceCursor = 3;
			lastChoice = 0;
		}
	}
	else if(lastChoice != 0)
	{
		DrawSetCursor(0);
		lastChoice = 0;
	}
	return choiceCursor;
}

void SummonBoard(int seed, int r0, int c0)//生成后台总板
{
	int r, c, i, ra, ca;
	srand(seed);
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			isShown[r][c] = 0;//清零显示方式矩阵
		}
	}
	while(1)
	{
		/*--重置--*/
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				isMine[r][c] = 0;//雷，0无1有
				numberOfMineAround[r][c] = 0;
				board[r][c] = 0;
			}
		}
		/*--生成雷--*/
		for(i=0; i<numberOfMine; )//不校验第1次也可能爆哦(doge)
		{
			r = rand() % heightOfBoard;
			c = rand() % widthOfBoard;
			if(isMine[r][c] == 0)
			{
				isMine[r][c] = 1;
				i++;
			}
		}
		/*if(debug == 2)
		{
			printf("[Debug]已生成雷区：\n");
			for(r=0; r<heightOfBoard; r++)
			{
				printf("  ");
				for(c=0; c<widthOfBoard; c++)
				{
					printf("%d ", isMine[r][c]);
				}
				printf("\n");
			}
			printf("\n");
		}*/
		/*校验*/
		if(isMine[r0][c0] == 1 && summonCheckMode > 0) continue;//第1次就爆则循环
		/*--生成雷周围数字--*/
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				if(isMine[r][c] == 1)//循环遍历雷，在雷周围生成数字
				{
					for(ra=r-1; ra<=r+1; ra++)
					{
						for(ca=c-1; ca<=c+1; ca++)
						{
							if(ra>=0 && ra<heightOfBoard && ca>=0 && ca<widthOfBoard)//确认在范围内
							{
								numberOfMineAround[ra][ca]++;
							}
						}
					}
				}//挨得过紧的雷也会被数字覆盖
			}
		}
		/*if(debug == 2)
		{
			printf("[Debug]已生成数字矩阵：\n");
			for(r=0; r<heightOfBoard; r++)
			{
				printf("  ");
				for(c=0; c<widthOfBoard; c++)
				{
					printf("%d ", numberOfMineAround[r][c]);
				}
				printf("\n");
			}
			printf("\n");
		}*/
		/*校验*/
		if(numberOfMineAround[r0][c0] != 0 && summonCheckMode > 1) continue;//第1次翻开位置不为0则循环
		/*--生成后台总板--*/
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				if(isMine[r][c] == 1)
				{
					board[r][c] = 9;//雷标记为9，解决数字覆盖掉雷的情况
				}
				else
				{
					board[r][c] = numberOfMineAround[r][c];
				}
			}
		}
		/*if(debug == 2)
		{
			printf("[Debug]已生成后台总板：\n");
			for(r=0; r<heightOfBoard; r++)
			{
				printf("  ");
				for(c=0; c<widthOfBoard; c++)
				{
					printf("%d ", board[r][c]);
				}
				printf("\n");
			}
			printf("\n");
			//system("pause");
		}*/
		/*完毕*/
		break;
	}	
}

int Place(int n)//计算某数所占位数
{
	int i = 0;
	if(n == 0) return 1;
	while(n > 0)
	{
		i++;
		n /= 10;
	}
	return i;
}

void PrintCell(int board, int isShown, int bkcolor)//打印地图方块内字符
{
	/*----------
	  9 2: #
	  0 2: _#
	  n 2: n#(0<n<9)
	  9 0: *
	  n 0: % (n!=9 使用0 0)
	  9 1: @
	  0 1:   (空格)
	  n 1: n (0<n<9)
	----------*/
	if(isShown == 2)
	{
		if(board == 9)
		{
			ColorStr("#", 0x0c + bkcolor);
			printf(" ");
			//ColorStr("↑", 0x0c + bkcolor);
			//ColorStr("◎", 0x0c + bkcolor);
		}
		else//错误标记
		{
			if(board == 0) ColorStr("_", 0x04 + bkcolor);
			else ColorNumber(board, 0x04 + bkcolor);
			ColorStr("#", 0x0c + bkcolor);
		}
	}
	else if(isShown == 0)
	{
		if(board == 9)
		{
			ColorStr("* ", 0x0c + bkcolor);
			//ColorStr("⊕", 0x0c + bkcolor);
			//ColorStr("●", 0x0c + bkcolor);
		}
		else
		{
			printf("%% ");
			//printf("■");
			//printf("□");
		}
	}
	else
	{
		if(board == 9)
		{
			ColorStr("@", 0x04 + bkcolor);
			printf(" ");
			//ColorStr("※", 0x04 + bkcolor);
			//ColorStr("●", 0x04 + bkcolor);
		}
		else if(board == 0)
		{
			printf("  ");
			//printf("　");//我超，全角空格？！
		}
		else if(board == 1)
		{
			ColorStr("1 ", 0x01 + bkcolor);
			//ColorStr("①", 0x01 + bkcolor);
			//ColorStr("１", 0x01 + bkcolor);
		}
		else if(board == 2)
		{
			ColorStr("2 ", 0x02 + bkcolor);
			//ColorStr("②", 0x02 + bkcolor);
			//ColorStr("２", 0x02 + bkcolor);
		}
		else if(board == 3)
		{
			ColorStr("3 ", 0x04 + bkcolor);
			//ColorStr("③", 0x04 + bkcolor);
			//ColorStr("３", 0x04 + bkcolor);
		}
		else if(board == 4)
		{
			ColorStr("4 ", 0x04 + bkcolor);
			//ColorStr("④", 0x04 + bkcolor);
			//ColorStr("４", 0x04 + bkcolor);
		}
		else if(board == 5)
		{
			ColorStr("5 ", 0x04 + bkcolor);
			//ColorStr("⑤", 0x04 + bkcolor);
			//ColorStr("５", 0x04 + bkcolor);
		}
		else if(board == 6)
		{
			ColorStr("6", 0x04 + bkcolor);
			printf(" ");//浅色模式678显示淡黄色底纹
			//ColorStr("⑥", 0x04 + bkcolor);
			//ColorStr("６", 0x04 + bkcolor);
		}
		else if(board == 7)
		{
			ColorStr("7", 0x04 + bkcolor);
			printf(" ");
			//ColorStr("⑦", 0x04 + bkcolor);
			//ColorStr("７", 0x04 + bkcolor);
		}
		else if(board == 8)
		{
			ColorStr("8", 0x04 + bkcolor);
			printf(" ");
			//ColorStr("⑧", 0x04 + bkcolor);
			//ColorStr("８", 0x04 + bkcolor);
		}
	}
}

void ShowBlock(int r, int c, int mode)//根据背景颜色和模式显示地图方块
{
	if(backgroundColor == 0xf0)//浅色模式
	{
		if(mode == 1)//后台
		{
			if(isShown[r][c] == 2)//错误标记
			{
				PrintCell(board[r][c], 2, 0xf0);
			}
			else if(board[r][c] == 0)
			{
				PrintCell(0, 1, 0xf0);
			}
			else if(board[r][c] == 9)
			{
				PrintCell(9, isShown[r][c], 0xf0);
			}
			else
			{
				PrintCell(board[r][c], 1, 0xf0);
			}
		}
		else if(mode == 0)//前台
		{
			if(isShown[r][c] == 2)//浅色模式标记加灰色底纹
			{
				PrintCell(9, 2, 0x70);
			}
			else if(isShown[r][c] == 0)
			{
				PrintCell(0, 0, 0xf0);
			}
			else//isShown[r][c] == 1
			{
				if(board[r][c] > 5)//浅色模式678显示淡黄色底纹
				{
					PrintCell(board[r][c], 1, 0xe0);
				}
				else
				{
					PrintCell(board[r][c], 1, 0xf0);
				}
			}
		}
		else if(mode == 2)//浅色模式前台高亮
		{
			if(isShown[r][c] == 2)//#%空格加淡黄色底纹
			{
				//PrintCell(9, 2, 0xe0);//底纹不覆盖空格
				ColorStr("# ", 0xec);//底纹覆盖空格
			}
			else if(isShown[r][c] == 0)
			{
				//PrintCell(0, 0, 0xe0);
				//ColorStr("%", 0xe0);//PrintCell%采用背景颜色
				//printf(" ");//底纹不覆盖空格
				ColorStr("% ", 0xe0);
			}
			else//isShown[r][c] == 1
			{
				if(board[r][c] == 0)
				{
					//ColorStr(" ", 0xe0);//PrintCell空格采用背景颜色
					//printf(" ");//底纹不覆盖全部空格
					ColorStr("  ", 0xe0);
				}
				else if(board[r][c] > 5)
				{
					ColorNumber(board[r][c], 0xe4);//PrintCell 6-8不覆盖空格颜色
					ColorStr(" ", 0xe0);
				}
				else
				{
					PrintCell(board[r][c], 1, 0xe0);
				}
			}
		}
	}
	else//深色模式和自定义模式
	{
		if(mode == 1)//后台
		{
			if(isShown[r][c] == 2)//错误标记
			{
				PrintCell(board[r][c], 2, backgroundColor/16*16);
			}
			else if(board[r][c] == 0)
			{
				PrintCell(0, 1, backgroundColor/16*16);
			}
			else if(board[r][c] == 9)
			{
				PrintCell(9, isShown[r][c], backgroundColor/16*16);
			}
			else if(board[r][c] < 6)
			{
				PrintCell(board[r][c], 1, backgroundColor/16*16);
			}
			else//6-8
			{
				PrintCell(board[r][c], 1, backgroundColor/16*16 + 0x0e - 0x04);
			}
		}
		else if(mode == 0)//前台
		{
			if(isShown[r][c] == 2)
			{
				PrintCell(9, 2, backgroundColor/16*16);
			}
			else if(isShown[r][c] == 0)
			{
				PrintCell(0, 0, backgroundColor/16*16);
			}
			else//isShown[r][c] == 1
			{
				if(board[r][c] > 5)//深色模式678以淡黄色显示
				{
					PrintCell(board[r][c], 1, backgroundColor/16*16 + 0x0e - 0x04);
				}
				else
				{
					PrintCell(board[r][c], 1, backgroundColor/16*16);
				}
			}
		}
		else if(mode == 2)//深色模式前台高亮
		{
			if(isShown[r][c] == 2)//以淡黄色显示#%
			{
				PrintCell(9, 2, backgroundColor/16*16 + 0x0e - 0x0c);
			}
			else if(isShown[r][c] == 0)
			{
				ColorStr("% ", backgroundColor/16*16 + 0x0e);
			}
			else//isShown[r][c] == 1
			{
				if(board[r][c] == 0)//以淡黄色_显示空格
				{
					ColorStr("_ ", backgroundColor/16*16 + 0x0e);
				}
				else if(board[r][c] > 5)//深色模式678以淡黄色底纹和红色显示
				{
					PrintCell(board[r][c], 1, 0xe0);
				}
				else
				{
					//PrintCell(board[r][c], isShown[r][c], backgroundColor/16*16);
					ColorNumber(board[r][c], backgroundColor/16*16 + 0x0e);
					printf(" ");
				}
			}
		}
	}
}

void ShowBoard(int mode)//在控制台显示地图，0前台，1后台
{
	int r, c, i, j, n;
	for(i=Place(widthOfBoard-1); i>0; i--)//列坐标
	{
		for(j=0; j<Place(heightOfBoard-1)+1; j++)
		{
			printf(" ");
		}
		for(c=0; c<widthOfBoard; c++)
		{
			n = c;
			for(j=1; j<i; j++)
			{
				n /= 10;
			}
			n %= 10;
			if(i != 1 && n == 0)
			{
				printf("  ");
			}
			else
			{
				printf("%d ", n);
			}
		}
		printf("\n");
	}
	for(r=0; r<heightOfBoard; r++)
	{
		for(i=0; i<Place(heightOfBoard-1)-Place(r); i++)//行坐标
		{
			printf(" ");
		}
		printf("%d ", r);
		for(c=0; c<widthOfBoard; c++)
		{
			ShowBlock(r, c, mode);
			//ShowBlock(r, c, 2);
		}
		printf("\n");
	}
	//ShowPartBoard(0, 0, heightOfBoard-1, widthOfBoard-1, mode);
}
/*
void ShowPartBoard(int r1, int c1, int r2, int c2, int mode)//在控制台显示部分地图，用于超大地图显示
{
	int r, c, i, j, n;
	for(i=Place(c2); i>0; i--)//列坐标
	{
		for(j=0; j<Place(r2)+1; j++)
		{
			printf(" ");
		}
		for(c=c1; c<=c2; c++)
		{
			n = c;
			for(j=1; j<i; j++)
			{
				n /= 10;
			}
			n %= 10;
			if(i != 1 && n == 0)
			{
				printf("  ");
			}
			else
			{
				printf("%d ", n);
			}
		}
		printf("\n");
	}
	for(r=r1; r<=r2; r++)
	{
		for(i=0; i<Place(r2)-Place(r); i++)//行坐标
		{
			printf(" ");
		}
		printf("%d ", r);
		for(c=c1; c<=c2; c++)
		{
			ShowBlock(r, c, mode);
		}
		printf("\n");
	}
}
*/
void ReShowBoard(int r, int c, int mode, int yOfMap)//地图的行/列/点重绘
{
	int i, j, n;
	if(r != -1 && c == -1)//重绘行
	{
		gotoxy(0, 0);//防止行重绘不显示控制栏
		gotoxy(0, yOfMap+rs0+r);
		for(i=0; i<Place(heightOfBoard-1)-Place(r); i++)//重绘行坐标
		{
			printf(" ");
		}
		if(mode == 2)
		{
			if(backgroundColor == 0xf0)
			{
				ColorNumber(r, 0xe0);
			}
			else
			{
				ColorNumber(r, backgroundColor/16*16 + 0x0e);
			}
			printf(" ");
		}
		else
		{
			printf("%d ", r);
		}
		for(c=0; c<widthOfBoard; c++)//重绘该行方块
		{
			ShowBlock(r, c, mode);
		}
		printf("\n");
	}
	else if(r == -1 && c != -1)//重绘列
	{
		for(i=Place(widthOfBoard-1); i>0; i--)//重绘列坐标
		{
			n = c;
			for(j=1; j<i; j++)
			{
				n /= 10;
			}
			n %= 10;
			gotoxy(cs0+2*c, yOfMap+Place(widthOfBoard-1)-i);
			if(i != 1 && n == 0)
			{
				printf("  ");
			}
			else
			{
				if(mode == 2)
				{
					if(backgroundColor == 0xf0)
					{
						ColorNumber(n, 0xe0);
					}
					else
					{
						ColorNumber(n, backgroundColor/16*16 + 0x0e);
					}
					printf(" ");
				}
				else
				{
					printf("%d ", n);
				}
			}
		}
		for(r=0; r<heightOfBoard; r++)//重绘该列方块
		{
			gotoxy(cs0+2*c, yOfMap+rs0+r);
			ShowBlock(r, c, mode);
		}
	}
	else if(r != -1 && c != -1)//重绘点
	{
		gotoxy(cs0+2*c, yOfMap+rs0+r);
		ShowBlock(r, c, mode);
	}
	else//(-1,-1)全部重绘
	{
		gotoxy(0, yOfMap);
		ShowBoard(mode);
	}
}

void FreshCursor(int r, int c, int yOfMap)//刷新光标
{
	//yOfMap = yOfMapEnd-heightOfMapShown+1
	//yOfMapEnd-heightOfBoard+1 = yOfMap+rs0
	static int ra = 0, ca = 0;//初始化为左上角，防止淡黄色高亮光标闪退
	if(newCursor > 0)
	{
		if(newCursor > 1)//绘制淡黄色高亮光标
		{
			if(ra != r || ca != c)//浅用ra,ca实现抗闪烁(doge)
			{
				//ReShowBoard(-1, -1, 0, yOfMap);//重绘全图
				ReShowBoard(ra, ca, 0, yOfMap);
				ReShowBoard(r, c, 2, yOfMap);
				if(newCursor == 3)//淡黄色高亮行列
				{
					if(ra != r) ReShowBoard(ra, -1, 0, yOfMap);
					if(ca != c) ReShowBoard(-1, ca, 0, yOfMap);
					ReShowBoard(r, -1, 2, yOfMap);
					ReShowBoard(-1, c, 2, yOfMap);
				}
				ra = r;
				ca = c;
			}
		}
		else if(newCursor == 1)//绘制><光标
		{
			//去除拖影
			// % %3%4% %
			// %1%>@<%2%
			// % %5%6% %
			/*gotoxy(cs0+2*c-3, yOfMap+rs0+r);
			printf(" ");//去除左拖影
			gotoxy(cs0+2*c+3, yOfMap+rs0+r);
			printf(" ");//去除右拖影
			gotoxy(cs0+2*c-1, yOfMap+rs0+r-1);
			printf(" ");//去除左上拖影
			gotoxy(cs0+2*c+1, yOfMap+rs0+r-1);
			printf(" ");//去除右上拖影
			gotoxy(cs0+2*c-1, yOfMap+rs0+r+1);
			printf(" ");//去除左下拖影
			gotoxy(cs0+2*c+1, yOfMap+rs0+r+1);
			printf(" ");//去除右下拖影*/
			/*for(ra=r-2; ra<=r+2; ra++)
			{
				for(ca=c-2; ca<=c+3; ca++)//向右偏移
				{
					if(ra>=0 && ra<heightOfBoard && ca>=0 && ca<=widthOfBoard)//确认在范围内并清除右边界拖影
					{
						gotoxy(cs0+2*ca-1, yOfMap+rs0+ra);
						printf(" ");//去除左侧拖影
					}
				}
			}*/
			if(ra != r || ca != c)
			{
				//去除拖影
				gotoxy(cs0+2*ca-1, yOfMap+rs0+ra);
				printf(" ");
				gotoxy(cs0+2*ca+1, yOfMap+rs0+ra);
				printf(" ");
				if(ca == -1) ReShowBoard(-1, -1, 0, yOfMap);//@#rc仅输入行坐标时光标位于坐标轴，需重绘全图
				//绘制><
				gotoxy(cs0+2*c-1, yOfMap+rs0+r);
				if(backgroundColor == 0xf0) ColorStr(cursorLeft, 0xfd);
				else ColorStr(cursorLeft, backgroundColor/16*16 + 0x0e);
				gotoxy(cs0+2*c+1, yOfMap+rs0+r);
				if(backgroundColor == 0xf0) ColorStr(cursorRight, 0xfd);
				else ColorStr(cursorRight, backgroundColor/16*16 + 0x0e);
				ra = r;
				ca = c;
			}
		}
		//gotoxy(0, yOfMapEnd+4);
	}
	//光标移动至对应点
	gotoxy(cs0+2*c, yOfMap+rs0+r);//当实际位置为该位置左部，光标显示在该位置
}
/*
void DrawMine(int r, int c)//绘制地图地雷
{
	//setcolor(LIGHTRED);
	//xyprintf(c*widthOfBlock+xOfChar, r*heightOfBlock+yOfChar, "@");
	setfillcolor(BLACK);
	ege_fillellipse((c+0.2)*widthOfBlock, (r+0.2)*heightOfBlock, 0.6*widthOfBlock, 0.6*heightOfBlock);
	setcolor(BLACK);
	setlinewidth(1);
	line(c*widthOfBlock+widthOfBlock/8, r*heightOfBlock+heightOfBlock/2, (c+1)*widthOfBlock-widthOfBlock/8, r*heightOfBlock+heightOfBlock/2);
	line(c*widthOfBlock+widthOfBlock/2, r*heightOfBlock+heightOfBlock/8, c*widthOfBlock+widthOfBlock/2, (r+1)*heightOfBlock-heightOfBlock/8);
	line(c*widthOfBlock+widthOfBlock/4, r*heightOfBlock+heightOfBlock/4, (c+1)*widthOfBlock-widthOfBlock/4, (r+1)*heightOfBlock-heightOfBlock/4);
	line(c*widthOfBlock+widthOfBlock/4, (r+1)*heightOfBlock-heightOfBlock/4, (c+1)*widthOfBlock-widthOfBlock/4, r*heightOfBlock+heightOfBlock/4);
	setfillcolor(WHITE);
	ege_fillrect((c+11.0/32)*widthOfBlock, (r+11.0/32)*heightOfBlock, (4.0/32)*widthOfBlock, (4.0/32)*heightOfBlock);
}

void DrawMineA(int x0, int y0, int r)//绘制地雷图形
{
	setfillcolor(BLACK);
	//r=16
	ege_fillellipse(x0-0.6*r, y0-0.6*r, 0.6*2*r, 0.6*2*r);
	setcolor(BLACK);
	setlinewidth(1);
	line(x0-r+r/4, y0, x0+r-r/4, y0);
	line(x0, y0-r+r/4, x0, y0+r-r/4);
	line(x0-r/2, y0-r/2, x0+r/2, y0+r/2);
	line(x0-r/2, y0+r/2, x0+r/2, y0-r/2);
	setfillcolor(WHITE);
	ege_fillrect(x0-r+11.0/16*r, y0-r+11.0/16*r, 4.0/16*r, 4.0/16*r);
}

void DrawFlag(int r, int c)//绘制地图旗帜
{
	//setcolor(LIGHTRED);
	//xyprintf(c*widthOfBlock+xOfChar, r*heightOfBlock+yOfChar, "#");
	setfillcolor(BLACK);
	//绘制底座
	ege_fillrect((c+8.0/32)*widthOfBlock, (r+24.0/32)*heightOfBlock, 16.0/32*widthOfBlock, 2.0/32*heightOfBlock);
	ege_fillrect((c+10.0/32)*widthOfBlock, (r+22.0/32)*heightOfBlock, 12.0/32*widthOfBlock, 2.0/32*heightOfBlock);
	//绘制旗杆
	ege_fillrect((c+15.0/32)*widthOfBlock, (r+16.0/32)*heightOfBlock, 2.0/32*widthOfBlock, 8.0/32*heightOfBlock);
	//绘制旗帜
	setfillcolor(RED);
	ege_point polyPoints[3] =
	{
		{(c+6.0/32)*widthOfBlock, (r+11.0/32)*heightOfBlock},
		{(c+17.0/32)*widthOfBlock, (r+6.0/32)*heightOfBlock},
		{(c+17.0/32)*widthOfBlock, (r+16.0/32)*heightOfBlock}
	};
	ege_fillpoly(3, polyPoints);
}

void DrawBlock(int r, int c, int board, int isShown)//绘制方块
{
	//绘制底纹
	if(isShown == 1)
	{
		setfillcolor(DARKGRAY);
		setfontbkcolor(DARKGRAY);
	}
	else
	{
		setfillcolor(LIGHTGRAY);
		setfontbkcolor(LIGHTGRAY);
	}
	ege_fillrect(c*widthOfBlock, r*heightOfBlock, widthOfBlock, heightOfBlock);
	//绘制文字
	//xyprintf(c*widthOfBlock+12, r*heightOfBlock+8, "%d", board);
	//rectprintf(c*widthOfBlock, r*heightOfBlock, widthOfBlock, heightOfBlock, "%d", board);
	if(isShown == 1 && board != 0)
	{
		if(board == 9)
		{
			DrawMine(r, c);
		}
		else
		{
			if(board == 1)
			{
				setcolor(BLUE);
			}
			else if(board == 2)
			{
				setcolor(GREEN);
			}
			else if(board < 6)//3-5
			{
				setcolor(RED);
			}
			else
			{
				setcolor(YELLOW);
			}
			xyprintf(c*widthOfBlock+xOfChar, r*heightOfBlock+yOfChar, "%d", board);
		}
	}
	else if(isShown == 2)
	{
		DrawFlag(r, c);
	}
	//绘制边框
	if(isShown == 1)
	{
		setlinewidth(1);
		setcolor(GRAY);
		ege_rectangle(c*widthOfBlock, r*heightOfBlock, widthOfBlock, heightOfBlock);
	}
	else
	{
		setlinewidth(2);
		setcolor(WHITE);
		line(c*widthOfBlock, r*heightOfBlock, (c+1)*widthOfBlock, r*heightOfBlock);
		line(c*widthOfBlock, r*heightOfBlock, c*widthOfBlock, (r+1)*heightOfBlock);
		setcolor(GRAY);
		line((c+1)*widthOfBlock-1, r*heightOfBlock, (c+1)*widthOfBlock-1, (r+1)*heightOfBlock);
		line(c*widthOfBlock, (r+1)*heightOfBlock-1, (c+1)*widthOfBlock, (r+1)*heightOfBlock-1);
	}
}

void DrawLineA(int x0, int y0, int r, int angle)//绘制时钟指针
{
	float rad;
	int x1, y1;
	rad = angle*PI/180;
	x1 = x0 + r*cos(rad);
	y1 = y0 + r*sin(rad);
	line(x0, y0, x1, y1);
}

void DrawClock(int x0, int y0, int r, int time)//绘制时钟
{
	int second, minute, hour;
	// 获取时间
	second = time%60;
	minute = time/60;
	hour = minute/60+8;
	minute = minute%60;
	hour = hour%24;
	// 绘制时钟
	setfillcolor(WHITE);
	ege_fillellipse(x0-r, y0-r, 2*r, 2*r);
	setlinewidth(2);
	setcolor(BLACK);
	circle(x0, y0, r);
	//秒针
	setlinewidth(1);
	setcolor(RED);
	DrawLineA(x0, y0, r*4/5, 270+second*6);
	setcolor(BLACK);
	//分针
	setlinewidth(1);
	DrawLineA(x0, y0, r*3/4, 270+minute*6);
	//时针
	setlinewidth(2);
	DrawLineA(x0, y0, r/2, 270+hour%12*30);
	//转轴
	setfillcolor(RED);
	ege_fillellipse(x0-r/10, y0-r/10, r/5, r/5);
	// 输出文字时间
	//xyprintf(x0+r+r/5, y0+r-r/5, "%2d:%2d:%2d", hour, minute, second);
}

void DrawBoard(int mode, int remainder, int t)//绘制总外部窗口
{
	int r, c;
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(mode == 1)//后台
			{
				if(isShown[r][c] == 2)
				{
					DrawBlock(r+2, c, board[r][c], 2);
				}
				else
				{
					DrawBlock(r+2, c, board[r][c], 1);
				}
			}
			else if(mode == 0)//前台
			{
				DrawBlock(r+2, c, board[r][c], isShown[r][c]);
			}
		}
	}
	if(widthOfBoard > 10)
	{
		//剩余雷数
		DrawMineA(2*widthOfBlock, heightOfBar/2, 20*heightOfBar/64*4/3);
		setcolor(RED);
		setfontbkcolor(BLACK);
		xyprintf(3*widthOfBlock+xOfChar, (heightOfBar-heightOfChar)/2, " %d ", remainder);
		//用时
		DrawClock((widthOfBoard-2)*widthOfBlock, heightOfBar/2, 20*heightOfBar/64, time(0));//按真实时间走的钟(doge)
		setcolor(RED);
		setfontbkcolor(BLACK);
		xyprintf((widthOfBoard-5)*widthOfBlock+xOfChar, (heightOfBar-heightOfChar)/2, " %d ", t);
	}
	else
	{
		//剩余雷数
		DrawMineA(1*widthOfBlock, heightOfBar/2, 20*heightOfBar/64*4/3);
		setcolor(RED);
		setfontbkcolor(BLACK);
		xyprintf(2*widthOfBlock+xOfChar, (heightOfBar-heightOfChar)/2, " %d ", remainder);
		//用时
		DrawClock((widthOfBoard-1)*widthOfBlock, heightOfBar/2, 20*heightOfBar/64, time(0));//按真实时间走的钟(doge)
		setcolor(RED);
		setfontbkcolor(BLACK);
		xyprintf((widthOfBoard-4)*widthOfBlock+xOfChar, (heightOfBar-heightOfChar)/2, " %d ", t);
	}
}
*/
int Difficulty(int height, int width, int mine)//根据地图信息判断难度
{
	if(height == 10 && width == 10 && mine == 10) return 0;
	else if(height == 9 && width == 9 && mine == 10) return 1;
	else if(height == 16 && width == 16 && mine == 40) return 2;
	else if(height == 16 && width == 30 && mine == 99) return 3;
	else if(height == 42 && width == 88 && mine == 715) return 4;
	else return 5;
}

int IsAroundZeroChain(int r0, int c0)
{
	int r, c;
	for(r=r0-1; r<=r0+1; r++)
	{
		for(c=c0-1; c<=c0+1; c++)
		{
			if(r>=0 && r<heightOfBoard && c>=0 && c<widthOfBoard)//确认在范围内
			{
				if(zeroChain[r][c] == 1)
				{
					return 1;
				}
			}
		}
	}
	return 0;
}

void OpenZeroChain(int r0, int c0)//翻开0连锁翻开
{
	int r, c, isRising;
	if(isShown[r0][c0] == 1 && board[r0][c0] == 0)
	{
		//生成0链SummonZeroChain(r0, c0);
		for(r=0; r<heightOfBoard; r++)//初始化
		{
			for(c=0; c<widthOfBoard; c++)
			{
				zeroChain[r][c] = 0;
			}
		}
		zeroChain[r0][c0] = 1;
		isRising = 1;
		while(isRising == 1)//0链向四边生长
		{
			isRising = 0;
			for(r=0; r<heightOfBoard; r++)
			{
				for(c=0; c<widthOfBoard; c++)
				{
					if(board[r][c] == 0 && zeroChain[r][c] == 0)//可能的生长点
					{
						if(IsAroundZeroChain(r, c) == 1)//与0链连接
						{
							zeroChain[r][c] = 1;
							isRising = 1;
						}
					}
				}
			}
		}
		/*if(debug == 2)
		{
			printf("[Debug]已生成0链：\n");
			for(r=0; r<heightOfBoard; r++)
			{
				printf("  ");
				for(c=0; c<widthOfBoard; c++)
				{
					printf("%d ", zeroChain[r][c]);
				}
				printf("\n");
			}
			system("pause");
		}*/
		for(r=0; r<heightOfBoard; r++)//周围有0链则翻开
		{
			for(c=0; c<widthOfBoard; c++)
			{
				if(isShown[r][c] == 0)
				{
					if(IsAroundZeroChain(r, c) == 1)//在0链上或与0链连接
					{
						isShown[r][c] = 1;
					}
				}
			}
		}
	}
}

int RealRemainder()//真实的剩余雷数
{
	int r, c, remainder = 0;
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(isMine[r][c] == 1 && isShown[r][c] == 0)
			{
				remainder++;
			}
		}
	}
	return remainder;
}

int NumberOfNotShown()//未翻开或标记的数量(%)
{
	int r, c, n = 0;
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(isShown[r][c] == 0)
			{
				n++;
			}
		}
	}
	return n;
}

int NumberOfSign()//标记的数量(#)
{
	int r, c, n = 0;
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(isShown[r][c] == 2)
			{
				n++;
			}
		}
	}
	return n;
}

int NumberOfNotShownAround(int r0, int c0)
{
	int r, c, n = 0;
	for(r=r0-1; r<=r0+1; r++)
	{
		for(c=c0-1; c<=c0+1; c++)
		{
			if(r>=0 && r<heightOfBoard && c>=0 && c<widthOfBoard)//确认在范围内
			{
				if(isShown[r][c] == 0)
				{
					n++;
				}
			}
		}
	}
	return n;
}

int NumberOfSignAround(int r0, int c0)
{
	int r, c, n = 0;
	for(r=r0-1; r<=r0+1; r++)
	{
		for(c=c0-1; c<=c0+1; c++)
		{
			if(r>=0 && r<heightOfBoard && c>=0 && c<widthOfBoard)//确认在范围内
			{
				if(isShown[r][c] == 2)
				{
					n++;
				}
			}
		}
	}
	return n;
}

int WASDMove(int* r, int* c, WORD wVirtualKeyCode)
{
	if(wVirtualKeyCode == 'W' || wVirtualKeyCode == VK_UP)//W/w/方向键上
	{
		if((*r) > 0)//*r > 0
		{
			(*r)--;//*与++优先级一致，具有右结合性，无法取消括号
			return 1;
		}
	}
	else if(wVirtualKeyCode == 'A' || wVirtualKeyCode == VK_LEFT)
	{
		if((*c) > 0)
		{
			(*c)--;
			return 1;
		}
	}
	else if(wVirtualKeyCode == 'S' || wVirtualKeyCode == VK_DOWN)
	{
		if((*r) < heightOfBoard-1)
		{
			(*r)++;
			return 1;
		}
	}
	else if(wVirtualKeyCode == 'D' || wVirtualKeyCode == VK_RIGHT)
	{
		if((*c) < widthOfBoard-1)
		{
			(*c)++;
			return 1;
		}
	}
	return 0;
}

void LookMap()//实时游戏中获取地图、已知数字
{
	int r, c;
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			numberShown[r][c] = 9;
			if(isShown[r][c] == 1)
			{
				numberShown[r][c] = board[r][c];
				if(board[r][c] == 0)
				{
					map[r][c] = ' ';
				}
				else
				{
					map[r][c] = board[r][c]-1+'1';
				}
			}
			else if(isShown[r][c] == 2)
			{
				map[r][c] = '#';
			}
			else
			{
				map[r][c] = '%';
			}
		}
	}
	/*if(debug == 2)
	{
		printf("[Debug]已获取地图：\n");
		printf("    ");
		for(c=0; c<widthOfBoard; c++)
		{
			printf("%d ", c%10);
		}
		printf("\n");
		for(r=0; r<heightOfBoard; r++)
		{
			printf("%3d ", r);
			for(c=0; c<widthOfBoard; c++)
			{
				printf("%c ", map[r][c]);
			}
			printf("\n");
		}
		printf("[Debug]已获取已知数字：\n");
		for(r=0; r<heightOfBoard; r++)
		{
			printf("  ");
			for(c=0; c<widthOfBoard; c++)
			{
				printf("%d ", numberShown[r][c]);
			}
			printf("\n");
		}
		//system("pause");
	}*/
}

void TranslateMap(int rs0, int cs0)//将输入翻译为地图、显示方式矩阵、已知数字
{
	int r, c, rs, cs;
	//获取地图
	for(r=0,rs=rs0; r<heightOfBoard; r++,rs++)
	{
		for(c=0,cs=cs0; c<widthOfBoard; c++,cs+=2)
		{
			map[r][c] = mapShown[rs][cs];
		}
	}
	if(debug == 1 || debug == 2)
	{
		printf("[Debug]已获取地图：\n");
		printf("    ");
		for(c=0; c<widthOfBoard; c++)
		{
			printf("%d ", c%10);
		}
		printf("\n");
		for(r=0; r<heightOfBoard; r++)
		{
			printf("%3d ", r);
			for(c=0; c<widthOfBoard; c++)
			{
				printf("%c ", map[r][c]);
			}
			printf("\n");
		}
	}
	//识别显示方式矩阵
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(map[r][c] == '%' || map[r][c] == '?')
			{
				isShown[r][c] = 0;
			}
			else if(map[r][c] == '#')
			{
				isShown[r][c] = 2;
			}
			else
			{
				isShown[r][c] = 1;
			}
		}
	}
	/*if(debug == 2)
	{
		printf("[Debug]已获取显示方式矩阵：\n");
		for(r=0; r<heightOfBoard; r++)
		{
			printf("  ");
			for(c=0; c<widthOfBoard; c++)
			{
				printf("%d ", isShown[r][c]);
			}
			printf("\n");
		}
	}*/
	//获取已知数字
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			numberShown[r][c] = 9;
			if(isShown[r][c] == 1)
			{
				if(map[r][c] == ' ' || map[r][c] == '_')
				{
					numberShown[r][c] = 0;
				}
				else if(map[r][c]>='1' && map[r][c]<='9')
				{
					numberShown[r][c] = map[r][c]-'1'+1;//转为数字
				}
			}
		}
	}
	/*if(debug == 2)
	{
		printf("[Debug]已获取已知数字：\n");
		for(r=0; r<heightOfBoard; r++)
		{
			printf("  ");
			for(c=0; c<widthOfBoard; c++)
			{
				printf("%d ", numberShown[r][c]);
			}
			printf("\n");
		}
		//system("pause");
	}*/
}

int Solve()//程序核心部分(doge)
{
	int r, c, ra, ca;
	int isSolving, isThinking, isFound, isFoundOpen;
	float minMineRateNotZero;
	/*重置*/
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			solution[r][c] = 0;//0无 1@ 2#
			isThought[r][c] = 0;//0无，1交界线处未知方块，2交界线处非0数字，3处理后不确定的1，4因过长放弃的1
			isMineRate[r][c] = 0;
		}
	}
	/*寻找突破点*/
	isSolving = 1;
	while(isSolving == 1)
	{
		isSolving = 0;
		/*执行策略*/
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				if(isShown[r][c] == 1 && numberShown[r][c] != 0 && NumberOfNotShownAround(r, c) != 0)//寻找数字
				{
					if(numberShown[r][c] == NumberOfNotShownAround(r, c) + NumberOfSignAround(r, c))
					{
						/*策略一
						当某块的数字与未知方块的个数相同时，那么未知方块都是雷*/
						if(debug == 2) printf("[Debug]找到1类点(%d,%d)->#", r, c);
						//标记周围未知方块
						for(ra=r-1; ra<=r+1; ra++)
						{
							for(ca=c-1; ca<=c+1; ca++)
							{
								if(ra>=0 && ra<heightOfBoard && ca>=0 && ca<widthOfBoard)//确认在范围内
								{
									if(isShown[ra][ca] == 0)
									{
										solution[ra][ca] = 2;
										if(debug == 2) printf("(%d,%d)", ra, ca);
									}
								}
							}
						}
						if(debug == 2) printf("\n");
						isSolving = 1;
					}
					if(numberShown[r][c] == NumberOfSignAround(r, c))
					{
						/*策略二
						当某块的数字与周围旗子的个数相同时，那么未知方块都不是雷*/
						if(debug == 2) printf("[Debug]找到2类点(%d,%d)->@", r, c);
						//翻开周围未知方块
						for(ra=r-1; ra<=r+1; ra++)
						{
							for(ca=c-1; ca<=c+1; ca++)
							{
								if(ra>=0 && ra<heightOfBoard && ca>=0 && ca<widthOfBoard)//确认在范围内
								{
									if(isShown[ra][ca] == 0)
									{
										solution[ra][ca] = 1;
										if(debug == 2) printf("(%d,%d)", ra, ca);
									}
								}
							}
						}
						if(debug == 2) printf("\n");
						isSolving = 1;
					}
				}
			}
		}
		if(isSolving == 0)//策略一二无法进行
		{
			/*标记交界线处未知方块*/
			/*for(r=0; r<heightOfBoard; r++)
			{
				for(c=0; c<widthOfBoard; c++)
				{
					if(isShown[r][c] == 0 && isThought[r][c] == 0 && NumberOfNumberAround(r, c) != 0)
					{
						isThought[r][c] = 1;
						map[r][c] = '?';
					}
				}
			}*/
			/*检查待推理方块存在*/
			/*isThinking = 0;
			for(r=0; r<heightOfBoard; r++)
			{
				for(c=0; c<widthOfBoard; c++)
				{
					if(isThought[r][c] == 1)
					{
						isThinking = 1;
						break;
					}
				}
				if(isThinking == 1) break;
			}*/
			/*检查交界线处未知方块存在*/
			isThinking = 0;
			for(r=0; r<heightOfBoard; r++)
			{
				for(c=0; c<widthOfBoard; c++)
				{
					if(isThought[r][c] == 1) isThinking = 1;
					else if(isShown[r][c] == 0 && isThought[r][c] == 0 && NumberOfNumberAround(r, c) != 0)
					{
						isThought[r][c] = 1;//标记交界线处未知方块
						map[r][c] = '?';
						isThinking = 1;//检查待推理方块存在
					}
				}
			}
			if(isThinking == 1)
			{
				/*策略三
				逻辑推理*/
				if(debug == 2) printf("[Debug]正在深入思考. . . \n");
				isSolving = Think();
				if(isSolving == 0)//策略三无法进行
				{
					/*策略四
					枚举判断*/
					isSolving = DeepThink();//0放弃，1尝试下一未知链
					/*if(isSolving == 0)//全部未知链枚举完毕
					{
						//策略五 根据多块枚举的整体结果进行剩余雷数判断
						isSolving = WholeThink();
					}*/
				}
			}
		}
		/*更新地图和显示方式矩阵*/
		if(isSolving == 1)
		{
			for(r=0; r<heightOfBoard; r++)
			{
				for(c=0; c<widthOfBoard; c++)
				{
					if(solution[r][c] == 2)
					{
						map[r][c] = '#';
						isShown[r][c] = 2;
						isThought[r][c] = 0;
					}
					else if(solution[r][c] == 1)
					{
						map[r][c] = '@';
						isShown[r][c] = 1;
						isThought[r][c] = 0;
					}
				}
			}
			if(debug == 2)
			{
				printf("[Debug]已更新地图：\n");
				printf("    ");
				for(c=0; c<widthOfBoard; c++)
				{
					printf("%d ", c%10);
				}
				printf("\n");
				for(r=0; r<heightOfBoard; r++)
				{
					printf("%3d ", r);
					for(c=0; c<widthOfBoard; c++)
					{
						printf("%c ", map[r][c]);
					}
					printf("\n");
				}
				/*printf("[Debug]已更新显示方式矩阵：\n");
				for(r=0; r<heightOfBoard; r++)
				{
					printf("  ");
					for(c=0; c<widthOfBoard; c++)
					{
						printf("%d ", isShown[r][c]);
					}
					printf("\n");
				}*/
			}
		}
	}
	/*准备输出*/
	isFound = 0;
	isFoundOpen = 0;
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(solution[r][c] == 1)
			{
				isFound = 1;
				isFoundOpen = 1;
			}
			else if(solution[r][c] == 2)
			{
				isFound = 1;
			}
			if(isShown[r][c] == 2)
			{
				isMineRate[r][c] = 1;
			}
		}
	}
	SummonMineRateForNotShown();
	minMineRateNotZero = 1;
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(isMineRate[r][c] != 0 && isMineRate[r][c] < minMineRateNotZero)
			{
				minMineRateNotZero = isMineRate[r][c];
			}
			//枚举后根据剩余雷数判断
			if(isMineRate[r][c] == 0 && isShown[r][c] != 1)//雷率可能为-0或负数
			{
				solution[r][c] = 1;
				isShown[r][c] = 1;
				isFound = 1;
				isFoundOpen = 1;
				if(debug == 2) printf("[Debug]找到7类点@(%d,%d)\n", r, c);
			}
			if(isMineRate[r][c] == 1 && isShown[r][c] != 2)
			{
				solution[r][c] = 2;
				isShown[r][c] = 2;
				isFound = 1;
				if(debug == 2) printf("[Debug]找到8类点#(%d,%d)\n", r, c);
			}
		}
	}
	if(debug == 1 || debug == 2)
	{
		printf("[Debug]已生成雷率矩阵：\n");
		printf("    ");
		for(c=0; c<widthOfBoard; c++)
		{
			printf("%4d ", c);
		}
		printf("\n");
		for(r=0; r<heightOfBoard; r++)
		{
			printf("%3d ", r);
			for(c=0; c<widthOfBoard; c++)
			{
				if(isMineRate[r][c] == 0) printf("     ");
				else if(isMineRate[r][c] == 1) printf("Mine ");
				else printf("%.2f ", isMineRate[r][c]);
			}
			printf("\n");
		}
		printf("[Debug]最低非0雷率%.2f\n", minMineRateNotZero);
	}
	if(isFound == 0)
	{
		/*策略四
		没活了，乱翻一个(doge)*/
		/*if(debug == 1 || debug == 2)
		{
			printf("[Debug]正在随机输出. . . \n");
		}
		while(1)
		{
			r = rand()%heightOfBoard;
			c = rand()%widthOfBoard;
			//if(isShown[r][c] == 0) break;
			if(isMineRate[r][c] == minMineRateNotZero) break;
		}
		solution[r][c] = 1;*/
		if(debug == 1 || debug == 2)
		{
			printf("[Debug]无确定解\n");
		}
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				if(isMineRate[r][c] == minMineRateNotZero)
				{
					solution[r][c] = 1;
				}
			}
		}
	}
	if(debug == 1 || debug == 2)
	{
		printf("[Debug]已生成方案矩阵：\n");
		printf("    ");
		for(c=0; c<widthOfBoard; c++)
		{
			printf("%d ", c%10);
		}
		printf("\n");
		for(r=0; r<heightOfBoard; r++)
		{
			printf("%3d ", r);
			for(c=0; c<widthOfBoard; c++)
			{
				if(solution[r][c] == 0)
				{
					printf("  ");
				}
				else if(solution[r][c] == 1)
				{
					printf("@ ");
				}
				else if(solution[r][c] == 2)
				{
					printf("# ");
				}
			}
			printf("\n");
		}
		printf("[Debug]所有方案：\n");
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				if(solution[r][c] != 0)
				{
					if(solution[r][c] == 1)
					{
						printf("@ %d %d\n", r, c);
					}
					else if(solution[r][c] == 2)
					{
						printf("# %d %d\n", r, c);
					}
				}
			}
		}
	}
	/*输出*/
	return isFoundOpen;
}

int Think()
{
	int r, c, r1, c1, r2, c2;//循环变量
	int numberOfMine1, numberOfMine2, numberOfNotShown1, numberOfNotShown2;//数对变量
	int isFound = 0;
	/*标记未知方块附近数字，作为数对之一*/
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(isThought[r][c] == 2)//重置数对之一
			{
				isThought[r][c] = 0;
			}
			if(isShown[r][c] == 1 && numberShown[r][c] != 9 && NumberOfNotShownAround(r, c) != 0)
			{
				isThought[r][c] = 2;
			}
		}
	}
	if(debug == 2)
	{
		printf("[Debug]已追踪：\n");
		for(r=0; r<heightOfBoard; r++)
		{
			printf("  ");
			for(c=0; c<widthOfBoard; c++)
			{
				if(isThought[r][c] == 0) printf("  ");
				else printf("%d ", isThought[r][c]);
			}
			printf("\n");
		}
	}
	/*找数对之二*/
	for(r1=0; r1<heightOfBoard; r1++)
	{
		for(c1=0; c1<widthOfBoard; c1++)
		{
			if(isThought[r1][c1] == 2)//遍历数对之一
			{
				for(r2=r1-2; r2<=r1+2; r2++)
				{
					for(c2=c1-2; c2<=c1+2; c2++)
					{
						if(r2!=r1 || c2!=c1)//遍历周围
						{
							if(r2>=0 && r2<heightOfBoard && c2>=0 && c2<widthOfBoard)//确认在范围内
							{
								if(isShown[r2][c2] == 1 && numberShown[r2][c2] != 9 && numberShown[r2][c2] != 0)//已找到数对之二
								{
									/*计算数对共享区和独占区*/
									for(r=0; r<heightOfBoard; r++)
									{
										for(c=0; c<widthOfBoard; c++)
										{
											numberTeam[r][c] = 0;
										}
									}
									for(r=r1-1; r<=r1+1; r++)
									{
										for(c=c1-1; c<=c1+1; c++)
										{
											if(r>=0 && r<heightOfBoard && c>=0 && c<widthOfBoard)//确认在范围内
											{
												numberTeam[r][c] += 1;
											}
										}
									}
									for(r=r2-1; r<=r2+1; r++)
									{
										for(c=c2-1; c<=c2+1; c++)
										{
											if(r>=0 && r<heightOfBoard && c>=0 && c<widthOfBoard)//确认在范围内
											{
												numberTeam[r][c] += 2;
											}
										}
									}
									numberTeam[r1][c1] = 9;
									numberTeam[r2][c2] = 9;
									//数对之一独占区1，数对之二独占区2，共享区3
									/*求独占区雷数和未知数*/
									numberOfMine1 = 0;
									numberOfMine2 = 0;
									numberOfNotShown1 = 0;
									numberOfNotShown2 = 0;
									for(r=0; r<heightOfBoard; r++)
									{
										for(c=0; c<widthOfBoard; c++)
										{
											if(numberTeam[r][c] == 1)
											{
												if(isShown[r][c] == 2)
												{
													numberOfMine1++;//已过滤界外值，不计入雷数
												}
												else if(isShown[r][c] == 0)
												{
													numberOfNotShown1++;//已过滤界外值，不计入未知数
												}
											}
											else if(numberTeam[r][c] == 2)
											{
												if(isShown[r][c] == 2)
												{
													numberOfMine2++;
												}
												else if(isShown[r][c] == 0)
												{
													numberOfNotShown2++;
												}
											}
										}
									}
									/*逻辑推理*/
									/*if(debug == 2)
									{
										printf("[Debug]正在研究数对：(%d,%d)(%d,%d)\n", r1, c1, r2, c2);
										for(r=0; r<heightOfBoard; r++)
										{
											printf("  ");
											for(c=0; c<widthOfBoard; c++)
											{
												if(numberTeam[r][c] == 0) printf("  ");
												else printf("%d ", numberTeam[r][c]);
											}
											printf("\n");
										}
									}*/
									if(numberOfNotShown2 == 0)//数对之二独占区全已知，1234类数对
									{
										if(numberShown[r1][c1] == numberShown[r2][c2])//相等数对，12类数对
										{
											if(numberOfMine1 == numberOfMine2)//已知雷数相等
											{
												//翻开数对之一独占区未知方块
												for(r=r1-1; r<=r1+1; r++)
												{
													for(c=c1-1; c<=c1+1; c++)
													{
														if(r>=0 && r<heightOfBoard && c>=0 && c<widthOfBoard)//确认在范围内
														{
															if(numberTeam[r][c] == 1 && isThought[r][c] == 1)
															{
																solution[r][c] = 1;
																if(debug == 2) printf("[Debug]找到1类数对(%d,%d)(%d,%d)->@(%d,%d)\n", r1, c1, r2, c2, r, c);
																//return 1;
																isFound = 1;
															}
														}
													}
												}
											}
											if(numberOfMine2 - numberOfMine1 == numberOfNotShown1)//已知雷数之差等于数对之一独占区未知量
											{
												//标记数对之一独占区未知方块
												for(r=r1-1; r<=r1+1; r++)
												{
													for(c=c1-1; c<=c1+1; c++)
													{
														if(r>=0 && r<heightOfBoard && c>=0 && c<widthOfBoard)//确认在范围内
														{
															if(numberTeam[r][c] == 1 && isThought[r][c] == 1)
															{
																solution[r][c] = 2;
																if(debug == 2) printf("[Debug]找到2类数对(%d,%d)(%d,%d)->#(%d,%d)\n", r1, c1, r2, c2, r, c);
																//return 1;
																isFound = 1;
															}
														}
													}
												}
											}
										}
										else//不相等数对，34类数对
										{
											if(numberShown[r2][c2] - numberShown[r1][c1] == numberOfMine2 - numberOfMine1)//数差等于已知雷数差
											{
												//翻开数对之一独占区未知方块
												for(r=r1-1; r<=r1+1; r++)
												{
													for(c=c1-1; c<=c1+1; c++)
													{
														if(r>=0 && r<heightOfBoard && c>=0 && c<widthOfBoard)//确认在范围内
														{
															if(numberTeam[r][c] == 1 && isThought[r][c] == 1)
															{
																solution[r][c] = 1;
																if(debug == 2) printf("[Debug]找到3类数对(%d,%d)(%d,%d)->@(%d,%d)\n", r1, c1, r2, c2, r, c);
																//return 1;
																isFound = 1;
															}
														}
													}
												}
											}
											if(numberShown[r1][c1] - numberShown[r2][c2] == numberOfMine1 - numberOfMine2 + numberOfNotShown1)//数差等于已知雷数差加数对之一独占区未知量
											{
												//标记数对之一独占区未知方块
												for(r=r1-1; r<=r1+1; r++)
												{
													for(c=c1-1; c<=c1+1; c++)
													{
														if(r>=0 && r<heightOfBoard && c>=0 && c<widthOfBoard)//确认在范围内
														{
															if(numberTeam[r][c] == 1 && isThought[r][c] == 1)
															{
																solution[r][c] = 2;
																if(debug == 2) printf("[Debug]找到4类数对(%d,%d)(%d,%d)->#(%d,%d)\n", r1, c1, r2, c2, r, c);
																//return 1;
																isFound = 1;
															}
														}
													}
												}
											}
										}
									}
									else//数对之二独占区不全已知，5678类数对
									{
										if(numberShown[r1][c1] == numberShown[r2][c2])//相等数对，56类数对
										{
											if(numberOfMine1 - numberOfMine2 == numberOfNotShown2)//数对之一和数对之二独占区已知雷数之差等于数对之二独占区未知量
											{
												//翻开数对之一独占区未知方块
												for(r=r1-1; r<=r1+1; r++)
												{
													for(c=c1-1; c<=c1+1; c++)
													{
														if(r>=0 && r<heightOfBoard && c>=0 && c<widthOfBoard)//确认在范围内
														{
															if(numberTeam[r][c] == 1 && isThought[r][c] == 1)
															{
																solution[r][c] = 1;
																if(debug == 2) printf("[Debug]找到5类数对(%d,%d)(%d,%d)->@(%d,%d)\n", r1, c1, r2, c2, r, c);
																//return 1;
																isFound = 1;
															}
														}
													}
												}
											}
											if(numberOfMine2 - numberOfMine1 == numberOfNotShown1)//数对之二和数对之一独占区已知雷数之差等于数对之一独占区未知量
											{
												//标记数对之一独占区未知方块
												for(r=r1-1; r<=r1+1; r++)
												{
													for(c=c1-1; c<=c1+1; c++)
													{
														if(r>=0 && r<heightOfBoard && c>=0 && c<widthOfBoard)//确认在范围内
														{
															if(numberTeam[r][c] == 1 && isThought[r][c] == 1)
															{
																solution[r][c] = 2;
																if(debug == 2) printf("[Debug]找到6类数对(%d,%d)(%d,%d)->#(%d,%d)\n", r1, c1, r2, c2, r, c);
																//return 1;
																isFound = 1;
															}
														}
													}
												}
											}
										}
										else//不相等数对，78类数对，实际是所有数对的通式(doge)
										{
											if(numberShown[r2][c2] - numberShown[r1][c1] == numberOfMine2 - numberOfMine1 + numberOfNotShown2)//数对之二与数对之一之差等于已知雷数差加数对之二独占区未知量
											{
												//翻开数对之一独占区未知方块
												for(r=r1-1; r<=r1+1; r++)
												{
													for(c=c1-1; c<=c1+1; c++)
													{
														if(r>=0 && r<heightOfBoard && c>=0 && c<widthOfBoard)//确认在范围内
														{
															if(numberTeam[r][c] == 1 && isThought[r][c] == 1)
															{
																solution[r][c] = 1;
																if(debug == 2) printf("[Debug]找到7类数对(%d,%d)(%d,%d)->@(%d,%d)\n", r1, c1, r2, c2, r, c);
																//return 1;
																isFound = 1;
															}
														}
													}
												}
											}
											if(numberShown[r1][c1] - numberShown[r2][c2] == numberOfMine1 - numberOfMine2 + numberOfNotShown1)//数对之一与数对之二之差等于已知雷数差加数对之一独占区未知量
											{
												//标记数对之一独占区未知方块
												for(r=r1-1; r<=r1+1; r++)
												{
													for(c=c1-1; c<=c1+1; c++)
													{
														if(r>=0 && r<heightOfBoard && c>=0 && c<widthOfBoard)//确认在范围内
														{
															if(numberTeam[r][c] == 1 && isThought[r][c] == 1)
															{
																solution[r][c] = 2;
																if(debug == 2) printf("[Debug]找到8类数对(%d,%d)(%d,%d)->#(%d,%d)\n", r1, c1, r2, c2, r, c);
																//return 1;
																isFound = 1;
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return isFound;
}

int DeepThink()//枚举
{
	int r, c, r1, c1, r2, c2, i;//循环变量
	int /*isFound, */isFoundThinkChain, isRising, isAroundThinkChain;//未知链生成
	int numberOfThought, numberOfPossibility, remainedMine, remainedNotShown;//枚举准备
	int isWrong, realNumberOfPossibility, temp;//枚举判断
	/*是否枚举*/
	/*if(solveMode == 1)
	{
		isFound = 0;
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				if(solution[r][c] != 0)
				{
					isFound = 1;
				}
			}
		}
		if(isFound == 1) return 0;
	}*/
	/*生成未知链*/
	/*for(r=0; r<heightOfBoard; r++)//重置
	{
		for(c=0; c<widthOfBoard; c++)
		{
			thinkChain[r][c] = 0;
		}
	}
	isFoundThinkChain = 0;
	for(r=0; r<heightOfBoard; r++)//找到生成点
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(isThought[r][c] == 1)
			{
				thinkChain[r][c] = 1;
				isFoundThinkChain = 1;
				break;
			}
		}
		if(isFoundThinkChain == 1) break;
	}*/
	/*枚举准备*/
	isFoundThinkChain = 0;
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(solution[r][c] != 0 && solveMode == 1)
			{
				return 0;//游戏模式已有解不枚举
			}
			thinkChain[r][c] = 0;//重置未知链
			if(isThought[r][c] == 1 && isFoundThinkChain == 0)
			{
				thinkChain[r][c] = 1;
				isFoundThinkChain = 1;//找到生成点
			}
		}
	}
	if(isFoundThinkChain == 0) return 0;//退出
	/*生成未知链*/
	isRising = 1;
	while(isRising == 1)//向周围生长
	{
		isRising = 0;
		for(r2=0; r2<heightOfBoard; r2++)//标记周围数字用于校验
		{
			for(c2=0; c2<widthOfBoard; c2++)
			{
				if(isShown[r2][c2] == 1 && numberShown[r2][c2] != 9)
				{
					isAroundThinkChain = 0;
					for(r=r2-1; r<=r2+1; r++)
					{
						for(c=c2-1; c<=c2+1; c++)
						{
							if(r>=0 && r<heightOfBoard && c>=0 && c<widthOfBoard)//确认在范围内
							{
								if(thinkChain[r][c] == 1)
								{
									isAroundThinkChain = 1;
								}
							}
						}
					}
					if(isAroundThinkChain == 1)
					{
						thinkChain[r2][c2] = 2;
					}
				}
			}
		}
		for(r1=0; r1<heightOfBoard; r1++)//3*3范围有生成点或数字则生长
		{
			for(c1=0; c1<widthOfBoard; c1++)
			{
				if(isThought[r1][c1] == 1 && thinkChain[r1][c1] == 0)//可能的生长点
				{
					isAroundThinkChain = 0;
					for(r=r1-1; r<=r1+1; r++)
					{
						for(c=c1-1; c<=c1+1; c++)
						{
							if(r>=0 && r<heightOfBoard && c>=0 && c<widthOfBoard)//确认在范围内
							{
								if(thinkChain[r][c] == 1 || thinkChain[r][c] == 2)
								{
									isAroundThinkChain = 1;
								}
							}
						}
					}
					if(isAroundThinkChain == 1)
					{
						thinkChain[r1][c1] = 1;
						isRising = 1;
					}
				}
			}
		}
	}
	if(debug == 2)
	{
		printf("[Debug]已选择未知链：\n");
		for(r=0; r<heightOfBoard; r++)
		{
			printf("  ");
			for(c=0; c<widthOfBoard; c++)
			{
				if(thinkChain[r][c] == 0) printf("  ");
				else printf("%d ", thinkChain[r][c]);
			}
			printf("\n");
		}
	}
	/*枚举验证*/
	numberOfThought = 0;
	numberOfPossibility = 1;
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(thinkChain[r][c] == 1)
			{
				numberOfThought++;//未知链长度
			}
		}
	}
	if(numberOfThought > lengthOfThinkChain)
	{
		//int上限2147483647=2^31-1，最多支持长度30的未知链
		//0关 17效率 30最大
		if(debug == 2) printf("[Debug]该未知链过长，已放弃\n");
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				if(thinkChain[r][c] == 1)
				{
					isThought[r][c] = 4;
				}
			}
		}
		numberOfAbandonedThinkChain++;//Bench计数
		return 1;
	}
	for(i=0; i<numberOfThought; i++)//总可能数
	{
		numberOfPossibility *= 2;//妈的，gmon.out给我滚啊
	}
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			numberCheckBase[r][c] = 0;
		}
	}
	for(r1=0; r1<heightOfBoard; r1++)//计算校验基底
	{
		for(c1=0; c1<widthOfBoard; c1++)
		{
			if(isShown[r1][c1] == 2)
			{
				for(r2=r1-1; r2<=r1+1; r2++)
				{
					for(c2=c1-1; c2<=c1+1; c2++)
					{
						if(r2>=0 && r2<heightOfBoard && c2>=0 && c2<widthOfBoard)//确认在范围内
						{
							numberCheckBase[r2][c2]++;
						}
					}
				}
			}
		}
	}
	if(debug == 2)
	{
		printf("[Debug]%d个未知点共%d种可能性\n", numberOfThought, numberOfPossibility);
	}
	remainedMine = numberOfMine - NumberOfSign();//剩余雷数
	remainedNotShown = NumberOfNotShown() - numberOfThought;//不含未知链的剩余%数
	realNumberOfPossibility = 0;
	for(i=0; i<numberOfPossibility; i++)//遍历可能性
	{
		/*进度条*/
		if(numberOfPossibility > 262144 && (i+1)%131072 == 0)
		{
			printf("\r[Loading...]%d/%d", (i+1)/131072, numberOfPossibility/131072);
		}//2^19起等待时间较长，每2^17刷新一次，最多2^30，刷新8192次，对性能影响小
		/*重置*/
		for(c=0; c<numberOfThought; c++)
		{
			possibility[c] = 0;
		}
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				//numberCheck[r][c] = 0;
				numberCheck[r][c] = numberCheckBase[r][c];
			}
		}
		/*生成可能性*/
		r = i;//r既是row又是remainder
		c = 0;//c既是column又是count，一变量多用了属于是(doge)
		while(r > 0)
		{
			possibility[c] = r%2;
			r /= 2;
			c++;
		}
		/*根据雷数排除*/
		r = 0;
		for(c=0; c<numberOfThought; c++)
		{
			r += possibility[c];
		}
		if(r > remainedMine//未知链中雷数大于剩余雷数
			|| remainedNotShown < remainedMine - r)//不含未知链的剩余%数小于剩余雷数减未知链中雷数
		{
			/*if(numberOfPossibility > 262144)//进度条加速
			{
				printf(">>>");
			}*/
			continue;
		}
		/*绘制地图数字*/
		/*c = 0;
		for(r1=0; r1<heightOfBoard; r1++)
		{
			for(c1=0; c1<widthOfBoard; c1++)
			{
				if(thinkChain[r1][c1] == 1)
				{
					if(possibility[c] == 1)
					{
						for(r2=r1-1; r2<=r1+1; r2++)
						{
							for(c2=c1-1; c2<=c1+1; c2++)
							{
								if(r2>=0 && r2<heightOfBoard && c2>=0 && c2<widthOfBoard)//确认在范围内
								{
									numberCheck[r2][c2]++;
								}
							}
						}
					}
					c++;
				}
				if(isShown[r1][c1] == 2)
				{
					for(r2=r1-1; r2<=r1+1; r2++)
					{
						for(c2=c1-1; c2<=c1+1; c2++)
						{
							if(r2>=0 && r2<heightOfBoard && c2>=0 && c2<widthOfBoard)//确认在范围内
							{
								numberCheck[r2][c2]++;
							}
						}
					}
				}
			}
		}*/
		/*if(debug == 2)
		{
			printf("[Debug]假设%d：\n", i);
			for(r=0; r<heightOfBoard; r++)
			{
				printf("  ");
				for(c=0; c<widthOfBoard; c++)
				{
					printf("%d ", numberCheck[r][c]);
				}
				printf("\n");
			}
		}*/
		/*计算并校验地图数字*/
		isWrong = 0;
		c = 0;
		for(r1=0; r1<heightOfBoard; r1++)
		{
			for(c1=0; c1<widthOfBoard; c1++)
			{
				if(thinkChain[r1][c1] == 1)
				{
					if(possibility[c] == 1)
					{
						for(r2=r1-1; r2<=r1+1; r2++)
						{
							for(c2=c1-1; c2<=c1+1; c2++)
							{
								if(r2>=0 && r2<heightOfBoard && c2>=0 && c2<widthOfBoard)//确认在范围内
								{
									numberCheck[r2][c2]++;
									if(numberCheck[r2][c2] > numberShown[r2][c2]) isWrong = 1;
								}
							}
						}
					}
					c++;
				}
				/*if(isShown[r1][c1] == 2)
				{
					for(r2=r1-1; r2<=r1+1; r2++)
					{
						for(c2=c1-1; c2<=c1+1; c2++)
						{
							if(r2>=0 && r2<heightOfBoard && c2>=0 && c2<widthOfBoard)//确认在范围内
							{
								numberCheck[r2][c2]++;
								if(numberCheck[r2][c2] > numberShown[r2][c2]) isWrong = 1;
							}
						}
					}
				}*/
				if(isWrong == 1) break;//提前排除该可能性
			}
			if(isWrong == 1) break;
		}
		if(isWrong == 1) continue;
		/*数字校验*/
		isWrong = 0;
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				if(thinkChain[r][c] == 2)//仅校验未知链附近数字
				{
					if(numberShown[r][c] != numberCheck[r][c])
					{
						isWrong = 1;
					}
				}
			}
		}
		if(isWrong == 0)
		{
			if(realNumberOfPossibility < LimDictionary)
			{
				for(c=0; c<numberOfThought; c++)//写入字典
				{
					dictionary[realNumberOfPossibility][c] = possibility[c];
				}
			}
			realNumberOfPossibility++;
			temp = i;
			if(debug == 2)
			{
				printf("\r[Debug]已找到第%d种可能性%d：", realNumberOfPossibility, temp);//覆盖进度条
				for(c=0; c<numberOfThought; c++)
				{
					printf("%d", possibility[c]);
				}
				printf("\n");
				c = 0;
				printf("    ");
				for(c1=0; c1<widthOfBoard; c1++)
				{
					printf("%d ", c1%10);
				}
				printf("\n");
				for(r1=0; r1<heightOfBoard; r1++)
				{
					printf("%3d ", r1);
					for(c1=0; c1<widthOfBoard; c1++)
					{
						if(thinkChain[r1][c1] == 1 && possibility[c] == 1)
						{
							printf("#?");
							c++;
						}
						else if(thinkChain[r1][c1] == 1 && possibility[c] == 0)
						{
							printf("@?");
							c++;
						}
						else
						{
							printf("%c ", map[r1][c1]);
						}
					}
					printf("\n");
				}
			}
		}
	}
	if(numberOfPossibility > 262144)//进度条
	{
		printf("\n");
	}
	if(realNumberOfPossibility == 1)//输出唯一可能
	{
		for(c=0; c<numberOfThought; c++)
		{
			possibility[c] = 0;
		}
		r = temp;
		c = 0;
		while(r > 0)
		{
			possibility[c] = r%2;
			r /= 2;
			c++;
		}
		if(debug == 2)
		{
			printf("[Debug]已找到唯一可能性%d：", temp);
			for(c=0; c<numberOfThought; c++)
			{
				printf("%d", possibility[c]);
			}
			printf("\n");
		}
		c = 0;
		for(r1=0; r1<heightOfBoard; r1++)
		{
			for(c1=0; c1<widthOfBoard; c1++)
			{
				if(thinkChain[r1][c1] == 1)
				{
					if(possibility[c] == 1)
					{
						solution[r1][c1] = 2;
						if(debug == 2) printf("[Debug]找到3类点#(%d,%d)\n", r1, c1);
					}
					if(possibility[c] == 0)
					{
						solution[r1][c1] = 1;
						if(debug == 2) printf("[Debug]找到4类点@(%d,%d)\n", r1, c1);
					}
					c++;
				}
			}
		}
	}
	else
	{
		/*处理字典*/
		if(realNumberOfPossibility > LimDictionary)
		{
			if(debug == 1 || debug == 2) printf("[Debug]字典大小不足%d！\n", realNumberOfPossibility);
			realNumberOfPossibility = LimDictionary;
		}
		if(debug == 2)
		{
			printf("[Debug]已生成字典，共%d种可能\n", realNumberOfPossibility);
			for(i=0; i<realNumberOfPossibility; i++)
			{
				printf("%3d: ", i);
				for(c=0; c<numberOfThought; c++)
				{
					printf("%d ", dictionary[i][c]);
				}
				printf("\n");
			}
		}
		for(c=0; c<numberOfThought; c++)
		{
			possibility[c] = 0;//求和
			for(i=0; i<realNumberOfPossibility; i++)
			{
				possibility[c] += dictionary[i][c];
			}
		}
		if(debug == 2)
		{
			printf("Sum: ");
			for(c=0; c<numberOfThought; c++)
			{
				printf("%d ", possibility[c]);
			}
			printf("\n");
		}
		c = 0;
		for(r1=0; r1<heightOfBoard; r1++)//处理该链可解部分
		{
			for(c1=0; c1<widthOfBoard; c1++)
			{
				if(thinkChain[r1][c1] == 1)
				{
					if(possibility[c] == realNumberOfPossibility)
					{
						solution[r1][c1] = 2;
						if(debug == 2) printf("[Debug]找到5类点#(%d,%d)\n", r1, c1);
					}
					else if(possibility[c] == 0)
					{
						solution[r1][c1] = 1;
						if(debug == 2) printf("[Debug]找到6类点@(%d,%d)\n", r1, c1);
					}
					else
					{
						isMineRate[r1][c1] =(float) possibility[c] / realNumberOfPossibility;
						isThought[r1][c1] = 3;//放弃该链
					}
					c++;
				}
			}
		}
	}
	return 1;//该链枚举完毕，尝试下一未知链
}
/*
int WholeThink()
{
	
}
*/
void SummonMineRateForNotShown()
{
	int r, c, realNumberOfNotShown;
	float aveRemainedMine, isMineRateOfNotShown;
	/*计算平均剩余雷数和未枚举的未知方块数*/
	aveRemainedMine = numberOfMine - NumberOfSign();
	realNumberOfNotShown = NumberOfNotShown();
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(isThought[r][c] == 3)
			{
				aveRemainedMine -= isMineRate[r][c];
				realNumberOfNotShown--;
			}
		}
	}
	isMineRateOfNotShown = aveRemainedMine / realNumberOfNotShown;
	if(debug == 2)
	{
		printf("<Debug>\n");
		printf("aveRemainedMine=%.2f\n", aveRemainedMine);
		printf("realNumberOfNotShown=%d\n", realNumberOfNotShown);
		printf("isMineRateOfNotShown=%.2f\n", isMineRateOfNotShown);
	}
	/*生成*/
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(isShown[r][c] == 0 && isThought[r][c] != 3)
			{
				isMineRate[r][c] = isMineRateOfNotShown;
			}
		}
	}
}

int NumberOfNumberAround(int r0, int c0)//周围已显示数字的量，用来确定交界线
{
	int r, c, n = 0;
	for(r=r0-1; r<=r0+1; r++)
	{
		for(c=c0-1; c<=c0+1; c++)
		{
			if(r>=0 && r<heightOfBoard && c>=0 && c<widthOfBoard)//确认在范围内
			{
				if(isShown[r][c] == 1 && numberShown[r][c] != 9)//isShown1存在@numberShown9
				{
					n++;
				}
			}
		}
	}
	return n;
}

void ShowAnswer(int isFoundOpen, int uncertainOpen)//输出一个方案
{
	int rc, cc;
	if(isFoundOpen == 1)//优先翻开左上角
	{
		for(rc=0; rc<heightOfBoard; rc++)
		{
			for(cc=0; cc<widthOfBoard; cc++)
			{
				if(solution[rc][cc] == 1) break;
			}
			if(solution[rc][cc] == 1) break;
		}
	}
	else//随机标记确定点和翻开不确定点
	{
		while(1)//随机选择
		{
			rc = rand()%heightOfBoard;
			cc = rand()%widthOfBoard;
			if(solution[rc][cc] != 0)
			{
				break;
			}
		}
	}
	if(solution[rc][cc] == 1)
	{
		if(uncertainOpen == 1)
		{
			isShown[rc][cc] = 1;//翻开不确定点
		}
		else
		{
			printf(":)\n>@ %d %d\n", rc, cc);
		}
	}
	else if(solution[rc][cc] == 2 && uncertainOpen == 0)
	{
		printf(":)\n># %d %d\n", rc, cc);
	}
}

void ShownModeBak(int mode)//1备份，0恢复
{
	int r, c;
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(mode == 1)
			{
				isShownBak[r][c] = isShown[r][c];
			}
			else if(mode == 0)
			{
				isShown[r][c] = isShownBak[r][c];
			}
		}
	}
}

int IsSolvableMap(int seed, int r0, int c0)
{
	int r, c, temp, isOpenMine = 0;
	ShownModeBak(1);
	SummonBoard(seed, r0, c0);
	isShown[r0][c0] = 1;
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			solution[r][c] = 0;
		}
	}
	OpenZeroChain(r0, c0);
	if(debug == 1 || debug == 2) printf("[Debug]正在分析seed=%d,%d,%d\n", seed, r0, c0);
	while(1)
	{
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				if(solution[r][c] == 1)
				{
					isShown[r][c] = 1;//不确定解必炸
				}
				if(isShown[r][c] == 1)
				{
					if(board[r][c] == 0)
					{
						OpenZeroChain(r, c);
					}
					else if(board[r][c] == 9)
					{
						isOpenMine = 1;
					}
				}
			}
		}
		if(isOpenMine == 1) break;
		if(RealRemainder() == 0) break;
		LookMap();
		temp = debug;
		debug = 0;//不显示求解信息
		Solve();
		debug = temp;
	}
	if(debug == 2)
	{
		ShowBoard(1);
	}
	ShownModeBak(0);
	if(isOpenMine == 1)
	{
		if(debug == 1 || debug == 2) printf("[Debug]该地图不可解\n");
		return 0;
	}
	else
	{
		if(debug == 1 || debug == 2) printf("[Debug]该地图可解\n");
		return 1;
	}
}

void ShowSolution(int yOfMap)//在原地图显示方案矩阵
{
	int r, c;
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(solution[r][c] == 1)//调用PrinrCell显示@
			{
				gotoxy(cs0+2*c, yOfMap+rs0+r);
				if(backgroundColor == 0xf0)
				{
					PrintCell(9, 1, 0xe0);
				}
				else
				{
					PrintCell(9, 1, backgroundColor/16*16 + 0x0e - 0x04);
				}
			}
			else if(solution[r][c] == 2)//调用PrinrCell显示#
			{
				gotoxy(cs0+2*c, yOfMap+rs0+r);
				if(backgroundColor == 0xf0)
				{
					PrintCell(9, 2, 0xe0);
				}
				else
				{
					PrintCell(9, 2, backgroundColor/16*16 + 0x0e - 0x0c);
				}
			}
		}
	}
}

int IsEffectiveRecord(struct Record record)
{
	if(record.difficulty != 5 && record.isHelped == 0 && record.solved3BV == record.total3BV)
	{
		return 1;//胜利记录、非自定义难度记录、未被实时求解指令帮助过的记录
	}
	else
	{
		return 0;//未胜利记录、自定义难度记录、被实时求解指令帮助过的记录
	}
}

void PrintRecords(struct Records records)
{
	struct Record* record = records.record;
	int numberOfRecords = records.numberOfRecords, i;
	//printf("[Records Editer]\n");
	printf("**************************************************************\n");//宽62
	for(i=0; i<numberOfRecords; i++)
	{
		if(IsEffectiveRecord(record[i]))
		{
			ColorStr("有效", backgroundColor/16*16 + 0x02);
		}
		else
		{
			ColorStr("无效", backgroundColor/16*16 + 0x04);
		}
		printf("记录%d:", i);
		printf("Map:%d*%d-%d ", record[i].heightOfBoard, record[i].widthOfBoard, record[i].numberOfMine);
		printf("seed=%d,%d,%d ", record[i].seed, record[i].r0, record[i].c0);
		printf("summonCheckMode=%d ", record[i].summonCheckMode);
		printf("time=%d ", record[i].time);
		printf("3BV:%d/%d ", record[i].solved3BV, record[i].total3BV);
		printf("isHelped=%d\n", record[i].isHelped);
		/*printf("记录%d：地图", i);
		if(record[i].difficulty != 5) printf("%d ", record[i].difficulty);
		else printf("%d*%d-%d ", record[i].heightOfBoard, record[i].widthOfBoard, record[i].numberOfMine);
		printf("种子=(%d,%d,%d) ", record[i].seed, record[i].r0, record[i].c0);
		printf("校验=%d ", record[i].summonCheckMode);
		printf("用时=%d ", record[i].time);
		printf("3BV:%d/%d ", record[i].solved3BV, record[i].total3BV);*/
	}
	printf("**************************************************************\n");
	printf("* 已读取记录%d条\n", numberOfRecords);
	for(i=0; i<5; i++)
	{
		printf("* %d难度时间纪录:%4d | 3BV/s纪录: %.2f\n", i, records.minimumTime[i], records.fastestSpeed[i]);
	}
	/*printf("* 难度0默认|时间纪录:%4d | 3BV/s纪录: %.2f\n", records.minimumTime[0], records.fastestSpeed[0]);
	printf("* 难度1初级|时间纪录:%4d | 3BV/s纪录: %.2f\n", records.minimumTime[1], records.fastestSpeed[1]);
	printf("* 难度2中级|时间纪录:%4d | 3BV/s纪录: %.2f\n", records.minimumTime[2], records.fastestSpeed[2]);
	printf("* 难度3高级|时间纪录:%4d | 3BV/s纪录: %.2f\n", records.minimumTime[3], records.fastestSpeed[3]);
	printf("* 难度4顶级|时间纪录:%4d | 3BV/s纪录: %.2f\n", records.minimumTime[4], records.fastestSpeed[4]);*/
	printf("* 总时间：%d(%.2f小时) 总翻开数：%d\n", records.totalTime, (float)records.totalTime/3600, records.totalSolved3BV);
	printf("* 最大翻开数字：%d\n", records.maxOpenNumber);
	printf("* 玩家等级：Lv.%d(", records.gamerLevel);
	//显示称号
	if(records.gamerLevel == 1) printf("Gamer");
	else if(records.gamerLevel == 2) printf("Gamer*");
	else if(records.gamerLevel == 3) printf("Gamer**");
	else if(records.gamerLevel == 4) printf("Gamer***");
	else if(records.gamerLevel == 5) printf("ProGamer");
	else if(records.gamerLevel == 6) printf("ProGamer*");
	else if(records.gamerLevel == 7) printf("ProGamer**");
	else if(records.gamerLevel == 8) printf("ProGamer***");
	else if(records.gamerLevel == 9) printf("Miner");
	else if(records.gamerLevel == 10) printf("Miner*");
	else if(records.gamerLevel == 11) printf("Miner**");
	else if(records.gamerLevel == 12) printf("Miner***");
	else printf("User");
	printf(")\n");
	printf("*******************************\n");
}

struct Records ReadRecords()
{
	FILE* file;
	struct Records records;
	struct Record* record;
	int numberOfRecords, i;
	//初始化
	records.numberOfRecords = 0;
	for(i=0; i<5; i++)
	{
		records.minimumTime[i] = -1;
		records.fastestSpeed[i] = 0;
	}
	records.totalTime = 0;
	records.totalSolved3BV = 0;
	records.maxOpenNumber = 0;
	records.gamerLevel = 0;
	if((file = fopen("minesweeper-records.txt", "r")))
	{
		fscanf(file, "numberOfRecords=%d\n", &numberOfRecords);
		record =(struct Record*) calloc(numberOfRecords, sizeof(struct Record));
		for(i=0; i<numberOfRecords; i++)
		{
			fscanf(file, "\n");
			fscanf(file, "Map:%d*%d-%d\n", &(record[i].heightOfBoard), &(record[i].widthOfBoard), &(record[i].numberOfMine));
			fscanf(file, "seed=%d,%d,%d\n", &(record[i].seed), &(record[i].r0), &(record[i].c0));
			fscanf(file, "summonCheckMode=%d\n", &(record[i].summonCheckMode));
			fscanf(file, "time=%d\n", &(record[i].time));
			fscanf(file, "3BV:%d/%d\n", &(record[i].solved3BV), &(record[i].total3BV));
			fscanf(file, "isHelped=%d\n", &(record[i].isHelped));
			//计算3BV/s
			record[i].speed =(float) record[i].solved3BV / record[i].time;
			//计算难度
			record[i].difficulty = Difficulty(record[i].heightOfBoard, record[i].widthOfBoard, record[i].numberOfMine);
			//读入一个纪录
			/*if(IsEffectiveRecord(record[i]))
			{
				records.minimumTime[record[i].difficulty] = record[i].time;
				//records.fastestSpeed[record[i].difficulty] = record[i].speed;
			}*/
			//计算总时间和总翻开数
			records.totalTime += record[i].time;
			records.totalSolved3BV += record[i].solved3BV;
		}
		fclose(file);
		records.numberOfRecords = numberOfRecords;
		records.record = record;
		//计算纪录
		for(i=0; i<numberOfRecords; i++)
		{
			if(IsEffectiveRecord(record[i]))
			{
				if(record[i].time < records.minimumTime[record[i].difficulty] || records.minimumTime[record[i].difficulty] == -1)
				{
					records.minimumTime[record[i].difficulty] = record[i].time;
				}
				if(record[i].speed > records.fastestSpeed[record[i].difficulty])
				{
					records.fastestSpeed[record[i].difficulty] = record[i].speed;
				}
			}
		}
		//计算玩家等级
		records.gamerLevel = GamerLevel(records);
		//计算最大翻开数字
		//records.maxOpenNumber = MaxOpenNumber(records);//可暂不计算提升启动速度
	}
	return records;//值传递而不是指针传递
}

void WriteRecords(struct Records records)
{
	FILE* file;
	struct Record* record = records.record;
	int numberOfRecords = records.numberOfRecords, i;
	file = fopen("minesweeper-records.txt", "w");
	fprintf(file, "numberOfRecords=%d\n", numberOfRecords);
	for(i=0; i<numberOfRecords; i++)
	{
		fprintf(file, "\n");
		fprintf(file, "Map:%d*%d-%d\n", record[i].heightOfBoard, record[i].widthOfBoard, record[i].numberOfMine);
		fprintf(file, "seed=%d,%d,%d\n", record[i].seed, record[i].r0, record[i].c0);
		fprintf(file, "summonCheckMode=%d\n", record[i].summonCheckMode);
		fprintf(file, "time=%d\n", record[i].time);
		fprintf(file, "3BV:%d/%d\n", record[i].solved3BV, record[i].total3BV);
		fprintf(file, "isHelped=%d\n", record[i].isHelped);
	}
	fclose(file);
	if(numberOfRecords == 0)//删除文件
	{
		remove("minesweeper-records.txt");
	}
}

struct Records AddRecord(struct Records records, struct Record newRecord)
{
	int i;
	int numberOfRecords = records.numberOfRecords + 1;
	struct Record* record =(struct Record*) calloc(numberOfRecords, sizeof(struct Record));
	//读取record数据
	for(i=0; i<numberOfRecords-1; i++)
	{
		record[i] = records.record[i];
	}
	record[numberOfRecords-1] = newRecord;
	//更新records数据
	if(numberOfRecords != 1) free(records.record);//records非空时释放
	records.record = record;
	records.numberOfRecords++;
	records.totalTime += newRecord.time;
	records.totalSolved3BV += newRecord.solved3BV;
	if(IsEffectiveRecord(newRecord))
	{
		if(newRecord.time < records.minimumTime[newRecord.difficulty] || records.minimumTime[newRecord.difficulty] == -1)
		{
			records.minimumTime[newRecord.difficulty] = newRecord.time;
			printf("恭喜！你更新了当前难度的时间纪录！\n");
			if(debug == 1 || debug == 2)
			{
				printf("[Debug]已更新%d难度时间纪录为%d\n", newRecord.difficulty, newRecord.time);
			}
		}
		if(newRecord.speed > records.fastestSpeed[newRecord.difficulty])
		{
			records.fastestSpeed[newRecord.difficulty] = newRecord.speed;
			printf("恭喜！你更新了当前难度的3BV/s纪录！\n");
			if(debug == 1 || debug == 2)
			{
				printf("[Debug]已更新%d难度3BV/s纪录为%.2f\n", newRecord.difficulty, newRecord.speed);
			}
		}
	}
	return records;
}

struct Records DeleteRecord(struct Records records, int deleteNumber)
{
	int i;
	int numberOfRecords = records.numberOfRecords - 1;
	struct Record* record =(struct Record*) calloc(numberOfRecords, sizeof(struct Record));
	if(deleteNumber >= 0 && deleteNumber < records.numberOfRecords)
	{
		//读取record数据
		for(i=0; i<deleteNumber; i++)
		{
			record[i] = records.record[i];
		}
		for(i=deleteNumber; i<numberOfRecords; i++)
		{
			record[i] = records.record[i+1];
		}
		//更新records数据
		free(records.record);
		records.record = record;
		records.numberOfRecords--;
	}
	return records;
}

int MaxOpenNumber(struct Records records)//计算有效记录最大翻开数字，需申请足够空间
{
	int i, r, c, maxOpenNumber = 0;
	int numberOfRecords = records.numberOfRecords;
	struct Record* record = records.record;
	struct Record temp;
	temp.numberOfMine = numberOfMine;
	temp.heightOfBoard = heightOfBoard;
	temp.widthOfBoard = widthOfBoard;
	temp.summonCheckMode = summonCheckMode;
	//ReallocMemory(42, 88, 1);//程序启动时计算
	if(debug == 2)
	{
		printf("[Debug]即将计算最大翻开数字\n");
		system("pause");
	}
	for(i=0; i<numberOfRecords; i++)
	{
		if(IsEffectiveRecord(record[i]))
		{
			numberOfMine = record[i].numberOfMine;
			heightOfBoard = record[i].heightOfBoard;
			widthOfBoard = record[i].widthOfBoard;
			summonCheckMode = record[i].summonCheckMode;
			SummonBoard(record[i].seed, record[i].r0, record[i].c0);
			//ShowBoard(1);
			for(r=0; r<heightOfBoard; r++)
			{
				for(c=0; c<widthOfBoard; c++)
				{
					if(board[r][c] != 9 && board[r][c] > maxOpenNumber)
					{
						maxOpenNumber = board[r][c];
					}
				}
			}
		}
	}
	if(debug == 2)
	{
		printf("[Debug]最大翻开数字：%d\n", maxOpenNumber);
		system("pause");//现代CPU这么快么？
	}
	//ReallocMemory(42, 88, 0);
	numberOfMine = temp.numberOfMine;
	heightOfBoard = temp.heightOfBoard;
	widthOfBoard = temp.widthOfBoard;
	summonCheckMode = temp.summonCheckMode;
	return maxOpenNumber;
}

int GamerLevel(struct Records records)//计算玩家等级并显示称号
{
	int level = 0;//"User":初始
	//完成任意一条件获取
	if(records.minimumTime[0] != -1 || records.minimumTime[1] != -1)
	{
		level = 1;//"Gamer":赢1场默认或初级地图
	}
	if((records.minimumTime[0] != -1 && records.minimumTime[0] <= 26)//25.60秒内赢默认地图
		|| (records.minimumTime[1] != -1 && records.minimumTime[1] <= 30)//29.44秒内赢初级地图
		|| records.minimumTime[2] != -1)//赢1场中级地图
	{
		level = 2;//"Gamer*"
	}
	if((records.minimumTime[0] != -1 && records.minimumTime[0] <= 19)//18.10秒内赢默认地图
		|| (records.minimumTime[1] != -1 && records.minimumTime[1] <= 21)//20.82秒内赢初级地图
		|| (records.minimumTime[2] != -1 && records.minimumTime[2] <= 129)//128.76秒内赢中级地图
		|| records.minimumTime[3] != -1)//赢1场高级地图
	{
		level = 3;//"Gamer**"
	}
	if((records.minimumTime[0] != -1 && records.minimumTime[0] <= 13)//默认基准12.80秒
		|| (records.minimumTime[1] != -1 && records.minimumTime[1] <= 15)//初级基准14.72秒
		|| (records.minimumTime[2] != -1 && records.minimumTime[2] <= 92)//91.04秒内赢中级地图
		|| (records.minimumTime[3] != -1 && records.minimumTime[3] <= 344))//343.14秒内赢高级地图
	{
		level = 4;//"Gamer***"
	}
	//完成所有条件获取
	if(records.minimumTime[0] != -1 && records.minimumTime[1] != -1
		&& records.minimumTime[2] != -1 && records.minimumTime[3] != -1)
	{
		if(records.minimumTime[0] <= 10//9.05秒内赢默认地图
			&& records.minimumTime[1] <= 11//10.41秒内赢初级地图
			&& records.minimumTime[2] <= 65//中级基准64.38秒
			&& records.minimumTime[3] <= 243)//242.64秒内赢高级地图
		{
			level = 5;//"ProGamer"
			if(records.minimumTime[0] <= 7//6.40秒内赢默认地图
				&& records.minimumTime[1] <= 8//7.36秒内赢初级地图
				&& records.minimumTime[2] <= 46//45.52秒内赢中级地图
				&& records.minimumTime[3] <= 172//高级基准171.57秒
				&& records.minimumTime[4] != -1)//赢1场顶级地图
			{
				level = 6;//"ProGamer*"(Ltabsyy: 3 6 43 146 1621)
				if(records.minimumTime[0] <= 5//4.53秒内赢默认地图
					&& records.minimumTime[1] <= 6//5.21秒内赢初级地图
					&& records.minimumTime[2] <= 33//32.19秒内赢中级地图
					&& records.minimumTime[3] <= 122//121.34秒内赢高级地图
					&& records.minimumTime[4] <= 2478)//2477.04秒内赢顶级地图
				{
					level = 7;//"ProGamer**"
					if(records.minimumTime[0] <= 4//3.20秒内赢默认地图
						&& records.minimumTime[1] <= 4//3.68秒内赢初级地图
						&& records.minimumTime[2] <= 23//22.77秒内赢中级地图
						&& records.minimumTime[3] <= 86//85.79秒内赢高级地图
						&& records.minimumTime[4] <= 1752)//1751.27秒内赢顶级地图
					{
						level = 8;//"ProGamer***"
						//"ProGamer***"完成任意一条件获取
						if(records.minimumTime[0] <= 3//2.26秒内赢默认地图
							|| records.minimumTime[1] <= 3//2.60秒内赢初级地图
							|| records.minimumTime[2] <= 17//16.10秒内赢中级地图
							|| records.minimumTime[3] <= 61//60.67秒内赢高级地图
							|| records.minimumTime[4] <= 1239)//顶级基准1238.52秒
						{
							level = 9;//"Miner"
						}
						if(records.minimumTime[0] <= 2//1.60秒内赢默认地图
							|| records.minimumTime[1] <= 2//1.84秒内赢初级地图
							|| records.minimumTime[2] <= 12//11.38秒内赢中级地图
							|| records.minimumTime[3] <= 43//42.89秒内赢高级地图
							|| records.minimumTime[4] <= 876)//875.90秒内赢顶级地图
						{
							level = 10;//"Miner*"
						}
						if(records.minimumTime[0] <= 2//1.13秒内赢默认地图
							|| records.minimumTime[1] <= 2//1.30秒内赢初级地图
							|| records.minimumTime[2] <= 9//8.05秒内赢中级地图
							|| records.minimumTime[3] <= 31//30.33秒内赢高级地图
							|| records.minimumTime[4] <= 620)//619.26秒内赢顶级地图
						{
							level = 11;//"Miner**"
						}
						if(records.minimumTime[0] <= 1//0.80秒内赢默认地图
							|| records.minimumTime[1] <= 1//0.92秒内赢初级地图
							|| records.minimumTime[2] <= 6//5.69秒内赢中级地图
							|| records.minimumTime[3] <= 22//21.45秒内赢高级地图
							|| records.minimumTime[4] <= 438)//437.95秒内赢顶级地图
						{
							level = 12;//"Miner***"(纪录 0.09 5.80 26.59)
						}
					}
				}
			}
		}
	}
	return level;
}

void PrintMap(struct Map map, int showNumber)
{
	int r, c, ra, ca, i, j, n;
	printf("name:%s", map.name);
	printf("author:%s", map.author);
	printf("Map:%d*%d-%d\n", map.height, map.width, map.numberOfMine);
	//雷区转为后台总板
	for(r=0; r<map.height; r++)
	{
		for(c=0; c<map.width; c++)
		{
			map.board[r][c] = 0;
			if(map.isMine[r][c] == 1)
			{
				map.board[r][c] = 9;
			}
			else if(showNumber == 1)
			{
				for(ra=r-1; ra<=r+1; ra++)
				{
					for(ca=c-1; ca<=c+1; ca++)
					{
						if(ra>=0 && ra<map.height && ca>=0 && ca<map.width)
						{
							if(map.isMine[ra][ca] == 1)
							{
								map.board[r][c]++;
							}
						}
					}
				}
			}
		}
	}
	//集成输出避免多地图内存频繁分配
	for(i=Place(map.width-1); i>0; i--)//列坐标
	{
		for(j=0; j<Place(map.height-1)+1; j++)
		{
			printf(" ");
		}
		for(c=0; c<map.width; c++)
		{
			n = c;
			for(j=1; j<i; j++)
			{
				n /= 10;
			}
			n %= 10;
			if(i != 1 && n == 0)
			{
				printf("  ");
			}
			else
			{
				printf("%d ", n);
			}
		}
		printf("\n");
	}
	for(r=0; r<map.height; r++)
	{
		for(i=0; i<Place(map.height-1)-Place(r); i++)//行坐标
		{
			printf(" ");
		}
		printf("%d ", r);
		for(c=0; c<map.width; c++)
		{
			if(map.board[r][c] == 0)
			{
				PrintCell(0, 1, backgroundColor/16*16);
			}
			else if(map.board[r][c] == 9)
			{
				PrintCell(9, 0, backgroundColor/16*16);
			}
			else if(map.board[r][c] < 6)
			{
				PrintCell(map.board[r][c], 1, backgroundColor/16*16);
			}
			else//6-8
			{
				PrintCell(map.board[r][c], 1, backgroundColor/16*16 + 0x0e - 0x04);
			}
		}
		printf("\n");
	}
}

void PrintMaps(struct Maps maps)
{
	int i;
	int numberOfMaps = maps.numberOfMaps;
	//printf("[Map Editer]\n");
	printf("**************************************************************\n");//宽62
	for(i=0; i<numberOfMaps; i++)
	{
		printf("地图%d：\n", i);
		PrintMap(maps.map[i], 1);
	}
	printf("**************************************************************\n");
	printf("* 已读取地图%d张\n", numberOfMaps);
	printf("*******************************\n");
}

struct Maps ReadMaps()
{
	FILE* file;
	struct Maps maps;
	struct Map* map;
	int** isMine;
	int i, r, c;
	int numberOfMaps;
	maps.numberOfMaps = 0;
	if((file = fopen("minesweeper-maps.txt", "r")))
	{
		fscanf(file, "numberOfMaps=%d\n", &numberOfMaps);
		map =(struct Map*) calloc(numberOfMaps, sizeof(struct Map));
		for(i=0; i<numberOfMaps; i++)
		{
			fscanf(file, "\n");
			map[i].name =(char*) calloc(LimName, sizeof(char));
			fscanf(file, "name:");
			fgets(map[i].name, LimName, file);//读取空格和回车
			map[i].author =(char*) calloc(LimName, sizeof(char));
			fscanf(file, "author:");
			fgets(map[i].author, LimName, file);
			fscanf(file, "Map:%d*%d\n", &(map[i].height), &(map[i].width));
			//分配矩阵空间
			isMine =(int**) MatrixMemory((void**)isMine, map[i].height, map[i].width, sizeof(int), 1);
			map[i].board =(int**) MatrixMemory((void**)map[i].board, map[i].height, map[i].width, sizeof(int), 1);
			//读取地图
			for(r=0; r<map[i].height; r++)
			{
				fscanf(file, "  ");
				for(c=0; c<map[i].width; c++)
				{
					fscanf(file, "%d ", &isMine[r][c]);
				}
				fscanf(file, "\n");
			}
			//计算雷数
			map[i].numberOfMine = 0;
			for(r=0; r<map[i].height; r++)
			{
				for(c=0; c<map[i].width; c++)
				{
					if(isMine[r][c] == 1)
					{
						map[i].numberOfMine++;
					}
				}
			}
			map[i].isMine = isMine;
		}
		fclose(file);
		maps.numberOfMaps = numberOfMaps;
		maps.map = map;
	}
	return maps;
}

void WriteMaps(struct Maps maps)
{
	FILE* file;
	struct Map map;
	int numberOfMaps = maps.numberOfMaps;
	int i, r, c;
	file = fopen("minesweeper-maps.txt", "w");
	fprintf(file, "numberOfMaps=%d\n", numberOfMaps);
	for(i=0; i<numberOfMaps; i++)
	{
		map = maps.map[i];//减少访问次数
		fprintf(file, "\n");
		fprintf(file, "name:%s", map.name);//输出空格和回车
		fprintf(file, "author:%s", map.author);
		fprintf(file, "Map:%d*%d\n", map.height, map.width);
		for(r=0; r<map.height; r++)
		{
			fprintf(file, "  ");
			for(c=0; c<map.width; c++)
			{
				fprintf(file, "%d ", map.isMine[r][c]);
			}
			fprintf(file, "\n");
		}
	}
	fclose(file);
	if(numberOfMaps == 0)
	{
		remove("minesweeper-maps.txt");
	}
}

struct Maps AddMaps(struct Maps maps, struct Map newMap)
{
	int i;
	int numberOfMaps = maps.numberOfMaps + 1;
	struct Map* map =(struct Map*) calloc(numberOfMaps, sizeof(struct Map));
	for(i=0; i<numberOfMaps-1; i++)
	{
		map[i] = maps.map[i];
	}
	map[numberOfMaps-1] = newMap;
	if(numberOfMaps != 1) free(maps.map);
	maps.map = map;
	maps.numberOfMaps++;
	return maps;
}

struct Maps DeleteMap(struct Maps maps, int deleteNumber)
{
	int i;
	int numberOfMaps = maps.numberOfMaps - 1;
	struct Map* map =(struct Map*) calloc(numberOfMaps, sizeof(struct Map));
	if(deleteNumber >= 0 && deleteNumber < maps.numberOfMaps)
	{
		for(i=0; i<deleteNumber; i++)
		{
			map[i] = maps.map[i];
		}
		//释放删除位置空间
		map[i] = maps.map[i];//i=deleteNumber
		free(map[i].name);
		free(map[i].author);
		map[i].isMine =(int**) MatrixMemory((void**)map[i].isMine, map[i].height, map[i].width, sizeof(int), 0);
		map[i].board =(int**) MatrixMemory((void**)map[i].board, map[i].height, map[i].width, sizeof(int), 0);
		for(i=deleteNumber; i<numberOfMaps; i++)
		{
			map[i] = maps.map[i+1];
		}
		free(maps.map);
		maps.map = map;
		maps.numberOfMaps--;
	}
	return maps;
}

struct Maps DeleteMaps(struct Maps maps)//删除所有并释放空间
{
	int i;
	//释放空间
	for(i=0; i<maps.numberOfMaps; i++)
	{
		maps = DeleteMap(maps, i);
	}
	if(maps.numberOfMaps > 0) free(maps.map);
	maps.numberOfMaps = 0;
	return maps;
}

struct Map EditMap(struct Map map)
{
	int r, c;
	int showNumber = 1;
	// 鼠标点击操作
	int rs0 = Place(map.width-1);
	int cs0 = Place(map.height-1) + 1;
	HANDLE hdin = GetStdHandle(STD_INPUT_HANDLE);
	COORD mousePos = {0, 0};
	COORD mouseOperatedPos = {0, 0};
	INPUT_RECORD rcd;
	DWORD rcdnum;
	int isReadyRefreshMouseOperatedPos = 0;
	if(operateMode == 2) SetConsoleMouseMode(1);
	clrscr();
	while(1)
	{
		gotoxy(0, 0);
		PrintMap(map, showNumber);
		if(operateMode == 2)
		{
			printf("[鼠标左键:布雷/地图右键:切换数字显示/地图外右键:退出编辑]\n");
			while(1)
			{
				isReadyRefreshMouseOperatedPos = 1;
				ReadConsoleInput(hdin, &rcd, 1, &rcdnum);
				if(rcd.EventType == MOUSE_EVENT)
				{
					mousePos = rcd.Event.MouseEvent.dwMousePosition;
					if(IsPosInRectangle((mousePos.X-cs0)/2, mousePos.Y-rs0-3, 0, 0, map.width-1, map.height-1))
					{
						r = mousePos.Y-rs0-3;
						c = (mousePos.X-cs0)/2;
						printf("\r>* %d %d  ", r, c);
						if(rcd.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
						{
							if(IsPosInRectangle(mousePos.X, mousePos.Y, mouseOperatedPos.X-1, mouseOperatedPos.Y, mouseOperatedPos.X+1, mouseOperatedPos.Y))
							{
								isReadyRefreshMouseOperatedPos = 0;
							}
							else
							{
								mouseOperatedPos = mousePos;
								break;
							}
						}
						if(rcd.Event.MouseEvent.dwButtonState == RIGHTMOST_BUTTON_PRESSED)
						{
							showNumber = 1-showNumber;
							r = -1;
							c = -1;
							break;//地图区域右键切换数字显示
						}
					}
					else
					{
						if(rcd.Event.MouseEvent.dwButtonState == RIGHTMOST_BUTTON_PRESSED)
						{
							r = -1;
							c = 0;
							break;//其他区域右键退出编辑
						}
					}
				}
				if(isReadyRefreshMouseOperatedPos == 1)
				{
					mouseOperatedPos.X = 0;
					mouseOperatedPos.Y = 0;
				}
			}
		}
		else
		{
			printf("[r/-1:退出] [c/-1:切换数字显示]\n");
			printf(">");
			scanf("%d", &r);
			if(r == -1) break;//读入一个-1就退出
			scanf("%d", &c);
			if(c == -1) showNumber = 1-showNumber;//切换数字显示
			clrscr();
		}
		if(r>=0 && r<map.height && c>=0 && c<map.width)//确认在范围内
		{
			if(map.isMine[r][c] == 0)
			{
				map.isMine[r][c] = 1;
				map.numberOfMine++;
			}
			else
			{
				map.isMine[r][c] = 0;
				map.numberOfMine--;
			}
		}
		if(r == -1 && c == 0) break;//鼠标操作退出
	}
	if(operateMode == 2) SetConsoleMouseMode(0);
	return map;
}

void RCScan(char* operation, int* r, int* c, int yOfCommand, struct Record information)//@#rc指令输入模块
{
	int p = 0;
	char key;
	int remainder = information.numberOfMine;//解包信息
	int t0 = information.time, t1;
	int solved3BV = information.solved3BV;
	int total3BV = information.total3BV;
	int showInformation = information.summonCheckMode;
	//yOfCommand = yOfMapEnd+4
	gotoxy(0, yOfCommand);
	printf(">%c _     \r>%c ", *operation, *operation);//覆写>@ 128 92
	while(p != 4)
	{
		if(kbhit())
		{
			key = getch();//接收字符但不显示
			gotoxy(0, yOfCommand);
			//切为鼠标
			if(key == 'm' || key == 'M')
			{
				*operation = 'm';
				break;
			}
			//wasd处理
			if(p == 3 && (key == 'w' || key == 'W' || key == 'a' || key == 'A'
				|| key == 's' || key == 'S' || key == 'd' || key == 'D'))
			{
				//wasd改变坐标
				if(key == 'w' || key == 'a' || key == 's' || key == 'd')
				{
					WASDMove(r, c, key-'a'+'A');//小写转大写
				}
				else if(key == 'W' || key == 'A' || key == 'S' || key == 'D')
				{
					WASDMove(r, c, key);
				}
				//立即显示完整坐标
				printf("\r>%c %d %d \b", *operation, *r, *c);//\b覆写并回移光标
				continue;
			}
			if((p == 1 || p == 2) && (key == 'w' || key == 'W' || key == 's' || key == 'S'))
			{
				//仅输入r时ws改变r
				if(key == 'w' || key == 's')
				{
					WASDMove(r, c, key-'a'+'A');
				}
				else if(key == 'W' || key == 'S')
				{
					WASDMove(r, c, key);
				}
				p = 2;//改变后可立即输入c
				printf("\r>%c %d _ \b\b", *operation, *r);
				continue;
			}
			//退格符处理
			if(key == '\b')
			{
				if(p == 1 || p == 2)
				{
					*r = 0;
					p = 0;
					//gotoxy(0, yOfCommand);
					printf("\r>%c _   \b\b\b\b", *operation);
					//gotoxy(3, yOfCommand);
				}
				else if(p == 3)
				{
					*c = 0;
					p = 2;
					//gotoxy(0, yOfCommand);
					printf("\r>%c %d _  \b\b\b", *operation, *r);
					//gotoxy(4+Place(*r), yOfCommand);
				}
				else if(p == 0)//清空已输入操作
				{
					*operation = 0;
					//gotoxy(0, yOfCommand);
					printf("\r>   \b\b\b");
					//gotoxy(1, yOfCommand);
					break;
				}
				continue;
			}
			//输入坐标
			if(p == 0)//未输入r
			{
				*r = 0;
				*c = 0;
				if(key >= '0' && key <= '9')
				{
					*r = key-'0'+0;
					p = 1;
				}
			}
			else if(p == 1)//正在输入r
			{
				if(key >= '0' && key <= '9')
				{
					*r *= 10;
					*r += key-'0'+0;
					if(*r >= heightOfBoard)
					{
						*r = 0;
						p = 0;
					}
				}
				else//按空格转到c
				{
					p = 2;
				}
			}
			else if(p == 2)//未输入c
			{
				if(key >= '0' && key <= '9')
				{
					*c = key-'0'+0;
					p = 3;
				}
			}
			else if(p == 3)//正在输入c
			{
				if(key >= '0' && key <= '9')
				{
					*c *= 10;
					*c += key-'0'+0;
					if(*c >= widthOfBoard)
					{
						*c = 0;
						p = 2;
					}
				}
				else//按任意键结束
				{
					p = 4;
				}
			}
			//显示坐标
			//gotoxy(0, yOfCommand);
			if(p == 0)
			{
				printf("\r>%c _  \b\b\b", *operation);
			}
			else if(p == 1)
			{
				//printf(">%c %d", *operation, *r);
				printf("\r>%c ", *operation);
				ColorNumber(*r, 0xff-backgroundColor);
				printf("_ \b\b");
			}
			else if(p == 2)
			{
				printf("\r>%c %d _  \b\b\b", *operation, *r);
			}
			else if(p == 3)
			{
				//printf(">%c %d %d", *operation, *r, *c);
				printf("\r>%c %d ", *operation, *r);
				ColorNumber(*c, 0xff-backgroundColor);
				printf("_ \b\b");
			}
			else if(p == 4)
			{
				printf("\r>%c %d %d \b", *operation, *r, *c);
			}
		}
		else
		{
			if(showInformation == 1)//刷新信息
			{
				gotoxy(0, yOfCommand-3);
				printf("剩余雷数: %d ", remainder);
				t1 = time(0);
				//temp = BBBV(seed, r0, c0, 0);
				if(showTime == 1) printf("用时：%d ", t1-t0);
				if(show3BV == 1) printf("3BV：%d/%d 3BV/s：%.2f ", solved3BV, total3BV, (float)solved3BV/(t1-t0));
				//printf("\n");
				//gotoxy(cs0+2*(*c), yOfCommand-4-heightOfBoard+1+(*r));//光标移到坐标对应位置
				if(p == 1 || p == 2)
				{
					FreshCursor(*r, -1, yOfCommand-4-heightOfMapShown+1);
				}
				else
				{
					FreshCursor(*r, *c, yOfCommand-4-heightOfMapShown+1);
				}
			}
			else//第一次翻开
			{
				//gotoxy(cs0+2*(*c), yOfCommand-2-heightOfBoard+1+(*r));
				if(p == 1 || p == 2)
				{
					FreshCursor(*r, -1, yOfCommand-2-heightOfMapShown+1);
				}
				else
				{
					FreshCursor(*r, *c, yOfCommand-2-heightOfMapShown+1);
				}
				//yOfCommand = heightOfMapShown-1+3+2
			}
		}
		//Sleep(refreshCycle);
	}
}

void Solution()//地图求解模块
{
	int r, c, yOfMap;
	gotoxy(0, 0);
	//printf("MineSweeper Solution\n");
	printf("**************************************************************\n");//宽62
	printf("** 地图求解器使用说明：\n");
	printf("** (1)在本界面，对文本地图求解\n");
	printf("** 在键盘模式，鼠标从含坐标地图左上角到右下角整个选中，\n");
	printf("** 按Ctrl+C或右键复制，在此处Ctrl+V或右键粘贴，回车。\n");
	printf("** 游戏若为鼠标模式，按K切换到键盘模式，按M切回鼠标模式。\n");
	printf("** 需要在设置内选择正确的难度！\n");
	printf("** (2)在实时游戏，对当前局面求解\n");
	printf("** 按!或[(2)地图求解]，在原地图显示当前局面所有方案。\n");
	printf("** 按Tab或[(4)Bench]，将显示并立即执行所有方案。\n");
	printf("** 使用后该局游戏将不会打破纪录！\n");
	printf("** (3)按 ` 并回车退出\n");
	printf("**************************************************************\n");
	printf("当前雷数:%d|当前界面大小:%d*%d\n", numberOfMine, heightOfBoard, widthOfBoard);
	yOfMap = 14;
	srand(time(0));
	if(debug == 2)
	{
		printf("<Debug>\n");
		printf("pointer ->(%d,%d)\n", rs0, cs0);
		printf("heightOfMapShown=%d\n", heightOfMapShown);
		printf("widthOfMapShown=%d\n", widthOfMapShown);
		yOfMap += 4;
	}
	/*扫雷*/
	//getchar();//吃回车
	while(1)
	{
		/*输入*/
		printf("请粘贴最新地图：\n");
		yOfMap++;
		fflush(stdin);//清除不稳定的回车
		for(r=0; r<heightOfMapShown; r++)
		{
			for(c=0; c<widthOfMapShown; c++)//默认为空格
			{
				mapShown[r][c] = ' ';
			}
			//gets(mapShown[r]);//从键盘获取该行地图
			fgets(mapShown[r], 2*LimWidth+5, stdin);
			if(mapShown[r][0] == '`') break;
		}
		if(r < heightOfMapShown && mapShown[r][0] == '`')//不访问未申请空间
		{
			break;
		}
		if(debug == 2)
		{
			printf("[Debug]已获取输入：\n");
			printf("     ");
			for(c=0; c<widthOfMapShown; c++)
			{
				if(c%5 == 0)
				{
					printf("%d", c%10);
				}
				else
				{
					printf(" ");
				}
			}
			printf("\n");
			for(r=0; r<heightOfMapShown; r++)
			{
				printf("%3d: ", r);
				for(c=0; c<widthOfMapShown; c++)
				{
					printf("%c", mapShown[r][c]);
				}
				//printf("\n");//fgets不丢弃回车
			}
		}
		TranslateMap(rs0, cs0);
		/*扫雷策略和输出*/
		ShowAnswer(Solve(), 0);
		ShowSolution(yOfMap);
		yOfMap += heightOfMapShown+2;
		gotoxy(0, yOfMap);
	}
}

void Bench(int seedMin, int seedMax, int r0, int c0, int showStep, int showSolution)//Bench模块
{
	int seed, r, c, isOpenMine, temp;
	int t0, t1, countOfWin = 0, countOfSolution = 0, countOfStep = 0;//Bench统计
	//int isWinningOfSeed[LimDictionary]={0};
	//int BBBVOfSeed[LimDictionary][2]={0};
	int* isWinningOfSeed =(int*) calloc(seedMax-seedMin+1, sizeof(int));
	int** BBBVOfSeed =(int**) MatrixMemory(NULL, seedMax-seedMin+1, 2, sizeof(int), 1);
	int countOf3BV = 0, countOfSolved3BV = 0;
	int maxPlaceOfBBBV = 0, i;//根据最大3BV位数补全3BV矩阵空格
	//int clock0, clock1;
	/*初始化*/
	numberOfAbandonedThinkChain = 0;
	for(seed=seedMin; seed<=seedMax; seed++)
	{
		temp = BBBV(seed, r0, c0, 1);
		BBBVOfSeed[seed-seedMin][1] = temp;
		countOf3BV += temp;
		if(Place(temp) > maxPlaceOfBBBV) maxPlaceOfBBBV = Place(temp);//计算最大3BV位数
	}
	countOfSolved3BV = countOf3BV;
	t0 = time(0);//计时
	//clock0 = clock();
	/*开跑！*/
	for(seed=seedMin; seed<=seedMax; seed++)
	{
		isOpenMine = 0;
		SummonBoard(seed, r0, c0);//Bench时不显示多余信息
		isShown[r0][c0] = 1;
		OpenZeroChain(r0, c0);//第一次打开0链
		//if(debug == 2) ShowBoard(0);//仅显示开局和结束
		while(1)
		{
			if(showStep > 2)
			{
				clrscr();
				DrawProgressBar(100 * (seed-1-seedMin+1) / (seedMax-seedMin+1));
			}
			gotoxy(0, 3);//覆写
			if(showStep > 0)
			{
				//1行Bench置顶信息
				printf("** Map:%d*%d-%d | ", heightOfBoard, widthOfBoard, numberOfMine);
				printf("seed=%d,%d,%d | ", seed, r0, c0);
				printf("Frame %d | ", countOfStep);
				t1 = time(0);//显示Ave FPS（步数/时间）
				printf("FPS %.1f \n", (float)countOfStep/(t1-t0));//覆写掉帧和inf字符
				//if(debug == 1 || debug == 2) ShowBoard(1);
			}
			/*全局已翻开点检查*/
			for(r=0; r<heightOfBoard; r++)
			{
				for(c=0; c<widthOfBoard; c++)
				{
					if(isShown[r][c] == 1)
					{
						if(board[r][c] == 0)
						{
							OpenZeroChain(r, c);
						}
						else if(board[r][c] == 9)
						{
							isOpenMine = 1;
						}
					}
				}
			}
			/*胜负判断*/
			if(isOpenMine == 1) break;
			if(RealRemainder() == 0)
			{
				countOfWin++;
				isWinningOfSeed[seed]++;
				break;
			}
			/*地图求解*/
			if(showStep > 0) ShowBoard(0);
			LookMap();
			if(showStep > 2)
			{
				printf("\n");//隔开Loading显示调试信息
				ShowAnswer(Solve(), 1);
			}
			else
			{
				temp = debug;
				debug = 0;//Bench时不显示多余信息
				gotoxy(0, heightOfMapShown+4);//控制仅显示结束帧时Loading位置相同
				ShowAnswer(Solve(), 1);
				debug = temp;
			}
			if(showStep > 0 && showSolution == 1)//Bench仅显示方案矩阵
			{
				ShowSolution(4);//yOfMap=8
			}
			gotoxy(0, heightOfMapShown+4);
			printf("                     ");//覆写[Loading...]8192/8192，21空格
			/*实时计数*/
			countOfStep++;
			for(r=0; r<heightOfBoard; r++)
			{
				for(c=0; c<widthOfBoard; c++)
				{
					if(solution[r][c] != 0)
					{
						countOfSolution++;
					}
				}
			}
			if(showStep > 1)
			{
				gotoxy(0, heightOfMapShown+4);
				system("pause");//显示在行首
			}
		}
		if(showStep == 0)
		{
			printf("** Map:%d*%d-%d | ", heightOfBoard, widthOfBoard, numberOfMine);
			printf("seed=%d,%d,%d | ", seed, r0, c0);
			printf("Frame %d | ", countOfStep);
			t1 = time(0);
			printf("FPS %.1f \n", (float)countOfStep/(t1-t0));
		}
		ShowBoard(1);
		temp = BBBV(seed, r0, c0, 0);
		BBBVOfSeed[seed-seedMin][0] = BBBVOfSeed[seed-seedMin][1] - temp;
		countOfSolved3BV -= temp;//减未解3BV计算已解3BV
		if(showTime == 1) DrawProgressBar(100 * (seed-seedMin+1) / (seedMax-seedMin+1));//进度条
		if(showStep > 1)
		{
			gotoxy(0, heightOfMapShown+4);//防止显示进度条后光标位置变化
			system("pause");
		}
	}
	/*输出结果*/
	//clock1 = clock();
	t1 = time(0);
	//clrscr();
	gotoxy(0, 3);
	printf("** Map:%d*%d-%d | ", heightOfBoard, widthOfBoard, numberOfMine);
	printf("seed=%d,%d,%d | ", seedMax, r0, c0);
	printf("Frame %d | ", countOfStep);
	printf("FPS %.1f \n", (float)countOfStep/(t1-t0));
	ShowBoard(1);
	if(debug == 1 || debug == 2)
	{
		printf("胜利数：%d 地图数：%d\n", countOfWin, seedMax-seedMin+1);
		printf("方案数：%d 步数：%d 步均方案数：%.2f\n", countOfSolution, countOfStep, (float)countOfSolution/countOfStep);
		printf("用时：%d 3BV：%d/%d 3BV/s：%.2f\n", t1-t0, countOfSolved3BV, countOf3BV, (float)countOfSolved3BV/(t1-t0));
		printf("放弃链数：%d\n", numberOfAbandonedThinkChain);
		printf("胜利矩阵：\n");
		for(seed=seedMin; seed<=seedMax; seed++)
		{
			if(isWinningOfSeed[seed] == 1)
			{
				//printf("%2d ", seed);
				for(i=0; i<Place(seedMax)-Place(seed); i++)
				{
					printf(" ");
				}
				printf("%d ", seed);
			}
			else
			{
				//printf("   ");
				for(i=0; i<Place(seedMax)+1; i++)
				{
					printf(" ");
				}
			}
			if((seed-seedMin+1)%10 == 0) printf("\n");
		}
		if((seedMax-seedMin+1)%10 != 0) printf("\n");
		printf("已解3BV矩阵：\n");
		for(seed=seedMin; seed<=seedMax; seed++)
		{
			if(BBBVOfSeed[seed-seedMin][0] == BBBVOfSeed[seed-seedMin][1])
			{
				for(i=0; i<maxPlaceOfBBBV-Place(BBBVOfSeed[seed-seedMin][1]); i++)
				{
					printf(" ");
				}
				printf("%d ", BBBVOfSeed[seed-seedMin][1]);
				for(i=0; i<maxPlaceOfBBBV+1; i++)
				{
					printf(" ");
				}
			}
			else
			{
				for(i=0; i<maxPlaceOfBBBV-Place(BBBVOfSeed[seed-seedMin][0]); i++)
				{
					printf(" ");
				}
				printf("%d/", BBBVOfSeed[seed-seedMin][0]);
				for(i=0; i<maxPlaceOfBBBV-Place(BBBVOfSeed[seed-seedMin][1]); i++)
				{
					printf(" ");
				}
				printf("%d ", BBBVOfSeed[seed-seedMin][1]);
			}
			if((seed-seedMin+1)%10 == 0) printf("\n");
		}
		if((seedMax-seedMin+1)%10 != 0) printf("\n");
	}
	else
	{
		printf("胜率：%d%% ", countOfWin*100/(seedMax-seedMin+1));
		if(showTime == 1) printf("用时：%d ", t1-t0);
		if(show3BV == 1) printf("3BV：%d/%d 3BV/s：%.2f", countOfSolved3BV, countOf3BV, (float)countOfSolved3BV/(t1-t0));
		printf("\n");
	}
	//if(debug == 2) printf("CPU Clock: %d\n", clock1-clock0);
}

int BBBV(int seed, int r0, int c0, int mode)//计算地图3BV
{
	int bbbv = 0, r, c;
	ShownModeBak(1);
	if(mode == 1)//1计算全部3BV，0计算未解3BV
	{
		SummonBoard(seed, r0, c0);
	}
	/*计算0链数量*/
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(board[r][c] == 0 && isShown[r][c] == 0)
			{
				isShown[r][c] = 1;
				OpenZeroChain(r, c);
				bbbv++;
			}
		}
	}
	/*计算未显示数字数量*/
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(isShown[r][c] == 0 && board[r][c] != 9)
			{
				//isShown[r][c] = 1;
				bbbv++;
			}
		}
	}
	ShownModeBak(0);
	return bbbv;
}

void MapSearch(int seedMin, int seedMax, int r0, int c0)//地图搜索模块
{
	int seed, bbbv, temp;
	for(seed=seedMin; seed<=seedMax; seed++)
	{
		//SummonBoard(seed, r0, c0);
		bbbv = BBBV(seed, r0, c0, 1);
		printf("Map:%d*%d-%d\n", heightOfBoard, widthOfBoard, numberOfMine);
		printf("seed=%d,%d,%d\n", seed, r0, c0);
		printf("3BV=%d\n", bbbv);
		temp = debug;
		debug = 0;//不显示重复信息
		printf("IsSolvableMap=%d\n", IsSolvableMap(seed, r0, c0));
		debug = temp;
		ShowBoard(1);
		//system("pause");
	}
	/*for(seed=seedMin; seed<=seedMax; seed++)
	{
		for(r0=0; r0<heightOfBoard; r0++)
		{
			for(c0=0; c0<widthOfBoard; c0++)
			{
				bbbv = BBBV(seed, r0, c0, 1);
				if(bbbv < 3)//3BV为2只需1次点击
				{
					printf("Map:%d*%d-%d\n", heightOfBoard, widthOfBoard, numberOfMine);
					printf("seed=%d,%d,%d\n", seed, r0, c0);
					printf("3BV=%d\n", bbbv);
					ShowBoard(1);
					//system("pause");
				}
			}
		}
	}*/
	/*int* bbbvCount =(int*) calloc(2*LimDictionary, sizeof(int));
	int i = 0;
	for(seed=seedMin; seed<=seedMax; seed++)
	{
		bbbv = BBBV(seed, r0, c0, 1);
		bbbvCount[bbbv]++;
	}
	for(i=1; i<2*LimDictionary; i++)
	{
		bbbvCount[i] += bbbvCount[i-1];//总数
	}
	for(i=0; i<2*LimDictionary; i++)
	{
		printf("%4d:%4d ", i, bbbvCount[i]);
		if((i+1)%10==0) printf("\n");
	}*/
}

void** MatrixMemory(void** matrix, int rm, int cm, int sizeOfElements, int mode)//矩阵内存操作，1申请，0释放
{
	int r;
	if(mode == 1)
	{
		matrix =(void**) calloc(rm, sizeof(void*));
		for(r=0; r<rm; r++)
		{
			matrix[r] =(void*) calloc(cm, sizeOfElements);
		}
	}
	else if(mode == 0)
	{
		for(r=0; r<rm; r++)
		{
			free(matrix[r]);
		}
		free(matrix);
	}
	return matrix;
}

void ReallocMemory(int height, int width, int mode)//全局矩阵内存重分配，1申请，0释放
{
	//1申请新矩阵空间，0释放旧矩阵空间，2释放旧矩阵空间并申请新矩阵空间
	isMine =(int**) MatrixMemory((void**)isMine, height, width, sizeof(int), mode);
	numberOfMineAround =(int**) MatrixMemory((void**)numberOfMineAround, height, width, sizeof(int), mode);
	board =(int**) MatrixMemory((void**)board, height, width, sizeof(int), mode);
	isShown =(int**) MatrixMemory((void**)isShown, height, width, sizeof(int), mode);
	zeroChain =(int**) MatrixMemory((void**)zeroChain, height, width, sizeof(int), mode);
	isShownBak =(int**) MatrixMemory((void**)isShownBak, height, width, sizeof(int), mode);
	mapShown =(char**) MatrixMemory((void**)mapShown, height+2, 2*width+5, sizeof(char), mode);
	map =(char**) MatrixMemory((void**)map, height, width, sizeof(char), mode);
	numberShown =(int**) MatrixMemory((void**)numberShown, height, width, sizeof(int), mode);
	solution =(int**) MatrixMemory((void**)solution, height, width, sizeof(int), mode);
	isThought =(int**) MatrixMemory((void**)isThought, height, width, sizeof(int), mode);
	numberTeam =(int**) MatrixMemory((void**)numberTeam, height, width, sizeof(int), mode);
	thinkChain =(int**) MatrixMemory((void**)thinkChain, height, width, sizeof(int), mode);
	//possibility =(int*) calloc(32, sizeof(int));
	numberCheckBase =(int**) MatrixMemory((void**)numberCheckBase, height, width, sizeof(int), mode);
	numberCheck =(int**) MatrixMemory((void**)numberCheck, height, width, sizeof(int), mode);
	dictionary =(int**) MatrixMemory((void**)dictionary, LimDictionary, 32, sizeof(int), mode);
	isMineRate =(float**) MatrixMemory((void**)isMineRate, height, width, sizeof(float), mode);
	if(mode == 0)//清除指针残留值
	{
		isMine = 0;
		numberOfMineAround = 0;
		board = 0;
		isShown = 0;
		zeroChain = 0;
		isShownBak = 0;
		mapShown = 0;
		map = 0;
		numberShown = 0;
		solution = 0;
		isThought = 0;
		numberTeam = 0;
		thinkChain = 0;
		//possibility = 0;
		numberCheckBase = 0;
		numberCheck = 0;
		dictionary = 0;
		isMineRate = 0;
	}
}

struct Records RecordsEditer(struct Records records)//记录编辑器模块
{
	int choice, i;
	struct Record newRecord;
	if(dynamicMemory == 1)//计算最大翻开数字
	{
		ReallocMemory(heightOfBoard, widthOfBoard, 0);
		ReallocMemory(42, 88, 1);
		records.maxOpenNumber = MaxOpenNumber(records);
		ReallocMemory(42, 88, 0);
		ReallocMemory(heightOfBoard, widthOfBoard, 1);
	}
	else
	{
		records.maxOpenNumber = MaxOpenNumber(records);
	}
	while(1)
	{
		clrscr();
		PrintRecords(records);
		//printf("[Records Editer]\n");
		printf("(1)删除一条记录\n");
		printf("(2)删除无效记录\n");
		printf("(3)仅保留纪录\n");
		printf("(4)删除全部记录\n");
		printf("(5)保存并重新读取\n");
		printf("(6)保存并退出\n");
		printf("(7)重新读取\n");
		printf("(8)直接退出\n");
		printf("(9)插入一条记录\n");
		printf("*******************************\n");//宽31
		choice = 0;
		printf(">");
		scanf("%d", &choice);
		if(choice == 1)//删除一条记录
		{
			printf("[序号]>");
			scanf("%d", &i);
			if(IsEffectiveRecord(records.record[i]))//是有效记录
			{
				if(records.record[i].time == records.minimumTime[records.record[i].difficulty]
					|| records.record[i].speed == records.fastestSpeed[records.record[i].difficulty])//是纪录
				{
					//纪录初始化
					/*if(records.record[i].time == records.minimumTime[records.record[i].difficulty])
					{
						records.minimumTime[i] = -1;
					}
					if(records.record[i].speed == records.fastestSpeed[records.record[i].difficulty])
					{
						records.fastestSpeed[i] = 0;
					}*/
					printf("[Records Editer]你删除了纪录，建议保存并重新读取以更新纪录\n");
					system("pause");
				}
			}
			records = DeleteRecord(records, i);
		}
		else if(choice == 2)//删除无效记录
		{
			for(i=records.numberOfRecords-1; i>=0; i--)
			{
				if(IsEffectiveRecord(records.record[i]) == 0)
				{
					records = DeleteRecord(records, i);
				}
			}
		}
		else if(choice == 3)//仅保留纪录
		{
			for(i=records.numberOfRecords-1; i>=0; i--)
			{
				if(IsEffectiveRecord(records.record[i]) == 0//无效记录
					|| (records.record[i].time != records.minimumTime[records.record[i].difficulty]
						&& records.record[i].speed != records.fastestSpeed[records.record[i].difficulty]))//不是纪录
				{
					records = DeleteRecord(records, i);
				}
			}
		}
		else if(choice == 4)//删除全部记录
		{
			records.numberOfRecords = 0;
			for(i=0; i<5; i++)
			{
				records.minimumTime[i] = -1;
				records.fastestSpeed[i] = 0;
			}
			free(records.record);
		}
		else if(choice == 5)//保存并重新读取
		{
			WriteRecords(records);
			records = ReadRecords();
			if(dynamicMemory == 1)//计算最大翻开数字
			{
				ReallocMemory(heightOfBoard, widthOfBoard, 0);
				ReallocMemory(42, 88, 1);
				records.maxOpenNumber = MaxOpenNumber(records);
				ReallocMemory(42, 88, 0);
				ReallocMemory(heightOfBoard, widthOfBoard, 1);
			}
			else
			{
				records.maxOpenNumber = MaxOpenNumber(records);
			}
		}
		else if(choice == 6)//保存并退出
		{
			WriteRecords(records);
			break;
		}
		else if(choice == 7)//重新读取
		{
			records = ReadRecords();
			if(dynamicMemory == 1)//计算最大翻开数字
			{
				ReallocMemory(heightOfBoard, widthOfBoard, 0);
				ReallocMemory(42, 88, 1);
				records.maxOpenNumber = MaxOpenNumber(records);
				ReallocMemory(42, 88, 0);
				ReallocMemory(heightOfBoard, widthOfBoard, 1);
			}
			else
			{
				records.maxOpenNumber = MaxOpenNumber(records);
			}
		}
		else if(choice == 8)//退出
		{
			break;
		}
		else if(choice == 9)//插入一条记录
		{
			printf("[插入序号]>");
			scanf("%d", &choice);
			if(choice < 0) choice = 0;
			if(choice > records.numberOfRecords) choice = records.numberOfRecords;
			printf("[Records Editer]将在%d位置插入新记录\n", choice);
			printf("(0)默认10*10-10 (1)初级9*9-10 (2)中级16*16-40 (3)高级16*30-99 (4)顶级42*88-715 (5)自定义\n");
			printf("[地图难度]>");
			scanf("%d", &(newRecord.difficulty));
			if(newRecord.difficulty == 0)
			{
				newRecord.numberOfMine = 10;
				newRecord.heightOfBoard = 10;
				newRecord.widthOfBoard = 10;
			}
			else if(newRecord.difficulty == 1)
			{
				newRecord.numberOfMine = 9;
				newRecord.heightOfBoard = 9;
				newRecord.widthOfBoard = 10;
			}
			else if(newRecord.difficulty == 2)
			{
				newRecord.numberOfMine = 16;
				newRecord.heightOfBoard = 16;
				newRecord.widthOfBoard = 40;
			}
			else if(newRecord.difficulty == 3)
			{
				newRecord.numberOfMine = 16;
				newRecord.heightOfBoard = 30;
				newRecord.widthOfBoard = 99;
			}
			else if(newRecord.difficulty == 4)
			{
				newRecord.numberOfMine = 42;
				newRecord.heightOfBoard = 88;
				newRecord.widthOfBoard = 715;
			}
			else
			{
				newRecord.difficulty = 5;
				printf("[地图高度]>");
				scanf("%d", &(newRecord.heightOfBoard));
				if(newRecord.heightOfBoard < 1) newRecord.heightOfBoard = 1;
				if(newRecord.heightOfBoard > LimHeight) newRecord.heightOfBoard = LimHeight;
				printf("[地图宽度]>");
				scanf("%d", &(newRecord.widthOfBoard));
				if(newRecord.widthOfBoard < 1) newRecord.widthOfBoard = 1;
				if(newRecord.widthOfBoard > LimWidth) newRecord.widthOfBoard = LimWidth;
				printf("[雷数]>");
				scanf("%d", &(newRecord.numberOfMine));
				if(newRecord.numberOfMine < 0) newRecord.numberOfMine = 0;
				if(newRecord.numberOfMine > newRecord.heightOfBoard * newRecord.widthOfBoard)
				{
					newRecord.numberOfMine = newRecord.heightOfBoard * newRecord.widthOfBoard;
				}
			}
			printf("(0)关闭校验 (1)起始点必非雷 (2)起始点必为空 (3)地图可解\n");
			printf("[地图生成校验]>");
			scanf("%d", &(newRecord.summonCheckMode));
			if(newRecord.summonCheckMode < 0) newRecord.summonCheckMode = 0;
			if(newRecord.summonCheckMode > 3) newRecord.summonCheckMode = 3;
			printf("地图种子(例如%d)，起始点位置\n", time(0));
			printf("[seed] [r0] [c0]>");
			scanf("%d%d%d", &(newRecord.seed), &(newRecord.r0), &(newRecord.c0));
			if(newRecord.r0 < 0) newRecord.r0 = 0;
			if(newRecord.r0 >= newRecord.heightOfBoard) newRecord.r0 = newRecord.heightOfBoard-1;
			if(newRecord.c0 < 0) newRecord.c0 = 0;
			if(newRecord.c0 >= newRecord.widthOfBoard) newRecord.c0 = newRecord.widthOfBoard-1;
			printf("[用时]>");
			scanf("%d", &(newRecord.time));
			if(newRecord.time < 0) newRecord.time = 0;
			//计算地图3BV
			numberOfMine = newRecord.numberOfMine;
			heightOfBoard = newRecord.heightOfBoard;
			widthOfBoard = newRecord.widthOfBoard;
			summonCheckMode = newRecord.summonCheckMode;
			newRecord.total3BV = BBBV(newRecord.seed, newRecord.r0, newRecord.c0, 1);
			printf("当前地图3BV：%d\n", newRecord.total3BV);
			printf("[已解3BV]>");
			scanf("%d", &(newRecord.solved3BV));
			if(newRecord.solved3BV < 0) newRecord.solved3BV = 0;
			if(newRecord.solved3BV > newRecord.total3BV) newRecord.solved3BV = newRecord.total3BV;
			if(debug == 1 || debug == 2)
			{
				printf("是否被实时求解指令帮助\n");
				printf("[1/0]>");
				scanf("%d", &(newRecord.isHelped));
				if(newRecord.isHelped != 0) newRecord.isHelped = 1;
			}
			else
			{
				newRecord.isHelped = 1;//插入无效记录
			}
			newRecord.speed =(float) newRecord.solved3BV / newRecord.time;
			records = AddRecord(records, newRecord);
		}
		else
		{
			Error();
		}
	}
	return records;
}

int CustomMapsEditer()
{
	int choice, i, r, c;
	struct Maps maps = ReadMaps();
	struct Map newMap;
	while(1)
	{
		clrscr();
		PrintMaps(maps);
		//printf("[Map Editer]\n");
		printf("(1)新建地图 (2)编辑地图\n");
		printf("(3)删除地图 (4)重命名地图\n");
		printf("(5)删除所有 (6)复制地图\n");
		printf("(7)重新读取 (8)进入游戏\n");
		printf("(9)保存并退出 (0)直接退出\n");
		printf("*******************************\n");//宽31
		choice = -1;
		printf(">");
		scanf("%d", &choice);
		if(choice == 1)//新建地图
		{
			//初始化
			printf("[name]>");
			newMap.name =(char*) calloc(LimName, sizeof(char));
			getchar();
			//gets(newMap.name);
			fgets(newMap.name, LimName, stdin);//存在回车
			//gets_s(newMap.name, LimName);//不能编译？！
			printf("[author]>");
			newMap.author =(char*) calloc(LimName, sizeof(char));
			fgets(newMap.author, LimName, stdin);
			printf("[height] [width]>");
			scanf("%d%d", &(newMap.height), &(newMap.width));
			//分配矩阵空间
			newMap.isMine =(int**) MatrixMemory((void**)newMap.isMine, newMap.height, newMap.width, sizeof(int), 1);
			newMap.board =(int**) MatrixMemory((void**)newMap.board, newMap.height, newMap.width, sizeof(int), 1);
			//编辑地图
			newMap.numberOfMine = 0;
			newMap = EditMap(newMap);
			maps = AddMaps(maps, newMap);
		}
		else if(choice == 2)//编辑地图
		{
			printf("[序号]>");
			scanf("%d", &i);
			if(i < 0 || i >= maps.numberOfMaps) continue;
			maps.map[i] = EditMap(maps.map[i]);
		}
		else if(choice == 3)//删除地图
		{
			printf("[序号]>");
			scanf("%d", &i);
			if(i < 0 || i >= maps.numberOfMaps) continue;
			maps = DeleteMap(maps, i);
		}
		else if(choice == 4)//重命名地图
		{
			printf("[序号]>");
			scanf("%d", &i);
			if(i < 0 || i >= maps.numberOfMaps) continue;
			printf("[name]>");
			getchar();
			//gets(maps.map[i].name);
			fgets(maps.map[i].name, LimName, stdin);
		}
		else if(choice == 5)//删除所有
		{
			maps = DeleteMaps(maps);
		}
		else if(choice == 6)//复制地图
		{
			printf("[序号]>");
			scanf("%d", &i);
			if(i < 0 || i >= maps.numberOfMaps) continue;
			//其他值直接复制
			newMap = maps.map[i];
			//指针重新分配空间
			newMap.name =(char*) calloc(LimName, sizeof(char));
			newMap.author =(char*) calloc(LimName, sizeof(char));
			newMap.isMine =(int**) MatrixMemory((void**)newMap.isMine, newMap.height, newMap.width, sizeof(int), 1);
			newMap.board =(int**) MatrixMemory((void**)newMap.board, newMap.height, newMap.width, sizeof(int), 1);
			for(r=0; r<LimName && maps.map[i].name[r] != 0; r++)
			{
				newMap.name[r] = maps.map[i].name[r];
			}
			for(r=0; r<LimName && maps.map[i].author[r] != 0; r++)
			{
				newMap.author[r] = maps.map[i].author[r];
			}
			for(r=0; r<newMap.height; r++)
			{
				for(c=0; c<newMap.width; c++)
				{
					newMap.isMine[r][c] = maps.map[i].isMine[r][c];
					newMap.board[r][c] = maps.map[i].board[r][c];
				}
			}
			//添加到地图集
			maps = AddMaps(maps, newMap);
		}
		else if(choice == 7)//重新读取
		{
			maps = DeleteMaps(maps);//释放旧空间
			maps = ReadMaps();
		}
		else if(choice == 8)//游玩地图
		{
			printf("[序号]>");
			scanf("%d", &i);
			if(i < 0 || i >= maps.numberOfMaps) continue;
			//读取地图
			numberOfMine = maps.map[i].numberOfMine;
			heightOfBoard = maps.map[i].height;
			widthOfBoard = maps.map[i].width;
			for(r=0; r<heightOfBoard; r++)
			{
				for(c=0; c<widthOfBoard; c++)
				{
					//isShown[r][c] = 0;//自定义地图第一次翻开无需校验
					isMine[r][c] = maps.map[i].isMine[r][c];
					numberOfMineAround[r][c] = maps.map[i].board[r][c];//雷区位置与正常生成不同
					board[r][c] = maps.map[i].board[r][c];//board在显示时已计算
				}
			}
			if(debug == 1 || debug == 2)
			{
				printf("[Debug]即将游玩地图：\n");
				ShowBoard(1);
				system("pause");
			}
			return 1;//返回并进入游戏
		}
		else if(choice == 9)//保存并退出
		{
			WriteMaps(maps);
			break;
		}
		else if(choice == 0)//直接退出
		{
			break;
		}
		else
		{
			Error();
		}
	}
	maps = DeleteMaps(maps);//释放内存空间
	return 0;
}

void Error()//错误
{
	int c;
	//system("color 1f");
	setbgcolor(0x1f);
	printf(":(\n"
		"你的设备遇到问题，需要重启。\n"
		"我们只收集某些错误信息，然后你可以重新启动。\n\n");
	for(c=0; c<=10; c++)
	{
		printf("\r%3d%% 完成", c*10);
		Sleep(200);
	}
	Sleep(4*200);
	printf("\n");
	//getchar();
	fflush(stdin);//清空输入缓冲区，多字符仅蓝屏一次(doge)
	//system("color 07");
	setbgcolor(backgroundColor);
	clrscr();
}

/*--------------------------------
更新日志：
  # Minesweeper
  Jws使用C语言实现扫雷
Minesweeper1.0
——新增 主页
——新增 设置游戏难度
——新增 标记功能
——优化 界面
——优化 现在x为行，y为列
——优化 第一次打开
——优化 展开0
  # MineSweeper R
  Ltabsyy按照Jws设计重构扫雷代码
MineSweeper R 1.1
——新增 现在胜利或GG后会显示全图
——优化 雷数可以不规则输入了
——优化 现在调试只需改变一个数了
——修复 输入后总显示未选择操作模式
MineSweeper R 1.2
——优化 第一次必为0
MineSweeper R 1.5
——新增 第一次提示x为行，y为列
——新增 在没有标记的情况下扫完所有的格子判定为赢
——新增 全局Debug功能
MineSweeper R 2.0
——新增 标记完一个数字附近的雷，@它就可以把附近没有翻开的自动翻开
MineSweeper R 2.1
——优化 初始化时更不容易卡住
MineSweeper R 2.2
——优化 @数字自动翻开时可以翻开0链
MineSweeper R 2.3
——优化 现在设置界面时先高后宽
——优化 现在返回菜单时会检查雷数是否过多
——优化 更改初级难度界面大小与Windows原版统一
  # MineSweeper Solution 1.0
  Ltabsyy实现对键盘输入的地图求解
MineSweeper Solution 1.1
——优化 全局Debug现在分级调整
——修复 可能出现的过于激进的标记
MineSweeper R 2.4
——优化 部分功能用函数表示
——优化 全局Debug现在分级调整
MineSweeper Solution 1.2
——新增 根据判断出的雷进一步扩大方案矩阵
——优化 优先翻开以节省步数
MineSweeper Solution 1.3
——新增 根据判断出的非雷进一步扩大方案矩阵
MineSweeper R 2.5
——新增 Debug支持更改种子
——优化 函数跟随Solution 1.2升级
——修复 @边缘使0链重复生成导致翻开越界，雷显示为9
MineSweeper Solution 1.4
——新增 Debug可输出显示方式矩阵更新和所有方案
MineSweeper R 2.6
——优化 支持游戏时Debug
——修复 0链不再导致越界翻开
MineSweeper Solution 1.5
——新增 现具有一定的逻辑推理能力
MineSweeper Solution 1.6
——优化 数对类型重新编号
——优化 逻辑推理能力更强
——修复 只能逻辑推理时随机翻开
MineSweeper Solution 1.7
——优化 忽视数对之二为0使逻辑推理更快
——优化 逻辑推理能力更强
——修复 只能逻辑推理时未知原因卡死
MineSweeper R 2.7
——新增 Debug时可搜索地图
——修复 Debug时输入瞬间界面闪烁
MineSweeper Solution 1.8
——新增 逻辑推理现已能枚举验证
——优化 Debug界面更简洁
MineSweeper Solution 2.0
——优化 通过枚举结果的共同点，逻辑推理能力已达极致
MineSweeper Solution 2.1
——优化 Debug1下也能看到所有方案
——优化 Debug显示可能性加?
——修复 只能枚举且可能性不唯一时卡死
MineSweeper Solution 2.2
——修复 只能枚举且可能性不唯一时随机输出
MineSweeper Solution 2.3
——优化 全新初始界面与游戏风格统一
——优化 枚举时间过长时显示进度条
——优化 强化逻辑推理以减少枚举
——优化 更精细的分割未知链
MineSweeper Solution 2.4
——新增 无路可走时可根据最低非0雷率输出
——优化 初始界面更简洁
——优化 根据剩余雷数进行枚举判断
——修复 只能枚举且未知链过长时卡死
——修复 逻辑推理的翻开不优先
MineSweeper Solution 2.5
——新增 全新界面和设置
——新增 游戏模式和自定义未知链长度
——新增 游戏中可输入`返回菜单
——修复 不能自定义雷数
MineSweeper R 2.8
//——新增 显示用时
——优化 代码效率
——优化 游戏结束后现在直接返回菜单
——优化 函数跟随Solution 2.4升级
——修复 在首页输入非数字字符无限重启
MineSweeper Solution 2.6
——优化 上调游戏模式枚举效率长度
——优化 Debug可看出推理逻辑
——优化 重写答案显示算法
——优化 统一界面设计语言
——修复 返回菜单需要二次按`
  # MineSweeper Run Alpha
  Ltabsyy将MineSweeper R 2.8和MineSweeper Solution 2.6合并
MineSweeper Run 1.0
——新增 调试Bench可显示帧率
——新增 游戏时输入! 0 0实时求解
MineSweeper Run 1.1
——优化 实时求解指令支持更多选择
MineSweeper Run 1.2
——修复 全部标记仍可胜利
——修复 Bench放弃链数总为0
MineSweeper Run 1.3
——新增 对Windows实现彩色字符
——优化 通过覆写代替清屏使Bench提速80%并消除闪烁
MineSweeper Run 1.4
——新增 可解地图生成
——优化 枚举后根据剩余雷数判断
——修复 Bench非Debug时枚举进度条滞留
MineSweeper Run 1.5
——新增 游戏时实时显示用时
——新增 浅色模式
——优化 游戏Debug不再显示显示方式矩阵
——修复 地图高度最大时顶部雷生成异常
MineSweeper Run 1.6
——新增 wasd23操作模式
——优化 游戏不再称行列为xy而称rc
——优化 回车等字符不再能输入为操作模式
MineSweeper Run 2.0
——新增 wasd23模式增加wasdhu
——新增 游戏时可在@#rc和wasd23自动切换
——新增 设置地图生成，设置显示用时，默认操作模式不再是调试选项
——新增 顶级难度
——优化 678使用淡黄色显示
——优化 浅色模式
——修复 wasd23模式坐标位数减少时显示错误
MineSweeper Run 2.1
——新增 调试选项可设置刷新率
——新增 调试选项可设置大写WASD为移动5格
——新增 调试选项可设置#数字可将周围未翻开方块都标记，即使数字不与周围未翻开方块数量相等
——新增 调试选项可设置><光标
——优化 现在默认显示用时，wasd23模式(JwsDefault)
——优化 顶级难度更改为42*88-715
——优化 现在切换模式后操作提示会立即更新，并屏蔽该用于切换的操作
——优化 实时求解指令更改为!/Tab一键式启动
——优化 实时求解指令方案提示，Debug时Bench的方案矩阵直接在原地图显示
——优化 现在第一次翻开也支持wasd23
——修复 顶级难度wasd23模式闪烁
MineSweeper Run 2.2
——新增 继续上一次游戏
——新增 保存上一次设置
——新增 地图求解也在原地图显示全部方案
——修复 游戏Debug关闭时相关字符不消失
——修复 低难度游戏Bench锁定校验时字符不消失
MineSweeper Run 2.3
——新增 调试选项可启用鼠标点击
——新增 游戏时可按M/K在鼠标和键盘操作间切换
——新增 浅色模式不再是调试选项
——新增 调试选项可设置任意背景颜色
——新增 调试选项可设置隐藏光标
//——优化 鼠标点击时!(Shift+1)可直接按1
——优化 鼠标点击时wasd可使用方向键
MineSweeper Run 2.4
——新增 鼠标点击不再是调试选项
——优化 现在按K无缝切换键盘操作
——优化 鼠标点击时支持!指令(Shift+1)
——修复 上一次游戏地图与设置不同时显示错误
——修复 游戏Debug时检测鼠标位置偏移
——修复 启用过鼠标操作后切换到键盘操作无法全选复制地图
——修复 鼠标右击胜利时出现右键菜单
——修复 启用过鼠标操作后地图求解右击不能粘贴
——修复 鼠标模式Debug不能更改种子
MineSweeper Run 2.5
——新增 命令行窗口命名为MineSweeper Console
——优化 定义功能模块函数以缩减main函数
——修复 鼠标点击偶发的双击
——修复 鼠标模式无操作时用时显示停滞
MineSweeper Run 2.6
——新增 现在默认鼠标点击操作模式
——新增 全新界面
——新增 现在主页，设置菜单，设置难度支持鼠标点击
——优化 删除部分不必要的调试
——优化 现在打开程序直接继续上一次游戏
——修复 继续上一次游戏会丢失标记
MineSweeper Run 3.0
——新增 现在设置内除调试选项均支持鼠标点击
——新增 游戏时点击新游戏即可停止游戏，再次点击可立即重开
——新增 调试选项可启用鼠标点击屏蔽空格
——优化 默认刷新周期从100ms提高到50ms
——优化 移除大写WASD移动5格功能
——修复 鼠标翻开偶发的双击
MineSweeper Run 3.1
——新增 全新难度选择界面
——新增 调试选项可启用游戏结束时显示3BV和3BV/s
——优化 自定义难度时地图生成校验会自动更改
——优化 地图高度大于40时游戏不显示控制栏
——修复 自定义背景时不显示地图
MineSweeper Run 3.2
——新增 3BV和3BV/s现在可以游戏实时显示
——修复 浅色模式678淡黄色底纹显示过多
——修复 难度设置键盘操作无效
——修复 鼠标模式无操作时光标不停留在原来位置
MineSweeper Run 3.3
——新增 Bench进度条，3BV
——新增 显示3BV不再是调试选项
——优化 @#rc输入坐标体验（'_'光标，阴影显示正在输入坐标，退格键一键删除整个坐标）
——修复 Debug时Bench的Loading覆盖位置错误
——修复 显示用时时@#rc显示位置错误
MineSweeper Run 3.4
——新增 鼠标点击模式支持左右键功能切换，用于触屏标记
——新增 历史记录
——优化 游戏重新支持一定的调试
——优化 设置为顶级难度时自动隐藏3BV
——修复 翻开标记后剩余雷数显示错误
  # MineSweeper Records Editer
  Ltabsyy实现对历史记录文件的编辑
MineSweeper Records Editer 1.1
——新增 仅保留纪录
MineSweeper Run 3.5
——新增 Bench调试时显示已解3BV矩阵
——新增 RecordsEditer模块，可在主页按7使用
——优化 Debug的地图搜索功能独立为MapSearch模块，可在主页按6使用
——优化 游戏剩余雷数，用时，3BV信息在一行显示，设置为顶级难度时不再自动隐藏3BV
——优化 重排版部分if内条件以减少代码行数
——优化 现在切键盘操作处无需按任意键
——修复 进入调试选项有时会立即退出
MineSweeper Run 3.6
——新增 MapSearch模块显示地图是否可解
——新增 可速解地图生成
——新增 调试选项可启用动态内存分配
——优化 鼠标右键长按拖动时可以正常连续标记
——优化 现在地图生成时显示方式矩阵必然清零
——优化 现在非鼠标点击模式可全选复制第一次全未翻开的地图
——优化 第一次翻开时也可按键盘选择控制栏3-5，5为返回主页
——优化 减少鼠标操作栏的判断次数
——修复 wasd23模式第一次翻开无法使用
——修复 第一次翻开或未完成地图时，选择控制栏其他功能会增加历史记录
MineSweeper Run 3.7
——优化 统一代码书写风格
——优化 选择可速解地图时提示易生成重复地图
——修复 可解地图生成后不翻开起始点
——修复 无历史记录时游戏结束闪退
——修复 动态内存分配时地图求解输入后闪退
MineSweeper Run 3.8
——新增 RecordsEditer模块可显示玩家等级称号
——优化 现在RecordsEditer纪录显示在记录后
——优化 程序初始化效率
——优化 地图求解输入的稳定性
——修复 Bench调试的已解3BV矩阵不能对齐
——修复 自定义背景时不在原地图显示方案矩阵
——修复 修改难度后不保存有时不能在原地图正确显示方案
——修复 启动后不修改难度可能不显示打破纪录
MineSweeper Run 3.9
——新增 调试选项可启用淡黄色高亮光标
——优化 RecordsEditer界面
——优化 @#rc输入坐标体验（输入过程坐标对应位置显示控制台光标，支持wasd改变坐标）
——修复 鼠标从(10,10)移动到(9,9)时坐标可能显示错误
——修复 被实时求解指令帮助过后重启游戏可打破纪录
——修复 游戏调试不显示控制栏和3BV
MineSweeper Run 4.0
——新增 RecordsEditer可显示所有记录总时间，总翻开数
——优化 游戏时按下退出后显示暂停
——优化 对设置的缓存文件条目重排序
——优化 标记过多时只自动取消上一个标记
——修复 @#rc第一次翻开时光标位置错误
——修复 键盘模式重启后不能复制地图
MineSweeper Run 4.1
——新增 RecordsEditer可显示有效记录最大翻开数字
——新增 BenchDebug2功能（可自定义种子范围，不显示中间帧，每帧暂停）
——新增 难度选择界面显示窗口大小，字体大小，最大地图大小，当前密度
——优化 BenchDebug2体验（可左键暂停右键继续，Bench结束暂停）
——优化 游戏后记录的总时间和总翻开数自动更新
——优化 简化难度与有效记录判断代码
——优化 显示记录时显示是否有效
——优化 消除鼠标操作栏闪烁
——优化 RecordsEditer界面
——修复 Bench不显示胜利局面
MineSweeper Run 4.2
——新增 Bench调试显示帧序号，Debug2可显示Solve调试信息
——新增 主页按8可进入自定种子游戏
——优化 现在最大翻开数字不在启动时计算，提升启动速度
——优化 读取历史记录时纪录计算效率
——修复 重新读取记录后设置难度或进入游戏闪退
  # MineSweeper Map Painter
  Ltabsyy实现对自制地图文件的编辑
MineSweeper Run 4.3
——新增 自制地图的编辑，可在主页按9使用
——新增 自制地图的游戏
——修复 暂停后继续的地图重置和上一次地图时间错误
MineSweeper Map Painter 1.1
——优化 功能与函数跟随Run 4.3升级
MineSweeper Run 4.4
——新增 调试选项可启用淡黄色高亮行列
——优化 不再使用bool
——优化 #数字一键标记周围升级为快速标记（可一键取消所有标记，仅%+#=数字时标记）
——优化 确保自制地图游戏为无效记录
——优化 淡黄色高亮光标抗闪烁
——修复 Debug2重新读取记录时闪烁
——修复 ><光标覆盖行坐标轴十位，对角移动易出现拖影
——修复 快速标记不改变剩余雷数
——修复 重新读取记录后最大翻开数字为0
MineSweeper Run 4.5
——新增 快速标记，设置光标不再是调试选项
——优化 现在默认隐藏控制台光标，显示淡黄色高亮光标
——修复 淡黄色高亮光标可能引起闪退
——修复 玩家等级4级计算错误
MineSweeper Run 4.6
——新增 游戏结束时错误标记显示为数字和#
——新增 更详细的地图求解器提示信息
——优化 现在Bench总是显示一行置顶信息，在原地图显示方案矩阵
——优化 Bench范围不再被字典大小限制
——优化 减少Bench置顶信息的刷新次数
——优化 现在控制台光标在游戏开始时检查隐藏，防止调整窗口大小导致光标显示
——优化 现在深色模式淡黄色高亮光标的空地以_显示，678以红色加淡黄色底纹显示
——优化 地图前台显示方式的判断效率
——优化 现在游戏时点击地图求解按钮与按!执行实时求解指令等效
——优化 现在游戏时点击Bench按钮与按Tab执行实时求解指令等效
——优化 蓝屏体验，多字符仅蓝屏一次
——修复 Bench帧率为无穷大时闪退
——修复 Bench不显示中间帧时列坐标轴闪烁
——修复 RecordsEditer和自制地图界面输入字符后不断闪烁
MineSweeper Run 4.7
——新增 调试选项可设置Bench
——优化 通过连续操作减少逻辑推理的次数
——优化 逻辑推理和枚举的准备效率
——优化 修改可速解地图标准
——优化 屏蔽打开地图前的多次点击
——优化 第一次翻开也支持自选光标
——优化 @#rc输入坐标体验（支持自选光标，仅输入行坐标时高亮整行）
——优化 ><光标抗闪烁，且不再会出现拖影
——修复 自定种子游戏不刷新显示方式矩阵
——修复 @#rc输入坐标时用时显示停滞
MineSweeper Run 4.8
——新增 调试选项可自定义><光标字符
——优化 通过提前校验加速枚举判断
——优化 通过校验基底加速枚举判断
——优化 错误标记空地现在显示为_#
——优化 @#rc输入坐标体验（仅输入行坐标时可用ws改变坐标，中途按M立即切为鼠标）
——优化 鼠标点击，wasd23可按C直切@#rc
——优化 地图求解现在可识别?和_字符
——优化 微调部分设置界面
——修复 Bench鼠标双击
——修复 Bench仅显示结束帧时Loading位置错误
——修复 @#rc进入新游戏闪退
//——新增 根据多块枚举的整体结果进行剩余雷数判断
//——新增 超大地图支持翻页操作（大于42行或88列时RF上下16行，ET左右30列）
//——新增 可启用在外部窗口进行游戏
//——优化 现在地图求解可选择从外部文件读取地图，界面支持鼠标点击
//——优化 wasd23与鼠标点击模式复用部分代码
//——优化 枚举判断的剪枝算法，支持无限枚举
//——优化 重新设计自定义难度设置，以密度设置雷数不再是调试选项
//——修复 雷率为-0和负数时不视为确定解
--------------------------------*/
