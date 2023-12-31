//#define _CRT_SECURE_NO_WARNINGS//vs专属(doge)
#include <stdio.h>
#include <stdlib.h>
#include <time.h>//时间戳作种子生成随机数，用于生成地图和无确定解时随机翻开
#include <conio.h>//非阻塞式输入
#include <windows.h>//面向Windows输出彩色字符
/**
 * 扫雷 MineSweeper Run
 *
 * by Ltabsyy & Jws
 **/
#define LimHeight 128//最大高度，限制行数
#define LimWidth 93//最大宽度，限制列数
#define LimDictionary 1024//字典大小
//内存占用至少为 53*H*W+128*D Byte，默认约0.7MB

// 地图生成和显示
void SummonBoard(int r0,int c0);//生成雷，数字和后台总板
int Place(int n);//计算某数所占位数，用于对齐坐标轴
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
void ShowBoard(int mode);//显示地图，0前台，1后台

// 后台计算
void OpenZeroChain(int r0,int c0);//翻开0连锁翻开
void SummonZeroChain(int r0,int c0);
int RealRemainder();//真实的剩余雷数，非显示的剩余雷数
int NumberOfNotShown();//%的数量
int NumberOfSign();//#的数量
int NumberOfNotShownAround(int r0,int c0);//某坐标附近%的数量
int NumberOfSignAround(int r0,int c0);//某坐标附近#的数量
int IsAroundZeroChain(int r0,int c0);

// 方案思考
void LookMap();//从实时游戏获取地图
void TranslateMap(int rs0,int cs0);//从键盘输入获取地图
int Solve();
int Think();
void SummonMineRateForNotShown();
int NumberOfNumberAround(int r0,int c0);
void ShowAnswer(int isFoundOpen);
void ShownModeBak(int mode);//备份显示方式矩阵，1备份，0恢复

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
int isBench = 0;//0方案显示1解，1无确定解时随机翻开

// 是否调试(0:关 1:开，显示部分 2:开，显示全部)
int debug = 0;

int main()
{
	int choiceMode;//游戏功能的选择
	int seed, r0, c0, seedMin, seedMax;//地图生成
	char operation, key;//游戏操作和设置的字符读取，用户按键
	int r, c, remainder, isOpenMine, ra, ca;//通用坐标和游戏胜利与连锁反应判断
	int t0, t1, yOfMapEnd, ro, co;//显示用时，定位，上一个操作坐标
	int set, temp;//设置
	int difficulty = 0;//当前难度显示，实际没有必要定义(doge)
	float density =(float) numberOfMine / heightOfBoard / widthOfBoard;//雷密度
	int heightOfMapShown, widthOfMapShown, rs0, cs0;//地图求解输入翻译
	int countOfWin, countOfSolution, countOfStep;//Bench统计
	// main()局部变量及初始化
	int showTime = 1;//显示用时，0不显示，1显示
	int operateMode = 1;//操作模式，0@#rc，1wasd23
	int refreshCycle = 100;//刷新周期，默认100ms，一般鼠标8ms，游戏鼠标1ms
	int fastMove = 0;//大写WASD快速移动
	int fastSign = 0;//#数字一键标记周围
	int newCursor = 0;//><光标
	//很好，我已经看不清我定义些啥了(doge)
	if(backgroundColor == 0xf0) system("color f0");//默认浅色模式
	while(1)//main内循环防止变量重复定义
	{
		system("cls");//清屏
		if(debug == 2) printf("<Debug>\nHello! Administrator Ltabsyy or Jws!\n");
		/*printf("*******************************\n"
			   "**新游戏(1)**设置(2)**退出(3)**\n"
			   "*******************************\n");*/
		printf("*******************************\n"//宽31
			   "(1)新游戏\n"
			   "(2)地图求解\n"
			   "(3)设置\n"
			   "(4)Bench\n"
			   "(5)退出\n"
			   "*******************************\n");
		rs0 = Place(widthOfBoard-1);//行起始指针，为列坐标显示占用行数
		cs0 = Place(heightOfBoard-1)+1;//列起始指针，为行坐标显示占用列数
		heightOfMapShown = heightOfBoard + rs0;//计算包含坐标的行数
		widthOfMapShown = 2*widthOfBoard + cs0 + 1;//计算包含坐标的每行字符数加回车
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
			printf(">@ ");
			if(operateMode == 0)
			{
				scanf("%d%d", &r0, &c0);//通过键盘获取第一次选择的坐标
			}
			else if(operateMode == 1)
			{
				r0 = heightOfBoard/2;
				c0 = widthOfBoard/2;
				while(1)//简易版wasd23
				{
					if(kbhit())
					{
						key = getch();
						if((key == 'w' || key == 'W') && r0 > 0) r0--;//上移
						if((key == 'a' || key == 'A') && c0 > 0) c0--;//左移
						if((key == 's' || key == 'S') && r0 < heightOfBoard-1) r0++;//下移
						if((key == 'd' || key == 'D') && c0 < widthOfBoard-1) c0++;//下移
						if(key != 'w' && key != 'W' && key != 'a' && key != 'A'
						&& key != 's' && key != 'S' && key != 'd' && key != 'D') break;//任意键翻开
					}
					else
					{
						gotoxy(0, heightOfMapShown-1+2);
						printf(">@ %d %d ", r0, c0);//覆写尾部
						gotoxy(cs0+2*c0, heightOfMapShown-1-heightOfBoard+1+r0);
						Sleep(refreshCycle);
					}
				}
				gotoxy(0, heightOfMapShown-1+3);
			}
			seed = time(0);//当前时间戳作种子生成随机数
			if(debug == 2)
			{
				set = 0;
				printf("[Debug]seed=%d,%d,%d\n", seed, r0, c0);
				printf("[0:生成/1:更改/2:搜索]\n>");
				scanf("%d", &set);
				if(set == 1)
				{
					printf("[seed] [r0] [c0]\n>");
					scanf("%d%d%d", &seed, &r0, &c0);
				}
				else if(set == 2)
				{
					printf("[seedMin] [seedMax] [r0] [c0]\n>");
					scanf("%d%d%d%d", &seedMin, &seedMax, &r0, &c0);
					debug = 0;//搜索不显示多余信息
					for(seed=seedMin; seed<=seedMax; seed++)
					{
						srand(seed);
						SummonBoard(r0,c0);
						printf("Map:%d*%d-%d\n", heightOfBoard, widthOfBoard, numberOfMine);
						printf("seed=%d,%d,%d\n", seed, r0, c0);
						ShowBoard(1);
					}
					debug = 2;
					system("pause");
					continue;
				}
			}
			if(summonCheckMode == 3)//可解地图生成
			{
				isOpenMine=1;
				isBench=1;
				while(1)
				{
					srand(seed);
					isOpenMine=0;
					SummonBoard(r0,c0);
					isShown[r0][c0] = 1;
					ShownModeBak(1);
					for(r=0; r<heightOfBoard; r++)
					{
						for(c=0; c<widthOfBoard; c++)
						{
							solution[r][c]=0;
						}
					}
					OpenZeroChain(r0,c0);
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
										OpenZeroChain(r,c);
									}
									else if(board[r][c] == 9)
									{
										isOpenMine=1;
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
				isBench=0;
			}
			else
			{
				srand(seed);
				r = r0;
				c = c0;
				SummonBoard(r,c);
				isShown[r][c] = 1;
			}
			ro = r0;
			co = c0;
			t0 = time(0);
			system("cls");
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
						t1 = time(0);
						if(showTime == 1) printf("用时：%d\n", t1-t0);
						system("pause");
						break;
					}
				}
				if(RealRemainder() == 0//正确标记所有雷则胜利
				 ||NumberOfNotShown() == remainder)//未翻开的都是雷则胜利
				{
					if(NumberOfSign() > numberOfMine)//标记量不能超过雷数
					{
						printf(":(\n标记过多！请重新标记。\n");
						system("pause");
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
					system("cls");
					ShowBoard(1);
					printf(":)\nYou Win!\n");
					t1 = time(0);
					if(showTime == 1) printf("用时：%d\n", t1-t0);
					system("pause");
					break;
				}
				/*显示*/
				gotoxy(0,0);
				yOfMapEnd = heightOfMapShown-1;
				if(debug == 1 || debug == 2)
				{
					system("cls");
					printf("<Debug>\n");
					printf("Map:%d*%d-%d\n", heightOfBoard, widthOfBoard, numberOfMine);
					printf("seed=%d,%d,%d\n", seed, r0, c0);
					ShowBoard(1);
					printf("RealRemainder=%d \n", RealRemainder());
					/*printf("[Debug]显示方式矩阵：\n");
					for(r=0; r<heightOfBoard; r++)
					{
						printf("  ");
						for(c=0; c<widthOfBoard; c++)
						{
							printf("%d ", isShown[r][c]);
						}
						printf("\n");
					}*/
					printf("NumberOfNotShown=%d\n", NumberOfNotShown());
					printf("\n");
					yOfMapEnd += heightOfMapShown+6;
				}
				ShowBoard(0);
				printf("剩余雷数: %d \n", remainder);//打印剩余雷数
				t1 = time(0);
				if(showTime == 1) printf("用时：%d\n", t1-t0);
				printf("选择模式与坐标\n");
				if(operateMode == 0) printf("[@:翻开/#:标记] [r] [c]  \n");
				if(operateMode == 1) printf("[wasd:移动/2:翻开/3:标记]\n");
				/*输入和显示用时*/
				printf(">");
				operation = 0;
				r = ro;
				c = co;
				while(1)
				{
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
							ShownModeBak(1);
							LookMap();
							gotoxy(0, yOfMapEnd+5);//输出debug信息
							if(showTime == 1) gotoxy(0, yOfMapEnd+6);
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
							if(key == '!')
							{
								ShownModeBak(0);
								r = ro;
								c = co;
							}
							else if(key == '\t')
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
						//操作模式切换
						if((key == '@' || key == '#' || key == '$') && operateMode == 1)
						{
							gotoxy(0, yOfMapEnd+4);
							if(showTime == 1) gotoxy(0, yOfMapEnd+5);
							printf(">        ");//覆写@ 128 93
							if(debug == 1 || debug == 2)
							{
								printf("\n:(\n检测到@#rc操作指令！正在识别...\n");
								system("pause");
							}
							gotoxy(1, yOfMapEnd+4);
							if(showTime == 1) gotoxy(1, yOfMapEnd+5);
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
							if(fastMove == 1)
							{
								if(key == 'w' && r > 0) r--;
								if(key == 'W')
								{
									if(r-5 > 0)
									{
										r -= 5;
									}
									else if(r > 0)
									{
										r = 0;
									}
								}
								if(key == 'a' && c > 0) c--;
								if(key == 'A')
								{
									if(c-5 > 0)
									{
										c -= 5;
									}
									else if(c > 0)
									{
										c = 0;
									}
								}
								if(key == 's' && r < heightOfBoard-1) r++;
								if(key == 'S')
								{
									if(r+5 < heightOfBoard-1)
									{
										r += 5;
									}
									else if(r < heightOfBoard-1)
									{
										r = heightOfBoard-1;
									}
								}
								if(key == 'd' && c < widthOfBoard-1) c++;
								if(key == 'D')
								{
									if(c+5 < widthOfBoard-1)
									{
										c += 5;
									}
									else if(c < widthOfBoard-1)
									{
										c = widthOfBoard-1;
									}
								}
							}
							else
							{
								if((key == 'w' || key == 'W') && r > 0) r--;//上移
								if((key == 'a' || key == 'A') && c > 0) c--;//左移
								if((key == 's' || key == 'S') && r < heightOfBoard-1) r++;//下移
								if((key == 'd' || key == 'D') && c < widthOfBoard-1) c++;//下移
							}
							if(key == '2' || key == 'h' || key == 'H') operation = '@';//2翻开
							if(key == '3' || key == 'u' || key == 'U') operation = '#';//3标记
							//if(operation != 0) break;
						}
					}
					else//无按键
					{
						if(showTime == 1)
						{
							gotoxy(0, yOfMapEnd+2);
							t1 = time(0);
							printf("用时：%d", t1-t0);//从行首写防止文字重影
							gotoxy(1, yOfMapEnd+5);
						}
						if(operateMode == 1)//wasd操作设计
						{
							//显示操作对应指令
							gotoxy(0, yOfMapEnd+4);
							if(showTime == 1) gotoxy(0, yOfMapEnd+5);
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
								//gotoxy(0, yOfMapEnd+4);
								//if(showTime == 1) gotoxy(0, yOfMapEnd+5);
							}
							//光标移动至对应点
							gotoxy(cs0+2*c, yOfMapEnd-heightOfBoard+1+r);//当实际位置为该位置左部，光标显示在该位置
							if(operation != 0)
							{
								Sleep(refreshCycle);
								gotoxy(0, yOfMapEnd+5);
								if(showTime == 1) gotoxy(0, yOfMapEnd+6);
								break;
							}
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
												OpenZeroChain(ra,ca);
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
							/*printf(":(\n该坐标已翻开！\n");
							system("pause");*/
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
					}
				}
				/*else
				{
					printf(":(\n未选择操作模式！\n");
				}*/
			}
		}
		/*--地图求解--*/
		else if(choiceMode == 2)
		{
			system("cls");//清屏
			//printf("MineSweeper Solution\n");
			if(debug == 1)
			{
				printf("*******************************\n"//宽31
					   "使用方法：\n"
					   "从含坐标地图左上角到右下角整个选中，\n"
					   "复制粘贴并回车。\n"
					   "可以把所有方案批量复制到游戏处粘贴，\n"
					   "最后一个方案如未执行别忘了按下回车。\n"
					   "按 ` 返回菜单。\n"
					   "*******************************\n");
			}
			else if(debug == 2)
			{
				printf("*******************************\n"//宽31
					   "使用方法：\n"
					   "没有必要，你已是开发者。(doge)\n"
					   "*******************************\n");
			}
			else
			{
				printf("*******************************\n"//宽31
					   "使用方法：\n"
					   "从含坐标地图左上角到右下角整个选中，\n"
					   "复制粘贴并回车。\n"
					   "按 ` 返回菜单。\n"
					   "*******************************\n");
			}
			printf("当前雷数:%d|当前界面大小:%d*%d\n", numberOfMine, heightOfBoard, widthOfBoard);
			srand(time(0));
			if(debug == 2)
			{
				printf("<Debug>\n");
				printf("pointer ->(%d,%d)\n", rs0, cs0);
				printf("heightOfMapShown=%d\n", heightOfMapShown);
				printf("widthOfMapShown=%d\n", widthOfMapShown);
			}
			/*扫雷*/
			getchar();//吃回车
			while(1)
			{
				/*输入*/
				printf("请粘贴最新地图：\n");
				for(r=0; r<heightOfMapShown; r++)
				{
					for(c=0; c<widthOfMapShown; c++)//默认为空格
					{
						mapShown[r][c] = ' ';
					}
					gets(mapShown[r]);//从键盘获取该行地图
					if(mapShown[r][0] == '`') break;
				}
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
				ShowAnswer(Solve());
			}
		}
		/*--设置--*/
		else if(choiceMode == 3)
		{
			while(1)
			{
				system("cls");
				printf("*******************************\n"//宽31
					   "(1)设置游戏难度\n"
					   "(2)设置地图生成校验\n");
				if(showTime == 0) printf("(3)启用游戏时实时显示用时\n");
				if(showTime == 1) printf("(3)关闭游戏时实时显示用时\n");
				printf("(4)更改默认操作模式\n"
					   "(5)调试选项\n"
					   "(6)返回菜单\n"
					   "*******************************\n");
				printf("当前雷数:%d|当前界面大小:%d*%d\n", numberOfMine, heightOfBoard, widthOfBoard);
				density =(float) numberOfMine / heightOfBoard / widthOfBoard;
				printf("当前密度:%.2f\n", density);
				printf(">");
				scanf("%d", &set);
				if(set == 1)//设置游戏难度
				{
					printf("*******************************\n"//宽31
						   "(0)默认：10*10 - 10\n"
						   "(1)初级： 9*9  - 10\n"
						   "(2)中级：16*16 - 40\n"
						   "(3)高级：16*30 - 99\n"
						   "(4)顶级：42*88 - 715\n"
						   "(5)自定义**********\n"
						   "*******************************\n");
					printf("/set difficulty ");
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
						/*设置雷数*/
						printf("请输入雷数. . . \n>");
						scanf("%d", &temp);
						if(temp < 0)
						{
							numberOfMine = 0;
						}
						else if(temp > LimHeight * LimWidth)
						{
							numberOfMine = LimHeight * LimWidth;
						}
						else
						{
							numberOfMine = temp;
						}
						printf("/set numberOfMine %d\n", numberOfMine);
						/*设置界面高度*/
						printf("请输入界面高度/行数（最大行坐标+1）. . . \n>");
						scanf("%d", &temp);
						if(temp < 1)
						{
							heightOfBoard = 1;
						}
						else if(temp > LimHeight)
						{
							heightOfBoard = LimHeight;
						}
						else
						{
							heightOfBoard = temp;
						}
						printf("/set heightOfBoard %d\n", heightOfBoard);
						/*设置界面宽度*/
						printf("请输入界面宽度/列数（最大列坐标+1）. . . \n>");
						scanf("%d", &temp);
						if(temp < 1)
						{
							widthOfBoard = 1;
						}
						else if(temp > LimWidth)
						{
							widthOfBoard = LimWidth;
						}
						else
						{
							widthOfBoard = temp;
						}
						printf("/set widthOfBoard %d\n", widthOfBoard);
						/*界面校正*/
						if(widthOfBoard == LimWidth && heightOfBoard > 100)
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
						}
						/*雷数校正*/
						if(heightOfBoard * widthOfBoard < numberOfMine)
						{
							printf(":(\n界面不足以容纳当前雷数！\n");
							numberOfMine = heightOfBoard * widthOfBoard;
							printf("/set numberOfMine %d\n", numberOfMine);
						}
						/*更新密度*/
						density =(float) numberOfMine / heightOfBoard / widthOfBoard;
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
						}
					}
				}
				else if(set == 2)//设置地图生成校验
				{
					printf("*******************************\n"//宽31
						   "(0)关闭校验\n"
						   "(1)起始点必非雷\n"
						   "(2)起始点必为空\n"
						   "(3)地图可解（生成时间可能较长）\n"
						   "*******************************\n");
					printf("当前:%d\n", summonCheckMode);
					printf("/set summonCheckMode ");
					scanf("%d", &summonCheckMode);
				}
				else if(set == 3)//更改游戏结束是否显示用时
				{
					if(showTime == 0)
					{
						showTime = 1;
					}
					else
					{
						showTime = 0;
					}
				}
				else if(set == 4)//设置默认操作模式
				{
					printf("*******************************\n");
					printf("@#rc操作模式：\n");
					printf("输入[@:翻开/#:标记] [r:行坐标] [c:列坐标]，\n");
					printf("并按回车后执行，如@ 0 0指翻开(0,0)。\n");
					printf("wasd23操作模式：\n");
					printf("通过wasd移动光标，按2翻开，按3标记，\n");
					printf("无小键盘可用hu代替23。\n");
					printf("游戏时将根据按键自动切换操作模式！\n");
					printf("*******************************\n");
					printf("当前默认操作模式：");
					if(operateMode == 0) printf("@#rc\n");
					if(operateMode == 1) printf("wasd23\n");
					getchar();
					printf("按任意键切换. . . ");
					getchar();
					if(operateMode == 0)
					{
						operateMode = 1;
					}
					else
					{
						operateMode = 0;
					}
				}
				else if(set == 5)//调试选项
				{
					printf("*******************************\n"//宽31
						   "调试选项仅提供给专业玩家和开发者使用。\n"
						   "除非你知道你在做什么，否则请退出设置。\n"
						   "(1)退出\n"
						   "(2)设置求解模式\n"
						   "(3)以密度设置雷数\n");
					if(backgroundColor == 0x07) printf("(4)浅色模式\n");
					if(backgroundColor == 0xf0) printf("(4)深色模式\n");
					printf("(5)设置刷新周期\n");
					if(fastMove == 0) printf("(6)启用大写WASD移动5格\n");
					if(fastMove == 1) printf("(6)关闭大写WASD移动5格\n");
					if(fastSign == 0) printf("(7)启用#数字一键标记周围\n");
					if(fastSign == 1) printf("(7)关闭#数字一键标记周围\n");
					if(newCursor == 0) printf("(8)启用><追踪光标位置\n");
					if(newCursor == 1) printf("(8)关闭><追踪光标位置\n");
					printf("*******************************\n");
					getchar();
					printf(">");
					operation = getchar();
					if(operation == '$')//调试
					{
						scanf("%d", &set);
						if(set == debug)
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
						scanf("%d", &set);
						if(set == 1)
						{
							printf("*******************************\n"//宽31
								   "(1)游戏模式：先不枚举，无解则枚举\n"
								   "(2)分析模式：始终以设置的限制枚举\n"
								   "*******************************\n");
							printf("/set solveMode ");
							scanf("%d", &set);
							if(set==1 || set==2) solveMode=set;
						}
						else if(set == 2)
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
							scanf("%d", &set);
							if(set>=0 && set<=30) lengthOfThinkChain=set;
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
					else if(operation == '4')//浅色模式/深色模式
					{
						if(backgroundColor == 0x07)
						{
							system("color f0");
							backgroundColor = 0xf0;
						}
						else if(backgroundColor == 0xf0)
						{
							system("color 07");
							backgroundColor = 0x07;
						}
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
					else if(operation == '6')
					{
						if(fastMove == 0)
						{
							fastMove = 1;
						}
						else
						{
							fastMove = 0;
						}
					}
					else if(operation == '7')
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
					else if(operation == '8')
					{
						if(newCursor == 0)
						{
							newCursor = 1;
						}
						else
						{
							newCursor = 0;
						}
					}
				}
				else//返回菜单
				{
					break;
				}
			}
		}
		/*--Bench--*/
		else if(choiceMode == 4)
		{
			system("cls");//清屏
			/*初始化*/
			isBench=1;
			numberOfAbandonedThinkChain=0;
			r0 = heightOfBoard/2;
			c0 = widthOfBoard/2;
			countOfWin=0;
			countOfSolution=0;
			countOfStep=0;
			seedMin=0;
			seedMax=99;
			for(seed=seedMin; seed<=seedMax; seed++)
			{
				isWinningOfSeed[seed]=0;
			}
			if(summonCheckMode != 2)//锁定校验
			{
				summonCheckMode=2;
				printf("/set summonCheckMode 2\n");
				system("pause");
			}
			t0=time(0);//计时
			/*计算覆写溢出字符所需空格数*/
			cs0 = Place(heightOfBoard-1)+1;//行坐标占用字符数
			widthOfMapShown = 2*widthOfBoard + cs0;//计算包含坐标的每行字符数不加回车
			/*开跑！*/
			for(seed=seedMin; seed<=seedMax; seed++)
			{
				srand(seed);
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
					SummonBoard(r0,c0);
					debug = 2;
				}
				else
				{
					SummonBoard(r0,c0);
				}
				isShown[r0][c0] = 1;
				OpenZeroChain(r0,c0);//第一次打开0链
				while(1)
				{
					//system("cls");
					gotoxy(0,0);//覆写
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
					ShowAnswer(Solve());
					debug=temp;
					if(debug == 1 || debug == 2)//Bench仅显示方案矩阵
					{
						/*printf("[Debug]已生成方案矩阵：");
						for(c=0; c<widthOfMapShown-23; c++)
						{
							printf(" ");//预置空格覆写Loading下移时原列坐标
						}
						printf("\n    ");
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
						for(c=0; c<widthOfMapShown; c++)
						{
							printf(" ");//预置空格覆写Loading时溢出行
						}
						printf("\n");*/
						for(r=0; r<heightOfBoard; r++)//在原地图显示方案矩阵
						{
							for(c=0; c<widthOfBoard; c++)
							{
								if(solution[r][c] == 1)
								{
									gotoxy(cs0+2*c, heightOfMapShown-1+4-heightOfBoard+1+r);
									if(backgroundColor == 0x07) ColorStr("@", 0x0e);
									if(backgroundColor == 0xf0) ColorStr("@", 0xe4);
								}
								else if(solution[r][c] == 2)
								{
									gotoxy(cs0+2*c, heightOfMapShown-1+4-heightOfBoard+1+r);
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
			isBench=0;
			system("cls");
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
			system("pause");
		}
		/*--退出--*/
		else if(choiceMode == 5)
		{
			break;
		}
		else
		{
			printf(":(\n"
				   "你的设备遇到问题，需要重启。\n"
				   "我们只收集某些错误信息，然后你可以重新启动。\n\n");
			for(c=0; c<=10; c++)
			{
				printf("\r%3d%% 完成", c*10);
				Sleep(refreshCycle);
			}
			Sleep(4*refreshCycle);
			printf("\n");
			getchar();
			system("cls");
		}
	}
	return 0;
}

void SummonBoard(int r0,int c0)//生成后台总板
{
	int r, c, i, ra, ca;
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
		if(debug == 2)
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
		}
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
		if(debug == 2)
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
		}
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
		if(debug == 2)
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
		}
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
		if(backgroundColor == 0x07)//深色模式
		{
			for(c=0; c<widthOfBoard; c++)
			{
				if(mode == 1)//后台
				{
					if(board[r][c] == 0)
					{
						printf("  ");
					}
					else if(board[r][c] == 9)
					{
						if(isShown[r][c] == 1)
						{
							//printf("@ ");
							ColorStr("@ ", 0x04);
						}
						else if(isShown[r][c] == 2)
						{
							//printf("# ");
							ColorStr("# ", 0x0c);
						}
						else
						{
							//printf("* ");
							ColorStr("* ", 0x0c);
						}
					}
					else
					{
						//printf("%d ", board[r][c]);
						if(board[r][c] == 1)
						{
							ColorStr("1 ", 0x01);
						}
						else if(board[r][c] == 2)
						{
							ColorStr("2 ", 0x02);
						}
						else if(board[r][c] == 3)
						{
							ColorStr("3 ", 0x04);
						}
						else if(board[r][c] == 4)
						{
							ColorStr("4 ", 0x04);
						}
						else if(board[r][c] == 5)
						{
							ColorStr("5 ", 0x04);
						}
						else if(board[r][c] == 6)
						{
							ColorStr("6 ", 0x0e);
						}
						else if(board[r][c] == 7)
						{
							ColorStr("7 ", 0x0e);
						}
						else if(board[r][c] == 8)
						{
							ColorStr("8 ", 0x0e);
						}
					}
				}
				else if(mode == 0)//前台
				{
					if(isShown[r][c] == 1)
					{
						if(board[r][c] == 0)
						{
							printf("  ");
						}
						else
						{
							//printf("%d ", board[r][c]);
							if(board[r][c] == 1)
							{
								ColorStr("1 ", 0x01);
							}
							else if(board[r][c] == 2)
							{
								ColorStr("2 ", 0x02);
							}
							else if(board[r][c] == 3)
							{
								ColorStr("3 ", 0x04);
							}
							else if(board[r][c] == 4)
							{
								ColorStr("4 ", 0x04);
							}
							else if(board[r][c] == 5)
							{
								ColorStr("5 ", 0x04);
							}
							else if(board[r][c] == 6)
							{
								ColorStr("6 ", 0x0e);
							}
							else if(board[r][c] == 7)
							{
								ColorStr("7 ", 0x0e);
							}
							else if(board[r][c] == 8)
							{
								ColorStr("8 ", 0x0e);
							}
						}
					}
					else if(isShown[r][c] == 2)
					{
						//printf("# ");
						ColorStr("# ", 0x0c);
					}
					else
					{
						printf("%% ");
					}
				}	
			}
			printf("\n");
		}
		else if(backgroundColor == 0xf0)//浅色模式
		{
			for(c=0; c<widthOfBoard; c++)
			{
				if(mode == 1)//后台
				{
					if(board[r][c] == 0)
					{
						printf("  ");
					}
					else if(board[r][c] == 9)
					{
						if(isShown[r][c] == 1)
						{
							//printf("@ ");
							ColorStr("@ ", 0xf4);
						}
						else if(isShown[r][c] == 2)
						{
							//printf("# ");
							ColorStr("# ", 0xfc);
						}
						else
						{
							//printf("* ");
							ColorStr("* ", 0xfc);
						}
					}
					else
					{
						//printf("%d ", board[r][c]);
						if(board[r][c] == 1)
						{
							ColorStr("1 ", 0xf1);
						}
						else if(board[r][c] == 2)
						{
							ColorStr("2 ", 0xf2);
						}
						else if(board[r][c] == 3)
						{
							ColorStr("3 ", 0xf4);
						}
						else if(board[r][c] == 4)
						{
							ColorStr("4 ", 0xf4);
						}
						else if(board[r][c] == 5)
						{
							ColorStr("5 ", 0xf4);
						}
						else if(board[r][c] == 6)
						{
							ColorStr("6 ", 0xf4);
						}
						else if(board[r][c] == 7)
						{
							ColorStr("7 ", 0xf4);
						}
						else if(board[r][c] == 8)
						{
							ColorStr("8 ", 0xf4);
						}
					}
				}
				else if(mode == 0)//前台
				{
					if(isShown[r][c] == 1)
					{
						if(board[r][c] == 0)
						{
							printf("  ");
						}
						else
						{
							//printf("%d ", board[r][c]);
							if(board[r][c] == 1)
							{
								ColorStr("1 ", 0xf1);
							}
							else if(board[r][c] == 2)
							{
								ColorStr("2 ", 0xf2);
							}
							else if(board[r][c] == 3)
							{
								ColorStr("3 ", 0xf4);
							}
							else if(board[r][c] == 4)
							{
								ColorStr("4 ", 0xf4);
							}
							else if(board[r][c] == 5)
							{
								ColorStr("5 ", 0xf4);
							}
							else if(board[r][c] == 6)
							{
								ColorStr("6", 0xe4);
								printf(" ");
							}
							else if(board[r][c] == 7)
							{
								ColorStr("7", 0xe4);
								printf(" ");
							}
							else if(board[r][c] == 8)
							{
								ColorStr("8", 0xe4);
								printf(" ");
							}
						}
					}
					else if(isShown[r][c] == 2)
					{
						//printf("# ");
						ColorStr("#", 0x7c);
						printf(" ");
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
}

void OpenZeroChain(int r0,int c0)//翻开0连锁翻开
{
	int r, c;
	if(isShown[r0][c0] == 1 && board[r0][c0] == 0)
	{
		SummonZeroChain(r0,c0);
		for(r=0; r<heightOfBoard; r++)//周围有0链则翻开
		{
			for(c=0; c<widthOfBoard; c++)
			{
				if(isShown[r][c] == 0)
				{
					if(IsAroundZeroChain(r,c) == 1)//在0链上或与0链连接
					{
						isShown[r][c] = 1;
					}
				}
			}
		}
	}
}

void SummonZeroChain(int r0,int c0)//对0链进行标记
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
					if(IsAroundZeroChain(r,c) == 1)//与0链连接
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
	remainder=0;
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
	n=0;
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
	n=0;
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

int NumberOfNotShownAround(int r0,int c0)
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
				if(isShown[r][c] == 0)
				{
					n++;
				}
			}
		}
	}
	return n;
}

int NumberOfSignAround(int r0,int c0)
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
				if(isShown[r][c] == 2)
				{
					n++;
				}
			}
		}
	}
	return n;
}

int IsAroundZeroChain(int r0,int c0)
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

void TranslateMap(int rs0,int cs0)//将输入翻译为地图、显示方式矩阵、已知数字
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
	if(debug == 2)
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
	}
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
	if(debug == 2)
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
	}
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
			if(isBench == 1)
			{
				printf("        ");//8空格覆写[案矩阵：]
			}
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

int NumberOfNumberAround(int r0,int c0)//周围已显示数字的量，用来确定交界线
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

void ShowAnswer(int isFoundOpen)//输出一个方案
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
		if(isBench == 1)
		{
			isShown[rc][cc] = 1;//翻开不确定点
		}
		else
		{
			printf(":)\n>@ %d %d\n", rc, cc);
		}
	}
	else if(solution[rc][cc] == 2 && isBench == 0)
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
//——修复 雷率为-0和负数时不视为确定解
--------------------------------*/
