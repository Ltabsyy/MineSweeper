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
#define LimHeight 42//最大高度，限制行数
#define LimWidth 88//最大宽度，限制列数

// EGE窗口形态
int sideLength = 32;//外部窗口方块边长
#define heightOfBlock sideLength
#define widthOfBlock sideLength//锁定纵横比
#define heightOfBar 2*sideLength
#define xOfChar widthOfBlock*10/32
#define yOfChar heightOfBlock/32
#define heightOfChar heightOfBlock

// 地图生成和显示
void SummonBoard(int seed, int r0, int c0);
int Place(int n);
void ShowBoard(int mode);

// EGE窗口显示
void DrawMine(int r, int c);//绘制地图地雷
void DrawMineA(int x0, int y0, int r);//绘制地雷图形
void DrawFlag(int r, int c);//绘制地图旗帜
void DrawBlock(int r, int c, int board, int isShown);//绘制方块
void DrawLineA(int x0, int y0, int r, int angle);//绘制时钟指针
void DrawClock(int x0, int y0, int r, int time);//绘制时钟
void DrawBoard(int mode, int remainder, int t);//绘制总外部窗口
void InitWindow();
void GetWindowOperation(char* operation, int* r, int* c);
void CloseWindow(int isWinning, int remainder, int time);

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

// 全局变量及初始化
int numberOfMine = 10;//雷数量
int heightOfBoard = 10;//界面高度
int widthOfBoard = 10;//界面宽度

int main()
{
	int choiceMode;//游戏功能的选择
	int seed, r0, c0;//地图生成
	int r, c, remainder, isOpenMine, ra, ca;
	int t0, t1;
	char operation;
	int set, temp, difficulty;//设置
	while(1)//main内循环防止变量重复定义
	{
		system("cls");//清屏
		printf("*******************************\n"//宽31
			   "(1)新游戏\n"
			   "(2)设置\n"
			   "(3)退出\n"
			   "*******************************\n");
		printf(">");
		scanf("%d", &choiceMode);
		/*--新游戏--*/
		if(choiceMode == 1)
		{
			/*重置*/
			system("cls");
			remainder = numberOfMine;
			isOpenMine = 0;
			for(r=0; r<heightOfBoard; r++)
			{
				for(c=0; c<widthOfBoard; c++)
				{
					isShown[r][c] = 0;
				}
			}
			/*初始化*/
			ShowBoard(0);
			printf("选择坐标[r:行][c:列]\n");
			InitWindow();//创建窗口
			r0 = -1;
			while(r0 == -1)
			{
				DrawBoard(0, numberOfMine, 0);
				GetWindowOperation(&operation, &r0, &c0);
			}
			seed = time(0);//当前时间戳作种子生成随机数
			t0 = time(0);
			SummonBoard(seed, r0, c0);
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
						ShowBoard(1);
						printf(":(\nGame Over!\n");
						CloseWindow(0, remainder, t1-t0);
						break;
					}
				}
				if(NumberOfNotShown() == remainder)//未翻开的都是雷则胜利
				{
					system("cls");
					ShowBoard(1);
					printf(":)\nYou Win!\n");
					CloseWindow(1, remainder, t1-t0);
					break;
				}
				/*显示*/
				system("cls");
				ShowBoard(0);
				printf("剩余雷数: %d 用时: %d\n", remainder, t1-t0);//打印剩余雷数
				/*输入*/
				while(1)
				{
					operation = 0;
					while(operation == 0)
					{
						t1 = time(0);
						DrawBoard(0, remainder, t1-t0);
						GetWindowOperation(&operation, &r, &c);
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
							remainder++;//取消标记，剩余雷数+1
							isShown[r][c] = 1;
						}
						else
						{
							isShown[r][c] = 1;//翻开
						}
						break;
					}
					else if(operation == '#')
					{
						if(isShown[r][c] == 0)//标记
						{
							isShown[r][c] = 2;
							remainder--;
							break;
						}
						else if(isShown[r][c] == 2)//取消标记
						{
							isShown[r][c] = 0;
							remainder++;
							break;
						}
						else
						{
							printf(":(\n该坐标已翻开！\n");
						}
					}
					else
					{
						printf(":(\n未选择操作模式！\n");
					}
				}
			}
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
		}
		/*--退出--*/
		else if(choiceMode == 3)
		{
			break;
		}
		else
		{
			getchar();
		}
	}
	return 0;
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
		/*校验*/
		if(isMine[r0][c0] == 1) continue;//第1次就爆则循环
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
		/*校验*/
		if(numberOfMineAround[r0][c0] != 0) continue;//第1次翻开位置不为0则循环
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
			if(mode == 1)
			{
				if(board[r][c] == 0)
				{
					printf("  ");
				}
				else if(board[r][c] == 9)
				{
					if(isShown[r][c] == 1)
					{
						printf("@ ");
					}
					else if(isShown[r][c] == 2)
					{
						printf("# ");
					}
					else
					{
						printf("* ");
					}
				}
				else
				{
					printf("%d ", board[r][c]);
				}
			}
			else if(mode == 0)
			{
				if(isShown[r][c] == 1)
				{
					if(board[r][c] == 0)
					{
						printf("  ");
					}
					else
					{
						printf("%d ", board[r][c]);
					}
				}
				else if(isShown[r][c] == 2)
				{
					printf("# ");
				}
				else
				{
					printf("%% ");
				}
			}	
		}
		printf("\n");
	}
}

void DrawMine(int r, int c)//绘制地图地雷
{
	//setcolor(LIGHTRED);
	//xyprintf(c*widthOfBlock+xOfChar, r*heightOfBlock+yOfChar, "@");
	setfillcolor(BLACK);
	ege_fillellipse((c+7.4/32)*widthOfBlock, (r+7.4/32)*heightOfBlock, 0.6*widthOfBlock, 0.6*heightOfBlock);
	setcolor(BLACK);
	setlinewidth(sideLength/16);
	line(c*widthOfBlock+widthOfBlock*5/32, r*heightOfBlock+heightOfBlock*17/32, c*widthOfBlock+widthOfBlock*29/32, r*heightOfBlock+heightOfBlock*17/32);
	line(c*widthOfBlock+widthOfBlock*17/32, r*heightOfBlock+heightOfBlock*5/32, c*widthOfBlock+widthOfBlock*17/32, r*heightOfBlock+heightOfBlock*29/32);
	line(c*widthOfBlock+widthOfBlock*9/32, r*heightOfBlock+heightOfBlock*9/32, c*widthOfBlock+widthOfBlock*25/32, r*heightOfBlock+heightOfBlock*25/32);
	line(c*widthOfBlock+widthOfBlock*9/32, r*heightOfBlock+heightOfBlock*25/32, c*widthOfBlock+widthOfBlock*25/32, r*heightOfBlock+heightOfBlock*9/32);
	setfillcolor(WHITE);
	ege_fillrect((c+12.0/32)*widthOfBlock, (r+12.0/32)*heightOfBlock, (4.0/32)*widthOfBlock, (4.0/32)*heightOfBlock);
}

void DrawMineA(int x0, int y0, int r)//绘制地雷图形
{
	setfillcolor(BLACK);
	//r=16
	ege_fillellipse(x0-0.6*r, y0-0.6*r, 0.6*2*r, 0.6*2*r);
	setcolor(BLACK);
	setlinewidth(r/8);
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
	int x = c*widthOfBlock;
	int y = r*heightOfBlock;
	//绘制边框和底纹
	if(isShown == 1 || (board == 9 && isShown == 0))
	{
		setfillcolor(GRAY);
		ege_fillrect(x, y, widthOfBlock, heightOfBlock);
		setfillcolor(DARKGRAY);
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
		setfillcolor(LIGHTGRAY);
		//ege_fillrect(x+widthOfBlock*4/32, y+heightOfBlock*4/32, widthOfBlock*24/32, heightOfBlock*24/32);
		ege_fillrect(x+widthOfBlock*2/32, y+heightOfBlock*2/32, widthOfBlock*28/32, heightOfBlock*28/32);
		//setfontbkcolor(LIGHTGRAY);
	}
	//绘制文字或图形
	//xyprintf(c*widthOfBlock+12, r*heightOfBlock+8, "%d", board);
	//rectprintf(c*widthOfBlock, r*heightOfBlock, widthOfBlock, heightOfBlock, "%d", board);
	if(isShown == 2)
	{
		if(board != 9)//错误标记
		{
			setfillcolor(LIGHTRED);
			ege_fillrect(x+widthOfBlock*2/32, y+heightOfBlock*2/32, widthOfBlock*28/32, heightOfBlock*28/32);
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
			/*if(board == 1) setcolor(BLUE);
			else if(board == 2) setcolor(GREEN);
			else if(board < 6) setcolor(RED);//3-5
			else setcolor(YELLOW);*/
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
			xyprintf(c*widthOfBlock+xOfChar, r*heightOfBlock+yOfChar, "%d", board);
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
	// 绘制时钟(r=20)
	setfillcolor(WHITE);
	ege_fillellipse(x0-r, y0-r, 2*r, 2*r);
	setlinewidth(r/20);
	setcolor(BLACK);
	//circle(x0, y0, r);
	ege_ellipse(x0-r, y0-r, 2*r, 2*r);
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
				else if(board[r][c] == 0)
				{
					DrawBlock(r+2, c, 0, 1);
				}
				else if(board[r][c] == 9)
				{
					DrawBlock(r+2, c, 9, isShown[r][c]);
				}
				else
				{
					DrawBlock(r+2, c, board[r][c], 1);
				}
			}
			else if(mode == 0)//前台
			{
				if(isShown[r][c] == 2)
				{
					DrawBlock(r+2, c, 9, 2);
				}
				else if(isShown[r][c] == 0)
				{
					DrawBlock(r+2, c, 0, 0);
				}
				else
				{
					DrawBlock(r+2, c, board[r][c], 1);
				}
			}
		}
	}
	setbkmode(OPAQUE);
	if(widthOfBoard > 10)
	{
		//剩余雷数
		DrawMineA(2*widthOfBlock, heightOfBar/2, 20*heightOfBar/64*4/3);
		setcolor(RED);
		setfontbkcolor(BLACK);
		xyprintf(3*widthOfBlock+xOfChar, (heightOfBar-heightOfChar)/2, " %d ", remainder);
		//用时
		DrawClock(7*widthOfBlock, heightOfBar/2, 20*heightOfBar/64, time(0));//按真实时间走的钟(doge)
		setcolor(RED);
		setfontbkcolor(BLACK);
		xyprintf(8*widthOfBlock+xOfChar, (heightOfBar-heightOfChar)/2, " %d ", t);
	}
	else
	{
		//剩余雷数
		DrawMineA(1*widthOfBlock, heightOfBar/2, 20*heightOfBar/64*4/3);
		setcolor(RED);
		setfontbkcolor(BLACK);
		xyprintf(2*widthOfBlock, (heightOfBar-heightOfChar)/2, " %d ", remainder);
		//用时
		DrawClock(5*widthOfBlock, heightOfBar/2, 20*heightOfBar/64, time(0));//按真实时间走的钟(doge)
		setcolor(RED);
		setfontbkcolor(BLACK);
		xyprintf(6*widthOfBlock, (heightOfBar-heightOfChar)/2, " %d ", t);
	}
	setbkmode(TRANSPARENT);
}

void InitWindow()//创建窗口
{
	sideLength = 32;
	while(widthOfBlock*widthOfBoard > 1920 || heightOfBar+heightOfBlock*(heightOfBoard+4) > 1080)
	{
		sideLength -= 4;
	}
	setcaption("MineSweeper Window");
	SetProcessDPIAware();//避免Windows缩放造成模糊
	initgraph(widthOfBlock*widthOfBoard, heightOfBar+heightOfBlock*heightOfBoard, INIT_RENDERMANUAL);
	setbkcolor(LIGHTGRAY);
	setfont(heightOfChar, 0, "Consolas");
	setbkmode(TRANSPARENT);//默认设置为无背景字体
	ege_enable_aa(true);
}

void GetWindowOperation(char* operation, int* r, int* c)
{
	int xm, ym;
	mouse_msg mouseMsg;
	while(mousemsg())
	{
		mouseMsg = getmouse();
		if(mouseMsg.is_left() && mouseMsg.is_down())//鼠标左键按下
		{
			mousepos(&xm, &ym);
			if(ym >= heightOfBar)
			{
				*operation = '@';
				*r = (ym-heightOfBar)/heightOfBlock;
				*c = xm/widthOfBlock;
			}
		}
		if(mouseMsg.is_right() && mouseMsg.is_down())//鼠标右键按下
		{
			mousepos(&xm, &ym);
			if(ym >= heightOfBar)
			{
				*operation = '#';
				*r = (ym-heightOfBar)/heightOfBlock;
				*c = xm/widthOfBlock;
			}
		}
	}
	delay_ms(50);
}

void CloseWindow(int isWinning, int remainder, int time)
{
	int r, c;
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
	DrawBoard(1, remainder, time);
	if(isWinning == 1)
	{
		setcolor(BLACK);//显示阴影
		xyprintf(widthOfBlock+2, heightOfBar+2, "You Win!");
		setcolor(YELLOW);
		xyprintf(widthOfBlock, heightOfBar, "You Win!");
	}
	else
	{
		setcolor(BLACK);
		xyprintf(widthOfBlock+2, heightOfBar+2, "Game Over!");
		setcolor(RED);
		xyprintf(widthOfBlock, heightOfBar, "Game Over!");
	}
	setfont(heightOfChar/2, 0, "黑体");
	setcolor(BLACK);
	xyprintf(widthOfBlock+1, heightOfBar+heightOfBlock+1, "请按键盘任意键关闭窗口");
	setcolor(RED);
	xyprintf(widthOfBlock, heightOfBar+heightOfBlock, "请按键盘任意键关闭窗口");
	//xyprintf(widthOfBlock, heightOfBar+heightOfBlock*3/2, "请勿按右上角退出！");
	//xyprintf(widthOfBlock, heightOfBar+heightOfBlock, "Press any key to continue . . .");
	setfont(heightOfChar, 0, "Consolas");
	getch();
	//delay_ms(2000);
	cleardevice();
	closegraph();
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
--------------------------------*/
