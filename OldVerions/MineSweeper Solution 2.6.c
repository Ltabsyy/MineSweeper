//#define _CRT_SECURE_NO_WARNINGS//vs专属(doge)
#include <stdio.h>
#include <stdlib.h>
#include <time.h>//时间戳作种子生成随机数
/**
 * 扫雷解决器 MineSweeper Solution
 *
 * by Ltabsyy & Jws
 **/
#define LimHeight 128//最大高度，限制行数
#define LimWidth 93//最大宽度，限制列数
#define LimDictionary 1024//字典大小
//内存占用至少为 35*H*W+128*D Byte，默认约0.5MB

// 后台计算
void translateMap(int rs0,int cs0);
void Solve();
int Think();
void SummonMineRateForNotShown();
int NumberOfNotShownAround(int r0,int c0);
int NumberOfNumberAround(int r0,int c0);
int NumberOfSignAround(int r0,int c0);
int NumberOfNotShown();
int NumberOfSign();
int Place(int n);

// 输出处理
void ShowAnswer(int isFoundOpen);

// 全局矩阵
char mapShown[LimHeight+2][2*LimWidth+5]={0};
char map[LimHeight][LimWidth]={0};
int isShown[LimHeight][LimWidth]={0};
int numberShown[LimHeight][LimWidth]={0};
int solution[LimHeight][LimWidth]={0};

// 逻辑推理
int isThought[LimHeight][LimWidth]={0};
int numberTeam[LimHeight][LimWidth]={0};
int thinkChain[LimHeight][LimWidth]={0};
int possibility[32]={0};
int numberCheck[LimHeight][LimWidth]={0};
int dictionary[LimDictionary][32]={0};
float isMineRate[LimHeight][LimWidth]={0};

// 全局变量及初始化
int numberOfMine = 10;//雷数量
int heightOfBoard = 10;//界面高度
int widthOfBoard = 10;//界面宽度
int lengthOfThinkChain = 19;//未知链长度
int solveMode = 1;//1游戏模式，2分析模式

// 是否调试(0:关 1:开，显示部分 2:开，显示全部)
int debug = 0;

int main()
{
	int choiceMode, set;
	int heightOfMapShown, widthOfMapShown;
	int r, c, rs0, cs0;
	while(1)//main内循环防止变量重复定义
	{
		system("cls");//清屏
		if(debug == 2) printf("<Debug>\nHello! Administrator Ltabsyy or Jws!\n");
		printf("*******************************\n"//宽31
			   "**新游戏(1)**设置(2)**退出(3)**\n"
			   "*******************************\n");
		printf(">");
		scanf("%d", &choiceMode);
		/*--新游戏--*/
		if(choiceMode == 1)
		{
			system("cls");
			printf("*******************************\n"//宽31
				   "(0)默认：10*10 - 10\n"
				   "(1)初级： 9*9  - 10\n"
				   "(2)中级：16*16 - 40\n"
				   "(3)高级：16*30 - 99\n"
				   "(4)自定义**********\n"
				   "*******************************\n");
			printf("/set difficulty ");
			/*初始化*/
			scanf("%d", &set);
			if(set == 0)
			{
				numberOfMine = 10;
				heightOfBoard = 10;
				widthOfBoard = 10;
			}
			else if(set == 1)
			{
				numberOfMine = 10;
				heightOfBoard = 9;
				widthOfBoard = 9;
			}
			else if(set == 2)
			{
				numberOfMine = 40;
				heightOfBoard = 16;
				widthOfBoard = 16;
			}
			else if(set == 3)
			{
				numberOfMine = 99;
				heightOfBoard = 16;
				widthOfBoard = 30;
			}
			else
			{
				printf("请输入雷数. . . \n");
				printf("/set numberOfMine ");
				scanf("%d", &numberOfMine);
				printf("请输入行数（最大行坐标+1）. . . \n");
				printf("/set heightOfBoard ");
				scanf("%d", &heightOfBoard);
				printf("请输入列数（最大列坐标+1）. . . \n");
				printf("/set widthOfBoard ");
				scanf("%d", &widthOfBoard);
			}
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
			rs0 = Place(widthOfBoard-1);//行起始指针，为列坐标显示占用行数
			cs0 = Place(heightOfBoard-1)+1;//列起始指针，为行坐标显示占用列数
			heightOfMapShown = heightOfBoard + rs0;//计算包含坐标的行数
			widthOfMapShown = 2*widthOfBoard + cs0 + 1;//计算包含坐标的每行字符数加回车
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
				translateMap(rs0,cs0);
				/*扫雷策略和输出*/
				Solve();
			}
		}
		/*--设置--*/
		else if(choiceMode == 2)
		{
			while(1)
			{
				system("cls");
				printf("*******************************\n"//宽31
					   "(1)设置模式：游戏模式/分析模式\n"
					   "(2)自定义设置枚举限制\n"
					   "(3)调试选项\n"
					   "(4)返回菜单\n"
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
				else if(set == 3)
				{
					printf("*******************************\n"//宽31
						   "调试选项仅提供给专业玩家和开发者使用。\n"
						   "除非你知道你在做什么，否则请退出设置。\n"
						   "(1)退出\n"
						   "*******************************\n");
					getchar();
					printf(">");
					if(getchar() == '$')
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
						}
					}
				}
				else//返回菜单
				{
					break;
				}
			}
		}
		/*--退出--*/
		else if(choiceMode == 3)
		{
			break;
		}
	}
	return 0;
}

void translateMap(int rs0,int cs0)//将输入翻译为地图、显示方式矩阵、已知数字
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

void Solve()//程序核心部分(doge)
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
	ShowAnswer(isFoundOpen);
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

int NumberOfNumberAround(int r0,int c0)
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

void ShowAnswer(int isFoundOpen)
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
		printf(":)\n>@ %d %d\n", rc, cc);
	}
	else if(solution[rc][cc] == 2)
	{
		printf(":)\n># %d %d\n", rc, cc);
	}
}

/*--------------------------------
MineSweeper Solution 1.1
——优化 全局Debug现在分级调整
——修复 可能出现的过于激进的标记
MineSweeper Solution 1.2
——新增 根据判断出的雷进一步扩大方案矩阵
——优化 优先翻开以节省步数
MineSweeper Solution 1.3
——新增 根据判断出的非雷进一步扩大方案矩阵
MineSweeper Solution 1.4
——新增 Debug可输出显示方式矩阵更新和所有方案
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
MineSweeper Solution 2.6
——优化 上调游戏模式枚举效率长度
——优化 Debug可看出推理逻辑
——优化 重写答案显示算法
——优化 统一界面设计语言
——修复 返回菜单需要二次按`
--------------------------------*/
