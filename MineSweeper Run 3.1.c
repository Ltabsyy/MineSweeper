//#define _CRT_SECURE_NO_WARNINGS//vs专属(doge)
#include <stdio.h>
#include <stdlib.h>
#include <time.h>//时间戳作种子生成随机数，用于生成地图和无确定解时随机翻开
#include <conio.h>//非阻塞式输入
#include <windows.h>//面向Windows输出彩色字符
#include <stdbool.h>
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
//内存占用至少为 53*H*W+128*D Byte，默认约0.7MB

// 外部窗口形态
/*#define sideLength 32//外部窗口方块边长
int heightOfBlock = sideLength;
int widthOfBlock = sideLength;//锁定纵横比
int heightOfBar = 2*sideLength;
#define xOfChar widthOfBlock/3
#define yOfChar heightOfBlock/6//略低于居中的heightOfBlock/8
#define heightOfChar heightOfBlock*3/4//字符高度为方块的3/4*/

// 控制栏
int IsPosInRectangle(int x, int y, int x1, int y1, int x2, int y2);
void DrawControlBar(int select);
int ChoiceControlBar(COORD mousePos, DWORD dwButtonState);
int WASDMove(int* r, int* c, WORD wVirtualKeyCode);

// 鼠标操作栏
void DrawSettingsBar(int select);
int ChoiceSettingsBar(COORD mousePos, DWORD dwButtonState);
void DrawSetDifficulty(int select);
int ChoiceSetDifficulty(COORD mousePos, DWORD dwButtonState);
void DrawSetSummonCheckMode(int select);
int ChoiceSetSummonCheckMode(COORD mousePos, DWORD dwButtonState);
void DrawSetOperateMode(int select);
int ChoiceSetOperateMode(COORD mousePos, DWORD dwButtonState);

// 地图生成和显示
void SummonBoard(int seed, int r0, int c0);//生成雷，数字和后台总板
int Place(int n);//计算某数所占位数，用于对齐坐标轴
void PrintCell(int board, int isShown, int bkcolor);
void ShowBoard(int mode);//在控制台显示地图，0前台，1后台

// 外部窗口显示
/*void DrawMine(int r, int c);//绘制地图地雷
void DrawMineA(int x0, int y0, int r);//绘制地雷图形
void DrawFlag(int r, int c);//绘制地图旗帜
void DrawBlock(int r, int c, int board, int isShown);//绘制方块
void DrawLineA(int x0, int y0, int r, int angle);//绘制时钟指针
void DrawClock(int x0, int y0, int r, int time);//绘制时钟
void DrawBoard(int mode, int remainder, int t);//绘制总外部窗口*/

// 后台计算
void OpenZeroChain(int r0, int c0);//翻开0连锁翻开
void SummonZeroChain(int r0, int c0);
int RealRemainder();//真实的剩余雷数，非显示的剩余雷数
int NumberOfNotShown();//%的数量
int NumberOfSign();//#的数量
int NumberOfNotShownAround(int r0, int c0);//某坐标附近%的数量
int NumberOfSignAround(int r0, int c0);//某坐标附近#的数量
int IsAroundZeroChain(int r0, int c0);

// 方案思考
void LookMap();//从实时游戏获取地图
void TranslateMap(int rs0, int cs0);//从键盘输入获取地图
int Solve();
int Think();
void SummonMineRateForNotShown();
int NumberOfNumberAround(int r0, int c0);
void ShowAnswer(int isFoundOpen, int uncertainOpen);
void ShownModeBak(int mode);//备份显示方式矩阵，1备份，0恢复

// 功能模块
void Solution();//地图求解模块
void Bench(int seedMin, int seedMax, int r0, int c0);//Bench模块
int BBBV(int seed, int r0, int c0);//Bechtel'sBoardBenchmarkValue，计算地图3BV
void Error();//错误，目前无用

// 全局矩阵
int isMine[LimHeight][LimWidth]={0};//雷
int numberOfMineAround[LimHeight][LimWidth]={0};//数字
int board[LimHeight][LimWidth]={0};//后台总板，数字0~8雷9
int isShown[LimHeight][LimWidth]={0};//显示方式矩阵，0未翻开，1已翻开，2已标记
int zeroChain[LimHeight][LimWidth]={0};

// 方案矩阵
char mapShown[LimHeight+2][2*LimWidth+5]={0};//键盘输入的地图
char map[LimHeight][LimWidth]={0};//获取的地图
int isShownBak[LimHeight][LimWidth]={0};//显示方式矩阵备份
int numberShown[LimHeight][LimWidth]={0};//获取的数字，非数字为9
int solution[LimHeight][LimWidth]={0};//方案，1翻开，2标记
int isThought[LimHeight][LimWidth]={0};//1未知方块，2附近数字，3枚举后放弃，4过长放弃
int numberTeam[LimHeight][LimWidth]={0};//数对，1数对之一独占区，2数对之二独占区，3共享区
int thinkChain[LimHeight][LimWidth]={0};//未知链
int possibility[32]={0};
int numberCheck[LimHeight][LimWidth]={0};
int dictionary[LimDictionary][32]={0};
float isMineRate[LimHeight][LimWidth]={0};

// Bench统计
int numberOfAbandonedThinkChain = 0;
int isWinningOfSeed[LimDictionary]={0};

// 全局变量及初始化
int numberOfMine = 10;//雷数量
int heightOfBoard = 10;//界面高度
int widthOfBoard = 10;//界面宽度
int lengthOfThinkChain = 19;//未知链长度
int summonCheckMode = 2;//0不校验，1非雷，2必空，3可解
int solveMode = 1;//1游戏模式，2分析模式
bool showTime = 1;//显示用时，0不显示，1显示
int operateMode = 2;//操作模式，0@#rc，1wasd23，2鼠标点击，3Window
int heightOfMapShown, widthOfMapShown, rs0, cs0;//控制台地图显示

// 是否调试(0:关 1:开，显示部分 2:开，显示全部)
int debug = 0;

// 控制台显示
int backgroundColor = 0x07;//背景颜色，深色模式0x07，浅色模式0xf0
void ColorStr(char* content, int color)//输出彩色字符
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
	printf("%s", content);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), backgroundColor);
}
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
	COORD pos = {0,0};
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
	COORD pos = {0,0};
	FillConsoleOutputAttribute(hdout, color, size, pos, &num);
	SetConsoleTextAttribute(hdout, color);
}
void showCursor(bool visible)//显示或隐藏光标
{
	CONSOLE_CURSOR_INFO cursor_info = {20, visible};
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
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
	int difficulty = 0;//当前难度显示，实际没有必要定义(doge)
	float density =(float) numberOfMine / heightOfBoard / widthOfBoard;//雷密度
	// 控制台鼠标操作
	HANDLE hdin = GetStdHandle(STD_INPUT_HANDLE);
	COORD mousePos = {0,0};//鼠标坐标
	COORD mouseOperatedPos = {0,0};//鼠标已操作坐标，屏蔽双击
	INPUT_RECORD rcd;
	DWORD rcdnum;
	int isReadyRefreshMouseOperatedPos = 0;
	// main()局部变量及初始化
	bool lastMap = 0;//有上一次地图
	int refreshCycle = 50;//刷新周期，默认50ms，一般鼠标8ms，游戏鼠标1ms
	bool fastSign = 0;//#数字一键标记周围
	int newCursor = 0;//><光标
	bool visibleCursor = 1;//1显示光标，0隐藏光标
	bool clickSpace = 1;//0鼠标点击屏蔽空格
	bool show3BV = 0;//游戏结束时显示3BV和3BV/s
	// Window
	//int xm, ym;
	//mouse_msg mouseMsg;
	//很好，我已经看不清我定义些啥了(doge)
	if(file = fopen("minesweeper-settings.txt", "r"))//读取设置
	{
		fscanf(file, "Map:%d*%d-%d\n", &heightOfBoard, &widthOfBoard, &numberOfMine);
		fscanf(file, "summonCheckMode=%d\n", &summonCheckMode);
		fscanf(file, "showTime=%d\n", &showTime);
		fscanf(file, "operateMode=%d\n", &operateMode);
		fscanf(file, "solveMode=%d\n", &solveMode);
		fscanf(file, "lengthOfThinkChain=%d\n", &lengthOfThinkChain);
		fscanf(file, "backgroundColor=0x%x\n", &backgroundColor);
		fscanf(file, "refreshCycle=%d\n", &refreshCycle);
		fscanf(file, "fastSign=%d\n", &fastSign);
		fscanf(file, "newCursor=%d\n", &newCursor);
		fscanf(file, "visibleCursor=%d\n", &visibleCursor);
		fscanf(file, "clickSpace=%d\n", &clickSpace);
		fscanf(file, "show3BV=%d\n", &show3BV);
		fclose(file);
	}
	SetConsoleTitle("MineSweeper Console");
	if(backgroundColor != 0x07) setbgcolor(backgroundColor);//默认浅色模式
	if(visibleCursor == 0) showCursor(0);
	if(file = fopen("minesweeper-lastmap.txt", "r"))//读取上一次地图
	{
		lastMap = 1;
		choiceMode = 1;
		fscanf(file, "Map:%d*%d-%d\n", &heightOfBoard, &widthOfBoard, &numberOfMine);
		fscanf(file, "seed=%d,%d,%d\n", &seed, &r0, &c0);
		fscanf(file, "time=%d\n", &t2);
		fscanf(file, "pos=(%d,%d)\n", &ro, &co);
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
		/*if(debug == 1 || debug == 2)
		{
			printf("[Debug]已获取上一次地图：\n");
			printf("Map:%d*%d-%d\n", heightOfBoard, widthOfBoard, numberOfMine);
			printf("seed=%d,%d,%d\n", seed, r0, c0);
			printf("time=%d\n", t2);
			printf("pos=(%d,%d)\n", ro, co);
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
			system("pause");
		}*/
	}
	SetConsoleMode(hdin, ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS);
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
		if(debug == 2) printf("<Debug> Hello! Administrator Ltabsyy or Jws!\n");
		rs0 = Place(widthOfBoard-1);//行起始指针，为列坐标显示占用行数
		cs0 = Place(heightOfBoard-1) + 1;//列起始指针，为行坐标显示占用列数，含空格
		heightOfMapShown = heightOfBoard + rs0;//计算包含坐标的行数
		widthOfMapShown = 2*widthOfBoard + cs0 + 1;//计算包含坐标的每行字符数加回车
		//printf(">");
		while(choiceMode == 0)
		{
			ReadConsoleInput(hdin, &rcd, 1, &rcdnum);
			if(rcd.EventType == MOUSE_EVENT)
			{
				mousePos = rcd.Event.MouseEvent.dwMousePosition;
				choiceTemp = ChoiceControlBar(mousePos, rcd.Event.MouseEvent.dwButtonState);
				if(choiceTemp != 0) choiceMode = choiceTemp;
			}
			if(rcd.EventType == KEY_EVENT && rcd.Event.KeyEvent.bKeyDown == 1)
			{
				if(rcd.Event.KeyEvent.wVirtualKeyCode >= '1' && rcd.Event.KeyEvent.wVirtualKeyCode <= '5')
				{
					choiceMode = rcd.Event.KeyEvent.wVirtualKeyCode-'1'+1;//主键盘1-5
				}
				if(rcd.Event.KeyEvent.wVirtualKeyCode >= 'a' && rcd.Event.KeyEvent.wVirtualKeyCode <= 'e')
				{
					choiceMode = rcd.Event.KeyEvent.wVirtualKeyCode-'a'+1;//小键盘1-5
				}
			}
			Sleep(refreshCycle);
		}
		if(choiceMode == 1)
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
			else
			{
				t2 = 0;
				isOpenMine = 0;
				for(r=0; r<heightOfBoard; r++)
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
				if(operateMode == 0)
				{
					scanf("%d%d", &r0, &c0);//通过键盘获取第一次选择的坐标
				}
				else if(operateMode == 1 || operateMode == 2)
				{
					r0 = heightOfBoard/2;
					c0 = widthOfBoard/2;
					while(choiceMode == 1)
					{
						if(operateMode == 2)
						{
							ReadConsoleInput(hdin, &rcd, 1, &rcdnum);
							if(rcd.EventType == MOUSE_EVENT)
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
							if(rcd.EventType == KEY_EVENT && rcd.Event.KeyEvent.bKeyDown == 1)
							{
								if(WASDMove(&r0, &c0, rcd.Event.KeyEvent.wVirtualKeyCode));
								else
								{
									break;
								}
							}
						}
						gotoxy(0, heightOfMapShown+4);
						printf(">@ %d %d ", r0, c0);//覆写尾部
						gotoxy(cs0+2*c0, heightOfMapShown+2-heightOfBoard+1+r0);
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
						Sleep(refreshCycle);
					}
				}*/
				seed = time(0);//当前时间戳作种子生成随机数
				if(debug == 2)
				{
					temp = 0;
					printf("[Debug]seed=%d,%d,%d\n", seed, r0, c0);
					system("pause");//system指令使SetConsoleMode失效
					printf("[0:生成/1:更改/2:搜索]\n>");
					scanf("%d", &temp);
					if(temp == 1)
					{
						printf("[seed] [r0] [c0]\n>");
						scanf("%d%d%d", &seed, &r0, &c0);
					}
					else if(temp == 2)
					{
						printf("[seedMin] [seedMax] [r0] [c0]\n>");
						scanf("%d%d%d%d", &seedMin, &seedMax, &r0, &c0);
						debug = 0;//搜索不显示多余信息
						for(seed=seedMin; seed<=seedMax; seed++)
						{
							SummonBoard(seed, r0, c0);
							printf("Map:%d*%d-%d\n", heightOfBoard, widthOfBoard, numberOfMine);
							printf("seed=%d,%d,%d\n", seed, r0, c0);
							ShowBoard(1);
						}
						debug = 2;
						system("pause");
						continue;
					}
				}
			}
			/*生成地图*/
			if(lastMap == 0 && summonCheckMode == 3)//可解地图生成
			{
				isOpenMine = 1;
				while(1)
				{
					isOpenMine = 0;
					SummonBoard(seed, r0, c0);
					isShown[r0][c0] = 1;
					ShownModeBak(1);
					for(r=0; r<heightOfBoard; r++)
					{
						for(c=0; c<widthOfBoard; c++)
						{
							solution[r][c] = 0;
						}
					}
					OpenZeroChain(r0, c0);
					if(debug == 1 || debug == 2) printf("[Debug]正在分析seed=%d\n", seed);
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
						if(RealRemainder()==0) break;
						LookMap();
						temp=debug;
						debug=0;
						Solve();
						debug=temp;
					}
					if(debug == 2)
					{
						ShowBoard(1);
					}
					ShownModeBak(0);
					if(isOpenMine == 1)
					{
						seed++;
					}
					else
					{
						r = r0;
						c = c0;
						if(debug == 1 || debug == 2)
						{
							printf("[Debug]已找到可解seed=%d\n", seed);
							system("pause");
						}
						break;
					}
				}
			}
			else
			{
				r = r0;
				c = c0;
				SummonBoard(seed, r, c);
				isShown[r][c] = 1;
			}
			/*准备开始游戏*/
			if(lastMap == 0)
			{
				ro = r0;
				co = c0;
			}
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
						if(showTime == 1) printf("用时：%d\n", t1-t0+t2);
						if(show3BV == 1)
						{
							temp = BBBV(seed, r0, c0);
							printf("3BV：%d\n", temp);
						}
						remove("minesweeper-lastmap.txt");
						lastMap = 0;
						//SetConsoleMode(hdin, ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS);
						//system("pause");
						SetConsoleMode(hdin, ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS);
						choiceMode = 0;
						/*if(operateMode == 3)
						{
							DrawBoard(1, remainder, t1-t0+t2);
							setcolor(RED);
							setfontbkcolor(BLACK);
							xyprintf(widthOfBlock, heightOfBar, "请按键盘任意键关闭窗口");
							xyprintf(widthOfBlock, heightOfBar+heightOfBlock, "请勿按右上角退出！");
							getch();
							closegraph();
						}*/
						break;
					}
				}
				if(RealRemainder() == 0//正确标记所有雷则胜利
				 ||NumberOfNotShown() == remainder)//未翻开的都是雷则胜利
				{
					if(NumberOfSign() > numberOfMine)//标记量不能超过雷数
					{
						printf(":(\n标记过多！请重新标记。\n");
						if(operateMode == 2 && operation == '#') Sleep(refreshCycle);//防止鼠标出现右键菜单
						//system("pause");
						for(r=0; r<heightOfBoard; r++)
						{
							for(c=0; c<widthOfBoard; c++)
							{
								if(isShown[r][c] == 2)
								{
									isShown[r][c] = 0;
									remainder++;
								}
							}
						}
						continue;
					}
					clrscr();
					DrawControlBar(0);
					ShowBoard(1);
					printf(":)\nYou Win!\n");
					t1 = time(0);
					if(showTime == 1) printf("用时：%d\n", t1-t0+t2);
					if(show3BV == 1)
					{
						temp = BBBV(seed, r0, c0);
						printf("3BV：%d\n3BV/s：%.2f\n", temp, (float)temp/(t1-t0+t2));
					}
					remove("minesweeper-lastmap.txt");
					lastMap = 0;
					//SetConsoleMode(hdin, ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS);
					//system("pause");
					SetConsoleMode(hdin, ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS);
					choiceMode = 0;
					/*if(operateMode == 3)
					{
						DrawBoard(1, 0, t1-t0+t2);
						setcolor(RED);
						setfontbkcolor(BLACK);
						xyprintf(widthOfBlock, heightOfBar, "请按键盘任意键关闭窗口");
						xyprintf(widthOfBlock, heightOfBar+heightOfBlock, "请勿按右上角退出！");
						getch();
						closegraph();
					}*/
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
					gotoxy(0, 3);
					yOfMapEnd = heightOfMapShown+2;
				}
				/*if(debug == 1 || debug == 2)
				{
					clrscr();
					printf("<Debug>\n");
					printf("Map:%d*%d-%d\n", heightOfBoard, widthOfBoard, numberOfMine);
					printf("seed=%d,%d,%d\n", seed, r0, c0);
					ShowBoard(1);
					printf("RealRemainder=%d \n", RealRemainder());
					printf("NumberOfNotShown=%d\n", NumberOfNotShown());
					printf("\n");
					yOfMapEnd += heightOfMapShown+6;
				}*/
				ShowBoard(0);
				printf("剩余雷数: %d \n", remainder);//打印剩余雷数
				t1 = time(0);
				if(showTime == 1) printf("用时：%d\n", t1-t0+t2);
				printf("选择模式与坐标\n");
				if(operateMode == 0) printf("[@:翻开/#:标记] [r] [c]  \n");
				if(operateMode == 1) printf("[wasd:移动/2:翻开/3:标记]\n");
				if(operateMode == 2) printf("[鼠标左键:翻开/右键:标记]\n");
				if(operateMode == 3) printf("[请在MineSweeper Window进行游戏]\n");
				/*备份至文件*/
				file = fopen("minesweeper-lastmap.txt", "w");
				fprintf(file, "Map:%d*%d-%d\n", heightOfBoard, widthOfBoard, numberOfMine);
				fprintf(file, "seed=%d,%d,%d\n", seed, r0, c0);
				fprintf(file, "time=%d\n", t1-t0+t2);
				fprintf(file, "pos=(%d,%d)\n", ro, co);
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
				/*输入和显示用时*/
				printf(">");
				operation = 0;
				r = ro;
				c = co;
				if(operateMode == 2) SetConsoleMode(hdin, ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS);//确保win10控制台接收鼠标信息
				while(choiceMode == 1)
				{
					/*if(operateMode == 3)
					{
						DrawBoard(0, remainder, t1-t0+t2);
						while(mousemsg())
						{
							mouseMsg = getmouse();
							if(mouseMsg.is_left() && mouseMsg.is_up())//鼠标左键抬起
							{
								mousepos(&xm, &ym);
								operation = '@';
								r = (ym-heightOfBar)/heightOfBlock;
								c = xm/widthOfBlock;
							}
							if(mouseMsg.is_right() && mouseMsg.is_up())//鼠标右键抬起
							{
								mousepos(&xm, &ym);
								operation = '#';
								r = (ym-heightOfBar)/heightOfBlock;
								c = xm/widthOfBlock;
							}
						}
						Sleep(refreshCycle);
					}*/
					if(operateMode == 2)
					{
						isReadyRefreshMouseOperatedPos = 1;
						GetNumberOfConsoleInputEvents(hdin, &rcdnum);//检索在控制台输入缓冲区中未读取的输入事件记录个数
						if(rcdnum == 0 && showTime == 1)
						{
							gotoxy(0, yOfMapEnd+2);
							t1 = time(0);
							printf("用时：%d", t1-t0+t2);
							gotoxy(1, yOfMapEnd+5);
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
							if(choiceTemp != 0) choiceMode = choiceTemp;
							if(choiceTemp == 1)//停止当前游戏，再次点击进入新游戏
							{
								gotoxy(0, yOfMapEnd+showTime+3);
								printf("[已停止游戏，按1进入新游戏]\n");
								remove("minesweeper-lastmap.txt");
								lastMap = 0;
								choiceMode = 0;
							}
							//X=cs0+2*c, Y=yOfMapEnd-heightOfBoard+1+r
							//r=Y-yOfMapEnd+heightOfBoard-1, c=(X-cs0)/2
							if(IsPosInRectangle((mousePos.X-cs0)/2, mousePos.Y-yOfMapEnd+heightOfBoard-1, 0, 0, widthOfBoard-1, heightOfBoard-1))
							{
								if(clickSpace == 0 && (mousePos.X-cs0)%2 == 1) continue;//屏蔽空格
								r = mousePos.Y-yOfMapEnd+heightOfBoard-1;
								c = (mousePos.X-cs0)/2;
								if(rcd.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
								{
									if(mousePos.X == mouseOperatedPos.X && mousePos.Y == mouseOperatedPos.Y)
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
								if(rcd.Event.MouseEvent.dwButtonState == RIGHTMOST_BUTTON_PRESSED)
								{
									if(mousePos.X == mouseOperatedPos.X && mousePos.Y == mouseOperatedPos.Y)
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
						}
						if(rcd.EventType == KEY_EVENT && rcd.Event.KeyEvent.bKeyDown == 1)//按下而非松开键盘时
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
							else if(rcd.Event.KeyEvent.wVirtualKeyCode == 'K')
							{
								gotoxy(0, yOfMapEnd+showTime+5);
								if(debug == 1 || debug == 2)
								{
									printf("正在切换到键盘操作...\n");
									system("pause");
								}
								SetConsoleMode(hdin, ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS);
								operateMode = 1;
								//clrscr();
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
					if(kbhit())//按键
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
							gotoxy(0, yOfMapEnd+showTime+4);
							printf(">        ");//覆写@ 128 93
							if(debug == 1 || debug == 2)
							{
								printf("\n:(\n检测到@#rc操作指令！正在识别...\n");
								system("pause");
							}
							gotoxy(1, yOfMapEnd+showTime+4);
							operateMode = 0;
							break;//立即更新操作提示
						}
						if((key == 'w' || key == 'W' || key == 'a' || key == 'A'
						 || key == 's' || key == 'S' || key == 'd' || key == 'D') && operateMode == 0)
						{
							if(debug == 1 || debug == 2)
							{
								printf("\n:(\n检测到wasd移动操作！正在识别...\n");
								system("pause");
							}
							operateMode = 1;
							break;//立即更新操作提示
						}
						if((key == 'm' || key == 'M') && operateMode != 2)
						{
							gotoxy(0, yOfMapEnd+showTime+5);
							if(debug == 1 || debug == 2)
							{
								printf("正在切换到鼠标操作...\n");
								system("pause");
							}
							operateMode = 2;
							break;
						}
						//操作识别
						if(operateMode == 0)
						{
							if(key == '@' || key == '#' || key == '$')
							{
								operation = key;
							}
							/*if(key >= '0' && key <= '9')
							{
								r *= 10;
								r = key-'0'+0;
							}*/
							if(operation != 0)
							{
								//gotoxy(1, yOfMapEnd+5);
								printf("%c", operation);
								scanf("%d%d", &r, &c);
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
						gotoxy(0, yOfMapEnd+showTime+5);//输出debug信息
						Solve();//执行当前可知所有Solution
						printf("                     ");//覆写[Loading...]8192/8192，21空格
						for(r=0; r<heightOfBoard; r++)//在原地图显示方案矩阵
						{
							for(c=0; c<widthOfBoard; c++)
							{
								if(solution[r][c] == 1)
								{
									gotoxy(cs0+2*c, yOfMapEnd-heightOfBoard+1+r);
									if(backgroundColor == 0x07) ColorStr("@", 0x0e);
									if(backgroundColor == 0xf0) ColorStr("@", 0xe4);
								}
								else if(solution[r][c] == 2)
								{
									gotoxy(cs0+2*c, yOfMapEnd-heightOfBoard+1+r);
									if(backgroundColor == 0x07) ColorStr("#", 0x0e);
									if(backgroundColor == 0xf0) ColorStr("#", 0xec);
								}
							}
						}
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
					if(showTime == 1)//刷新用时
					{
						gotoxy(0, yOfMapEnd+2);
						t1 = time(0);
						printf("用时：%d", t1-t0+t2);//从行首写防止文字重影
						gotoxy(1, yOfMapEnd+5);
					}
					if(operateMode > 0)
					{
						//显示操作对应指令
						gotoxy(0, yOfMapEnd+showTime+4);
						if(operation == 0)
						{
							printf(">? %d %d ", r, c);//覆写尾部
						}
						else
						{
							printf(">%c %d %d", operation, r, c);
						}
						if(newCursor == 1)//绘制><光标
						{
							gotoxy(cs0+2*c-3, yOfMapEnd-heightOfBoard+1+r);
							printf(" ");//去除左拖影
							gotoxy(cs0+2*c+3, yOfMapEnd-heightOfBoard+1+r);
							printf(" ");//去除右拖影
							gotoxy(cs0+2*c-1, yOfMapEnd-heightOfBoard+1+r-1);
							printf(" ");//去除左上拖影
							gotoxy(cs0+2*c+1, yOfMapEnd-heightOfBoard+1+r-1);
							printf(" ");//去除右上拖影
							gotoxy(cs0+2*c-1, yOfMapEnd-heightOfBoard+1+r+1);
							printf(" ");//去除左下拖影
							gotoxy(cs0+2*c+1, yOfMapEnd-heightOfBoard+1+r+1);
							printf(" ");//去除右下拖影
							gotoxy(cs0+2*c-1, yOfMapEnd-heightOfBoard+1+r);
							if(backgroundColor == 0x07) ColorStr(">", 0x0e);
							if(backgroundColor == 0xf0) ColorStr(">", 0xfd);
							gotoxy(cs0+2*c+1, yOfMapEnd-heightOfBoard+1+r);
							if(backgroundColor == 0x07) ColorStr("<", 0x0e);
							if(backgroundColor == 0xf0) ColorStr("<", 0xfd);
							//gotoxy(0, yOfMapEnd+showTime+4);
						}
						//光标移动至对应点
						gotoxy(cs0+2*c, yOfMapEnd-heightOfBoard+1+r);//当实际位置为该位置左部，光标显示在该位置
						if(operation != 0)
						{
							Sleep(refreshCycle);
							gotoxy(0, yOfMapEnd+showTime+5);
							break;
						}
					}
					Sleep(refreshCycle);//每100ms刷新一次
				}
				ro = r;
				co = c;
				/*执行操作*/
				if(operation == '@')
				{
					if(isShown[r][c] == 1)//翻开数字则尝试翻开周围
					{
						if(NumberOfSignAround(r,c) == numberOfMineAround[r][c])
						{
							for(ra=r-1; ra<=r+1; ra++)
							{
								for(ca=c-1; ca<=c+1; ca++)
								{
									if(ra>=0 && ra<heightOfBoard
									&& ca>=0 && ca<widthOfBoard)//确认在范围内
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
					else if(isShown[r][c] == 1)
					{
						if(fastSign == 1)//标记周围
						{
							for(ra=r-1; ra<=r+1; ra++)
							{
								for(ca=c-1; ca<=c+1; ca++)
								{
									if(ra>=0 && ra<heightOfBoard
									&& ca>=0 && ca<widthOfBoard)//确认在范围内
									{
										if(isShown[ra][ca] == 0)//标记所有%
										{
											isShown[ra][ca] = 2;
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
		}
		if(choiceMode == 2)
		{
			clrscr();
			DrawControlBar(0);
			gotoxy(0, 1);
			printf("** 键盘模式鼠标全选含坐标地图，复制粘贴并回车|按 ` 回车退出 **\n");
			SetConsoleMode(hdin, ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS);
			gotoxy(0, 3);
			system("pause");
			Solution();
			clrscr();
			DrawControlBar(0);
			SetConsoleMode(hdin, ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS);
			choiceMode = 0;
		}
		if(choiceMode == 3)
		{
			clrscr();
			DrawControlBar(0);
			DrawSettingsBar(0);
			choiceSet = 0;
			while(choiceMode == 3 && choiceSet != 7)
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
				if(rcd.EventType == KEY_EVENT && rcd.Event.KeyEvent.bKeyDown == 1)
				{
					if(rcd.Event.KeyEvent.wVirtualKeyCode >= '1' && rcd.Event.KeyEvent.wVirtualKeyCode <= '7')
					{
						choiceSet = rcd.Event.KeyEvent.wVirtualKeyCode-'1'+1;//主键盘1-7
					}
					if(rcd.Event.KeyEvent.wVirtualKeyCode >= 'a' && rcd.Event.KeyEvent.wVirtualKeyCode <= 'g')
					{
						choiceSet = rcd.Event.KeyEvent.wVirtualKeyCode-'a'+1;//小键盘1-7
					}
				}
				if(choiceSet == 1)//设置游戏难度
				{
					clrscr();
					DrawSetDifficulty(-1);
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
						if(rcd.EventType == KEY_EVENT && rcd.Event.KeyEvent.bKeyDown == 1)
						{
							if(rcd.Event.KeyEvent.wVirtualKeyCode >= '0' && rcd.Event.KeyEvent.wVirtualKeyCode <= '5')
							{
								choiceTemp = rcd.Event.KeyEvent.wVirtualKeyCode-'0'+0;//主键盘0-5
								difficulty = choiceTemp;
							}
							if(rcd.Event.KeyEvent.wVirtualKeyCode == '`')//小键盘0
							{
								choiceTemp = 0;
								difficulty = choiceTemp;
							}
							if(rcd.Event.KeyEvent.wVirtualKeyCode >= 'a' && rcd.Event.KeyEvent.wVirtualKeyCode <= 'e')
							{
								choiceTemp = rcd.Event.KeyEvent.wVirtualKeyCode-'a'+1;//小键盘1-5
								difficulty = choiceTemp;
							}
						}
						Sleep(refreshCycle);
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
						if(rcd.EventType == KEY_EVENT && rcd.Event.KeyEvent.bKeyDown == 1)
						{
							if(rcd.Event.KeyEvent.wVirtualKeyCode >= '0' && rcd.Event.KeyEvent.wVirtualKeyCode <= '3')
							{
								choiceTemp = rcd.Event.KeyEvent.wVirtualKeyCode-'0'+0;//主键盘0-3
								summonCheckMode = choiceTemp;
							}
							if(rcd.Event.KeyEvent.wVirtualKeyCode == '`')//小键盘0
							{
								choiceTemp = 0;
								summonCheckMode = choiceTemp;
							}
							if(rcd.Event.KeyEvent.wVirtualKeyCode >= 'a' && rcd.Event.KeyEvent.wVirtualKeyCode <= 'c')
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
				else if(choiceSet == 3)//更改游戏结束是否显示用时
				{
					if(showTime == 0)
					{
						showTime = 1;
					}
					else
					{
						showTime = 0;
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
						setbgcolor(backgroundColor);
					}
					else
					{
						//system("color 07");
						backgroundColor = 0x07;
						setbgcolor(backgroundColor);
					}
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
						if(rcd.EventType == KEY_EVENT && rcd.Event.KeyEvent.bKeyDown == 1)
						{
							if(rcd.Event.KeyEvent.wVirtualKeyCode >= '0' && rcd.Event.KeyEvent.wVirtualKeyCode <= '2')
							{
								choiceTemp = rcd.Event.KeyEvent.wVirtualKeyCode-'0'+0;//主键盘0-2
								operateMode = choiceTemp;
							}
							if(rcd.Event.KeyEvent.wVirtualKeyCode == '`')//小键盘0
							{
								choiceTemp = 0;
								operateMode = choiceTemp;
							}
							if(rcd.Event.KeyEvent.wVirtualKeyCode >= 'a' && rcd.Event.KeyEvent.wVirtualKeyCode <= 'b')
							{
								choiceTemp = rcd.Event.KeyEvent.wVirtualKeyCode-'a'+1;//小键盘1-2
								operateMode = choiceTemp;
							}
						}
						Sleep(refreshCycle);
					}
					clrscr();
					DrawControlBar(0);
					DrawSettingsBar(0);
					choiceSet = 0;
				}
				else if(choiceSet == 6)//调试选项
				{
					printf("*******************************\n"//宽31
						   "调试选项仅提供给专业玩家和开发者使用。\n"
						   "除非你知道你在做什么，否则请退出设置。\n"
						   "(1)退出\n"
						   "(2)设置求解模式\n"
						   "(3)以密度设置雷数\n"
						   "(4)设置背景颜色\n"
						   "(5)设置刷新周期\n");
					if(fastSign == 0) printf("(6)启用#数字一键标记周围\n");
					if(fastSign == 1) printf("(6)关闭#数字一键标记周围\n");
					printf("(7)设置光标\n");
					if(clickSpace == 1) printf("(8)启用鼠标点击屏蔽空格\n");
					if(clickSpace == 0) printf("(8)关闭鼠标点击屏蔽空格\n");
					if(show3BV == 0) printf("(9)启用游戏结束时显示3BV和3BV/s\n");
					if(show3BV == 1) printf("(9)关闭游戏结束时显示3BV和3BV/s\n");
					printf("*******************************\n");
					SetConsoleMode(hdin, ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS);
					system("pause");
					//getchar();
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
							if(temp==1 || temp==2) solveMode=temp;
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
							if(temp>=0 && temp<=30) lengthOfThinkChain=temp;
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
						if(temp < 1)
						{
							refreshCycle = 1;
						}
						else
						{
							refreshCycle = temp;
						}
					}
					else if(operation == '6')//#数字一键标记周围
					{
						if(fastSign == 0)
						{
							fastSign = 1;
						}
						else
						{
							fastSign = 0;
						}
					}
					else if(operation == '7')//设置光标
					{
						printf("*******************************\n");//宽31
						if(newCursor == 0) printf("(1)启用><追踪光标位置\n");
						if(newCursor == 1) printf("(1)关闭><追踪光标位置\n");
						if(visibleCursor == 1) printf("(2)隐藏光标\n");
						if(visibleCursor == 0) printf("(2)显示光标\n");
						printf("(3)退出\n"
							   "*******************************\n");
						printf(">");
						scanf("%d", &temp);
						if(temp == 1)
						{
							if(newCursor == 0)
							{
								newCursor = 1;
								//showCursor(0);
							}
							else
							{
								newCursor = 0;
								//showCursor(1);
							}
						}
						else if(temp == 2)
						{
							if(visibleCursor == 1)
							{
								visibleCursor = 0;
							}
							else
							{
								visibleCursor = 1;
							}
							showCursor(visibleCursor);
						}
					}
					else if(operation == '8')//鼠标点击屏蔽空格
					{
						if(clickSpace == 1)
						{
							clickSpace = 0;
						}
						else
						{
							clickSpace = 1;
						}
					}
					else if(operation == '9')//游戏结束时显示3BV和3BV/s
					{
						if(show3BV == 0)
						{
							show3BV = 1;
						}
						else
						{
							show3BV = 0;
						}
					}
					SetConsoleMode(hdin, ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS);
					clrscr();
					DrawControlBar(0);
					DrawSettingsBar(0);
					choiceSet = 0;
				}
				Sleep(refreshCycle);
			}
			if(choiceSet == 7)//返回并保存
			{
				file = fopen("minesweeper-settings.txt", "w");
				fprintf(file, "Map:%d*%d-%d\n", heightOfBoard, widthOfBoard, numberOfMine);
				fprintf(file, "summonCheckMode=%d\n", summonCheckMode);
				fprintf(file, "showTime=%d\n", showTime);
				fprintf(file, "operateMode=%d\n", operateMode);
				fprintf(file, "solveMode=%d\n", solveMode);
				fprintf(file, "lengthOfThinkChain=%d\n", lengthOfThinkChain);
				fprintf(file, "backgroundColor=0x%x\n", backgroundColor);
				fprintf(file, "refreshCycle=%d\n", refreshCycle);
				fprintf(file, "fastSign=%d\n", fastSign);
				fprintf(file, "newCursor=%d\n", newCursor);
				fprintf(file, "visibleCursor=%d\n", visibleCursor);
				fprintf(file, "clickSpace=%d\n", clickSpace);
				fprintf(file, "show3BV=%d\n", show3BV);
				fclose(file);
				clrscr();
				choiceMode = 0;
			}
		}
		if(choiceMode == 4)
		{
			clrscr();
			DrawControlBar(0);
			gotoxy(0, 1);
			printf("**                   请等待Bench结束. . .                   **\n");
			Bench(0, 99, heightOfBoard/2, widthOfBoard/2);
			choiceMode = 0;
		}
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
	if(select == 1)
	{
		gotoxy(3, 1);
		ColorStr("[(1)新游戏]", 0xff-backgroundColor);
	}
	if(select == 2)
	{
		gotoxy(15, 1);
		ColorStr("[(2)地图求解]", 0xff-backgroundColor);
	}
	if(select == 3)
	{
		gotoxy(29, 1);
		ColorStr("[(3)设置]", 0xff-backgroundColor);
	}
	if(select == 4)
	{
		gotoxy(39, 1);
		ColorStr("[(4)Bench]", 0xff-backgroundColor);
	}
	if(select == 5)
	{
		gotoxy(50, 1);
		ColorStr("[(5)退出]", 0xff-backgroundColor);
	}
	gotoxy(0, 3);
}

int ChoiceControlBar(COORD mousePos, DWORD dwButtonState)
{
	int choiceMode = 0;
	if(IsPosInRectangle(mousePos.X, mousePos.Y, 3, 1, 13, 1))
	{
		DrawControlBar(1);
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceMode = 1;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 15, 1, 27, 1))
	{
		DrawControlBar(2);
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceMode = 2;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 29, 1, 37, 1))
	{
		DrawControlBar(3);
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceMode = 3;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 39, 1, 48, 1))
	{
		DrawControlBar(4);
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceMode = 4;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 50, 1, 58, 1))
	{
		DrawControlBar(5);
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceMode = 5;
		}
	}
	else
	{
		DrawControlBar(0);
	}
	return choiceMode;
}

int WASDMove(int* r, int* c, WORD wVirtualKeyCode)
{
	if(wVirtualKeyCode == 'W' || wVirtualKeyCode == VK_UP)//W/w/方向键上
	{
		if((*r) > 0)
		{
			(*r)--;
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

void DrawSettingsBar(int select)
{
	gotoxy(0, 3);
	printf("**************************************************************\n");//宽62
	printf("* [(1)设置游戏难度]      当前雷数:%d|界面大小:%d*%d|密度:%.2f\n", numberOfMine, heightOfBoard, widthOfBoard, (float)numberOfMine/heightOfBoard/widthOfBoard);
	if(summonCheckMode == 0) printf("* [(2)设置地图生成校验]  当前:关闭校验\n");
	if(summonCheckMode == 1) printf("* [(2)设置地图生成校验]  当前:起始点必非雷\n");
	if(summonCheckMode == 2) printf("* [(2)设置地图生成校验]  当前:起始点必为空\n");
	if(summonCheckMode == 3) printf("* [(2)设置地图生成校验]  当前:地图可解\n");
	if(showTime == 0) printf("* [(3)启用实时显示用时]  当前:关闭\n");
	if(showTime == 1) printf("* [(3)关闭实时显示用时]  当前:启用\n");
	if(backgroundColor == 0x07) printf("* [(4)浅色模式]          当前:深色模式\n");
	else if(backgroundColor == 0xf0) printf("* [(4)深色模式]          当前:浅色模式\n");
	else printf("* [(4)深色模式]          当前:自定义模式\n");
	if(operateMode == 0) printf("* [(5)设置默认操作模式]  当前:键盘@#rc\n");
	if(operateMode == 1) printf("* [(5)设置默认操作模式]  当前:键盘wasd23\n");
	if(operateMode == 2) printf("* [(5)设置默认操作模式]  当前:鼠标点击\n");
	if(operateMode == 3) printf("* [(5)设置默认操作模式]  当前:Window\n");
	printf("* [(6)调试选项]\n");
	printf("* [(7)返回并保存]\n");
	printf("*******************************\n");
	if(select == 1)
	{
		gotoxy(2, 4);
		ColorStr("[(1)设置游戏难度]", 0xff-backgroundColor);
	}
	if(select == 2)
	{
		gotoxy(2, 5);
		ColorStr("[(2)设置地图生成校验]", 0xff-backgroundColor);
	}
	if(select == 3)
	{
		gotoxy(2, 6);
		if(showTime == 0) ColorStr("[(3)启用实时显示用时]", 0xff-backgroundColor);
		if(showTime == 1) ColorStr("[(3)关闭实时显示用时]", 0xff-backgroundColor);
	}
	if(select == 4)
	{
		gotoxy(2, 7);
		if(backgroundColor == 0x07) ColorStr("[(4)浅色模式]", 0xff-backgroundColor);
		else ColorStr("[(4)深色模式]", 0xff-backgroundColor);
	}
	if(select == 5)
	{
		gotoxy(2, 8);
		ColorStr("[(5)设置默认操作模式]", 0xff-backgroundColor);
	}
	if(select == 6)
	{
		gotoxy(2, 9);
		ColorStr("[(6)调试选项]", 0xff-backgroundColor);
	}
	if(select == 7)
	{
		gotoxy(2, 10);
		ColorStr("[(7)返回并保存]", 0xff-backgroundColor);
	}
	gotoxy(0, 12);
}

int ChoiceSettingsBar(COORD mousePos, DWORD dwButtonState)
{
	int choiceSet = 0;
	if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 4, 18, 4))
	{
		DrawSettingsBar(1);
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSet = 1;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 5, 22, 5))
	{
		DrawSettingsBar(2);
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSet = 2;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 6, 22, 6))
	{
		DrawSettingsBar(3);
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSet = 3;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 7, 14, 7))
	{
		DrawSettingsBar(4);
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSet = 4;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 8, 22, 8))
	{
		DrawSettingsBar(5);
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSet = 5;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 9, 14, 9))
	{
		DrawSettingsBar(6);
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSet = 6;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 10, 16, 10))
	{
		DrawSettingsBar(7);
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSet = 7;
		}
	}
	else
	{
		DrawSettingsBar(0);
	}
	return choiceSet;
}

void DrawSetDifficulty(int select)
{
	int r, c;
	gotoxy(0, 0);
	printf("**************************************************************\n");//宽62
	printf("** [(0)默认：10*10 - 10] ** 空间占用示意 ** 可使用全屏，    **\n");
	printf("** [(1)初级： 9*9  - 10] ****************** 可使用Ctrl+滚轮 **\n");
	printf("** [(2)中级：16*16 - 40] **              ** 调整字体大小。  **\n");
	printf("** [(3)高级：16*30 - 99] **              *********************\n");
	printf("** [(4)顶级：42*88 -715] **    [清屏]    ** 如界面下滑，    **\n");
	printf("** [(5)自定义***** - **] **              ** 不建议选择！    **\n");
	printf("**************************************************************\n");
	if(select == 0)
	{
		gotoxy(3, 1);
		ColorStr("[(0)默认：10*10 - 10]", 0xff-backgroundColor);
	}
	if(select == 1)
	{
		gotoxy(3, 2);
		ColorStr("[(1)初级： 9*9  - 10]", 0xff-backgroundColor);
	}
	if(select == 2)
	{
		gotoxy(3, 3);
		ColorStr("[(2)中级：16*16 - 40]", 0xff-backgroundColor);
	}
	if(select == 3)
	{
		gotoxy(3, 4);
		ColorStr("[(3)高级：16*30 - 99]", 0xff-backgroundColor);
	}
	if(select == 4)
	{
		gotoxy(3, 5);
		ColorStr("[(4)顶级：42*88 -715]", 0xff-backgroundColor);
	}
	if(select == 5)
	{
		gotoxy(3, 6);
		ColorStr("[(5)自定义***** - **]", 0xff-backgroundColor);
	}
	if(select == 6)
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
	if(IsPosInRectangle(mousePos.X, mousePos.Y, 3, 1, 23, 1))
	{
		if(heightOfBoard > 10 || widthOfBoard > 10)
		{
			DrawSetDifficulty(7);
		}
		DrawSetDifficulty(0);
		//默认为10*10-10，比初级更简单(doge)
		numberOfMine = 10;//密度0.1
		heightOfBoard = 10;
		widthOfBoard = 10;
		SummonBoard(0, 5, 5);
		ShowBoard(1);
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceDifficulty = 0;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 3, 2, 23, 2))
	{
		if(heightOfBoard > 9 || widthOfBoard > 9)
		{
			DrawSetDifficulty(7);
		}
		DrawSetDifficulty(1);
		//初级，胜率96%
		numberOfMine = 10;//密度0.12345679
		heightOfBoard = 9;
		widthOfBoard = 9;
		SummonBoard(0, 4, 4);
		ShowBoard(1);
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceDifficulty = 1;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 3, 3, 23, 3))
	{
		if(heightOfBoard > 16 || widthOfBoard > 16)
		{
			DrawSetDifficulty(7);
		}
		DrawSetDifficulty(2);
		//中级，胜率84%
		numberOfMine = 40;//密度0.15625
		heightOfBoard = 16;
		widthOfBoard = 16;
		SummonBoard(0, 8, 8);
		ShowBoard(1);
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceDifficulty = 2;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 3, 4, 23, 4))
	{
		if(heightOfBoard > 16 || widthOfBoard > 30)
		{
			DrawSetDifficulty(7);
		}
		DrawSetDifficulty(3);
		//高级，胜率48%
		numberOfMine = 99;//密度0.20625
		heightOfBoard = 16;
		widthOfBoard = 30;
		SummonBoard(0, 8, 15);
		ShowBoard(1);
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceDifficulty = 3;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 3, 5, 23, 5))
	{
		DrawSetDifficulty(4);
		//顶级，胜率26%
		numberOfMine = 715;//密度0.193452
		heightOfBoard = 42;
		widthOfBoard = 88;//实测较为合适的全屏地图，雷数为胡桃生日
		SummonBoard(0, 21, 44);
		ShowBoard(1);
		gotoxy(0, 0);
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceDifficulty = 4;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 3, 6, 23, 6))
	{
		DrawSetDifficulty(5);
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)//自定义
		{
			choiceDifficulty = 5;
			DrawSetDifficulty(7);
			SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS);
			gotoxy(0, 8);
			system("pause");
			//设置界面高度
			gotoxy(0, 9);
			printf("[行数]>");
			scanf("%d", &heightOfBoard);
			if(heightOfBoard < 1) heightOfBoard = 1;
			if(heightOfBoard > LimHeight) heightOfBoard = LimHeight;
			gotoxy(13, 6);
			printf("%d", heightOfBoard);
			//设置界面宽度
			gotoxy(0, 10);
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
				if(temp == 1)
				{
					widthOfBoard--;
				}
				else if(temp == 2)
				{
					heightOfBoard = 100;
				}
			}*/
			//设置雷数
			gotoxy(0, 11);
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
			if(density > 0.72)
			{
				printf(":(\n当前密度%.2f，难以生成地图\n", density);
			}
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
			//显示预览
			clrscr();
			DrawSetDifficulty(-1);
			gotoxy(13, 6);
			printf("%d*%d-%d", heightOfBoard, widthOfBoard, numberOfMine);
			SummonBoard(0, heightOfBoard/2, widthOfBoard/2);
			gotoxy(0, 8);
			ShowBoard(1);
			gotoxy(0, 0);
			gotoxy(62, 7);
			system("pause");
			SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS);
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 31, 5, 36, 5))
	{
		DrawSetDifficulty(6);
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			//choiceDifficulty = -1;
			clrscr();
		}
	}
	else
	{
		DrawSetDifficulty(-1);
	}
	return choiceDifficulty;
}

void DrawSetSummonCheckMode(int select)
{
	gotoxy(0, 12);
	printf("*******************************\n"//宽31
		   "* [(0)关闭校验]\n"
		   "* [(1)起始点必非雷]\n"
		   "* [(2)起始点必为空]\n"
		   "* [(3)地图可解]（生成时间可能较长）\n"
		   "*******************************\n");
	if(select == 0)
	{
		gotoxy(2, 13);
		ColorStr("[(0)关闭校验]", 0xff-backgroundColor);
	}
	if(select == 1)
	{
		gotoxy(2, 14);
		ColorStr("[(1)起始点必非雷]", 0xff-backgroundColor);
	}
	if(select == 2)
	{
		gotoxy(2, 15);
		ColorStr("[(2)起始点必为空]", 0xff-backgroundColor);
	}
	if(select == 3)
	{
		gotoxy(2, 16);
		ColorStr("[(3)地图可解]", 0xff-backgroundColor);
	}
	gotoxy(0, 18);
}

int ChoiceSetSummonCheckMode(COORD mousePos, DWORD dwButtonState)
{
	int choiceSummonCheckMode = -1;
	if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 13, 14, 13))
	{
		DrawSetSummonCheckMode(0);
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSummonCheckMode = 0;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 14, 18, 14))
	{
		DrawSetSummonCheckMode(1);
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSummonCheckMode = 1;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 15, 18, 15))
	{
		DrawSetSummonCheckMode(2);
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSummonCheckMode = 2;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 16, 14, 16))
	{
		DrawSetSummonCheckMode(3);
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSummonCheckMode = 3;
		}
	}
	else
	{
		DrawSetSummonCheckMode(-1);
	}
	return choiceSummonCheckMode;
}

void DrawSetOperateMode(int select)
{
	gotoxy(0, 12);
	printf("*******************************\n");
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
	printf("*******************************\n");
	if(select == 0)
	{
		gotoxy(2, 15);
		ColorStr("[(0)@#rc操作模式]", 0xff-backgroundColor);
	}
	if(select == 1)
	{
		gotoxy(2, 18);
		ColorStr("[(1)wasd23操作模式]", 0xff-backgroundColor);
	}
	if(select == 2)
	{
		gotoxy(2, 21);
		ColorStr("[(2)鼠标点击操作模式]", 0xff-backgroundColor);
	}
	gotoxy(0, 25);
}

int ChoiceSetOperateMode(COORD mousePos, DWORD dwButtonState)
{
	int choiceOperateMode = -1;
	if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 15, 18, 15))
	{
		DrawSetOperateMode(0);
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceOperateMode = 0;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 18, 20, 18))
	{
		DrawSetOperateMode(1);
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceOperateMode = 1;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 21, 22, 21))
	{
		DrawSetOperateMode(2);
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceOperateMode = 2;
		}
	}
	else
	{
		DrawSetOperateMode(-1);
	}
	return choiceOperateMode;
}

void SummonBoard(int seed, int r0, int c0)//生成后台总板
{
	int r, c, i, ra, ca;
	srand(seed);
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
		/*调试*/
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
							if(ra>=0 && ra<heightOfBoard
							&& ca>=0 && ca<widthOfBoard)//确认在范围内
							{
								numberOfMineAround[ra][ca]++;
							}
						}
					}
				}//挨得过紧的雷也会被数字覆盖
			}
		}
		/*调试*/
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
		/*调试*/
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
	int i;
	i=0;
	if(n == 0) return 1;
	while(n>0)
	{
		i++;
		n /= 10;
	}
	return i;
}

void PrintCell(int board, int isShown, int bkcolor)
{
	if(isShown == 2)
	{
		ColorStr("#", 0x0c + bkcolor);
		printf(" ");
	}
	else if(isShown == 0)
	{
		if(board == 9)
		{
			ColorStr("* ", 0x0c + bkcolor);
		}
		else
		{
			printf("%% ");
		}
	}
	else
	{
		if(board == 9)
		{
			ColorStr("@ ", 0x04 + bkcolor);
		}
		else if(board == 0)
		{
			printf("  ");
		}
		else if(board == 1)
		{
			ColorStr("1 ", 0x01 + bkcolor);
		}
		else if(board == 2)
		{
			ColorStr("2 ", 0x02 + bkcolor);
		}
		else if(board == 3)
		{
			ColorStr("3 ", 0x04 + bkcolor);
		}
		else if(board == 4)
		{
			ColorStr("4 ", 0x04 + bkcolor);
		}
		else if(board == 5)
		{
			ColorStr("5 ", 0x04 + bkcolor);
		}
		else if(board == 6)
		{
			ColorStr("6 ", 0x04 + bkcolor);
		}
		else if(board == 7)
		{
			ColorStr("7 ", 0x04 + bkcolor);
		}
		else if(board == 8)
		{
			ColorStr("8 ", 0x04 + bkcolor);
		}
	}
}

void ShowBoard(int mode)
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
			n=c;
			for(j=1; j<i; j++)
			{
				n/=10;
			}
			n%=10;
			if(i!=1 && n==0)
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
		if(backgroundColor == 0xf0)//浅色模式
		{
			for(c=0; c<widthOfBoard; c++)
			{
				if(mode == 1)//后台
				{
					if(board[r][c] == 0)
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
					else if(board[r][c] == 9 && isShown[r][c] == 0)
					{
						PrintCell(0, 0, 0xf0);
					}
					else if(isShown[r][c] == 1 && board[r][c] > 5)
					{
						PrintCell(board[r][c], isShown[r][c], 0xe0);
					}
					else
					{
						PrintCell(board[r][c], isShown[r][c], 0xf0);
					}
				}
			}
			printf("\n");
		}
		else//深色模式和自定义模式
		{
			for(c=0; c<widthOfBoard; c++)
			{
				if(mode == 1)//后台
				{
					if(board[r][c] == 0)
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
					if(board[r][c] == 9 && isShown[r][c] == 0)
					{
						PrintCell(0, 0, backgroundColor/16*16);
					}
					else if(isShown[r][c] == 1 && board[r][c] > 5)
					{
						PrintCell(board[r][c], isShown[r][c], backgroundColor/16*16 + 0x0e - 0x04);
					}
					else
					{
						PrintCell(board[r][c], isShown[r][c], backgroundColor/16*16);
					}
				}
			}
			printf("\n");
		}
	}
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
*/
void OpenZeroChain(int r0, int c0)//翻开0连锁翻开
{
	int r, c;
	if(isShown[r0][c0] == 1 && board[r0][c0] == 0)
	{
		SummonZeroChain(r0, c0);
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

void SummonZeroChain(int r0, int c0)//对0链进行标记
{
	int r, c, isRising;
	/*重置*/
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			zeroChain[r][c] = 0;
		}
	}
	/*生成*/
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
	/*调试*/
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
}

int RealRemainder()//真实的剩余雷数
{
	int r, c, remainder;
	remainder = 0;
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
	int r, c, n;
	n = 0;
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
	int r, c, n;
	n = 0;
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
	int r, c, n;
	n = 0;
	for(r=r0-1; r<=r0+1; r++)
	{
		for(c=c0-1; c<=c0+1; c++)
		{
			if(r>=0 && r<heightOfBoard
			&& c>=0 && c<widthOfBoard)//确认在范围内
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
	int r, c, n;
	n = 0;
	for(r=r0-1; r<=r0+1; r++)
	{
		for(c=c0-1; c<=c0+1; c++)
		{
			if(r>=0 && r<heightOfBoard
			&& c>=0 && c<widthOfBoard)//确认在范围内
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

int IsAroundZeroChain(int r0, int c0)
{
	int r, c;
	for(r=r0-1; r<=r0+1; r++)
	{
		for(c=c0-1; c<=c0+1; c++)
		{
			if(r>=0 && r<heightOfBoard
			&& c>=0 && c<widthOfBoard)//确认在范围内
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

void LookMap()//实时游戏中获取地图、已知数字
{
	int r, c;
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			numberShown[r][c]=9;
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
	/*获取地图*/
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
	/*识别显示方式矩阵*/
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(map[r][c] == '%')
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
	/*获取已知数字*/
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			numberShown[r][c]=9;
			if(isShown[r][c] == 1)
			{
				if(map[r][c] == ' ')
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
	isSolving=1;
	while(isSolving == 1)
	{
		isSolving=0;
		/*执行策略*/
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				if(isShown[r][c]==1 && numberShown[r][c]!=0 && NumberOfNotShownAround(r,c) != 0)//寻找数字
				{
					if(numberShown[r][c] == NumberOfNotShownAround(r,c) + NumberOfSignAround(r,c))
					{
						/*策略一
						当某块的数字与未知方块的个数相同时，那么未知方块都是雷*/
						if(debug == 2) printf("[Debug]找到1类点(%d,%d)->#", r, c);
						//标记周围未知方块
						for(ra=r-1; ra<=r+1; ra++)
						{
							for(ca=c-1; ca<=c+1; ca++)
							{
								if(ra>=0 && ra<heightOfBoard
								&& ca>=0 && ca<widthOfBoard)//确认在范围内
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
						isSolving=1;
					}
					if(numberShown[r][c] == NumberOfSignAround(r,c))
					{
						/*策略二
						当某块的数字与周围旗子的个数相同时，那么未知方块都不是雷*/
						if(debug == 2) printf("[Debug]找到2类点(%d,%d)->@", r, c);
						//翻开周围未知方块
						for(ra=r-1; ra<=r+1; ra++)
						{
							for(ca=c-1; ca<=c+1; ca++)
							{
								if(ra>=0 && ra<heightOfBoard
								&& ca>=0 && ca<widthOfBoard)//确认在范围内
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
						isSolving=1;
					}
				}
			}
		}
		if(isSolving == 0)//策略一二无法进行
		{
			/*标记交界线处未知方块*/
			for(r=0; r<heightOfBoard; r++)
			{
				for(c=0; c<widthOfBoard; c++)
				{
					if(isShown[r][c]==0 && isThought[r][c]==0
					&& NumberOfNumberAround(r,c)!=0)
					{
						isThought[r][c] = 1;
						map[r][c] = '?';
					}
				}
			}
			/*检查待推理方块存在*/
			isThinking=0;
			for(r=0; r<heightOfBoard; r++)
			{
				for(c=0; c<widthOfBoard; c++)
				{
					if(isThought[r][c] == 1)
					{
						isThinking=1;
						break;
					}
				}
				if(isThinking == 1) break;
			}
			if(isThinking == 1)
			{
				/*策略三
				逻辑推理*/
				if(debug == 2) printf("[Debug]正在深入思考. . . \n");
				isSolving=Think();//0放弃，1尝试下一未知链
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
	isFound=0;
	isFoundOpen=0;
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(solution[r][c] == 1)
			{
				isFound=1;
				isFoundOpen=1;
			}
			else if(solution[r][c] == 2)
			{
				isFound=1;
			}
			if(isShown[r][c] == 2)
			{
				isMineRate[r][c]=1;
			}
		}
	}
	SummonMineRateForNotShown();
	minMineRateNotZero=1;
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
				isFound=1;
				isFoundOpen=1;
				if(debug == 2) printf("[Debug]找到7类点@(%d,%d)\n", r, c);
			}
			if(isMineRate[r][c] == 1 && isShown[r][c] != 2)
			{
				solution[r][c] = 2;
				isShown[r][c] = 2;
				isFound=1;
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
				if(isMineRate[r][c] == 0)
				{
					printf("     ");
				}
				else if(isMineRate[r][c] == 1)
				{
					printf("Mine ");
				}
				else
				{
					printf("%.2f ", isMineRate[r][c]);
				}
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
	int r, c, r1, c1, r2, c2, i;//循环变量
	int numberOfMine1, numberOfMine2, numberOfNotShown1, numberOfNotShown2;//数对变量
	int isFound, isFoundThinkChain, isRising, isAroundThinkChain;//未知链生成
	int numberOfThought, numberOfPossibility, remainedMine, remainedNotShown;//枚举准备
	int isWrong, realNumberOfPossibility, temp;//枚举判断
	/*标记未知方块附近数字，作为数对之一*/
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(isThought[r][c] == 2)//重置数对之一
			{
				isThought[r][c] = 0;
			}
			if(isShown[r][c]==1 && numberShown[r][c]!=9
			&& NumberOfNotShownAround(r,c) != 0)
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
				if(isThought[r][c] == 0)
				{
					printf("  ");
				}
				else
				{
					printf("%d ", isThought[r][c]);
				}
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
							if(r2>=0 && r2<heightOfBoard
							&& c2>=0 && c2<widthOfBoard)//确认在范围内
							{
								if(isShown[r2][c2]==1 && numberShown[r2][c2]!=9 && numberShown[r2][c2]!=0)//已找到数对之二
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
											if(r>=0 && r<heightOfBoard
											&& c>=0 && c<widthOfBoard)//确认在范围内
											{
												numberTeam[r][c] += 1;
											}
										}
									}
									for(r=r2-1; r<=r2+1; r++)
									{
										for(c=c2-1; c<=c2+1; c++)
										{
											if(r>=0 && r<heightOfBoard
											&& c>=0 && c<widthOfBoard)//确认在范围内
											{
												numberTeam[r][c] += 2;
											}
										}
									}
									numberTeam[r1][c1]=9;
									numberTeam[r2][c2]=9;
									//数对之一独占区1，数对之二独占区2，共享区3
									/*求独占区雷数和未知数*/
									numberOfMine1=0;
									numberOfMine2=0;
									numberOfNotShown1=0;
									numberOfNotShown2=0;
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
												if(numberTeam[r][c] == 0)
												{
													printf("  ");
												}
												else
												{
													printf("%d ", numberTeam[r][c]);
												}
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
														if(r>=0 && r<heightOfBoard
														&& c>=0 && c<widthOfBoard)//确认在范围内
														{
															if(numberTeam[r][c] == 1 && isThought[r][c] == 1)
															{
																solution[r][c] = 1;
																if(debug == 2) printf("[Debug]找到1类数对(%d,%d)(%d,%d)->@(%d,%d)\n", r1, c1, r2, c2, r, c);
																return 1;
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
														if(r>=0 && r<heightOfBoard
														&& c>=0 && c<widthOfBoard)//确认在范围内
														{
															if(numberTeam[r][c] == 1 && isThought[r][c] == 1)
															{
																solution[r][c] = 2;
																if(debug == 2) printf("[Debug]找到2类数对(%d,%d)(%d,%d)->#(%d,%d)\n", r1, c1, r2, c2, r, c);
																return 1;
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
														if(r>=0 && r<heightOfBoard
														&& c>=0 && c<widthOfBoard)//确认在范围内
														{
															if(numberTeam[r][c] == 1 && isThought[r][c] == 1)
															{
																solution[r][c] = 1;
																if(debug == 2) printf("[Debug]找到3类数对(%d,%d)(%d,%d)->@(%d,%d)\n", r1, c1, r2, c2, r, c);
																return 1;
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
														if(r>=0 && r<heightOfBoard
														&& c>=0 && c<widthOfBoard)//确认在范围内
														{
															if(numberTeam[r][c] == 1 && isThought[r][c] == 1)
															{
																solution[r][c] = 2;
																if(debug == 2) printf("[Debug]找到4类数对(%d,%d)(%d,%d)->#(%d,%d)\n", r1, c1, r2, c2, r, c);
																return 1;
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
														if(r>=0 && r<heightOfBoard
														&& c>=0 && c<widthOfBoard)//确认在范围内
														{
															if(numberTeam[r][c] == 1 && isThought[r][c] == 1)
															{
																solution[r][c] = 1;
																if(debug == 2) printf("[Debug]找到5类数对(%d,%d)(%d,%d)->@(%d,%d)\n", r1, c1, r2, c2, r, c);
																return 1;
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
														if(r>=0 && r<heightOfBoard
														&& c>=0 && c<widthOfBoard)//确认在范围内
														{
															if(numberTeam[r][c] == 1 && isThought[r][c] == 1)
															{
																solution[r][c] = 2;
																if(debug == 2) printf("[Debug]找到6类数对(%d,%d)(%d,%d)->#(%d,%d)\n", r1, c1, r2, c2, r, c);
																return 1;
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
														if(r>=0 && r<heightOfBoard
														&& c>=0 && c<widthOfBoard)//确认在范围内
														{
															if(numberTeam[r][c] == 1 && isThought[r][c] == 1)
															{
																solution[r][c] = 1;
																if(debug == 2) printf("[Debug]找到7类数对(%d,%d)(%d,%d)->@(%d,%d)\n", r1, c1, r2, c2, r, c);
																return 1;
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
														if(r>=0 && r<heightOfBoard
														&& c>=0 && c<widthOfBoard)//确认在范围内
														{
															if(numberTeam[r][c] == 1 && isThought[r][c] == 1)
															{
																solution[r][c] = 2;
																if(debug == 2) printf("[Debug]找到8类数对(%d,%d)(%d,%d)->#(%d,%d)\n", r1, c1, r2, c2, r, c);
																return 1;
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
	/*是否枚举*/
	if(solveMode == 1)
	{
		isFound=0;
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				if(solution[r][c] != 0)
				{
					isFound=1;
				}
			}
		}
		if(isFound == 1)
		{
			return 0;
		}
	}
	/*生成未知链*/
	for(r=0; r<heightOfBoard; r++)//重置
	{
		for(c=0; c<widthOfBoard; c++)
		{
			thinkChain[r][c] = 0;
		}
	}
	isFoundThinkChain=0;
	for(r=0; r<heightOfBoard; r++)//找到生成点
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(isThought[r][c] == 1)
			{
				thinkChain[r][c] = 1;
				isFoundThinkChain=1;
				break;
			}
		}
		if(isFoundThinkChain == 1) break;
	}
	if(isFoundThinkChain == 0) return 0;//退出
	isRising=1;
	while(isRising == 1)//向周围生长
	{
		isRising = 0;
		for(r2=0; r2<heightOfBoard; r2++)//标记周围数字用于校验
		{
			for(c2=0; c2<widthOfBoard; c2++)
			{
				if(isShown[r2][c2]==1 && numberShown[r2][c2]!=9)
				{
					isAroundThinkChain = 0;
					for(r=r2-1; r<=r2+1; r++)
					{
						for(c=c2-1; c<=c2+1; c++)
						{
							if(r>=0 && r<heightOfBoard
							&& c>=0 && c<widthOfBoard)//确认在范围内
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
							if(r>=0 && r<heightOfBoard
							&& c>=0 && c<widthOfBoard)//确认在范围内
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
				if(thinkChain[r][c] == 0)
				{
					printf("  ");
				}
				else
				{
					printf("%d ", thinkChain[r][c]);
				}
			}
			printf("\n");
		}
	}
	/*枚举验证*/
	numberOfThought=0;
	numberOfPossibility=1;
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
	//int上限2147483647=2^31-1，最多支持长度30的未知链
	//0关 17效率 30最大
	{
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
	if(debug == 2)
	{
		printf("[Debug]%d个未知点共%d种可能性\n", numberOfThought, numberOfPossibility);
	}
	remainedMine = numberOfMine - NumberOfSign();//剩余雷数
	remainedNotShown = NumberOfNotShown() - numberOfThought;//不含未知链的剩余%数
	realNumberOfPossibility=0;
	for(i=0; i<numberOfPossibility; i++)//遍历可能性
	{
		/*进度条*/
		if(numberOfPossibility > 262144 && (i+1)%131072==0)
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
				numberCheck[r][c] = 0;
			}
		}
		/*生成可能性*/
		r=i;//r既是row又是remainder
		c=0;//c既是column又是count，一变量多用了属于是(doge)
		while(r>0)
		{
			possibility[c] = r%2;
			r /= 2;
			c++;
		}
		/*根据雷数排除*/
		r=0;
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
		c=0;
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
								if(r2>=0 && r2<heightOfBoard
								&& c2>=0 && c2<widthOfBoard)//确认在范围内
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
							if(r2>=0 && r2<heightOfBoard
							&& c2>=0 && c2<widthOfBoard)//确认在范围内
							{
								numberCheck[r2][c2]++;
							}
						}
					}
				}
			}
		}
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
		/*数字校验*/
		isWrong=0;
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				if(thinkChain[r][c] == 2)//仅校验未知链附近数字
				{
					if(numberShown[r][c] != numberCheck[r][c])
					{
						isWrong=1;
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
			temp=i;
			if(debug == 2)
			{
				printf("\r[Debug]已找到第%d种可能性%d：", realNumberOfPossibility, temp);//覆盖进度条
				for(c=0; c<numberOfThought; c++)
				{
					printf("%d", possibility[c]);
				}
				printf("\n");
				c=0;
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
		r=temp;
		c=0;
		while(r>0)
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
		c=0;
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
		c=0;
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
	return 1;
}

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
	int r, c, n;
	n=0;
	for(r=r0-1; r<=r0+1; r++)
	{
		for(c=c0-1; c<=c0+1; c++)
		{
			if(r>=0 && r<heightOfBoard
			&& c>=0 && c<widthOfBoard)//确认在范围内
			{
				if(isShown[r][c]==1 && numberShown[r][c]!=9)//isShown1存在@numberShown9
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

void Solution()//地图求解模块
{
	int yOfMapEnd, r, c;
	yOfMapEnd = 3;
	gotoxy(0, 3);
	//printf("MineSweeper Solution\n");
	printf("当前雷数:%d|当前界面大小:%d*%d\n", numberOfMine, heightOfBoard, widthOfBoard);
	srand(time(0));
	if(debug == 2)
	{
		printf("<Debug>\n");
		printf("pointer ->(%d,%d)\n", rs0, cs0);
		printf("heightOfMapShown=%d\n", heightOfMapShown);
		printf("widthOfMapShown=%d\n", widthOfMapShown);
		yOfMapEnd += 4;
	}
	/*扫雷*/
	//getchar();//吃回车
	while(1)
	{
		/*输入*/
		printf("请粘贴最新地图：\n");
		yOfMapEnd++;
		for(r=0; r<heightOfMapShown; r++)
		{
			for(c=0; c<widthOfMapShown; c++)//默认为空格
			{
				mapShown[r][c] = ' ';
			}
			gets(mapShown[r]);//从键盘获取该行地图
			if(mapShown[r][0] == '`') break;
		}
		yOfMapEnd += heightOfMapShown;
		if(mapShown[r][0] == '`') break;
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
				printf("\n");
			}
		}
		TranslateMap(rs0,cs0);
		/*扫雷策略和输出*/
		ShowAnswer(Solve(), 0);
		for(r=0; r<heightOfBoard; r++)//在原地图显示方案矩阵
		{
			for(c=0; c<widthOfBoard; c++)
			{
				if(solution[r][c] == 1)
				{
					gotoxy(cs0+2*c, yOfMapEnd-heightOfBoard+1+r);
					if(backgroundColor == 0x07) ColorStr("@", 0x0e);
					if(backgroundColor == 0xf0) ColorStr("@", 0xe4);
				}
				else if(solution[r][c] == 2)
				{
					gotoxy(cs0+2*c, yOfMapEnd-heightOfBoard+1+r);
					if(backgroundColor == 0x07) ColorStr("#", 0x0e);
					if(backgroundColor == 0xf0) ColorStr("#", 0xec);
				}
			}
		}
		gotoxy(0, yOfMapEnd+3);
		yOfMapEnd += 2;
	}
}

void Bench(int seedMin, int seedMax, int r0, int c0)//Bench模块
{
	int seed, r, c, isOpenMine, temp;
	int t0, t1, countOfWin = 0, countOfSolution = 0, countOfStep = 0;//Bench统计
	/*初始化*/
	numberOfAbandonedThinkChain = 0;
	for(seed=seedMin; seed<=seedMax; seed++)
	{
		isWinningOfSeed[seed]=0;
	}
	t0=time(0);//计时
	/*开跑！*/
	for(seed=seedMin; seed<=seedMax; seed++)
	{
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				isShown[r][c] = 0;
			}
		}
		isOpenMine=0;
		if(debug == 2)
		{
			debug = 0;//Bench时不显示多余信息
			SummonBoard(seed, r0, c0);
			debug = 2;
		}
		else
		{
			SummonBoard(seed, r0, c0);
		}
		isShown[r0][c0] = 1;
		OpenZeroChain(r0,c0);//第一次打开0链
		while(1)
		{
			//clrscr();
			gotoxy(0, 3);//覆写
			if(debug == 1 || debug == 2)
			{
				printf("<Debug>\n");
				printf("Map:%d*%d-%d\n", heightOfBoard, widthOfBoard, numberOfMine);
				printf("seed=%d,%d,%d\n", seed, r0, c0);
				t1=time(0);//显示Ave FPS（步数/时间）
				if(t1 != t0)
				{
					printf("Ave FPS %d \n", countOfStep/(t1-t0));
				}
				else
				{
					printf("Ave FPS 0\n");
				}
				//ShowBoard(1);
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
							OpenZeroChain(r,c);
						}
						else if(board[r][c] == 9)
						{
							isOpenMine=1;
							ShowBoard(1);
							//system("pause");
						}
					}
				}
			}
			/*胜负判断*/
			if(isOpenMine == 1) break;
			if(RealRemainder()==0)
			{
				countOfWin++;
				isWinningOfSeed[seed]++;
				break;
			}
			/*地图求解*/
			ShowBoard(0);
			LookMap();
			temp=debug;
			debug=0;//Bench时不显示多余信息
			ShowAnswer(Solve(), 1);
			debug=temp;
			if(debug == 1 || debug == 2)//Bench仅显示方案矩阵
			{
				for(r=0; r<heightOfBoard; r++)//在原地图显示方案矩阵
				{
					for(c=0; c<widthOfBoard; c++)
					{
						if(solution[r][c] == 1)
						{
							gotoxy(cs0+2*c, heightOfMapShown-1+7-heightOfBoard+1+r);
							if(backgroundColor == 0x07) ColorStr("@", 0x0e);
							if(backgroundColor == 0xf0) ColorStr("@", 0xe4);
						}
						else if(solution[r][c] == 2)
						{
							gotoxy(cs0+2*c, heightOfMapShown-1+7-heightOfBoard+1+r);
							if(backgroundColor == 0x07) ColorStr("#", 0x0e);
							if(backgroundColor == 0xf0) ColorStr("#", 0xec);
						}
					}
				}
				gotoxy(0, heightOfMapShown-1+5);
			}
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
			//system("pause");
		}
	}
	/*输出结果*/
	t1=time(0);
	//clrscr();
	gotoxy(0, 3);
	if(debug == 1 || debug == 2)
	{
		printf("<Debug>\n");
		printf("Map:%d*%d-%d\n", heightOfBoard, widthOfBoard, numberOfMine);
		printf("seed=%d,%d,%d\n", seedMax, r0, c0);
		printf("Ave FPS %d\n", countOfStep/(t1-t0));
		ShowBoard(1);
		printf("胜利数：%d\n方案数：%d\n步数：%d\n", countOfWin, countOfSolution, countOfStep);
		printf("步均方案数：%.2f\n", (float)countOfSolution/countOfStep);
		printf("参考时间：%d\n", t1-t0);
		printf("放弃链数：%d\n", numberOfAbandonedThinkChain);
		printf("[Debug]胜利矩阵：\n");
		for(seed=seedMin; seed<=seedMax; seed++)
		{
			if(isWinningOfSeed[seed] == 1)
			{
				printf("%2d ", seed);
			}
			else
			{
				printf("   ");
			}
			if((seed+1)%10 == 0) printf("\n");
		}
		printf("\n");
	}
	else
	{
		ShowBoard(1);
		printf("胜率：%d%%\n", countOfWin*100/(seedMax-seedMin+1));
	}
}

int BBBV(int seed, int r0, int c0)//计算地图3BV
{
	int bbbv = 0, r, c;
	SummonBoard(seed, r0, c0);
	//ShownModeBak(1);
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			isShown[r][c] = 0;
		}
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
	//ShownModeBak(0);
	return bbbv;
}

void Error()
{
	//system("color 1f");
	setbgcolor(0x1f);
	printf(":(\n"
		"你的设备遇到问题，需要重启。\n"
		"我们只收集某些错误信息，然后你可以重新启动。\n\n");
	for(int c=0; c<=10; c++)
	{
		printf("\r%3d%% 完成", c*10);
		Sleep(100);
	}
	Sleep(4*100);
	printf("\n");
	getchar();
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
——修复 继续上一次游戏不再会丢失标记
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
//——新增 调试选项可启用在外部窗口进行游戏
//——优化 不再支持单独的wasd23模式，可在鼠标模式使用，键盘模式仅支持@#rc
//——修复 @#rc输入坐标时用时显示停滞
//——修复 雷率为-0和负数时不视为确定解
--------------------------------*/
