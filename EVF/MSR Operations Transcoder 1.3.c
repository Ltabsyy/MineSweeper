#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// 操作记录
struct Operation
{
	int mstime;
	char operation;
	int r, c;
	struct Operation* next;//使用双向链表优化性能
	struct Operation* prev;
};
struct Operations
{
	int numberOfMine;
	int heightOfBoard, widthOfBoard;
	int summonMode, iterateMode;
	int seed, r0, c0;
	int time;//终局时整数时间，避免毫秒时间显示偏差
	int isWin;//胜局时显示3BV全解
	int r, c;//当前位置
	//int sideLength;
	struct Operation* head;//头节点，顺序存储便于回放
	struct Operation* tail;//尾节点，避免插入时搜索尾部
};
void AddOperation(struct Operations* operations, int mstime, char operation, int r, int c);
void ClearOperations(struct Operations operations);
struct Operations ReadOperations(char* fileName);

void SummonBoard(int seed, int r0, int c0, int summonMode, int iterateMode);//地图生成
int IsAroundZeroChain(int r0, int c0);
void OpenZeroChain(int r0, int c0);//翻开0连锁翻开
int NumberOfSignAround(int r0, int c0);
void ShownModeBak(int mode);//备份显示方式矩阵，1备份，0恢复
int BBBV(int mode);//Bechtel'sBoardBenchmarkValue，计算地图3BV

void WriteEVF(struct Operations operations);

#define LimHeight 255
#define LimWidth 255
int isMine[LimHeight][LimWidth]={0};
int numberOfMineAround[LimHeight][LimWidth]={0};
int board[LimHeight][LimWidth]={0};
int isShown[LimHeight][LimWidth]={0};
int isShownBak[LimHeight][LimWidth]={0};
int zeroChain[LimHeight][LimWidth]={0};

int numberOfMine = 10;//雷数量
int heightOfBoard = 10;//界面高度
int widthOfBoard = 10;//界面宽度
int summonCheckMode = 2;//0不校验，1非雷，2必空，3可解，4筛选

int main(int argc, char** argv)
{
	struct Operations operations;
	printf("MineSweeper Run Operations Transcoder\n"
			"该程序将扫雷操作记录文件转码到evf。\n");
	if(argc == 1)
	{
		operations = ReadOperations(NULL);
		WriteEVF(operations);
	}
	else
	{
		for(int i=1; i<argc; i++)
		{
			operations = ReadOperations(argv[i]);
			WriteEVF(operations);
			ClearOperations(operations);
		}
	}
	printf("转码完毕，按回车退出。");
	getchar();
	return 0;
}

void AddOperation(struct Operations* operations, int mstime, char operation, int r, int c)
{
	struct Operation* newNode =(struct Operation*) malloc(sizeof(struct Operation));
	if(operation == 0) operation = '?';//移动以?记录
	newNode->mstime = mstime;
	newNode->operation = operation;
	newNode->r = r;
	newNode->c = c;
	newNode->next = NULL;
	newNode->prev = operations->tail;
	if(operations->head == NULL) operations->head = newNode;
	if(operations->tail == NULL) operations->tail = newNode;
	else//NULL情况一般不可能发生因必存在第一次打开
	{
		if(operation == '?' && operations->r == r && operations->c == c)
		{//?方块定位操作与当前坐标相同
			free(newNode);
		}
		else if(operation == 'm' && operations->tail->operation == 'm'
				  && (operations->tail->mstime == mstime || (operations->tail->r == r && operations->tail->c == c)))
		{//连续鼠标且时间或坐标相同
			//不保存这一次
			free(newNode);
		}
		else
		{
			//保存这一次操作记录
			operations->tail->next = newNode;
			operations->tail = newNode;
			if(operation != 'm')
			{
				operations->r = r;
				operations->c = c;
			}
		}
	}
}

void ClearOperations(struct Operations operations)
{
	struct Operation* p;
	while(operations.head != NULL)
	{
		p = operations.head->next;
		free(operations.head);
		operations.head = p;
	}
}

char* InputFileName()
{
	char temp[256];
	int i, j;
	char* fileName =(char*) calloc(256, sizeof(char));
	printf("[File]>");
	fgets(temp, 256, stdin);//哼，回车
	j = 0;
	for(i=0; i<256; i++)
	{
		if(temp[i] == '\n') break;
		else if(temp[i] == '"');//文件名不能包含\/:*?"<>|
		else
		{
			fileName[j] = temp[i];
			j++;
		}
	}
	return fileName;
}

struct Operations ReadOperations(char* fileName)
{
	FILE* file;
	//char fileName[256] = "minesweeper-operations\\minesweeper-operations_16x30-99_137.651_1721448336.txt";
	struct Operations operations;
	int mstime, r, c;
	char operation;
	operations.head = NULL;
	operations.tail = NULL;
	if(fileName == NULL)
	{
		//输入文件名
		printf("输入含路径文件名或拖入文件\n");
		fileName = InputFileName();
	}
	if((file = fopen(fileName, "r")))
	{
		fscanf(file, "Map:%d*%d-%d,%d,%d,%d,%d,%d\n",
				 &(operations.heightOfBoard), &(operations.widthOfBoard), &(operations.numberOfMine),
				 &(operations.seed), &(operations.r0), &(operations.c0), &(operations.summonMode), &(operations.iterateMode));
		fscanf(file, "time=%d\n", &(operations.time));
		fscanf(file, "isWin=%d\n", &(operations.isWin));
		operations.r = operations.r0;
		operations.c = operations.c0;
		//fscanf(file, "sideLength=16\n");
		while(fscanf(file, "\n%d %c %d %d", &mstime, &operation, &r, &c) != EOF)
		{
			AddOperation(&operations, mstime, operation, r, c);
		}
		fclose(file);
	}
	return operations;
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
			/*for(r=rc1, c=cc1; c<=cc2; c++);
			for(r++, c--; r<=rc2; r++);
			for(r--, c--; c>=cc1; c--);
			for(r--, c++; r>rc1; r--);*///0链可能迂回
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

int BBBV(int mode)//计算地图3BV
{
	int bbbv = 0, r, c;
	ShownModeBak(1);
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(board[r][c] == 0 && isShown[r][c] != 1)
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
			if(board[r][c] != 9 && isShown[r][c] != 1)
			{
				//isShown[r][c] = 1;
				bbbv++;
			}
		}
	}
	ShownModeBak(0);
	return bbbv;
}
/*
int WriteBE(FILE* fp, void* pval, unsigned char bit)
{
	if(bit == 0) return 1;
	unsigned char byte = (bit+7)/8;
	unsigned char* buf =(unsigned char*) calloc(byte, 1);
	unsigned char* p =(unsigned char*) pval;
	for(int i=0; i<byte; i++)
	{
		buf[byte-1-i] = p[i];
	}
	int r = (fwrite(buf, 1, byte, fp) == byte);
	free(buf);
	free(p);
	return r;
}
*/
// 写入大端法 DeepSeek设计 Ltabsyy调整
int WriteBE_uint8(FILE *fp, uint8_t val)
{
	return fwrite(&val, 1, 1, fp) == 1;
}

int WriteBE_uint16(FILE *fp, uint16_t val)
{
	unsigned char buf[2];
	buf[0] = (val >> 8) & 0xFF; // 先写高字节，再写低字节
	buf[1] = val & 0xFF;
	return fwrite(buf, 1, 2, fp) == 2;
}

int WriteBE_uint32(FILE *fp, uint32_t val)
{
	unsigned char buf[4];
	buf[0] = (val >> 24) & 0xFF;
	buf[1] = (val >> 16) & 0xFF;
	buf[2] = (val >> 8) & 0xFF;
	buf[3] = val & 0xFF;
	return fwrite(buf, 1, 4, fp) == 4;
}

int WriteBE_uint64(FILE *fp, uint64_t val)
{
	unsigned char buf[8];
	buf[0] = (val >> 56) & 0xFF;
	buf[1] = (val >> 48) & 0xFF;
	buf[2] = (val >> 40) & 0xFF;
	buf[3] = (val >> 32) & 0xFF;
	buf[4] = (val >> 24) & 0xFF;
	buf[5] = (val >> 16) & 0xFF;
	buf[6] = (val >> 8) & 0xFF;
	buf[7] = val & 0xFF;
	return fwrite(buf, 1, 8, fp) == 8;
}

int WriteBE_int16(FILE *fp, int16_t val) // 按 uint16 处理
{
	// 将 int16_t 视为 uint16_t 写入，保持位模式
	return WriteBE_uint16(fp, (uint16_t)val);
}

void WriteEVF(struct Operations operations)
{
	FILE* file;
	char fileName[128];
	struct Operation* p;
	uint8_t val8;
	uint16_t val16;
	uint32_t val32;
	uint64_t val64;
	int16_t sval16;
	int bbbv;
	float mstime = (float)(operations.tail->mstime-operations.head->mstime)/1000;
	numberOfMine = operations.numberOfMine;
	heightOfBoard = operations.heightOfBoard;
	widthOfBoard = operations.widthOfBoard;
	summonCheckMode = operations.summonMode;
	SummonBoard(operations.seed, operations.r0, operations.c0, operations.summonMode, operations.iterateMode);
	bbbv = BBBV(1);
	sprintf(fileName, "msr_%dx%d-%d_%.3f_%d_%.3f_%d.evf",
			operations.heightOfBoard, operations.widthOfBoard, operations.numberOfMine,
			mstime, bbbv, (float)bbbv/mstime, operations.seed);
	file = fopen(fileName, "wb");
	//版本号
	fputc(4, file);
	//摘要
	val8 = 0x28;//公平转码
	if(operations.isWin == 1) val8 |= 0x80;//扫完
	int noflag = 1;
	for(p=operations.head; p!=NULL; p=p->next)
	{
		if(p->operation == '#') noflag = 0;
	}
	if(noflag) val8 |= 0x10;//盲扫
	WriteBE_uint8(file, val8);
	//游戏设置
	val8 = 0x00;
	WriteBE_uint8(file, val8);
	//行数
	if(operations.heightOfBoard > 255)
	{
		printf("行数过高！\n");
		return;
	}
	val8 = operations.heightOfBoard;
	WriteBE_uint8(file, val8);
	//列数
	if(operations.widthOfBoard > 255)
	{
		printf("列数过高！\n");
		return;
	}
	val8 = operations.widthOfBoard;
	WriteBE_uint8(file, val8);
	//雷数
	if(operations.numberOfMine > 32767)
	{
		printf("雷数过高！\n");
		return;
	}
	val16 = operations.numberOfMine;
	WriteBE_uint16(file, val16);
	//方格尺寸
	val8 = 64;
	WriteBE_uint8(file, val8);
	//游戏模式
	val16 = 4;//win7
	if(operations.summonMode <= 1) val16 = 1;//标准
	else if(operations.summonMode == 3) val16 = 5;//经典无猜
	WriteBE_uint16(file, val16);
	//3BV数值
	val16 = bbbv;
	WriteBE_uint16(file, val16);
	//用时
	val32 = operations.tail->mstime-operations.head->mstime;
	WriteBE_uint32(file, val32);
	//国家或地区的名称或代码
	val16 = ('C' << 8) + 'N';
	WriteBE_uint16(file, val16);
	//开始时间戳
	val64 =(uint64_t) operations.seed * 1000000;
	WriteBE_uint64(file, val64);
	//结束时间戳
	val64 +=(uint64_t) (operations.tail->mstime-operations.head->mstime)*1000;
	WriteBE_uint64(file, val64);
	//录像的最初来源
	fputs("MineSweeper Run 5.19+", file);
	fputc(0, file);
	//转码软件的名称
	fputs("MSR Operations Transcoder 1.3", file);
	fputc(0, file);
	//录像的用户标识、比赛标识、唯一性标识的可能的编码方式
	fputs("ascii", file);
	fputc(0, file);
	//用户标识
	char s[256]={0};
	printf("输入用户标识：");
	fgets(s, 256, stdin);
	//getchar();//fgets不会留回车在输入缓冲区
	for(int i=0; i<256; i++)
	{
		if(s[i] == '\n')
		{
			s[i] = 0;
			break;
		}
	}
	fputs(s, file);
	fputc(0, file);
	//比赛标识
	fputc(0, file);
	//唯一性标识
	fputc(0, file);
	//uuid 的占用字节数
	val16 = 0;//32
	WriteBE_uint16(file, val16);
	//for(int i=0; i<val16; i++) fputc('*', file);
	//局面
	int r, c, i = 7;
	val8 = 0;
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			val8 |= isMine[r][c] << i;
			i--;
			if(i == -1)
			{
				WriteBE_uint8(file, val8);
				val8 = 0;
				i = 7;
			}
		}
	}
	if(i != 7)
	{
		WriteBE_uint8(file, val8);
	}
	//自定义指标的数量
	val16 = 0;
	WriteBE_uint16(file, val16);
	//事件列表
	int t1 = 0, t2 = 0, x1 = 0, y1 = 0, x2, y2;
	int ra, ca, specialOpen;
	for(p=operations.head; p!=NULL; p=p->next)
	{
		if(p->operation == 'm')
		{
			x2 = p->c;
			y2 = p->r;//找第一个坐标
			break;
		}
	}
	if(y2/64 != operations.head->r) y2 = (operations.head->r)*64+32;
	if(x2/64 != operations.head->c) x2 = (operations.head->c)*64+32;
	val8 = 2;//按下
	WriteBE_uint8(file, val8);
	val8 = 0;
	WriteBE_uint8(file, val8);
	sval16 = x2;
	WriteBE_int16(file, sval16);
	sval16 = y2;
	WriteBE_int16(file, sval16);
	val8 = 3;//MSR玩家特有的瞬间按下再弹起
	WriteBE_uint8(file, val8);
	val8 = 0;
	WriteBE_uint8(file, val8);
	sval16 = 0;
	WriteBE_int16(file, sval16);
	sval16 = 0;
	WriteBE_int16(file, sval16);
	x1 = x2;
	y1 = y2;
	isShown[operations.head->r][operations.head->c] = 1;
	OpenZeroChain(operations.head->r, operations.head->c);
	for(; p!=NULL; p=p->next)
	{
		if(p->operation == 'm')
		{
			t2 = p->mstime;
			if(t2-t1 > 255)
			{
				while(t2-t1 > 65535)
				{
					val8 = 255;//停顿
					WriteBE_uint8(file, val8);
					val16 = 65535;
					WriteBE_uint16(file, val16);
					t2 -= 65535;
				}
				val8 = 255;
				WriteBE_uint8(file, val8);
				val16 = t2-t1-255;
				WriteBE_uint16(file, val16);
				t2 = t1+255;
			}
			val8 = 1;//移动
			WriteBE_uint8(file, val8);
			val8 = t2-t1;
			WriteBE_uint8(file, val8);
			x2 = p->c;
			y2 = p->r;
			sval16 = x2-x1;
			WriteBE_int16(file, sval16);
			sval16 = y2-y1;
			WriteBE_int16(file, sval16);
			x1 = x2;
			y1 = y2;
			t1 = p->mstime;
		}
		else if(p->operation == '@')
		{
			specialOpen = 0;
			r = p->r;
			c = p->c;
			if(isShown[r][c] == 1)
			{
				if(NumberOfSignAround(r, c) == numberOfMineAround[r][c])
				{
					for(ra=r-1; ra<=r+1; ra++)
					{
						for(ca=c-1; ca<=c+1; ca++)
						{
							if(ra>=0 && ra<heightOfBoard && ca>=0 && ca<widthOfBoard)//确认在范围内
							{
								if(isShown[ra][ca] == 0)
								{
									isShown[ra][ca] = 1;
									if(board[ra][ca] == 0) OpenZeroChain(ra, ca);
									specialOpen = 2;
								}
							}
						}
					}
				}
			}
			else
			{
				if(isShown[r][c] == 2) specialOpen = 1;
				isShown[r][c] = 1;
				if(board[r][c] == 0) OpenZeroChain(r, c);
			}
			t2 = p->mstime;
			if(t2-t1 > 255)
			{
				while(t2-t1 > 65535)
				{
					val8 = 255;//停顿
					WriteBE_uint8(file, val8);
					val16 = 65535;
					WriteBE_uint16(file, val16);
					t2 -= 65535;
				}
				val8 = 255;
				WriteBE_uint8(file, val8);
				val16 = t2-t1-255;
				WriteBE_uint16(file, val16);
				t2 = t1+255;
			}
			x2 = x1;
			y2 = y1;
			if(y1/64 != r || x1/64 != c)//修正坐标偏差
			{
				if(y1/64 != r)
				{
					if(y1 < r*64) y2 = r*64;
					else y2 = r*64+63;
				}
				if(x1/64 != c)
				{
					if(x1 < c*64) x2 = c*64;
					else x2 = c*64+63;
				}
			}
			if(specialOpen == 0)
			{
				val8 = 2;//按下
				WriteBE_uint8(file, val8);
				val8 = t2-t1;
				WriteBE_uint8(file, val8);
				sval16 = x2-x1;
				WriteBE_int16(file, sval16);
				sval16 = y2-y1;
				WriteBE_int16(file, sval16);
				val8 = 3;//MSR玩家特有的瞬间按下再弹起
				WriteBE_uint8(file, val8);
				val8 = 0;
				WriteBE_uint8(file, val8);
				sval16 = 0;
				WriteBE_int16(file, sval16);
				sval16 = 0;
				WriteBE_int16(file, sval16);
			}
			else if(specialOpen == 1)//MSR玩家特有的瞬间取标
			{
				val8 = 4;//按下
				WriteBE_uint8(file, val8);
				val8 = t2-t1;
				WriteBE_uint8(file, val8);
				sval16 = x2-x1;
				WriteBE_int16(file, sval16);
				sval16 = y2-y1;
				WriteBE_int16(file, sval16);
				val8 = 5;//MSR玩家特有的瞬间按下再弹起
				WriteBE_uint8(file, val8);
				val8 = 0;
				WriteBE_uint8(file, val8);
				sval16 = 0;
				WriteBE_int16(file, sval16);
				sval16 = 0;
				WriteBE_int16(file, sval16);
				val8 = 2;//按下
				WriteBE_uint8(file, val8);
				val8 = 1;//瞬间取标翻开识别为双击，故偏差1ms
				WriteBE_uint8(file, val8);
				sval16 = 0;
				WriteBE_int16(file, sval16);
				sval16 = 0;
				WriteBE_int16(file, sval16);
				val8 = 3;//MSR玩家特有的瞬间按下再弹起
				WriteBE_uint8(file, val8);
				val8 = 0;
				WriteBE_uint8(file, val8);
				sval16 = 0;
				WriteBE_int16(file, sval16);
				sval16 = 0;
				WriteBE_int16(file, sval16);
			}
			else if(specialOpen == 2)//MSR玩家特有的瞬间双击
			{
				val8 = 4;//按下
				WriteBE_uint8(file, val8);
				val8 = t2-t1;
				WriteBE_uint8(file, val8);
				sval16 = x2-x1;
				WriteBE_int16(file, sval16);
				sval16 = y2-y1;
				WriteBE_int16(file, sval16);
				val8 = 2;//按下
				WriteBE_uint8(file, val8);
				val8 = 0;
				WriteBE_uint8(file, val8);
				sval16 = 0;
				WriteBE_int16(file, sval16);
				sval16 = 0;
				WriteBE_int16(file, sval16);
				val8 = 3;//MSR玩家特有的瞬间按下再弹起
				WriteBE_uint8(file, val8);
				val8 = 0;
				WriteBE_uint8(file, val8);
				sval16 = 0;
				WriteBE_int16(file, sval16);
				sval16 = 0;
				WriteBE_int16(file, sval16);
				val8 = 5;//MSR玩家特有的瞬间按下再弹起
				WriteBE_uint8(file, val8);
				val8 = 0;
				WriteBE_uint8(file, val8);
				sval16 = 0;
				WriteBE_int16(file, sval16);
				sval16 = 0;
				WriteBE_int16(file, sval16);
			}
			x1 = x2;
			y1 = y2;
			t1 = p->mstime;
			if(specialOpen == 1) t1 += 1;
		}
		else if(p->operation == '#')
		{
			r = p->r;
			c = p->c;
			if(isShown[r][c] == 0)
			{
				isShown[r][c] = 2;
			}
			else if(isShown[r][c] == 2)
			{
				isShown[r][c] = 0;
			}
			t2 = p->mstime;
			if(t2-t1 > 255)
			{
				while(t2-t1 > 65535)
				{
					val8 = 255;//停顿
					WriteBE_uint8(file, val8);
					val16 = 65535;
					WriteBE_uint16(file, val16);
					t2 -= 65535;
				}
				val8 = 255;
				WriteBE_uint8(file, val8);
				val16 = t2-t1-255;
				WriteBE_uint16(file, val16);
				t2 = t1+255;
			}
			x2 = x1;
			y2 = y1;
			if(y1/64 != r || x1/64 != c)//修正坐标偏差
			{
				if(y1/64 != r)
				{
					if(y1 < r*64) y2 = r*64;
					else y2 = r*64+63;
				}
				if(x1/64 != c)
				{
					if(x1 < c*64) x2 = c*64;
					else x2 = c*64+63;
				}
			}
			val8 = 4;//按下
			WriteBE_uint8(file, val8);
			val8 = t2-t1;
			WriteBE_uint8(file, val8);
			sval16 = x2-x1;
			WriteBE_int16(file, sval16);
			sval16 = y2-y1;
			WriteBE_int16(file, sval16);
			val8 = 5;//MSR玩家特有的瞬间按下再弹起
			WriteBE_uint8(file, val8);
			val8 = 0;
			WriteBE_uint8(file, val8);
			sval16 = 0;
			WriteBE_int16(file, sval16);
			sval16 = 0;
			WriteBE_int16(file, sval16);
			x1 = x2;
			y1 = y2;
			t1 = p->mstime;
		}
	}
	/*if(operations.isWin == 1)
	{
		val8 = 82;
		WriteBE_uint8(file, val8);
		val8 = 0;
		WriteBE_uint8(file, val8);
	}
	else if(operations.isWin == 0)
	{
		val8 = 83;
		WriteBE_uint8(file, val8);
		val8 = 0;
		WriteBE_uint8(file, val8);
	}*/
	//事件结束标记
	fputc(0, file);
	//校验码长度
	val16 = 0;
	WriteBE_uint16(file, val16);
	fclose(file);
	printf("已输出到%s\n", fileName);
}

/*--------------------------------
更新日志：
MSR Operations Transcoder 1.1
——修复 用户标识中含有回车
MSR Operations Transcoder 1.2
——优化 使用新版编译器
——修复 上游evf文档错误导致开问号
MSR Operations Transcoder 1.3
——优化 调整写入大端法代码，风格统一
--------------------------------*/
