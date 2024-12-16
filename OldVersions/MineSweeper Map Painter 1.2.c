#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
/**
 * 扫雷地图绘制器 MineSweeper Map Painter
 * 
 * by Ltabsyy
 * 
 * https://github.com/Ltabsyy/MineSweeper
 **/
#define LimName 32//限制自制地图名称长度

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

// 引用MineSweeper Run
int backgroundColor = 0x07;
int operateMode = 2;
int fastShow = 1;
int visibleCursor = 0;
void gotoxy(short int x, short int y);
void clrscr();
void setbgcolor(int color);
void showCursor(int visible);
void ColorStr(const char* content, int color);
void SetConsoleMouseMode(int mode);
int IsPosInRectangle(int x, int y, int x1, int y1, int x2, int y2);
int Place(int n);
void PrintCell(int board, int isShown, int bkcolor);
void** MatrixMemory(void** matrix, int rm, int cm, int sizeOfElements, int mode);
void Error();

// 与MineSweeper Run相同功能函数
void PrintMap(struct Map map, int showNumber);
void PrintMaps(struct Maps maps);
struct Maps ReadMaps();
void WriteMaps(struct Maps maps);
struct Maps AddMaps(struct Maps maps, struct Map newMap);
struct Maps DeleteMap(struct Maps maps, int deleteNumber);
struct Maps DeleteMaps(struct Maps maps);

// 独立功能函数
struct Map EditMap(struct Map map)
{
	int r, c, ra, ca;
	int showNumber = 1;
	int** boardBuf;
	// 鼠标点击操作
	int rs0 = Place(map.width-1);
	int cs0 = Place(map.height-1) + 1;
	HANDLE hdin = GetStdHandle(STD_INPUT_HANDLE);
	COORD mousePos = {0, 0};
	COORD mouseOperatedPos = {0, 0};
	INPUT_RECORD rcd;
	DWORD rcdnum;
	int isReadyRefreshMouseOperatedPos = 0;
	if(fastShow == 1) boardBuf =(int**) MatrixMemory((void**)boardBuf, map.height, map.width, sizeof(int), 1);
	if(operateMode >= 2) SetConsoleMouseMode(1);
	clrscr();
	showCursor(visibleCursor);
	if(fastShow == 1)//快速显示准备
	{
		PrintMap(map, showNumber);
		for(r=0; r<map.height; r++)
		{
			for(c=0; c<map.width; c++)
			{
				boardBuf[r][c] = map.board[r][c];
			}
		}
	}
	while(1)
	{
		gotoxy(0, 0);
		if(fastShow == 0) PrintMap(map, showNumber);
		else//快速显示
		{
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
			//仅输出雷数和地图
			gotoxy(0, 2);
			printf("Map:%d*%d-%d ", map.height, map.width, map.numberOfMine);//雷数减少时覆写尾部
			for(r=0; r<map.height; r++)
			{
				for(c=0; c<map.width; c++)
				{
					if(map.board[r][c] != boardBuf[r][c])
					{
						gotoxy(cs0+2*c, 3+rs0+r);//yOfMap=3
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
						boardBuf[r][c] = map.board[r][c];
					}
				}
			}
			//定位到地图尾部
			gotoxy(0, 3+rs0+map.height);
		}
		if(operateMode > 1)
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
	if(operateMode >= 2) SetConsoleMouseMode(0);
	else fflush(stdin);
	showCursor(1);
	if(fastShow == 1) boardBuf =(int**) MatrixMemory((void**)boardBuf, map.height, map.width, sizeof(int), 0);
	return map;
}

// 主函数
int main()
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
		printf("(7)重新读取 \n");//(8)进入游戏
		printf("(9)保存并退出 (0)直接退出\n");
		printf("*******************************\n");//宽31
		choice = -1;
		printf(">");
		showCursor(1);
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
			/*printf("[序号]>");
			scanf("%d", &i);
			if(i < 0 || i >= maps.numberOfMaps) continue;
			//读取地图
			numberOfMine = maps.map[i].numberOfMine;
			heightOfBoard = maps.map[i].height;
			widthOfBoard = maps.map[i].width;
			ReallocMemory(heightOfBoard, widthOfBoard, dictionaryCapacity, lengthOfThinkMineCheck);
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
			return 1;//返回并进入游戏*/
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

// 引用函数定义区
void gotoxy(short int x, short int y)//以覆写代替清屏
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

void ColorStr(const char* content, int color)//输出彩色字符
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
	printf("%s", content);
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
			ColorStr("@", 0x04 + bkcolor);
			printf(" ");
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
			ColorStr("6", 0x04 + bkcolor);
			printf(" ");
		}
		else if(board == 7)
		{
			ColorStr("7", 0x04 + bkcolor);
			printf(" ");
		}
		else if(board == 8)
		{
			ColorStr("8", 0x04 + bkcolor);
			printf(" ");
		}
	}
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

void Error()//错误
{
	int c;
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
	fflush(stdin);//清空输入缓冲区，多字符仅蓝屏一次(doge)
	setbgcolor(backgroundColor);
	clrscr();
}

void PrintMap(struct Map map, int showNumber)
{
	int r, c, ra, ca, i, j, n;
	printf("name:%s", map.name);
	printf("author:%s", map.author);
	printf("Map:%d*%d-%d \n", map.height, map.width, map.numberOfMine);//雷数减少时覆写尾部
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
	struct Map* map;
	struct Map deleteMap;//删除最后一个地图时，map不存在暂存空间
	if(deleteNumber >= 0 && deleteNumber < maps.numberOfMaps)
	{
		map =(struct Map*) calloc(numberOfMaps, sizeof(struct Map));
		for(i=0; i<deleteNumber; i++)
		{
			map[i] = maps.map[i];
		}
		//释放删除位置空间
		deleteMap = maps.map[deleteNumber];
		free(deleteMap.name);
		free(deleteMap.author);
		deleteMap.isMine =(int**) MatrixMemory((void**)deleteMap.isMine, deleteMap.height, deleteMap.width, sizeof(int), 0);
		deleteMap.board =(int**) MatrixMemory((void**)deleteMap.board, deleteMap.height, deleteMap.width, sizeof(int), 0);
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
	struct Map map;
	//释放空间
	for(i=0; i<maps.numberOfMaps; i++)
	{
		//maps = DeleteMap(maps, i);//莫名直接退出时闪退，又莫名好了
		map = maps.map[i];
		free(map.name);
		free(map.author);
		map.isMine =(int**) MatrixMemory((void**)map.isMine, map.height, map.width, sizeof(int), 0);
		map.board =(int**) MatrixMemory((void**)map.board, map.height, map.width, sizeof(int), 0);
	}
	if(maps.numberOfMaps > 0) free(maps.map);
	maps.numberOfMaps = 0;
	return maps;
}

/*--------------------------------
更新日志：
MineSweeper Map Painter 1.1
——优化 功能与函数跟随Run 4.3升级
MineSweeper Map Painter 1.2
——优化 功能与函数跟随Run 5.12升级
--------------------------------*/
