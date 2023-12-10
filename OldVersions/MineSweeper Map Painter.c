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
#define LimName 32

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
int IsPosInRectangle(int x, int y, int x1, int y1, int x2, int y2);
int Place(int n);
void PrintCell(int board, int isShown, int bkcolor);
void** MatrixMemory(void** matrix, int rm, int cm, int sizeOfElements, int mode);
void gotoxy(int x, int y);
void ColorStr(char* content, int color);
void SetConsoleMouseMode(int mode);

// 功能函数
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
	printf("[Map Editer]已读取地图%d张\n", numberOfMaps);
	for(i=0; i<numberOfMaps; i++)
	{
		printf("地图%d：\n", i);
		PrintMap(maps.map[i], 1);
	}
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
	if(file = fopen("minesweeper-maps.txt", "r"))
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

struct Map EditMap(struct Map map)
{
	int r, c;
	int showNumber = 1;
	int rs0 = Place(map.width-1);
	int cs0 = Place(map.height-1) + 1;
	HANDLE hdin = GetStdHandle(STD_INPUT_HANDLE);
	COORD mousePos = {0, 0};
	COORD mouseOperatedPos = {0, 0};
	INPUT_RECORD rcd;
	DWORD rcdnum;
	int isReadyRefreshMouseOperatedPos = 0;
	system("cls");
	SetConsoleMouseMode(1);
	while(1)
	{
		gotoxy(0, 0);
		PrintMap(map, showNumber);
		/*printf("[r] [c]>");
		scanf("%d%d", &r, &c);
		if(r == -1 || c == -1) break;*/
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
		if(r != -1 && c != -1)
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
		if(r == -1 && c == 0) break;
	}
	SetConsoleMouseMode(0);
	return map;
}

// 主函数
int main()
{
	int choice = 0, i;
	struct Maps maps = ReadMaps();
	struct Map newMap;
	while(1)
	{
		system("cls");
		PrintMaps(maps);
		printf("[Map Editer]\n");
		printf("1.新建地图\n");
		printf("2.删除地图\n");
		printf("3.重命名地图\n");
		printf("4.编辑地图\n");
		printf("5.删除所有\n");
		printf("6.保存并退出\n");
		printf("7.重新读取\n");
		printf("8.直接退出\n");
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
		else if(choice == 2)//删除地图
		{
			printf("[序号]>");
			scanf("%d", &i);
			maps = DeleteMap(maps, i);
		}
		else if(choice == 3)//重命名地图
		{
			printf("[序号]>");
			scanf("%d", &i);
			printf("[name]>");
			getchar();
			//gets(maps.map[i].name);
			fgets(maps.map[i].name, LimName, stdin);
		}
		else if(choice == 4)//编辑地图
		{
			printf("[序号]>");
			scanf("%d", &i);
			maps.map[i] = EditMap(maps.map[i]);
		}
		else if(choice == 5)//删除所有
		{
			/*printf("[序号]>");//复制地图
			scanf("%d", &i);
			maps = AddMaps(maps, maps.map[i]);*/
			for(i=0; i<maps.numberOfMaps; i++)
			{
				maps = DeleteMap(maps, i);
			}
			free(maps.map);
			maps.numberOfMaps = 0;
		}
		else if(choice == 6)
		{
			WriteMaps(maps);
			break;
		}
		else if(choice == 7)
		{
			for(i=0; i<maps.numberOfMaps; i++)
			{
				maps = DeleteMap(maps, i);
			}
			free(maps.map);
			maps.numberOfMaps = 0;
			maps = ReadMaps();
		}
		else if(choice == 8)
		{
			break;
		}
	}
	
	return 0;
}

// 引用函数定义区
void gotoxy(int x, int y)//以覆写代替清屏
{
	COORD coord = {x, y};
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void ColorStr(char* content, int color)//输出彩色字符
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
