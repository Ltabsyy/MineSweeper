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
#define LimHeight 128//最大高度，限制行数
#define LimWidth 93//最大宽度，限制列数

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
int IsPosInRectangle(int x, int y, int x1, int y1, int x2, int y2);
void SummonBoard(int seed, int r0, int c0);
int Place(int n);
void ShowBoard(int mode);

// EGE窗口显示
int dx = 0, dy = 0;//地图偏移
int screenHeight, screenWidth;
void DrawMine(int r, int c);//绘制地图地雷
void DrawMineA(int x0, int y0, int r);//绘制地雷图形
void DrawFlag(int r, int c);//绘制地图旗帜
void DrawBlock(int r, int c, int board, int isShown, int highlight);//绘制方块
void DrawLineA(int x0, int y0, int r, int angle);//绘制时钟指针
void DrawClock(int x0, int y0, int r, int time);//绘制时钟
void DrawFace(int mode);//绘制笑脸
void DrawBoard(int mode, int remainder, int t, int solved3BV, int total3BV);//绘制总外部窗口
void DrawSolution();//在外部窗口绘制方案矩阵
void InitWindow();
void GetWindowOperation(char* operation, int* r, int* c, int remainder, int t, int solved3BV, int total3BV);
int CloseWindow(int isWinning, int remainder, int time, int solved3BV, int total3BV);

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
int showTime = 1;
int show3BV = 0;
int refreshCycle = 50;
int newCursor = 2;
int bbbv = 10;

int main()
{
	int choiceMode = 0;//游戏功能的选择
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
				DrawBoard(0, numberOfMine, 0, -1, -1);
				GetWindowOperation(&operation, &r0, &c0, numberOfMine, 0, -1, -1);
				delay_ms(refreshCycle);
			}
			seed = time(0);//当前时间戳作种子生成随机数
			t0 = time(0);
			t1 = t0;
			SummonBoard(seed, r0, c0);
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
						ShowBoard(1);
						printf(":(\nGame Over!\n");
						isOpenMine = 1;
						break;
					}
				}
				if(NumberOfNotShown() == remainder)//未翻开的都是雷则胜利
				{
					system("cls");
					ShowBoard(1);
					printf(":)\nYou Win!\n");
					break;
				}
				/*显示*/
				system("cls");
				if(heightOfBoard <= 20 && widthOfBoard <= 58) ShowBoard(0);
				printf("剩余雷数: %d 用时: %d\n", remainder, t1-t0);//打印剩余雷数
				/*输入*/
				operation = 0;
				while(1)
				{
					if(1)
					{
						t1 = time(0);
						DrawBoard(0, remainder, t1-t0, bbbv-temp, bbbv);
						DrawSolution();
						GetWindowOperation(&operation, &r, &c, remainder, t1-t0, bbbv-temp, bbbv);
						if(operation == '%')//重新生成地图
						{
							remainder = numberOfMine;
							for(r=0; r<heightOfBoard; r++)
							{
								for(c=0; c<widthOfBoard; c++)
								{
									isShown[r][c] = 0;
								}
							}
							r0 = -1;
							while(r0 == -1)
							{
								DrawBoard(0, numberOfMine, 0, -1, -1);
								GetWindowOperation(&operation, &r0, &c0, numberOfMine, 0, -1, -1);
							}
							seed = time(0);
							t0 = time(0);
							SummonBoard(seed, r0, c0);
							r = r0;
							c = c0;
							isShown[r][c] = 1;
						}
					}
					//if(operation != 0) break;
					delay_ms(refreshCycle);
					//delay_fps(1000/refreshCycle);//维持帧率稳定
					if(operation != 0) break;
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
			/*游戏结束*/
			choiceMode = CloseWindow(1-isOpenMine, remainder, t1-t0, bbbv-temp, bbbv);
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
	int x = c*widthOfBlock+widthOfBorder+dx;
	int y = r*heightOfBlock+heightOfBar+widthOfBorder+dy;
	//setcolor(LIGHTRED);
	//xyprintf(x+xOfChar, y+yOfChar, "@");
	setfillcolor(BLACK);
	ege_fillellipse(x+7.4/32*widthOfBlock, y+7.4/32*heightOfBlock, 0.6*widthOfBlock, 0.6*heightOfBlock);
	setcolor(BLACK);
	setlinewidth(sideLength/16);
	ege_line(x+widthOfBlock*5/32, y+heightOfBlock*17/32, x+widthOfBlock*29/32, y+heightOfBlock*17/32);
	ege_line(x+widthOfBlock*17/32, y+heightOfBlock*5/32, x+widthOfBlock*17/32, y+heightOfBlock*29/32);
	ege_line(x+widthOfBlock*9/32, y+heightOfBlock*9/32, x+widthOfBlock*25/32, y+heightOfBlock*25/32);
	ege_line(x+widthOfBlock*9/32, y+heightOfBlock*25/32, x+widthOfBlock*25/32, y+heightOfBlock*9/32);
	setfillcolor(WHITE);
	ege_fillrect(x+12.0/32*widthOfBlock, y+12.0/32*heightOfBlock, (4.0/32)*widthOfBlock, (4.0/32)*heightOfBlock);
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
	int x = c*widthOfBlock+widthOfBorder+dx;
	int y = r*heightOfBlock+heightOfBar+widthOfBorder+dy;
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
		{x+6.0/32*widthOfBlock, y+11.0/32*heightOfBlock},
		{x+17.0/32*widthOfBlock, y+6.0/32*heightOfBlock},
		{x+17.0/32*widthOfBlock, y+16.0/32*heightOfBlock}
	};
	ege_fillpoly(3, polyPoints);
}

void DrawBlock(int r, int c, int board, int isShown, int highlight)//绘制方块
{
	int x = c*widthOfBlock+widthOfBorder+dx;
	int y = r*heightOfBlock+heightOfBar+widthOfBorder+dy;
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
		ege_fillrect(x+widthOfBlock*2/32, y+heightOfBlock*2/32, widthOfBlock*28/32, heightOfBlock*28/32);
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
	DrawLineA(x0, y0, r/2, 270+hour%12*30);
	//转轴
	setfillcolor(RED);
	ege_fillellipse(x0-r/10, y0-r/10, r/5, r/5);
	// 输出文字时间
	//xyprintf(x0+r+r/5, y0+r-r/5, "%2d:%2d:%2d", hour, minute, second);
}

void DrawFace(int mode)//绘制笑脸
{
	int h = heightOfBlock*3/2;
	int w = widthOfBlock*3/2;
	int x = (widthOfBlock*widthOfBoard-w)/2+widthOfBorder;
	int y = (heightOfBar-h)/2;
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

void DrawBoard(int mode, int remainder, int t, int solved3BV, int total3BV)//绘制总外部窗口
{
	int r, c;
	int rc1, cc1, rc2, cc2;
	int xm, ym, rm = -1, cm = -1, highlight;
	setfillcolor(LIGHTGRAY);
	ege_fillrect(0, 0, widthOfBlock*widthOfBoard+widthOfBorder*2, heightOfBar);//清除旧顶栏减少锯齿感
	ege_point polyPoints1[5] =
	{
		{0+dx, heightOfBar+dy},
		{widthOfBlock*widthOfBoard+widthOfBorder*2+dx, heightOfBar+dy},
		{widthOfBlock*widthOfBoard+widthOfBorder+dx, heightOfBar+widthOfBorder+dy},
		{widthOfBorder+dx, heightOfBar+heightOfBlock*heightOfBoard+widthOfBorder+dy},
		{0+dx, heightOfBar+heightOfBlock*heightOfBoard+widthOfBorder*2+dy}
	};
	ege_point polyPoints2[5] =
	{
		{widthOfBlock*widthOfBoard+widthOfBorder*2+dx, heightOfBar+dy},
		{widthOfBlock*widthOfBoard+widthOfBorder+dx, heightOfBar+widthOfBorder+dy},
		{widthOfBorder+dx, heightOfBar+heightOfBlock*heightOfBoard+widthOfBorder+dy},
		{0+dx, heightOfBar+heightOfBlock*heightOfBoard+widthOfBorder*2+dy},
		{widthOfBlock*widthOfBoard+widthOfBorder*2+dx, heightOfBar+heightOfBlock*heightOfBoard+widthOfBorder*2+dy}
	};
	setfillcolor(GRAY);
	ege_fillpoly(5, polyPoints1);
	setfillcolor(WHITE);
	ege_fillpoly(5, polyPoints2);
	//悬浮高亮
	mousepos(&xm, &ym);
	if(IsPosInRectangle(xm-dx, ym-dy, widthOfBorder, heightOfBar+widthOfBorder,
		widthOfBlock*widthOfBoard+widthOfBorder, heightOfBar+heightOfBlock*heightOfBoard+widthOfBorder))
	{
		rm = (ym-dy-heightOfBar-widthOfBorder)/heightOfBlock;
		cm = (xm-dx-widthOfBorder)/widthOfBlock;
	}
	//裁剪优化
	for(rc1=0; (rc1+1)*heightOfBlock+heightOfBar+widthOfBorder+dy < 0; rc1++);
	for(cc1=0; (cc1+1)*widthOfBlock+widthOfBorder+dx < 0; cc1++);
	for(rc2=heightOfBoard-1; (rc2-1)*heightOfBlock+heightOfBar+widthOfBorder+dy > screenHeight; rc2--);
	for(cc2=widthOfBoard-1; (cc2-1)*widthOfBlock+widthOfBorder+dx > screenWidth; cc2--);
	for(r=rc1; r<=rc2; r++)
	{
		for(c=cc1; c<=cc2; c++)
		{
			highlight = 0;
			if(newCursor > 1)//淡黄色高亮光标
			{
				if(r == rm && c == cm) highlight = 1;
				if(newCursor == 3)
				{
					if(r == rm || c == cm) highlight = 1;
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
	if(widthOfBoard > 12)
	{
		DrawMineA(2*widthOfBlock, heightOfBar/2, 20*heightOfBar/64*4/3);
		setcolor(RED);
		setfontbkcolor(BLACK);
		xyprintf(3*widthOfBlock+xOfChar, (heightOfBar-heightOfChar)/2, " %d ", remainder);
	}
	else if(widthOfBoard > 8)
	{
		DrawMineA(1*widthOfBlock, heightOfBar/2, 20*heightOfBar/64*4/3);
		setcolor(RED);
		setfontbkcolor(BLACK);
		xyprintf(2*widthOfBlock, (heightOfBar-heightOfChar)/2, " %d ", remainder);
	}
	else if(widthOfBoard > 4)
	{
		setcolor(RED);
		setfontbkcolor(BLACK);
		xyprintf(widthOfBoard*widthOfBlock/4-widthOfBlock, (heightOfBar-heightOfChar)/2, " %d ", remainder);
	}
	//用时
	if(showTime == 1)
	{
		if(widthOfBoard > 23)
		{
			DrawClock(7*widthOfBlock, heightOfBar/2, 20*heightOfBar/64, time(0));//按真实时间走的钟(doge)
			setcolor(RED);
			setfontbkcolor(BLACK);
			xyprintf(8*widthOfBlock+xOfChar, (heightOfBar-heightOfChar)/2, " %d ", t);
		}
		else if(widthOfBoard > 12)
		{
			DrawClock((widthOfBoard+6)*widthOfBlock/2, heightOfBar/2, 20*heightOfBar/64, time(0));
			setcolor(RED);
			setfontbkcolor(BLACK);
			xyprintf((widthOfBoard+8)*widthOfBlock/2+xOfChar, (heightOfBar-heightOfChar)/2, " %d ", t);
		}
		else if(widthOfBoard > 10)
		{
			DrawClock((widthOfBoard+4)*widthOfBlock/2, heightOfBar/2, 20*heightOfBar/64, time(0));
			setcolor(RED);
			setfontbkcolor(BLACK);
			xyprintf((widthOfBoard+6)*widthOfBlock/2+xOfChar, (heightOfBar-heightOfChar)/2, " %d ", t);
		}
		else if(widthOfBoard == 10)
		{
			DrawClock(7*widthOfBlock, heightOfBar/2, 20*heightOfBar/64, time(0));
			setcolor(RED);
			setfontbkcolor(BLACK);
			xyprintf(8*widthOfBlock, (heightOfBar-heightOfChar)/2, " %d ", t);
		}
		else if(widthOfBoard == 9)
		{
			DrawClock(6*widthOfBlock+widthOfBorder, heightOfBar/2, 20*heightOfBar/64, time(0));
			setcolor(RED);
			setfontbkcolor(BLACK);
			if(t < 1000) xyprintf(7*widthOfBlock+widthOfBorder, (heightOfBar-heightOfChar)/2, " %d ", t);
			else xyprintf(7*widthOfBlock+widthOfBorder, (heightOfBar-heightOfChar)/2, "%d ", t);
		}
		else if(widthOfBoard > 4)
		{
			setcolor(RED);
			setfontbkcolor(BLACK);
			xyprintf(widthOfBoard*widthOfBlock*3/4, (heightOfBar-heightOfChar)/2, " %d ", t);
		}
	}
	//3BV
	if(show3BV == 1 && total3BV != -1)
	{
		if(widthOfBoard > 51)
		{
			setbkmode(TRANSPARENT);
			setcolor(BLACK);
			//setfontbkcolor(LIGHTGRAY);
			xyprintf(12*widthOfBlock+xOfChar, (heightOfBar-heightOfChar)/2, "3BV");
			xyprintf(19*widthOfBlock+xOfChar, (heightOfBar-heightOfChar)/2, "3BV/s");
			setbkmode(OPAQUE);
			setcolor(RED);
			setfontbkcolor(BLACK);
			xyprintf(14*widthOfBlock+xOfChar, (heightOfBar-heightOfChar)/2, " %d/%d ", solved3BV, total3BV);
			xyprintf(22*widthOfBlock+xOfChar, (heightOfBar-heightOfChar)/2, " %.2f ", (float)solved3BV/t);
		}
		else if(widthOfBoard > 27)
		{
			setbkmode(TRANSPARENT);
			setcolor(BLACK);
			//setfontbkcolor(LIGHTGRAY);
			xyprintf((widthOfBoard+3)*widthOfBlock/2+xOfChar, (heightOfBar-heightOfChar)/2, "3BV");
			xyprintf((widthOfBoard+17)*widthOfBlock/2+xOfChar, (heightOfBar-heightOfChar)/2, "3BV/s");
			setbkmode(OPAQUE);
			setcolor(RED);
			setfontbkcolor(BLACK);
			xyprintf((widthOfBoard+7)*widthOfBlock/2+xOfChar, (heightOfBar-heightOfChar)/2, " %d/%d ", solved3BV, total3BV);
			xyprintf((widthOfBoard+22)*widthOfBlock/2+xOfChar, (heightOfBar-heightOfChar)/2, " %.2f ", (float)solved3BV/t);
		}
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
				if(solution[r][c] == 1)
				{
					setfillcolor(LIGHTGREEN);
				}
				else if(solution[r][c] == 2)
				{
					setfillcolor(PINK);
				}
				else if(solution[r][c] == 3)
				{
					setfillcolor(LIGHTYELLOW);
				}
				ege_fillrect(x+widthOfBlock*2/32, y+heightOfBlock*2/32, widthOfBlock*28/32, heightOfBlock*28/32);
			}
		}
	}
}

void InitWindow()//创建窗口
{
	int r, c;
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
	else if(screenHeight >= 1440) sideLength = 40;
	else sideLength = 32;
	while(widthOfBlock*widthOfBoard+widthOfBorder*2 > screenWidth
		|| heightOfBar+heightOfBlock*(heightOfBoard+4)+widthOfBorder*2 > screenHeight)
	{
		sideLength -= 4;
	}
	setcaption("MineSweeper Window");
	SetProcessDPIAware();//避免Windows缩放造成模糊
	initgraph(widthOfBlock*widthOfBoard+widthOfBorder*2, heightOfBar+heightOfBlock*heightOfBoard+widthOfBorder*2, INIT_RENDERMANUAL);
	setbkcolor(LIGHTGRAY);
	setfont(heightOfChar, 0, "Consolas");
	setbkmode(TRANSPARENT);//默认设置为无背景字体
	ege_enable_aa(true);
	//flushmouse();//避免上一局鼠标消息选择起始点
	dx = 0;
	dy = 0;//偏移回正
	GetWindowOperation(NULL, NULL, NULL, numberOfMine, 0, -1, -1);
}

void GetWindowOperation(char* operation, int* r, int* c, int remainder, int t, int solved3BV, int total3BV)
{
	int xm, ym, xn, yn, clock0, clock1;
	static int isOpening, isSigning, ro, co, xo, yo;//拖动操作
	mouse_msg mouseMsg;
	key_msg keyMsg;
	if(operation == NULL || r == NULL || c == NULL)//重置键鼠消息
	{
		isOpening = 0;
		isSigning = 0;
		flushmouse();
		flushkey();
	}
	while(mousemsg() || kbmsg())//使用while代替if避免堆积消息产生延迟
	{
		//键鼠混动输入
		if(mousemsg()) mouseMsg = getmouse();
		if(kbmsg()) keyMsg = getkey();
		if(mouseMsg.is_up())
		{
			if(mouseMsg.is_left()) isOpening = 0;//鼠标左键抬起
			if(mouseMsg.is_right()) isSigning = 0;//鼠标右键抬起
		}
		if(mouseMsg.is_down())
		{
			if(mouseMsg.is_left())//鼠标左键按下
			{
				xm = mouseMsg.x;
				ym = mouseMsg.y;
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
							DrawBoard(0, remainder, t, solved3BV, total3BV);
							dx -= xn-xm;
							dy -= yn-ym;
							clock0 = clock();
						}
						if(mouseMsg.is_left() && mouseMsg.is_up()) break;
						//delay_ms(refreshCycle);
					}
					dx += xn-xm;
					dy += yn-ym;
					//if(dx > -widthOfBlock && dx < widthOfBlock) dx = 0;
					//if(dy > -heightOfBlock && dy < heightOfBlock) dy = 0;
					cleardevice();//避免重影
					DrawBoard(0, remainder, t, solved3BV, total3BV);//避免闪烁
				}
				else
				{
					if(IsPosInRectangle(xm, ym,
						widthOfBlock*widthOfBoard/2+widthOfBorder-widthOfBlock*3/4, heightOfBar/2-heightOfBlock*3/4,
						widthOfBlock*widthOfBoard/2+widthOfBorder+widthOfBlock*3/4, heightOfBar/2+heightOfBlock*3/4))
					{
						*operation = '%';
					}
					else if(IsPosInRectangle(xm-dx, ym-dy, widthOfBorder, heightOfBar+widthOfBorder,
						widthOfBlock*widthOfBoard+widthOfBorder, heightOfBar+heightOfBlock*heightOfBoard+widthOfBorder))
					{
						*operation = '@';
						*r = (ym-dy-heightOfBar-widthOfBorder)/heightOfBlock;
						*c = (xm-dx-widthOfBorder)/widthOfBlock;
						DrawFace(1);
						isOpening = 1;
						ro = *r;
						co = *c;
						xo = xm;
						yo = ym;
						break;
					}
				}
			}
			if(mouseMsg.is_right())//鼠标右键按下
			{
				xm = mouseMsg.x;
				ym = mouseMsg.y;
				if(IsPosInRectangle(xm-dx, ym-dy, widthOfBorder, heightOfBar+widthOfBorder,
					widthOfBlock*widthOfBoard+widthOfBorder, heightOfBar+heightOfBlock*heightOfBoard+widthOfBorder))
				{
					*operation = '#';
					*r = (ym-dy-heightOfBar-widthOfBorder)/heightOfBlock;
					*c = (xm-dx-widthOfBorder)/widthOfBlock;
					DrawFace(1);
					isSigning = 1;
					ro = *r;
					co = *c;
					xo = xm;
					yo = ym;
					break;
				}
			}
		}
		if(mouseMsg.is_move() && (isOpening == 1 || isSigning == 1))
		{
			//mousepos(&xm, &ym);//最新位置
			xm = mouseMsg.x;
			ym = mouseMsg.y;//缓冲位置
			if(IsPosInRectangle(xm-dx, ym-dy, widthOfBorder, heightOfBar+widthOfBorder,
				widthOfBlock*widthOfBoard+widthOfBorder, heightOfBar+heightOfBlock*heightOfBoard+widthOfBorder))
			{
				*r = (ym-dy-heightOfBar-widthOfBorder)/heightOfBlock;
				*c = (xm-dx-widthOfBorder)/widthOfBlock;
				//DrawFace(1);
				if(ro != *r || co != *c)//移动到其他方块
				{
					if(IsPosInRectangle(xm, ym, xo-widthOfBlock*3/4, yo-heightOfBlock*3/4, xo+widthOfBlock*3/4, yo+heightOfBlock*3/4))
					{
						//移动距离必须超过3/4个方块
					}
					else
					{
						if(isOpening == 1) *operation = '@';
						if(isSigning == 1) *operation = '#';
						DrawFace(1);
						ro = *r;
						co = *c;
						xo = xm;
						yo = ym;
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
		if(mouseMsg.is_wheel() && keystate(key_control))
		{
			if(mouseMsg.wheel > 0) sideLength += 4;
			else if(sideLength > 4) sideLength -= 4;
			initgraph(widthOfBlock*widthOfBoard+widthOfBorder*2, heightOfBar+heightOfBlock*heightOfBoard+widthOfBorder*2, INIT_RENDERMANUAL);
			setfont(heightOfChar, 0, "Consolas");
		}
	}
}

int CloseWindow(int isWinning, int remainder, int time, int solved3BV, int total3BV)
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
	DrawBoard(1, remainder, time, solved3BV, total3BV);
	if(isWinning == 1)
	{
		DrawFace(3);
		setcolor(BLACK);//显示阴影
		xyprintf(widthOfBlock+2, heightOfBar+widthOfBorder+2, "You Win!");
		setcolor(YELLOW);
		xyprintf(widthOfBlock, heightOfBar+widthOfBorder, "You Win!");
	}
	else
	{
		DrawFace(2);
		setcolor(BLACK);
		xyprintf(widthOfBlock+2, heightOfBar+widthOfBorder+2, "Game Over!");
		setcolor(RED);
		xyprintf(widthOfBlock, heightOfBar+widthOfBorder, "Game Over!");
	}
	delay_ms(1000);
	flushmouse();
	while(newGame == -1)
	{
		DrawBoard(1, remainder, time, solved3BV, total3BV);//刷新界面
		if(isWinning == 1)
		{
			DrawFace(3);
			setcolor(BLACK);
			xyprintf(widthOfBlock+2, heightOfBar+widthOfBorder+2, "You Win!");
			setcolor(YELLOW);
			xyprintf(widthOfBlock, heightOfBar+widthOfBorder, "You Win!");
		}
		else
		{
			DrawFace(2);
			setcolor(BLACK);
			xyprintf(widthOfBlock+2, heightOfBar+widthOfBorder+2, "Game Over!");
			setcolor(RED);
			xyprintf(widthOfBlock, heightOfBar+widthOfBorder, "Game Over!");
		}
		setfont(heightOfChar/2, 0, "黑体");
		setcolor(BLACK);
		//xyprintf(widthOfBlock+1, heightOfBar+widthOfBorder+heightOfBlock+1, "请按键盘任意键关闭窗口");
		xyprintf(widthOfBlock+1, heightOfBar+widthOfBorder+heightOfBlock+1, "左键新游戏，右键关闭窗口");
		setcolor(RED);
		//xyprintf(widthOfBlock, heightOfBar+widthOfBorder+heightOfBlock, "请按键盘任意键关闭窗口");
		xyprintf(widthOfBlock, heightOfBar+widthOfBorder+heightOfBlock, "左键新游戏，右键关闭窗口");
		//xyprintf(widthOfBlock, heightOfBar+heightOfBlock*3/2, "请勿按右上角退出！");
		//xyprintf(widthOfBlock, heightOfBar+heightOfBlock, "Press any key to continue . . .");
		setfont(heightOfChar, 0, "Consolas");
		//getch();
		//delay_ms(2000);
		//break;
		while(mousemsg())
		{
			mouseMsg = getmouse();
			if(mouseMsg.is_left() && mouseMsg.is_down())
			{
				if(keystate(key_control))
				{
					xm = mouseMsg.x;
					ym = mouseMsg.y;
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
							DrawBoard(1, remainder, time, solved3BV, total3BV);
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
					DrawBoard(1, remainder, time, solved3BV, total3BV);
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
				if(mouseMsg.wheel > 0) sideLength += 4;
				else if(sideLength > 4) sideLength -= 4;
				initgraph(widthOfBlock*widthOfBoard+widthOfBorder*2, heightOfBar+heightOfBlock*heightOfBoard+widthOfBorder*2, INIT_RENDERMANUAL);
				setfont(heightOfChar, 0, "Consolas");//重设字体大小
				DrawBoard(1, remainder, time, solved3BV, total3BV);
				if(isWinning == 1) DrawFace(3);
				else DrawFace(2);
			}
		}
		delay_ms(refreshCycle);
	}
	cleardevice();
	closegraph();
	return newGame;
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
--------------------------------*/
