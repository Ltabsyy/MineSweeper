#include <stdio.h>
#include <stdlib.h>

struct Record
{
	int numberOfMine;
	int heightOfBoard, widthOfBoard;
	int summonCheckMode;
	int seed, r0, c0;
	int time;
	int solved3BV, total3BV;
	int isHelped;
};
struct Records
{
	struct Record* record;
	int numberOfRecords;
};
void PrintRecords(struct Records records, int mode);
struct Records ReadRecords();
void WriteRecords(struct Records records);

int main()
{
	printf("MineSweeper Run Records Updater\n"
		   "该程序将旧版扫雷历史记录文件更新到新版。\n"
		   "旧版：minesweeper-records.txt文件，用于游戏版本Run 3.4 - Run 5.14\n"
		   "新版：minesweeper-records.csv文件，用于游戏版本Run 5.15起\n"
		   "请确保旧版文件在此程序的同一目录，按回车将开始读取。\n");
	getchar();
	struct Records records = ReadRecords();
	PrintRecords(records, 0);
	printf("按回车将开始写入，若已存在新版文件，将被覆盖。\n");
	getchar();
	WriteRecords(records);
	printf("程序执行完毕，按回车退出。\n");
	getchar();
	return 0;
}

void PrintRecords(struct Records records, int mode)
{
	struct Record* record = records.record;
	int numberOfRecords = records.numberOfRecords, i = 0;
	printf("**************************************************************\n");//宽62
	if(mode == 0 && numberOfRecords > 1024) i = numberOfRecords-1024;
	for(; i<numberOfRecords; i++)
	{
		printf("记录%d:", i);
		printf("Map:%d*%d-%d ", record[i].heightOfBoard, record[i].widthOfBoard, record[i].numberOfMine);
		printf("seed=%d,%d,%d ", record[i].seed, record[i].r0, record[i].c0);
		printf("summonCheckMode=%d ", record[i].summonCheckMode);
		printf("time=%d ", record[i].time);
		printf("3BV:%d/%d ", record[i].solved3BV, record[i].total3BV);
		printf("isHelped=%d\n", record[i].isHelped);
	}
	printf("**************************************************************\n");
	printf("* 已读取记录%d条\n", numberOfRecords);
	printf("*******************************\n");
}

struct Records ReadRecords()
{
	FILE* file;
	struct Records records;
	struct Record* record;
	int numberOfRecords, i;
	//初始化
	records.numberOfRecords = 0;
	if((file = fopen("minesweeper-records.txt", "r")))
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
		}
		fclose(file);
		records.numberOfRecords = numberOfRecords;
		records.record = record;
	}
	return records;//值传递而不是指针传递
}

void WriteRecords(struct Records records)
{
	FILE* file;
	struct Record* record = records.record;
	int numberOfRecords = records.numberOfRecords, i;
	file = fopen("minesweeper-records.csv", "w");
	fprintf(file, "heightOfBoard,widthOfBoard,numberOfMine,");
	fprintf(file, "seed,r0,c0,summonMode,iterateMode,sTime,msTime,solved3BV,total3BV,isHelped");
	for(i=0; i<numberOfRecords; i++)
	{
		fprintf(file, "\n");
		fprintf(file, "%d,%d,%d,", record[i].heightOfBoard, record[i].widthOfBoard, record[i].numberOfMine);
		fprintf(file, "%d,%d,%d,", record[i].seed, record[i].r0, record[i].c0);
		fprintf(file, "%d,%d,", record[i].summonCheckMode, -2);
		fprintf(file, "%d,%d,", record[i].time, record[i].time*1000);
		fprintf(file, "%d,%d,", record[i].solved3BV, record[i].total3BV);
		fprintf(file, "%d", record[i].isHelped);
	}
	fclose(file);
	if(numberOfRecords == 0)//删除文件
	{
		remove("minesweeper-records.csv");
	}
}
