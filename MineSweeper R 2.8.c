//#define _CRT_SECURE_NO_WARNINGS//vs专属(doge)
#include <stdio.h>
#include <stdlib.h>
#include <time.h>//时间戳作种子生成随机数
/**
 * 扫雷 MineSweeper R
 *
 * by Jws & Ltabsyy
 **/
#define LimHeight 128//最大高度，限制行数
#define LimWidth 93//最大宽度，限制列数
/*#define LimDictionary 1024//字典大小
//内存占用至少为 53*H*W+128*D Byte，默认约0.7MB*/

// 地图生成和显示
void SummonBoard(int r0,int c0);
int Place(int n);
void ShowBoard(int mode);

// 后台计算
void OpenZeroChain(int r0,int c0);
void SummonZeroChain(int r0,int c0);
int RealRemainder();
int NumberOfNotShown();
//int NumberOfSign();
//int NumberOfNotShownAround(int r0,int c0);
int NumberOfSignAround(int r0,int c0);
int IsAroundZeroChain(int r0,int c0);

/*void LookMap();
void Solve();
int Think();
void LoadNotShownAround(int r0,int c0,int mode);
void SummonMineRateForNotShown();
int NumberOfNumberAround(int r0,int c0);*/

// 全局矩阵
int isMine[LimHeight][LimWidth]={0};
int numberOfMineAround[LimHeight][LimWidth]={0};
int board[LimHeight][LimWidth]={0};
int isShown[LimHeight][LimWidth]={0};
int zeroChain[LimHeight][LimWidth]={0};

/*char map[LimHeight][LimWidth]={0};
int isShownBak[LimHeight][LimWidth]={0};
int numberShown[LimHeight][LimWidth]={0};
int solution[LimHeight][LimWidth]={0};
int isThought[LimHeight][LimWidth]={0};
int numberTeam[LimHeight][LimWidth]={0};
int thinkChain[LimHeight][LimWidth]={0};
int possibility[32]={0};
int numberCheck[LimHeight][LimWidth]={0};
int dictionary[LimDictionary][32]={0};
float isMineRate[LimHeight][LimWidth]={0};*/

// 全局变量及初始化
int numberOfMine = 10;//雷数量
int heightOfBoard = 10;//界面高度
int widthOfBoard = 10;//界面宽度

// 是否调试(0:关 1:开，显示部分 2:开，显示全部)
int debug = 0;

int main()
{
	int choiceMode;//游戏功能的选择
	int seed, r0, c0, seedMin, seedMax;//地图生成
	int r, c, remainder, isOpenMine, ra, ca, t0, t1;
	char operateMode;
	int set, temp;//设置
	while(1)//main内循环防止变量重复定义
	{
		system("cls");//清屏
		if(debug == 2) printf("<Debug>\nHello! Administrator Jws or Ltabsyy!\n");
		printf("*******************************\n"//宽31
			   "**新游戏(1)**设置(2)**退出(3)**\n"
			   "*******************************\n");
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
			printf("选择坐标[x:行][y:列]\n");
			printf(">@ ");
			scanf("%d%d", &r0, &c0);//通过键盘获取第一次选择的坐标
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
			srand(seed);
			r = r0;
			c = c0;
			SummonBoard(r,c);
			isShown[r][c] = 1;
			t0 = time(0);
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
						//printf("用时：%d\n", t1-t0);
						system("pause");
						break;
					}
				}
				if(RealRemainder() == 0//正确标记所有雷则胜利
				 ||NumberOfNotShown() == remainder)//未翻开的都是雷则胜利
				{
					system("cls");
					ShowBoard(1);
					printf(":)\nYou Win!\n");
					t1 = time(0);
					//printf("用时：%d\n", t1-t0);
					system("pause");
					break;
				}
				/*显示*/
				system("cls");
				if(debug == 1 || debug == 2)
				{
					printf("<Debug>\n");
					printf("Map:%d*%d-%d\n", heightOfBoard, widthOfBoard, numberOfMine);
					printf("seed=%d,%d,%d\n", seed, r0, c0);
					ShowBoard(1);
					printf("RealRemainder=%d\n", RealRemainder());
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
					printf("NumberOfNotShown=%d\n", NumberOfNotShown());
					printf("\n");
				}
				ShowBoard(0);
				printf("剩余雷数: %d\n", remainder);//打印剩余雷数
				/*输入*/
				printf("选择模式与坐标(@:翻开/#:标记)\n[@/#] [x] [y]\n");
				while(1)
				{
					printf(">");
					getchar();//吸收回车防止显示未选择操作模式
					scanf("%c%d%d", &operateMode, &r, &c);
					if(operateMode == '@')
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
						break;
					}
					else if(operateMode == '#')
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
					else if(operateMode == '$')
					{
						if(debug == r)
						{
							debug = c;
							if(debug == 2)
							{
								printf("<Debug>\nWelcome! Administrator Jws or Ltabsyy!\n");
							}
							system("pause");
							break;
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
			while(1)
			{
				system("cls");
				printf("*******************************\n"//宽31
					   "********设置游戏难度(1)********\n"
					   "*******自定义设置雷数(2)*******\n"
					   "*****自定义设置界面大小(3)*****\n"
					   "**********返回菜单(4)**********\n"
					   "*******************************\n");
				printf("当前雷数:%d|当前界面大小:%d*%d\n", numberOfMine, heightOfBoard, widthOfBoard);
				scanf("%d", &set);
				if(set == 1)//设置游戏难度
				{
					int difficulty;
					system("cls");
					printf("*******************************\n"//宽31
						   "************初级(1)************\n"
						   "************中级(2)************\n"
						   "************高级(3)************\n"
						   "*******************************\n");
					scanf("%d", &difficulty);
					if(difficulty == 1)
					{
						numberOfMine = 10;
						heightOfBoard = 9;
						widthOfBoard = 9;
					}
					else if(difficulty == 2)
					{
						numberOfMine = 40;
						heightOfBoard = 16;
						widthOfBoard = 16;
					}
					else if(difficulty == 3)
					{
						numberOfMine = 99;
						heightOfBoard = 16;
						widthOfBoard = 30;
					}
					else//默认为10*10-10，比初级更简单(doge)
					{
						numberOfMine = 10;
						heightOfBoard = 10;
						widthOfBoard = 10;
					}
				}
				else if(set == 2)//自定义设置雷数
				{
					printf("请输入雷数：");
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
				}
				else if(set == 3)//自定义设置界面大小
				{
					while(1)
					{
						printf("请输入界面高度：");
						scanf("%d", &temp);
						if(temp < 1)
						{
							printf(":(\n界面高度过小！\n");
							continue;
						}
						else if(temp > LimHeight)
						{
							printf(":(\n界面高度过大！\n");
							continue;
						}
						heightOfBoard = temp;
						printf("请输入界面宽度：");
						scanf("%d", &temp);
						if(temp < 1)
						{
							printf(":(\n界面宽度过小！\n");
							continue;
						}
						else if(temp * heightOfBoard < numberOfMine)
						{
							printf(":(\n不足以容纳%d个雷！\n", numberOfMine);
							continue;
						}
						else if(temp == LimWidth && heightOfBoard > 100)
						{
							printf(":(\n不足以容纳行坐标轴！\n");
							continue;
						}
						else if(temp > LimWidth)
						{
							printf(":(\n界面宽度过大！\n");
							continue;
						}
						widthOfBoard = temp;
						break;
					}
				}
				else//返回菜单
				{
					if(numberOfMine > heightOfBoard * widthOfBoard)
					{
						numberOfMine = heightOfBoard * widthOfBoard;
					}//检查雷数能够被界面容纳
					break;
				}
			}
		}
		/*--退出--*/
		else if(choiceMode == 3)
		{
			break;
		}
		else
		{
			printf(":(\n"
				   "你的设备遇到问题，需要重启。\n"
				   "我们只收集某些错误信息，然后你可以重新启动。\n\n");
			for(c=0; c<=4; c++)
			{
				printf("\r%3d%% 完成", c*25);
				sleep(1);
			}
			printf("\n");
			getchar();
			system("cls");
		}
	}
	return 0;
}

void SummonBoard(int r0,int c0)//生成后台总板
{
	int r, c, i;
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
			//system("pause");
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
					numberOfMineAround[r-1][c-1]++;
					numberOfMineAround[r-1][c]++;
					numberOfMineAround[r-1][c+1]++;
					numberOfMineAround[r][c-1]++;
					numberOfMineAround[r][c+1]++;
					numberOfMineAround[r+1][c-1]++;
					numberOfMineAround[r+1][c]++;
					numberOfMineAround[r+1][c+1]++;
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
			//system("pause");
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
	if(debug == 2)
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
	}
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
/*
int NumberOfSign()//标记的数量(#)，暂时无用
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

int NumberOfNotShownAround(int r0,int c0)//暂时无用
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
*/
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

/*--------------------------------
更新日志：
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
MineSweeper R 2.4
——优化 部分功能用函数表示
——优化 全局Debug现在分级调整
MineSweeper R 2.5
——新增 Debug支持更改种子
——优化 函数跟随Solution 1.2升级
——修复 @边缘使0链重复生成导致翻开越界，雷显示为9
MineSweeper R 2.6
——优化 支持游戏时Debug
——修复 0链不再导致越界翻开
MineSweeper R 2.7
——新增 Debug时可搜索地图
——修复 Debug时输入瞬间界面闪烁
MineSweeper R 2.8
//——新增 显示用时
——优化 代码效率
——优化 游戏结束后现在直接返回菜单
——优化 函数跟随Solution 2.4升级
——修复 在首页输入非数字字符无限重启
--------------------------------*/
