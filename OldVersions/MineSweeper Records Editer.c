#include <stdio.h>
#include <stdlib.h>
#include <time.h>
/**
 * 扫雷记录编辑器 MineSweeper Records Editer
 *
 * by Ltabsyy
 * 
 * https://github.com/Ltabsyy/MineSweeper
 **/
#define LimHeight 128//最大高度，限制行数
#define LimWidth 93//最大宽度，限制列数
//内存占用至少为 20*H*W，默认约0.23MB

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
	int minimumTime[5];//难度0-4
	float fastestSpeed[5];
};

struct Records ReadRecords();
void WriteRecords(struct Records records);
struct Records AddRecords(struct Records records, struct Record newRecord);
struct Records DeleteRecord(struct Records records, int deleteNumber);
void PrintRecords(struct Records records);

// 计算地图3BV
int isMine[LimHeight][LimWidth]={0};
int numberOfMineAround[LimHeight][LimWidth]={0};
int board[LimHeight][LimWidth]={0};
int isShown[LimHeight][LimWidth]={0};
int zeroChain[LimHeight][LimWidth]={0};
int numberOfMine;
int heightOfBoard;
int widthOfBoard;
int summonCheckMode;
void SummonBoard(int seed, int r0, int c0);
void OpenZeroChain(int r0, int c0);
void SummonZeroChain(int r0, int c0);
int IsAroundZeroChain(int r0, int c0);
int BBBV(int seed, int r0, int c0);

int main()
{
	int choice, i;
	struct Records records = ReadRecords();
	struct Record newRecord;
	while(1)
	{
		system("cls");
		PrintRecords(records);
		printf("[Records Editer]\n");
		printf("(1)删除一条记录\n");
		printf("(2)删除无效记录（未胜利记录、自定义难度记录、被实时求解指令帮助过的记录）\n");
		printf("(3)删除全部记录\n");
		printf("(4)保存并重新读取\n");
		printf("(5)保存并退出\n");
		printf("(6)不保存并重新读取\n");
		printf("(7)不保存并退出\n");
		printf("(8)插入一条记录\n");
		printf(">");
		scanf("%d", &choice);
		if(choice == 1)
		{
			printf("[序号]>");
			scanf("%d", &i);
			if(records.record[i].difficulty != 5
				&& records.record[i].isHelped == 0
				&& records.record[i].solved3BV == records.record[i].total3BV)//是有效记录
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
		else if(choice == 2)
		{
			for(i=records.numberOfRecords-1; i>=0; i--)
			{
				if(records.record[i].difficulty == 5
					|| records.record[i].isHelped == 1
					|| records.record[i].solved3BV != records.record[i].total3BV)
				{
					records = DeleteRecord(records, i);
				}
			}
		}
		else if(choice == 3)
		{
			records.numberOfRecords = 0;
			for(i=0; i<5; i++)
			{
				records.minimumTime[i] = -1;
				records.fastestSpeed[i] = 0;
			}
			free(records.record);
		}
		else if(choice == 4)
		{
			WriteRecords(records);
			records = ReadRecords();
		}
		else if(choice == 5)
		{
			WriteRecords(records);
			break;
		}
		else if(choice == 6)
		{
			records = ReadRecords();
		}
		else if(choice == 7)
		{
			break;
		}
		else if(choice == 8)
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
			newRecord.total3BV = BBBV(newRecord.seed, newRecord.r0, newRecord.c0);
			printf("当前地图3BV：%d\n", newRecord.total3BV);
			printf("[已解3BV]>");
			scanf("%d", &(newRecord.solved3BV));
			if(newRecord.solved3BV < 0) newRecord.solved3BV = 0;
			if(newRecord.solved3BV > newRecord.total3BV) newRecord.solved3BV = newRecord.total3BV;
			printf("是否被实时求解指令帮助\n");
			printf("[1/0]>");
			scanf("%d", &(newRecord.isHelped));
			if(newRecord.isHelped != 0) newRecord.isHelped = 1;
			newRecord.speed =(float) newRecord.solved3BV / newRecord.time;
			records = AddRecords(records, newRecord);
		}
	}
	return 0;
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
	if(file = fopen("minesweeper-records.txt", "r"))
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
			record[i].speed =(float) record[i].solved3BV / record[i].time;//计算3BV/s
			if(record[i].heightOfBoard == 10 && record[i].widthOfBoard == 10 && record[i].numberOfMine == 10)
			{
				record[i].difficulty = 0;
				if(record[i].isHelped == 0 && record[i].solved3BV == record[i].total3BV)
				{
					records.minimumTime[0] = record[i].time;
					records.fastestSpeed[0] = record[i].speed;
				}
			}
			else if(record[i].heightOfBoard == 9 && record[i].widthOfBoard == 9 && record[i].numberOfMine == 10)
			{
				record[i].difficulty = 1;
				if(record[i].isHelped == 0 && record[i].solved3BV == record[i].total3BV)
				{
					records.minimumTime[1] = record[i].time;
					records.fastestSpeed[1] = record[i].speed;
				}
			}
			else if(record[i].heightOfBoard == 16 && record[i].widthOfBoard == 16 && record[i].numberOfMine == 40)
			{
				record[i].difficulty = 2;
				if(record[i].isHelped == 0 && record[i].solved3BV == record[i].total3BV)
				{
					records.minimumTime[2] = record[i].time;
					records.fastestSpeed[2] = record[i].speed;
				}
			}
			else if(record[i].heightOfBoard == 16 && record[i].widthOfBoard == 30 && record[i].numberOfMine == 99)
			{
				record[i].difficulty = 3;
				if(record[i].isHelped == 0 && record[i].solved3BV == record[i].total3BV)
				{
					records.minimumTime[3] = record[i].time;
					records.fastestSpeed[3] = record[i].speed;
				}
			}
			else if(record[i].heightOfBoard == 42 && record[i].widthOfBoard == 88 && record[i].numberOfMine == 715)
			{
				record[i].difficulty = 4;
				if(record[i].isHelped == 0 && record[i].solved3BV == record[i].total3BV)
				{
					records.minimumTime[4] = record[i].time;
					records.fastestSpeed[4] = record[i].speed;
				}
			}
			else
			{
				record[i].difficulty = 5;
			}
		}
		fclose(file);
		records.numberOfRecords = numberOfRecords;
		records.record = record;
		//计算纪录
		for(i=0; i<numberOfRecords; i++)
		{
			if(record[i].difficulty != 5 && record[i].isHelped == 0 && record[i].solved3BV == record[i].total3BV)
			{
				if(record[i].time < records.minimumTime[record[i].difficulty])
				{
					records.minimumTime[record[i].difficulty] = record[i].time;
				}
				if(record[i].speed > records.fastestSpeed[record[i].difficulty])
				{
					records.fastestSpeed[record[i].difficulty] = record[i].speed;
				}
			}
		}
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

struct Records AddRecords(struct Records records, struct Record newRecord)
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
	free(records.record);
	records.record = record;
	records.numberOfRecords++;
	if(newRecord.difficulty != 5 && newRecord.isHelped == 0 && newRecord.solved3BV == newRecord.total3BV)
	{
		if(newRecord.time < records.minimumTime[newRecord.difficulty] || records.minimumTime[newRecord.difficulty] == -1)
		{
			records.minimumTime[newRecord.difficulty] = newRecord.time;
			printf("已更新%d难度时间纪录为%d\n", newRecord.difficulty, newRecord.time);
		}
		if(newRecord.speed > records.fastestSpeed[newRecord.difficulty])
		{
			records.fastestSpeed[newRecord.difficulty] = newRecord.speed;
			printf("已更新%d难度3BV/s纪录为%.2f\n", newRecord.difficulty, newRecord.speed);
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

void PrintRecords(struct Records records)
{
	struct Record* record = records.record;
	int numberOfRecords = records.numberOfRecords, i;
	printf("[Records Editer]已读取记录%d条\n", numberOfRecords);
	for(i=0; i<5; i++)
	{
		printf("%d难度时间纪录为%d 3BV/s纪录为%.2f\n", i, records.minimumTime[i], records.fastestSpeed[i]);
	}
	for(i=0; i<numberOfRecords; i++)
	{
		printf("记录%d:", i);
		printf("Map:%d*%d-%d ", record[i].heightOfBoard, record[i].widthOfBoard, record[i].numberOfMine);
		printf("seed=%d,%d,%d ", record[i].seed, record[i].r0, record[i].c0);
		printf("summonCheckMode=%d ", record[i].summonCheckMode);
		printf("time=%d ", record[i].time);
		printf("3BV:%d/%d ", record[i].solved3BV, record[i].total3BV);
		printf("isHelped=%d\n", record[i].isHelped);
	}
}

void SummonBoard(int seed, int r0, int c0)
{
	int r, c, i, ra, ca;
	srand(seed);
	while(1)
	{
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				isMine[r][c] = 0;
				numberOfMineAround[r][c] = 0;
				board[r][c] = 0;
			}
		}
		for(i=0; i<numberOfMine; )
		{
			r = rand() % heightOfBoard;
			c = rand() % widthOfBoard;
			if(isMine[r][c] == 0)
			{
				isMine[r][c] = 1;
				i++;
			}
		}
		if(isMine[r0][c0] == 1 && summonCheckMode > 0) continue;
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				if(isMine[r][c] == 1)
				{
					for(ra=r-1; ra<=r+1; ra++)
					{
						for(ca=c-1; ca<=c+1; ca++)
						{
							if(ra>=0 && ra<heightOfBoard
								&& ca>=0 && ca<widthOfBoard)
							{
								numberOfMineAround[ra][ca]++;
							}
						}
					}
				}
			}
		}
		if(numberOfMineAround[r0][c0] != 0 && summonCheckMode > 1) continue;
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				if(isMine[r][c] == 1)
				{
					board[r][c] = 9;
				}
				else
				{
					board[r][c] = numberOfMineAround[r][c];
				}
			}
		}
		break;
	}	
}

void OpenZeroChain(int r0, int c0)
{
	int r, c;
	if(isShown[r0][c0] == 1 && board[r0][c0] == 0)
	{
		SummonZeroChain(r0, c0);
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				if(isShown[r][c] == 0)
				{
					if(IsAroundZeroChain(r, c) == 1)
					{
						isShown[r][c] = 1;
					}
				}
			}
		}
	}
}

void SummonZeroChain(int r0, int c0)
{
	int r, c, isRising;
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			zeroChain[r][c] = 0;
		}
	}
	zeroChain[r0][c0] = 1;
	isRising = 1;
	while(isRising == 1)
	{
		isRising = 0;
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				if(board[r][c] == 0 && zeroChain[r][c] == 0)
				{
					if(IsAroundZeroChain(r, c) == 1)
					{
						zeroChain[r][c] = 1;
						isRising = 1;
					}
				}
			}
		}
	}
}

int IsAroundZeroChain(int r0, int c0)
{
	int r, c;
	for(r=r0-1; r<=r0+1; r++)
	{
		for(c=c0-1; c<=c0+1; c++)
		{
			if(r>=0 && r<heightOfBoard
			&& c>=0 && c<widthOfBoard)
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

int BBBV(int seed, int r0, int c0)//计算地图全部3BV
{
	int bbbv = 0, r, c;
	SummonBoard(seed, r0, c0);
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			isShown[r][c] = 0;
		}
	}
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
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(isShown[r][c] == 0 && board[r][c] != 9)
			{
				bbbv++;
			}
		}
	}
	return bbbv;
}
