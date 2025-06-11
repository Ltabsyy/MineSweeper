//#define _WIN32_WINNT 0x0600//使用SetProcessDPIAware()
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <graphics.h>
/**
 * 扫雷 MineSweeper EGE
 *
 * by Ltabsyy
 * 
 * https://github.com/Ltabsyy/MineSweeper
 **/
#define LimHeight 256//最大高度，限制行数
#define LimWidth 384//最大宽度，限制列数

// EGE窗口形态
int sideLength = 32;//外部窗口方块边长
#define heightOfBlock sideLength
#define widthOfBlock sideLength//锁定纵横比
#define heightOfBar 2*sideLength
#define widthOfBorder sideLength/4
#define xOfChar widthOfBlock*10/32
#define yOfChar heightOfBlock/32
#define heightOfChar heightOfBlock

// 地图生成和显示
struct Information
{
	int remainedMine;
	int t0, t1, t2;//显示用时
	int unsolved3BV, total3BV;//地图3BV
	int showInformation;//0第一次翻开，1刷新信息
	int clock0, clock1;//毫秒用时
}game;
int IsPosInRectangle(int x, int y, int x1, int y1, int x2, int y2);
void SummonBoard(int seed, int r0, int c0, int summonMode, int iterateMode);//地图生成

// EGE窗口显示
int dx = 0, dy = 0;//地图偏移
int screenHeight, screenWidth;
int windowHeight, windowWidth, xFace;
int xRemainedMine, xRemainedMineNumber, xTime, xTimeNumber, x3BV, x3BVNumber, x3BVps, x3BVpsNumber;
int mouseR = -1, mouseC = -1;//悬浮高亮
void DrawMine(int r, int c);//绘制地图地雷
void DrawMineA(int x0, int y0, int r);//绘制地雷图形
void DrawFlag(int r, int c);//绘制地图旗帜
void DrawBlock(int r, int c, int board, int isShown, int highlight);//绘制方块
void DrawLineA(int x0, int y0, int r, int angle);//绘制时钟指针
void DrawClock(int x0, int y0, int r, int time);//绘制时钟
void DrawFace(int mode);//绘制笑脸
void DrawBoard(int mode);//绘制总外部窗口
void DrawSolution();//在外部窗口绘制方案矩阵
void DrawMouse(int x, int y);//绘制鼠标
void UpdateWindowSize();//根据当前方块边长更新窗口大小
void InitWindow(int mode);
void ResizeWindow(char mode);//调整显示大小
int IsMousePosOutside();//鼠标在窗口边界外
void GetWindowOperation(char* operation, int* r, int* c);
int CloseWindow(int isWinning);

// 后台计算
int IsAroundZeroChain(int r0, int c0);
void OpenZeroChain(int r0, int c0);
int NumberOfNotShown();
int NumberOfSignAround(int r0, int c0);

// 全局矩阵
int isMine[LimHeight][LimWidth]={0};
int numberOfMineAround[LimHeight][LimWidth]={0};
int board[LimHeight][LimWidth]={0};
int isShown[LimHeight][LimWidth]={0};
int zeroChain[LimHeight][LimWidth]={0};
int solution[LimHeight][LimWidth]={0};

// 全局变量及初始化
int numberOfMine = 10;//雷数量
int heightOfBoard = 10;//界面高度
int widthOfBoard = 10;//界面宽度
int summonCheckMode = 2;//0不校验，1非雷，2必空，3可解，4筛选
int showTime = 1;
int show3BV = 0;
int refreshCycle = 50;
int newCursor = 2;

int main()
{
	int choiceMode = 0;//游戏功能的选择
	int seed, r0, c0;//地图生成
	int r, c, isOpenMine, ra, ca;
	char operation;
	int temp, difficulty;//设置
	while(1)//main内循环防止变量重复定义
	{
		system("cls");//清屏
		printf("*******************************\n"//宽31
			   "(1)新游戏\n"
			   "(2)设置\n"
			   "(3)退出\n"
			   "*******************************\n");
		if(choiceMode == 0)
		{
			printf(">");
			scanf("%d", &choiceMode);
		}
		/*--新游戏--*/
		if(choiceMode == 1)
		{
			/*重置*/
			system("cls");
			game.remainedMine = numberOfMine;
			game.t0 = 0;
			game.t1 = 0;
			game.showInformation = 0;//不刷新游戏信息
			isOpenMine = 0;
			for(r=0; r<heightOfBoard; r++)
			{
				for(c=0; c<widthOfBoard; c++)
				{
					isShown[r][c] = 0;
				}
			}
			/*初始化*/
			printf("选择坐标[r:行][c:列]\n");
			InitWindow(0);//创建窗口
			operation = 0;
			r0 = heightOfBoard/2;
			c0 = widthOfBoard/2;
			while(operation == 0)
			{
				DrawBoard(0);
				GetWindowOperation(&operation, &r0, &c0);
				delay_ms(refreshCycle);
			}
			seed = time(0);//当前时间戳作种子生成随机数
			game.t0 = time(0);
			game.t1 = game.t0;
			game.t2 = 0;
			game.showInformation = 1;
			SummonBoard(seed, r0, c0, summonCheckMode, 1);
			for(r=0; r<heightOfBoard; r++)
			{
				for(c=0; c<widthOfBoard; c++)
				{
					solution[r][c] = 0;
				}
			}
			r = r0;
			c = c0;
			isShown[r][c] = 1;
			/*游戏循环*/
			while(1)
			{
				/*计算*/
				if(isShown[r][c] == 1)//对翻开操作做出反应
				{
					if(board[r][c] == 0)//翻开0连锁翻开
					{
						OpenZeroChain(r,c);
					}
					else if(board[r][c] == 9)//寄
					{
						system("cls");
						printf(":(\nGame Over!\n");
						isOpenMine = 1;
						break;
					}
				}
				if(NumberOfNotShown() == game.remainedMine)//未翻开的都是雷则胜利
				{
					system("cls");
					printf(":)\nYou Win!\n");
					break;
				}
				/*显示*/
				//system("cls");
				//printf("剩余雷数: %d 用时: %d\n", game.remainedMine, t1-t0);//打印剩余雷数
				/*输入*/
				operation = 0;
				while(1)
				{
					if(1)
					{
						game.t1 = time(0);
						DrawBoard(0);
						DrawSolution();
						GetWindowOperation(&operation, &r, &c);
						if(operation == '%')//重新生成地图
						{
							game.remainedMine = numberOfMine;
							game.t0 = 0;
							game.t1 = 0;
							game.t2 = 0;
							game.showInformation = 0;
							for(r=0; r<heightOfBoard; r++)
							{
								for(c=0; c<widthOfBoard; c++)
								{
									isShown[r][c] = 0;
									solution[r][c] = 0;
								}
							}
							operation = 0;
							r0 = heightOfBoard/2;
							c0 = widthOfBoard/2;
							while(operation == 0)
							{
								DrawBoard(0);
								GetWindowOperation(&operation, &r0, &c0);
								delay_ms(refreshCycle);
							}
							seed = time(0);
							/*if(summonCheckMode > 2)//可解地图生成
							{
								seed = SearchSeed(seed, r0, c0, difficulty);
								for(r=0; r<heightOfBoard; r++)
								{
									for(c=0; c<widthOfBoard; c++)
									{
										solution[r][c] = 0;
									}
								}
							}*/
							SummonBoard(seed, r0, c0, summonCheckMode, 1);
							//game.total3BV = BBBV(seed, r0, c0, 3);
							//game.unsolved3BV = game.total3BV;
							game.showInformation = 1;
							r = r0;
							c = c0;
							isShown[r][c] = 1;
							//isHelped = 0;
							//ClearOperations(operationRecord);
							//operationRecord = AddOperations(seed, r0, c0);
							game.t0 = time(0);
						}
					}
					if(operation != 0)
					{
						delay_ms(0);//连续操作显示
						break;
					}
					else delay_ms(refreshCycle);
					//delay_fps(1000/refreshCycle);//维持帧率稳定
					//if(operation != 0) break;
				}
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
						game.remainedMine++;//取消标记，剩余雷数+1
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
						game.remainedMine--;
					}
					else if(isShown[r][c] == 2)//取消标记
					{
						isShown[r][c] = 0;
						game.remainedMine++;
					}
					/*else
					{
						printf(":(\n该坐标已翻开！\n");
					}*/
				}
				/*else
				{
					printf(":(\n未选择操作模式！\n");
				}*/
			}
			/*游戏结束*/
			choiceMode = CloseWindow(1-isOpenMine);
		}
		/*--设置--*/
		else if(choiceMode == 2)
		{
			system("cls");
			printf("*******************************\n"//宽31
				   "(0)默认：10*10 - 10\n"
				   "(1)初级： 9*9  - 10\n"
				   "(2)中级：16*16 - 40\n"
				   "(3)高级：16*30 - 99\n"
				   "(4)顶级：42*88 - 715\n"
				   "(5)自定义**********\n"
				   "*******************************\n");
			printf("当前雷数:%d|当前界面大小:%d*%d\n", numberOfMine, heightOfBoard, widthOfBoard);
			printf(">");
			scanf("%d", &difficulty);
			if(difficulty == 0)//默认为10*10-10，比初级更简单(doge)
			{
				numberOfMine = 10;//密度0.1
				heightOfBoard = 10;
				widthOfBoard = 10;
			}
			else if(difficulty == 1)//初级，胜率96%
			{
				numberOfMine = 10;//密度0.12345679
				heightOfBoard = 9;
				widthOfBoard = 9;
			}
			else if(difficulty == 2)//中级，胜率84%
			{
				numberOfMine = 40;//密度0.15625
				heightOfBoard = 16;
				widthOfBoard = 16;
			}
			else if(difficulty == 3)//高级，胜率48%
			{
				numberOfMine = 99;//密度0.20625
				heightOfBoard = 16;
				widthOfBoard = 30;
			}
			else if(difficulty == 4)//顶级，胜率26%
			{
				numberOfMine = 715;//密度0.193452
				heightOfBoard = 42;
				widthOfBoard = 88;//实测较为合适的全屏地图，雷数为胡桃生日
			}
			else
			{
				printf("[行数] [列数] [雷数]>");
				scanf("%d", &temp);
				if(temp < 1) heightOfBoard = 1;
				else if(temp > LimHeight) heightOfBoard = LimHeight;
				else heightOfBoard = temp;
				scanf("%d", &temp);
				if(temp < 1) widthOfBoard = 1;
				else if(temp > LimWidth) widthOfBoard = LimWidth;
				else widthOfBoard = temp;
				scanf("%d", &temp);
				if(temp < 0) numberOfMine = 0;
				else if(temp > heightOfBoard * widthOfBoard) numberOfMine = heightOfBoard * widthOfBoard;
				else numberOfMine = temp;
			}
			choiceMode = 0;
		}
		/*--退出--*/
		else if(choiceMode == 3)
		{
			break;
		}
		else
		{
			getchar();
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

void SummonBoard(int seed, int r0, int c0, int summonMode, int iterateMode)//地图生成，生成雷，数字和后台总板
{
	//生成模式：0不校验，1起始点必非雷，2起始点必为空
	//迭代模式：1初始迭代，0顺延迭代，2-2147483647定位迭代，-1无迭代，-2旧版迭代
	int r, c, i, ra, ca, it;
	int numberOfNotMine = heightOfBoard*widthOfBoard-numberOfMine;
	int ra1 = r0, ca1 = c0, ra2 = r0, ca2 = c0;
	if(ra1 > 0) ra1--;
	if(ca1 > 0) ca1--;
	if(ra2+1 < heightOfBoard) ra2++;
	if(ca2+1 < widthOfBoard) ca2++;
	// 重置
	if(iterateMode != 0) srand(seed);
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			numberOfMineAround[r][c] = 0;
			//board[r][c] = 0;
			isShown[r][c] = 0;//清零显示方式矩阵
		}
	}
	// 迭代生成雷场
	if(iterateMode >= 0)//使用第二代雷场生成算法
	{
		if(iterateMode == 0) iterateMode = 1;//迭代模式转为迭代次数
		for(it = 0; it < iterateMode; it++)
		{
			if(numberOfMine <= numberOfNotMine)//使用布雷策略
			{
				//初始化
				for(r=0; r<heightOfBoard; r++)
				{
					for(c=0; c<widthOfBoard; c++)
					{
						isMine[r][c] = 0;//雷，0无1有
					}
				}
				//布雷
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
				//布置起始点
				if(summonMode > 0 && numberOfNotMine > 0)//在不可能确保时自动放弃
				{//尽管过半密度一般使用布空策略，此处仍有自动放弃特性以作保障
					//确保起始点非雷
					if(isMine[r0][c0] == 1)
					{
						isMine[r0][c0] = 0;
						while(1)
						{
							r = rand() % heightOfBoard;
							c = rand() % widthOfBoard;
							if(isMine[r][c] == 0 && r != r0 && c != c0)
							{
								isMine[r][c] = 1;
								break;
							}
						}
					}
					//确保起始点为空，在不可能确保起始点为空时仅确保起始点非雷
					if(summonMode > 1 && numberOfNotMine >= (ra2-ra1+1)*(ca2-ca1+1))
					{
						for(ra=ra1; ra<=ra2; ra++)
						{
							for(ca=ca1; ca<=ca2; ca++)
							{
								if(isMine[ra][ca] == 1)
								{
									isMine[ra][ca] = 0;
									while(1)
									{
										r = rand() % heightOfBoard;
										c = rand() % widthOfBoard;
										if(r>=ra1 && r<=ra2 && c>=ca1 && c<=ca2);
										else if(isMine[r][c] == 0)
										{
											isMine[r][c] = 1;
											break;
										}
									}
								}
							}
						}
					}
				}
			}
			else//使用布空策略，避免高密度布雷后期随机数命中率过低
			{
				//初始化
				for(r=0; r<heightOfBoard; r++)
				{
					for(c=0; c<widthOfBoard; c++)
					{
						isMine[r][c] = 1;//默认为雷
					}
				}
				//布置起始点，不共用具有随机命中特性的布雷策略代码
				i = 0;
				if(summonMode > 0 && numberOfNotMine > 0)
				{
					//确保起始点非雷
					isMine[r0][c0] = 0;
					i++;
					//确保起始点为空
					if(summonMode > 1 && numberOfNotMine >= (ra2-ra1+1)*(ca2-ca1+1))
					{
						for(ra=ra1; ra<=ra2; ra++)
						{
							for(ca=ca1; ca<=ca2; ca++)
							{
								if(isMine[ra][ca] == 1)
								{
									isMine[ra][ca] = 0;
									i++;
								}
							}
						}
					}
				}
				//布空
				while(i < numberOfNotMine)
				{
					r = rand() % heightOfBoard;
					c = rand() % widthOfBoard;
					if(isMine[r][c] == 1)
					{
						isMine[r][c] = 0;
						i++;
					}
				}
			}
		}
	}
	else if(iterateMode == -2)//旧版迭代，使用第一代雷场生成算法，无迭代被跳过
	{
		while(1)
		{
			//初始化
			for(r=0; r<heightOfBoard; r++)
			{
				for(c=0; c<widthOfBoard; c++)
				{
					isMine[r][c] = 0;//雷，0无1有
				}
			}
			//布雷
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
			//校验
			if(isMine[r0][c0] == 1 && summonMode > 0) continue;//第1次就爆则循环
			if(summonMode > 1)
			{
				if((r0 > 0 && c0 > 0 && isMine[r0-1][c0-1] == 1)
					|| (r0 > 0 && isMine[r0-1][c0] == 1)
					|| (r0 > 0 && c0+1 < widthOfBoard && isMine[r0-1][c0+1] == 1)
					|| (c0 > 0 && isMine[r0][c0-1] == 1)
					|| (c0+1 < widthOfBoard && isMine[r0][c0+1] == 1)
					|| (r0+1 < heightOfBoard && c0 > 0 && isMine[r0+1][c0-1] == 1)
					|| (r0+1 < heightOfBoard && isMine[r0+1][c0] == 1)
					|| (r0+1 < heightOfBoard && c0+1 < widthOfBoard && isMine[r0+1][c0+1] == 1))
				{
					continue;//预判到第1次翻开位置不为0则循环
				}
			}
			//完毕
			break;
		}
	}
	// 生成雷周围数字
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
			//方法2：循环遍历方块，计算周围雷数，因为不跳过雷，方法1始终不弱于方法2
		}
	}
	// 生成后台总板
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
}

void DrawMine(int r, int c)//绘制地图地雷
{
	float x = c*widthOfBlock+widthOfBorder+dx;
	float y = r*heightOfBlock+heightOfBar+widthOfBorder+dy;
	//setcolor(LIGHTRED);
	//xyprintf(x+xOfChar, y+yOfChar, "@");
	setfillcolor(BLACK);
	ege_fillellipse(x+7.4/32*widthOfBlock, y+7.4/32*heightOfBlock, 0.6*widthOfBlock, 0.6*heightOfBlock);
	setcolor(BLACK);
	setlinewidth(sideLength/16);
	ege_line(x+5.0/32*widthOfBlock, y+17.0/32*heightOfBlock, x+29.0/32*widthOfBlock, y+17.0/32*heightOfBlock);
	ege_line(x+17.0/32*widthOfBlock, y+5.0/32*heightOfBlock, x+17.0/32*widthOfBlock, y+29.0/32*heightOfBlock);
	ege_line(x+9.0/32*widthOfBlock, y+9.0/32*heightOfBlock, x+25.0/32*widthOfBlock, y+25.0/32*heightOfBlock);
	ege_line(x+9.0/32*widthOfBlock, y+25.0/32*heightOfBlock, x+25.0/32*widthOfBlock, y+9.0/32*heightOfBlock);
	setfillcolor(WHITE);
	ege_fillrect(x+12.0/32*widthOfBlock, y+12.0/32*heightOfBlock, 4.0/32*widthOfBlock, 4.0/32*heightOfBlock);
}

void DrawMineA(int x0, int y0, int r)//绘制地雷图形
{
	setfillcolor(BLACK);
	//r=16
	ege_fillellipse(x0-0.6*r, y0-0.6*r, 0.6*2*r, 0.6*2*r);
	setcolor(BLACK);
	setlinewidth(r/8);
	ege_line(x0-r+r/4, y0, x0+r-r/4, y0);
	ege_line(x0, y0-r+r/4, x0, y0+r-r/4);
	ege_line(x0-r/2, y0-r/2, x0+r/2, y0+r/2);
	ege_line(x0-r/2, y0+r/2, x0+r/2, y0-r/2);
	setfillcolor(WHITE);
	ege_fillrect(x0-r+11.0/16*r, y0-r+11.0/16*r, 4.0/16*r, 4.0/16*r);
}

void DrawFlag(int r, int c)//绘制地图旗帜
{
	float x = c*widthOfBlock+widthOfBorder+dx;
	float y = r*heightOfBlock+heightOfBar+widthOfBorder+dy;
	//setcolor(LIGHTRED);
	//xyprintf(x+xOfChar, y+yOfChar, "#");
	setfillcolor(BLACK);
	//绘制底座
	ege_fillrect(x+8.0/32*widthOfBlock, y+24.0/32*heightOfBlock, 16.0/32*widthOfBlock, 2.0/32*heightOfBlock);
	ege_fillrect(x+10.0/32*widthOfBlock, y+22.0/32*heightOfBlock, 12.0/32*widthOfBlock, 2.0/32*heightOfBlock);
	//绘制旗杆
	ege_fillrect(x+15.0/32*widthOfBlock, y+16.0/32*heightOfBlock, 2.0/32*widthOfBlock, 8.0/32*heightOfBlock);
	//绘制旗帜
	setfillcolor(RED);
	ege_point polyPoints[3] =
	{
		{x+6.0f/32*widthOfBlock, y+11.0f/32*heightOfBlock},
		{x+17.0f/32*widthOfBlock, y+6.0f/32*heightOfBlock},
		{x+17.0f/32*widthOfBlock, y+16.0f/32*heightOfBlock}
	};
	ege_fillpoly(3, polyPoints);
}

void DrawBlock(int r, int c, int board, int isShown, int highlight)//绘制方块
{
	float x = c*widthOfBlock+widthOfBorder+dx;
	float y = r*heightOfBlock+heightOfBar+widthOfBorder+dy;
	//绘制边框和底纹
	if(isShown == 1 || (board == 9 && isShown == 0))
	{
		setfillcolor(GRAY);
		ege_fillrect(x, y, widthOfBlock, heightOfBlock);
		if(highlight == 1) setfillcolor(LIGHTBLUE);
		else setfillcolor(DARKGRAY);
		ege_fillrect(x+widthOfBlock*2/32, y+heightOfBlock*2/32, widthOfBlock*30/32, heightOfBlock*30/32);
		//setfontbkcolor(DARKGRAY);
	}
	else
	{
		ege_point polyPoints1[3] = {{x, y}, {x+widthOfBlock, y}, {x, y+heightOfBlock}};
		ege_point polyPoints2[3] = {{x+widthOfBlock, y}, {x, y+heightOfBlock}, {x+widthOfBlock, y+heightOfBlock}};
		setfillcolor(WHITE);
		ege_fillpoly(3, polyPoints1);
		setfillcolor(GRAY);
		ege_fillpoly(3, polyPoints2);
		if(highlight == 1) setfillcolor(LIGHTBLUE);
		else setfillcolor(LIGHTGRAY);
		//ege_fillrect(x+widthOfBlock*4/32, y+heightOfBlock*4/32, widthOfBlock*24/32, heightOfBlock*24/32);
		ege_fillrect(x+widthOfBlock*2.0/32, y+heightOfBlock*2.0/32, widthOfBlock*28/32, heightOfBlock*28/32);
		//setfontbkcolor(LIGHTGRAY);
	}
	//绘制文字或图形
	//xyprintf(x+12, y+8, "%d", board);
	//rectprintf(x, y, widthOfBlock, heightOfBlock, "%d", board);
	if(isShown == 2)
	{
		if(board != 9)//错误标记
		{
			setfillcolor(LIGHTRED);
			ege_fillrect(x+widthOfBlock*2.0/32, y+heightOfBlock*2.0/32, widthOfBlock*28/32, heightOfBlock*28/32);
		}
		DrawFlag(r, c);
	}
	else if(isShown == 0)
	{
		if(board == 9)
		{
			DrawMine(r, c);
		}
	}
	else
	{
		if(board == 9)
		{
			setfillcolor(RED);
			ege_fillrect(x+widthOfBlock*2/32, y+heightOfBlock*2/32, widthOfBlock*30/32, heightOfBlock*30/32);
			DrawMine(r, c);
		}
		else if(board == 0);
		else
		{
			//配色1
			//if(board == 1) setcolor(BLUE);
			//else if(board == 2) setcolor(GREEN);
			//else if(board < 6) setcolor(RED);//3-5
			//else setcolor(YELLOW);
			//配色2
			if(board == 1) setcolor(BLUE);
			else if(board == 2) setcolor(GREEN);
			else if(board == 3) setcolor(RED);
			else if(board == 4) setcolor(DARKBLUE);
			else if(board == 5) setcolor(BROWN);
			else if(board == 6) setcolor(DARKCYAN);
			else if(board == 7) setcolor(BLACK);
			else setcolor(GRAY);
			//数字
			//setbkmode(TRANSPARENT);
			xyprintf(x+xOfChar, y+yOfChar, "%d", board);
			//setbkmode(OPAQUE);
		}
	}
}

void DrawLineA(int x0, int y0, int r, int angle)//绘制时钟指针
{
	float rad;
	int x1, y1;
	rad = angle*PI/180;
	x1 = x0 + r*cos(rad);
	y1 = y0 + r*sin(rad);
	//line(x0, y0, x1, y1);
	ege_line(x0, y0, x1, y1);
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
	// 绘制时钟(r=20)
	setfillcolor(WHITE);
	ege_fillellipse(x0-r, y0-r, 2*r, 2*r);
	setlinewidth(r/10);
	setcolor(BLACK);
	//circle(x0, y0, r);
	ege_ellipse(x0-r, y0-r, 2*r, 2*r);
	//秒针
	setlinewidth(r/20);
	setcolor(RED);
	DrawLineA(x0, y0, r*4/5, 270+second*6);
	setcolor(BLACK);
	//分针
	setlinewidth(r/20);
	DrawLineA(x0, y0, r*3/4, 270+minute*6);
	//时针
	setlinewidth(r/10);
	DrawLineA(x0, y0, r/2, 270+hour%12*30+minute/12*6);//每12分钟跳一格
	//转轴
	setfillcolor(RED);
	ege_fillellipse(x0-r/10, y0-r/10, r/5, r/5);
	// 输出文字时间
	//xyprintf(x0+r+r/5, y0+r-r/5, "%2d:%2d:%2d", hour, minute, second);
}

void DrawFace(int mode)//绘制笑脸
{
	static int clickClock = 0;
	float h = heightOfBlock*3/2;
	float w = widthOfBlock*3/2;
	float x = xFace;//(widthOfBlock*widthOfBoard-w)/2+widthOfBorder
	float y = (heightOfBar-h)/2;
	//按未翻开方块1.5倍绘制边框和底纹
	ege_point polyPoints1[3] = {{x, y}, {x+w, y}, {x, y+h}};
	ege_point polyPoints2[3] = {{x+w, y}, {x, y+h}, {x+w, y+h}};
	setfillcolor(WHITE);
	ege_fillpoly(3, polyPoints1);
	setfillcolor(GRAY);
	ege_fillpoly(3, polyPoints2);
	setfillcolor(LIGHTGRAY);
	ege_fillrect(x+w*3/48, y+h*3/48, w*42/48, h*42/48);
	//setfontbkcolor(LIGHTGRAY);
	//绘制脸
	setfillcolor(YELLOW);
	ege_setpattern_ellipsegradient({x+w*16/48, y+h*18/48}, WHITE, x+w*6/48, y+h*6/48, w*36/48, h*36/48, GOLD);
	ege_fillellipse(x+w*6/48, y+h*6/48, w*36/48, h*36/48);
	setlinewidth(sideLength/16);
	setcolor(BLACK);
	ege_ellipse(x+w*6/48, y+h*6/48, w*36/48, h*36/48);//脸框
	setfillcolor(BLACK);
	//按下形态维持
	if(mode == 1) clickClock = clock();
	else if(clock() - clickClock < 200) mode = 1;
	if(mode == 0)//正常
	{
		ege_fillellipse(x+w*15/48, y+h*18/48, w*6/48, h*6/48);//左眼
		ege_fillellipse(x+w*27/48, y+h*18/48, w*6/48, h*6/48);//右眼
		ege_arc(x+w*19/48, y+h*24/48, w*9/48, h*9/48, 0, 180);//嘴
	}
	else if(mode == 1)//按下
	{
		ege_fillellipse(x+w*15/48, y+h*18/48, w*6/48, h*6/48);//左眼
		ege_fillellipse(x+w*27/48, y+h*18/48, w*6/48, h*6/48);//右眼
		ege_ellipse(x+w*19/48, y+h*27/48, w*9/48, h*9/48);//嘴
	}
	else if(mode == 2)//失败
	{
		ege_line(x+w*15/48, y+h*18/48, x+w*21/48, y+h*24/48);
		ege_line(x+w*15/48, y+h*24/48, x+w*21/48, y+h*18/48);
		//ege_fillellipse(x+w*15/48, y+h*18/48, w*6/48, h*6/48);//左眼
		ege_line(x+w*27/48, y+h*18/48, x+w*33/48, y+h*24/48);
		ege_line(x+w*27/48, y+h*24/48, x+w*33/48, y+h*18/48);
		//ege_fillellipse(x+w*27/48, y+h*18/48, w*6/48, h*6/48);//右眼
		ege_arc(x+w*19/48, y+h*28/48, w*9/48, h*9/48, 180, 180);//嘴
	}
	else if(mode == 3)//成功
	{
		ege_arc(x+w*19/48, y+h*24/48, w*9/48, h*9/48, 0, 180);//嘴
		ege_fillpie(x+w*12/48, y+h*12/48, w*12/48, h*12/48, 0, 180);//左眼镜
		ege_fillpie(x+w*24/48, y+h*12/48, w*12/48, h*12/48, 0, 180);//右眼镜
		ege_line(x+w*6/48, y+h*24/48, x+w*12/48, y+h*18/48);
		ege_line(x+w*42/48, y+h*24/48, x+w*36/48, y+h*18/48);//镜架
	}
}

void DrawBoard(int mode)//绘制总外部窗口
{
	int r, c;
	int rc1, cc1, rc2, cc2;
	int highlight;
	setfillcolor(LIGHTGRAY);
	ege_fillrect(0, 0, windowWidth, heightOfBar);//清除旧顶栏减少锯齿感
	ege_point polyPoints1[6] =
	{
		{(float)0+dx, (float)heightOfBar+dy},
		{(float)widthOfBlock*widthOfBoard+widthOfBorder*2+dx, (float)heightOfBar+dy},
		{(float)widthOfBlock*widthOfBoard+widthOfBorder+dx, (float)heightOfBar+widthOfBorder+dy},
		{(float)widthOfBorder+dx, (float)heightOfBar+widthOfBorder+dy},
		{(float)widthOfBorder+dx, (float)heightOfBar+heightOfBlock*heightOfBoard+widthOfBorder+dy},
		{(float)0+dx, (float)heightOfBar+heightOfBlock*heightOfBoard+widthOfBorder*2+dy}
	};
	ege_point polyPoints2[6] =
	{
		{(float)widthOfBlock*widthOfBoard+widthOfBorder*2+dx, (float)heightOfBar+dy},
		{(float)widthOfBlock*widthOfBoard+widthOfBorder+dx, (float)heightOfBar+widthOfBorder+dy},
		{(float)widthOfBlock*widthOfBoard+widthOfBorder+dx, (float)heightOfBar+heightOfBlock*heightOfBoard+widthOfBorder+dy},
		{(float)widthOfBorder+dx, (float)heightOfBar+heightOfBlock*heightOfBoard+widthOfBorder+dy},
		{(float)0+dx, (float)heightOfBar+heightOfBlock*heightOfBoard+widthOfBorder*2+dy},
		{(float)widthOfBlock*widthOfBoard+widthOfBorder*2+dx, (float)heightOfBar+heightOfBlock*heightOfBoard+widthOfBorder*2+dy}
	};
	setfillcolor(GRAY);
	ege_fillpoly(6, polyPoints1);
	setfillcolor(WHITE);
	ege_fillpoly(6, polyPoints2);
	//裁剪优化
	//for(rc1=0; (rc1+1)*heightOfBlock+heightOfBar+widthOfBorder+dy < 0; rc1++);
	//for(cc1=0; (cc1+1)*widthOfBlock+widthOfBorder+dx < 0; cc1++);
	//for(rc2=heightOfBoard-1; (rc2-1)*heightOfBlock+heightOfBar+widthOfBorder+dy > screenHeight; rc2--);
	//for(cc2=widthOfBoard-1; (cc2-1)*widthOfBlock+widthOfBorder+dx > screenWidth; cc2--);
	//for(x=0; k*x+b < 0; x++); -> if(b >= 0) x = 0; else x = (-b+k-1)/k;
	if(heightOfBlock+heightOfBar+widthOfBorder+dy >= 0) rc1 = 0;
	else rc1 = (-(heightOfBlock+heightOfBar+widthOfBorder+dy)+heightOfBlock-1)/heightOfBlock;
	if(widthOfBlock+widthOfBorder+dx >= 0) cc1 = 0;
	else cc1 = (-(widthOfBlock+widthOfBorder+dx)+widthOfBlock-1)/widthOfBlock;
	//for(x=M; k*x+b > 0; x--); -> if(k*M+b <= 0) x = M; else if(b <= 0) x = -b/k; else x = (-b-k+1)/k;
	if((heightOfBoard-2)*heightOfBlock+heightOfBar+widthOfBorder+dy <= screenHeight) rc2 = heightOfBoard-1;
	else if(-heightOfBlock+heightOfBar+widthOfBorder+dy <= screenHeight)
	{
		rc2 = -(-heightOfBlock+heightOfBar+widthOfBorder+dy-screenHeight)/heightOfBlock;
	}
	else rc2 = (-(-heightOfBlock+heightOfBar+widthOfBorder+dy-screenHeight)-heightOfBlock+1)/heightOfBlock;
	if((widthOfBoard-2)*widthOfBlock+widthOfBorder+dx <= screenWidth) cc2 = widthOfBoard-1;
	else if(-widthOfBlock+widthOfBorder+dx <= screenWidth)
	{
		cc2 = -(-widthOfBlock+widthOfBorder+dx-screenWidth)/widthOfBlock;
	}
	else cc2 = (-(-widthOfBlock+widthOfBorder+dx-screenWidth)-widthOfBlock+1)/widthOfBlock;
	for(r=rc1; r<=rc2; r++)
	{
		for(c=cc1; c<=cc2; c++)
		{
			highlight = 0;
			if(newCursor > 1)//淡黄色高亮光标
			{
				if(r == mouseR && c == mouseC) highlight = 1;
				if(newCursor == 3)
				{
					if(r == mouseR || c == mouseC) highlight = 1;
				}
			}
			if(mode == 1)//后台
			{
				if(isShown[r][c] == 2)
				{
					DrawBlock(r, c, board[r][c], 2, highlight);
				}
				else if(board[r][c] == 0)
				{
					DrawBlock(r, c, 0, 1, highlight);
				}
				else if(board[r][c] == 9)
				{
					DrawBlock(r, c, 9, isShown[r][c], highlight);
				}
				else
				{
					DrawBlock(r, c, board[r][c], 1, highlight);
				}
			}
			else if(mode == 0)//前台
			{
				if(isShown[r][c] == 2)
				{
					DrawBlock(r, c, 9, 2, highlight);
				}
				else if(isShown[r][c] == 0)
				{
					DrawBlock(r, c, 0, 0, highlight);
				}
				else
				{
					DrawBlock(r, c, board[r][c], 1, highlight);
				}
			}
		}
	}
	setbkmode(OPAQUE);
	//剩余雷数
	DrawMineA(xRemainedMine, heightOfBar/2, 20*heightOfBar/64*4/3);
	setcolor(RED);
	setfontbkcolor(BLACK);
	xyprintf(xRemainedMineNumber, (heightOfBar-heightOfChar)/2, " %d ", game.remainedMine);
	//用时
	if(showTime == 1)
	{
		DrawClock(xTime, heightOfBar/2, 20*heightOfBar/64, time(0));//按真实时间走的钟(doge)
		setcolor(RED);
		setfontbkcolor(BLACK);
		xyprintf(xTimeNumber, (heightOfBar-heightOfChar)/2, " %d ", game.t1-game.t0+game.t2);
	}
	//3BV
	if(show3BV == 1 && game.showInformation == 1 && widthOfBoard > 27)
	{
		setbkmode(TRANSPARENT);
		setcolor(BLACK);
		//setfontbkcolor(LIGHTGRAY);
		xyprintf(x3BV, (heightOfBar-heightOfChar)/2, "3BV");
		xyprintf(x3BVps, (heightOfBar-heightOfChar)/2, "3BV/s");
		setbkmode(OPAQUE);
		setcolor(RED);
		setfontbkcolor(BLACK);
		xyprintf(x3BVNumber, (heightOfBar-heightOfChar)/2, " %d/%d ", game.total3BV-game.unsolved3BV, game.total3BV);
		xyprintf(x3BVpsNumber, (heightOfBar-heightOfChar)/2, " %.2f ", (float)(game.total3BV-game.unsolved3BV)/(game.t1-game.t0+game.t2));
	}
	setbkmode(TRANSPARENT);
	DrawFace(0);
}

void DrawSolution()
{
	int r, c, x, y;
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(solution[r][c] != 0 && isShown[r][c] == 0)
			{
				x = c*widthOfBlock+widthOfBorder+dx;
				y = r*heightOfBlock+heightOfBar+widthOfBorder+dy;
				//填充式
				//if(solution[r][c] == 1) setfillcolor(LIGHTGREEN);
				//else if(solution[r][c] == 2) setfillcolor(PINK);
				//else if(solution[r][c] == 3) setfillcolor(LIGHTYELLOW);
				//ege_fillrect(x+widthOfBlock*2/32, y+heightOfBlock*2/32, widthOfBlock*28/32, heightOfBlock*28/32);
				//线框式
				if(solution[r][c] == 1) setcolor(DEEPSKYBLUE);
				else if(solution[r][c] == 2) setcolor(ORANGERED);
				else if(solution[r][c] == 3) setcolor(DEEPSKYBLUE);
				setlinewidth(sideLength/8-sideLength/8%2);//防止奇数线宽边缘模糊
				ege_rectangle(x+widthOfBlock*6/32, y+heightOfBlock*6/32, widthOfBlock*20/32, heightOfBlock*20/32);
				if(solution[r][c] == 3)
				{
					setcolor(PURPLE);
					xyprintf(x+xOfChar, y+yOfChar, "?");
				}
			}
		}
	}
}

void DrawMouse(int x, int y)//绘制鼠标
{
	int x0 = widthOfBorder+dx;
	int y0 = heightOfBar+widthOfBorder+dy;
	float k = 0.8*sideLength/32;
	//鼠标位置指示
	/*setcolor(BLACK);
	setlinewidth(2);
	ege_line(x0+x-sideLength/4, y0+y, x0+x+sideLength/4, y0+y);
	ege_line(x0+x, y0+y-sideLength/4, x0+x, y0+y+sideLength/4);
	setlinewidth(sideLength/16);
	ege_line(x0+x+sideLength/4, y0+y+sideLength/4, x0+x+sideLength*5/8, y0+y+sideLength*5/8);
	ege_line(x0+x+sideLength/4, y0+y+sideLength/4, x0+x+sideLength/2, y0+y+sideLength*3/8);
	ege_line(x0+x+sideLength/4, y0+y+sideLength/4, x0+x+sideLength*3/8, y0+y+sideLength/2);*/
	//鼠标指针
	ege_point polyPoints[8] = {
		{x0+x+0*k, y0+y+0*k}, {x0+x+0*k, y0+y+27*k}, {x0+x+6*k, y0+y+22*k}, {x0+x+11*k, y0+y+31*k},
		{x0+x+16*k, y0+y+28*k}, {x0+x+11*k, y0+y+20*k}, {x0+x+20*k, y0+y+20*k}, {x0+x+0*k, y0+y+0*k}
	};
	setfillcolor(DEEPSKYBLUE);
	ege_fillpoly(8, polyPoints);
	setcolor(BLACK);
	setlinewidth(2*k);
	ege_drawpoly(8, polyPoints);
}

void UpdateWindowSize()//根据当前方块边长更新窗口大小
{
	//限制不超过屏幕大小1.5*1
	if(widthOfBoard < 10)
	{
		if(widthOfBlock*10+widthOfBorder*2 > screenWidth*3/2) windowWidth = screenWidth*3/2;
		else windowWidth = widthOfBlock*10+widthOfBorder*2;
	}
	else
	{
		if(widthOfBlock*widthOfBoard+widthOfBorder*2 > screenWidth*3/2) windowWidth = screenWidth*3/2;
		else windowWidth = widthOfBlock*widthOfBoard+widthOfBorder*2;
	}
	if(heightOfBar+heightOfBlock*heightOfBoard+widthOfBorder*2 > screenHeight) windowHeight = screenHeight;
	else windowHeight = heightOfBar+heightOfBlock*heightOfBoard+widthOfBorder*2;
	//计算笑脸横坐标
	if(widthOfBoard > 53 && widthOfBlock*widthOfBoard/2+widthOfBorder+widthOfBlock*3/4 > screenWidth)//笑脸右界超出屏幕
	{
		xFace = 28*widthOfBlock;
	}
	else if(widthOfBoard < 10)
	{
		xFace = widthOfBlock*5+widthOfBorder-widthOfBlock*3/4;
	}
	else
	{
		xFace = widthOfBlock*widthOfBoard/2+widthOfBorder-widthOfBlock*3/4;
	}
	//剩余雷数
	if(widthOfBoard > 12)
	{
		xRemainedMine = 2*widthOfBlock;
		xRemainedMineNumber = 3*widthOfBlock+xOfChar;
	}
	else
	{
		xRemainedMine = 1*widthOfBlock;
		xRemainedMineNumber = 2*widthOfBlock;
	}
	//用时
	if(widthOfBoard > 23)
	{
		xTime = 7*widthOfBlock;
		xTimeNumber = 8*widthOfBlock+xOfChar;
	}
	else if(widthOfBoard > 12)
	{
		xTime = (widthOfBoard+6)*widthOfBlock/2;
		xTimeNumber = (widthOfBoard+8)*widthOfBlock/2+xOfChar;
	}
	else if(widthOfBoard > 10)
	{
		xTime = (widthOfBoard+4)*widthOfBlock/2;
		xTimeNumber = (widthOfBoard+6)*widthOfBlock/2+xOfChar;
	}
	else
	{
		xTime = 7*widthOfBlock;
		xTimeNumber = 8*widthOfBlock;
	}
	//3BV
	if(widthOfBoard > 53)
	{
		x3BV = 12*widthOfBlock+xOfChar;
		x3BVps = 20*widthOfBlock+xOfChar;
		x3BVNumber = 14*widthOfBlock+xOfChar;
		x3BVpsNumber = 23*widthOfBlock+xOfChar;
	}
	else if(widthOfBoard > 27)
	{
		x3BV = (widthOfBoard+3)*widthOfBlock/2+xOfChar;
		x3BVps = (widthOfBoard+17)*widthOfBlock/2+xOfChar;
		x3BVNumber = (widthOfBoard+7)*widthOfBlock/2+xOfChar;
		x3BVpsNumber = (widthOfBoard+22)*widthOfBlock/2+xOfChar;
	}
}

void InitWindow(int mode)//创建窗口
{
	static int fastInit = 0;
	if(mode == 1)//传入参数准备执行快速初始化
	{
		fastInit = 1;
		return;
	}
	if(fastInit == 0)
	{
		DEVMODE dm;
		dm.dmSize = sizeof(DEVMODE);
		if(EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm) == 0)//无法获取显示屏分辨率
		{
			screenWidth = 1920;
			screenHeight = 1080;
		}
		else
		{
			screenWidth = dm.dmPelsWidth;
			screenHeight = dm.dmPelsHeight;
		}
		if(screenHeight >= 2160) sideLength = 64;
		else if(screenHeight >= 1440) sideLength = 44;
		else if(screenHeight >= 1080) sideLength = 32;
		else sideLength = 24;
		while(widthOfBlock*widthOfBoard+widthOfBorder*2 > screenWidth
			|| heightOfBar+heightOfBlock*(heightOfBoard+4)+widthOfBorder*2 > screenHeight)
		{
			if(sideLength > 16) sideLength -= 4;
			else sideLength -= 1;
		}
		if(sideLength < 4) sideLength = 4;
		UpdateWindowSize();//保存窗口大小
		setcaption("MineSweeper Window");
		SetProcessDPIAware();//避免Windows缩放造成模糊
		initgraph(windowWidth, windowHeight, INIT_RENDERMANUAL);
		setbkcolor(LIGHTGRAY);
		setfont(heightOfChar, 0, "Consolas");
		setbkmode(TRANSPARENT);//默认设置为无背景字体
		ege_enable_aa(true);
	}
	fastInit = 0;
	//flushmouse();//避免上一局鼠标消息选择起始点
	if(widthOfBoard < 10) dx = widthOfBlock*(10-widthOfBoard)/2;
	else dx = 0;
	dy = 0;//偏移回正
	GetWindowOperation(NULL, NULL, NULL);
	//showmouse(0);//隐藏鼠标指针
}

void ResizeWindow(char mode)//调整显示大小
{
	int oldSideLength = sideLength;
	//调整方块边长
	if(mode == '+')//4-16时每格调整1，16-64时4，64+时16
	{
		if(sideLength >= 64) sideLength += 16;
		else if(sideLength >= 16) sideLength += 4;
		else sideLength += 1;
	}
	else if(mode == '-')
	{
		if(sideLength > 64) sideLength -= 16;
		else if(sideLength > 16) sideLength -= 4;
		else if(sideLength > 4) sideLength -= 1;
	}
	//调整窗口大小
	UpdateWindowSize();
	resizewindow(windowWidth, windowHeight);
	setfont(heightOfChar, 0, "Consolas");//更新字体大小
	//调整地图偏移
	dx = dx*sideLength/oldSideLength;//维持左上角不变
	dy = dy*sideLength/oldSideLength;
}

int IsMousePosOutside()//鼠标在窗口边界外
{
	//EGE无法区分鼠标静止和鼠标在窗口边界外，调用WindowsAPI
	HWND hwnd = getHWnd();//获取绘图窗口句柄
	RECT rect;
	POINT point;
	GetWindowRect(hwnd, &rect);//获取窗口四角坐标
	GetCursorPos(&point);//获取鼠标屏幕坐标
	return (point.x < rect.left || point.x > rect.right || point.y < rect.top || point.y > rect.bottom);
	//ScreenToClient(hwnd, &point);//转换为窗口坐标
	//窗口大小rect.right-rect.left+1, rect.bottom-rect.top+1
	//return (point.x <= 0 || point.x > rect.right-rect.left || point.y <= 0 || point.y > rect.bottom-rect.top);
}

void GetWindowOperation(char* operation, int* r, int* c)
{
	int xm, ym, xn, yn, xo, yo, clock0, clock1;
	static int isOpening, isSigning, ro, co;//拖动操作
	int isMouseInBoard = 0;
	mouse_msg mouseMsg;
	key_msg keyMsg;
	if(operation == NULL || r == NULL || c == NULL)//重置键鼠消息
	{
		isOpening = 0;
		isSigning = 0;
		flushmouse();
		flushkey();
		return;
	}
	if(IsMousePosOutside())
	{
		mouseR = -1;
		mouseC = -1;
	}
	while(mousemsg())//使用while代替if避免堆积消息产生延迟
	{
		mouseMsg = getmouse();
		//鼠标位置分析
		xm = mouseMsg.x;
		ym = mouseMsg.y;
		//if(game.showInformation == 1)
		//{
			//AddOperation(&operationRecord, clock()-game.clock0, 'm',
				//(ym-dy-heightOfBar-widthOfBorder)*64/sideLength, (xm-dx-widthOfBorder)*64/sideLength);//坐标拟合到64边长
		//}
		mouseR = (ym-dy-heightOfBar-widthOfBorder)/heightOfBlock;
		mouseC = (xm-dx-widthOfBorder)/widthOfBlock;
		if(IsPosInRectangle(xm-dx, ym-dy, widthOfBorder, heightOfBar+widthOfBorder,
			widthOfBlock*widthOfBoard+widthOfBorder-1, heightOfBar+heightOfBlock*heightOfBoard+widthOfBorder-1))
		{
			*r = mouseR;
			*c = mouseC;
			isMouseInBoard = 1;
		}
		else
		{
			mouseR = -1;
			mouseC = -1;
		}
		//鼠标操作处理
		if(mouseMsg.is_up())
		{
			if(mouseMsg.is_left()) isOpening = 0;//鼠标左键抬起
			if(mouseMsg.is_right()) isSigning = 0;//鼠标右键抬起
		}
		if(isOpening == 1 && keystate(key_control))
		{
			isOpening = 0;//Ctrl截止拖动翻开
		}
		if(mouseMsg.is_down())
		{
			if(mouseMsg.is_left())//鼠标左键按下
			{
				if(keystate(key_control))
				{
					clock0 = clock();
					while(1)
					{
						mouseMsg = getmouse();
						xn = mouseMsg.x;
						yn = mouseMsg.y;
						clock1 = clock();
						if(clock1 - clock0 > refreshCycle)//拖动预览
						{
							dx += xn-xm;
							dy += yn-ym;
							cleardevice();
							DrawBoard(0);
							dx -= xn-xm;
							dy -= yn-ym;
							clock0 = clock();
						}
						if(mouseMsg.is_left() && mouseMsg.is_up()) break;
						//delay_ms(refreshCycle);
					}
					dx += xn-xm;
					dy += yn-ym;
					if(dx > -widthOfBlock/8 && dx < widthOfBlock/8) dx = 0;//弱趋向回正
					if(dy > -heightOfBlock/8 && dy < heightOfBlock/8) dy = 0;
					cleardevice();//避免重影
					DrawBoard(0);//避免闪烁
				}
				else
				{
					if(IsPosInRectangle(xm, ym, xFace, heightOfBar/2-heightOfBlock*3/4,
						xFace+widthOfBlock*3/2, heightOfBar/2+heightOfBlock*3/4))
					{
						*operation = '%';
					}
					else if(isMouseInBoard == 1)
					{
						*operation = '@';
						DrawFace(1);
						isOpening = 1;
						ro = *r;
						co = *c;
						break;
					}
				}
			}
			if(mouseMsg.is_right())//鼠标右键按下
			{
				if(isMouseInBoard == 1)
				{
					*operation = '#';
					DrawFace(1);
					isSigning = 1;
					ro = *r;
					co = *c;
					break;
				}
			}
		}
		if(mouseMsg.is_move() && (isOpening == 1 || isSigning == 1))
		{
			//mousepos(&xm, &ym);//最新位置
			//xm = mouseMsg.x;
			//ym = mouseMsg.y;//缓冲位置
			if(isMouseInBoard == 1)
			{
				//DrawFace(1);
				if(ro != *r || co != *c)//移动到其他方块
				{
					xn = widthOfBorder+(*c)*widthOfBlock+widthOfBlock/2+dx-xm;
					yn = heightOfBar+widthOfBorder+(*r)*heightOfBlock+heightOfBlock/2+dy-ym;
					if(xn < 0) xn *= -1;
					if(yn < 0) yn *= -1;
					xo = widthOfBorder+co*widthOfBlock-widthOfBlock/4+dx;
					yo = heightOfBar+widthOfBorder+ro*heightOfBlock-heightOfBlock/4+dy;
					if(2*(heightOfBlock*xn+widthOfBlock*yn) <= heightOfBlock*widthOfBlock//必须移动到方块内菱形位置
						&& !IsPosInRectangle(xm, ym, xo, yo, xo+widthOfBlock*3/2, yo+heightOfBlock*3/2))//移动距离必须超过3/4个方块
					{
						if(isOpening == 1) *operation = '@';
						if(isSigning == 1) *operation = '#';
						DrawFace(1);
						ro = *r;
						co = *c;
						break;
					}
				}
			}
			else
			{
				isOpening = 0;
				isSigning = 0;
			}
		}
		if(mouseMsg.is_wheel() && keystate(key_control))
		{
			if(mouseMsg.wheel > 0) ResizeWindow('+');
			else ResizeWindow('-');
			cleardevice();//地图偏移时避免重影
			DrawBoard(0);//避免闪烁
		}
	}
	while(kbmsg())//鼠标专门处理，避免无尽滚动，随后处理纯键盘操作输入
	{
		keyMsg = getkey();
		if(keyMsg.msg == key_msg_down)
		{
			if(keyMsg.flags & key_flag_shift && keyMsg.key == '1')
			{
				*operation = '!';//实时求解指令
			}
			if(keyMsg.key == '\t')
			{
				*operation = '\t';
			}
		}
	}
}

int CloseWindow(int isWinning)
{
	int r, c, newGame = -1;
	int xm, ym, xn, yn, clock0, clock1;
	mouse_msg mouseMsg;
	if(isWinning == 1)//胜利后自动全部标记
	{
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				if(isMine[r][c] == 1 && isShown[r][c] == 0)
				{
					isShown[r][c] = 2;
				}
			}
		}
	}
	DrawBoard(1);
	if(isWinning == 1)
	{
		DrawFace(3);
		setcolor(BLACK);//显示阴影
		xyprintf(widthOfBlock+sideLength/16, heightOfBar+widthOfBorder+sideLength/16, "You Win!");
		setcolor(YELLOW);
		xyprintf(widthOfBlock, heightOfBar+widthOfBorder, "You Win!");
	}
	else
	{
		DrawFace(2);
		setcolor(BLACK);
		xyprintf(widthOfBlock+sideLength/16, heightOfBar+widthOfBorder+sideLength/16, "Game Over!");
		setcolor(RED);
		xyprintf(widthOfBlock, heightOfBar+widthOfBorder, "Game Over!");
	}
	delay_ms(1000);
	flushmouse();
	while(newGame == -1)
	{
		cleardevice();//清除旧游戏结束文字减少锯齿感
		DrawBoard(1);//刷新界面
		if(isWinning == 1)
		{
			DrawFace(3);
			setcolor(BLACK);
			xyprintf(widthOfBlock+sideLength/16, heightOfBar+widthOfBorder+sideLength/16, "You Win!");
			setcolor(YELLOW);
			xyprintf(widthOfBlock, heightOfBar+widthOfBorder, "You Win!");
		}
		else
		{
			DrawFace(2);
			setcolor(BLACK);
			xyprintf(widthOfBlock+sideLength/16, heightOfBar+widthOfBorder+sideLength/16, "Game Over!");
			setcolor(RED);
			xyprintf(widthOfBlock, heightOfBar+widthOfBorder, "Game Over!");
		}
		setfont(heightOfChar/2, 0, "黑体");
		setcolor(BLACK);
		//xyprintf(widthOfBlock+sideLength/32, heightOfBar+widthOfBorder+heightOfBlock+sideLength/32, "请按键盘任意键关闭窗口");
		xyprintf(widthOfBlock+sideLength/32, heightOfBar+widthOfBorder+heightOfBlock+sideLength/32, "左键新游戏，右键关闭窗口");
		setcolor(RED);
		//xyprintf(widthOfBlock, heightOfBar+widthOfBorder+heightOfBlock, "请按键盘任意键关闭窗口");
		xyprintf(widthOfBlock, heightOfBar+widthOfBorder+heightOfBlock, "左键新游戏，右键关闭窗口");
		//xyprintf(widthOfBlock, heightOfBar+heightOfBlock*3/2, "请勿按右上角退出！");
		//xyprintf(widthOfBlock, heightOfBar+heightOfBlock, "Press any key to continue . . .");
		setfont(heightOfChar, 0, "Consolas");
		//getch();
		//delay_ms(2000);
		//break;
		if(IsMousePosOutside())
		{
			mouseR = -1;
			mouseC = -1;
		}
		while(mousemsg())
		{
			mouseMsg = getmouse();
			xm = mouseMsg.x;
			ym = mouseMsg.y;
			if(IsPosInRectangle(xm-dx, ym-dy, widthOfBorder, heightOfBar+widthOfBorder,
				widthOfBlock*widthOfBoard+widthOfBorder-1, heightOfBar+heightOfBlock*heightOfBoard+widthOfBorder-1))
			{
				mouseR = (ym-dy-heightOfBar-widthOfBorder)/heightOfBlock;
				mouseC = (xm-dx-widthOfBorder)/widthOfBlock;
			}
			else
			{
				mouseR = -1;
				mouseC = -1;
			}
			if(mouseMsg.is_left() && mouseMsg.is_down())
			{
				if(keystate(key_control))
				{
					clock0 = clock();
					while(1)
					{
						mouseMsg = getmouse();
						xn = mouseMsg.x;
						yn = mouseMsg.y;
						clock1 = clock();
						if(clock1 - clock0 > refreshCycle)//拖动预览
						{
							dx += xn-xm;
							dy += yn-ym;
							cleardevice();
							DrawBoard(1);
							if(isWinning == 1) DrawFace(3);
							else DrawFace(2);
							dx -= xn-xm;
							dy -= yn-ym;
							clock0 = clock();
						}
						if(mouseMsg.is_left() && mouseMsg.is_up()) break;
					}
					dx += xn-xm;
					dy += yn-ym;
					if(dx > -widthOfBlock && dx < widthOfBlock) dx = 0;//趋向回正
					if(dy > -heightOfBlock && dy < heightOfBlock) dy = 0;
					cleardevice();
					DrawBoard(1);
					if(isWinning == 1) DrawFace(3);//保持笑脸
					else DrawFace(2);
				}
				else
				{
					newGame = 1;
					break;//仅处理点击，不处理移动
				}
			}
			if(mouseMsg.is_right() && mouseMsg.is_down())
			{
				newGame = 0;
				break;
			}
			if(mouseMsg.is_wheel() && keystate(key_control))
			{
				if(mouseMsg.wheel > 0) ResizeWindow('+');
				else ResizeWindow('-');
				cleardevice();
				DrawBoard(1);
				if(isWinning == 1) DrawFace(3);
				else DrawFace(2);
			}
		}
		delay_ms(refreshCycle);
	}
	cleardevice();
	if(newGame == 1) InitWindow(1);
	else closegraph();
	return newGame;
}

int IsAroundZeroChain(int r0, int c0)
{
	if(zeroChain[r0][c0] == 1
		|| (r0 > 0 && zeroChain[r0-1][c0] == 1)//四边的命中率高于四角，优先判断
		|| (c0 > 0 && zeroChain[r0][c0-1] == 1)
		|| (c0+1 < widthOfBoard && zeroChain[r0][c0+1] == 1)
		|| (r0+1 < heightOfBoard && zeroChain[r0+1][c0] == 1)
		|| (r0 > 0 && c0 > 0 && zeroChain[r0-1][c0-1] == 1)
		|| (r0 > 0 && c0+1 < widthOfBoard && zeroChain[r0-1][c0+1] == 1)
		|| (r0+1 < heightOfBoard && c0 > 0 && zeroChain[r0+1][c0-1] == 1)
		|| (r0+1 < heightOfBoard && c0+1 < widthOfBoard && zeroChain[r0+1][c0+1] == 1))
	{
		return 1;//以单向边界检查代替四向边界检查
	}
	return 0;
}

void OpenZeroChain(int r0, int c0)//翻开0连锁翻开
{
	int r, c, isRising;
	int rc1 = r0, cc1 = c0, rc2 = r0, cc2 = c0;//0链框架
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
		if(rc1 > 0) rc1--;//初始化0链框架
		if(cc1 > 0) cc1--;
		if(rc2+1 < heightOfBoard) rc2++;
		if(cc2+1 < widthOfBoard) cc2++;
		isRising = 1;
		while(isRising == 1)//0链向四边生长
		{
			isRising = 0;
			for(r=rc1; r<=rc2; r++)
			{
				for(c=cc1; c<=cc2; c++)
				{
					if(board[r][c] == 0 && zeroChain[r][c] == 0//可能的生长点
						&& IsAroundZeroChain(r, c) == 1)//与0链连接
					{
						zeroChain[r][c] = 1;
						isRising = 1;
					}
				}
			}
			if(isRising == 1)//调整0链框架
			{
				if(rc1 > 0) rc1--;
				if(cc1 > 0) cc1--;
				if(rc2+1 < heightOfBoard) rc2++;
				if(cc2+1 < widthOfBoard) cc2++;
			}
		}
		for(r=rc1; r<=rc2; r++)//周围有0链则翻开
		{
			for(c=cc1; c<=cc2; c++)
			{
				if(isShown[r][c] == 0 && IsAroundZeroChain(r, c) == 1)//在0链上或与0链连接
				{
					isShown[r][c] = 1;
				}
			}
		}
	}
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

/*--------------------------------
更新日志：
  # MineSweeper Lite
  仅保留扫雷核心算法
MineSweeper Lite 2
——新增 主页、设置、清屏、暂停
  # MineSweeper EGE
  修改为EGE版本
MineSweeper EGE 2
——新增 显示You Win!和Game Over!
——新增 胜利后自动标记全部雷
——新增 被翻开雷和错误标记红色显示
——优化 微调时钟边框
——优化 按任意键提示字由宋体改为一半大小黑体
——优化 方块边框更平滑细腻
——优化 大部分字体以透明背景输出
——优化 游戏结束字体增加阴影
——优化 使用XP版扫雷数字配色
MineSweeper EGE 3
——新增 根据屏幕分辨率调整显示大小
——优化 减少顶级地图按键延迟
MineSweeper EGE 4
——新增 地图边框
——修复 切换难度后闪退（使用新版EGE编译）
MineSweeper EGE 5
——新增 笑脸按钮
——新增 游戏时点击笑脸按钮重新生成地图
——新增 游戏结束时左键新游戏，右键关闭窗口
//——新增 实时求解指令
——优化 剩余雷数和用时在从1开始的所有界面宽度下适配
——优化 统一延时位置
——修复 上一局鼠标消息选择起始点
MineSweeper EGE 6
——新增 Ctrl+滚轮调整显示大小
——新增 Ctrl+左键拖动调整地图位置
——新增 拖动标记和翻开
——新增 悬浮高亮
——优化 微调时钟线条
——优化 游戏结束时时钟走动
//——优化 可以不显示用时
——修复 选择起始点时卡顿
MineSweeper EGE 7
//——新增 显示实时求解指令
//——新增 显示3BV和3BV/s
——优化 通过裁剪优化显示效率
——优化 线条抗锯齿
//——修复 按下时笑脸不张嘴
MineSweeper EGE 7.1
——优化 游戏结束文字抗锯齿
//——修复 顶级难度3BV显示空间不足
MineSweeper EGE 8
//——新增 绘制鼠标
——优化 操作后立即响应
——优化 笑脸张嘴形态维持时长
//——优化 实时求解指令改为线框式显示
——修复 重新生成地图时卡顿
MineSweeper EGE 8.1
——优化 适当调大1440P屏下默认显示大小
//——优化 避免游戏时选择控制台窗口
//——优化 与鼠标点击空地切换左右键、自制地图编辑的兼容性
——修复 边缘点击可能闪退
——修复 调整显示大小可能闪烁
MineSweeper EGE 8.2
——优化 操作识别效率
——优化 边框绘制效率
——修复 Ctrl+滚轮调整大小可能无尽滚动
//——修复 点击笑脸按钮重新生成地图后实时求解指令滞留
//——修复 点击笑脸按钮不能重新生成可解地图
//——修复 点击笑脸按钮重新生成地图后3BV计算错误
MineSweeper EGE 9
——优化 裁剪优化计算效率
——优化 限制窗口大小不超过屏幕大小2倍
——优化 悬浮高亮与操作位置统一性
MineSweeper EGE 9.1
——优化 进一步限制窗口大小不超过屏幕大小1.5倍
——优化 减少代码中的隐式类型转换
——优化 显示大小非16整数倍时方块边缘子像素处理
——优化 拖动操作识别逻辑
——修复 左上边缘异常悬浮高亮
MineSweeper EGE 9.2
——优化 进一步限制窗口高度不超过屏幕高度
——优化 窗口的高度和宽度同时限制
——优化 显示大小采用部分非线性调整
MineSweeper EGE 9.3
——优化 提高拖动操作灵敏度
——修复 点击笑脸按钮后鼠标直接选择起始点
//——修复 点击笑脸按钮重新生成地图后已解3BV计算错误
MineSweeper EGE 9.4
——优化 降低拖动操作启动灵敏度
MineSweeper EGE 9.5
——新增 720P和768P屏幕支持
——优化 地雷线条
——优化 游戏结束文字阴影随显示大小调整
——优化 拖动操作识别逻辑
MineSweeper EGE 9.6
——优化 顶栏绘制代码
——优化 拖动操作识别逻辑
MineSweeper EGE 9.7
——新增 笑脸位置的第二种选择
MineSweeper EGE 10
——优化 笑脸默认位置也有第二种选择
——优化 默认显示大小也采用部分非线性
//——修复 控制台键盘操作模式闪退（使用新版EGE）
MineSweeper EGE 10.1
——新增 非终局地图位置调整的弱趋向回正
——新增 调整显示大小时维持左上角不变
——优化 按下Ctrl时截止当前拖动翻开操作
——修复 地图偏移时调整显示大小可能出现重影
//——修复 第一次打开时控制台无光标
MineSweeper EGE 10.2
——新增 独立窗口大小以显示更多信息
——优化 提高时钟时针精度
//——修复 胜利后剩余雷数总是为0
MineSweeper EGE 10.3
——新增 窗口快速初始化
//——优化 对旧系统的兼容性
——修复 初始化窗口瞬间点击可能闪退
//——修复 使用实时求解指令后点击笑脸按钮重新生成地图不显示打破纪录
//——修复 点击笑脸按钮可能不能重新生成地图
//——修复 点击笑脸按钮重新生成地图时控制台3BV/s可能不能正确覆写
MineSweeper EGE 10.4
——修复 鼠标移至界外可能残留悬浮高亮
MineSweeper EGE 10.5
——优化 游戏信息横坐标仅在更新时计算
//——修复 继续上一次游戏时点击笑脸按钮不能正确重置时间
MineSweeper EGE 10.6
——修复 终局时鼠标移至界外可能残留悬浮高亮
//——新增 拖动标记根据起始操作统一标记/取消标记
//——新增 根据位数自动调整图标位置
//——优化 分立地图和窗口绘制代码
//——优化 编译体积（加链接参数-Wl,--gc-sections）
//——优化 根据位偏移统一图标位置设计语言
//——优化 通过地图快速显示技术降低按键延迟
//——修复 按下Alt时抬起Ctrl会进入Ctrl锁死状态
--------------------------------*/
