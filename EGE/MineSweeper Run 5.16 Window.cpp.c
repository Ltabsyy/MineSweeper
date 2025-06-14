//#define _CRT_SECURE_NO_WARNINGS//vs专属1(doge)
//#define _WIN32_WINNT 0x0600//使用SetProcessDPIAware()
#include <stdio.h>
#include <stdlib.h>
#include <time.h>//时间戳作种子生成随机数，用于生成地图和无确定解时随机翻开
//#include <conio.h>//非阻塞式输入
//#define kbhit _kbhit//vs专属2
//#define getch _getch
#include <windows.h>//面向Windows输出彩色字符
#include <math.h>
//#include <direct.h>//创建文件夹
//#define SHOW_CONSOLE//vs专属3
#include <graphics.h>//Easy Graphics Engine
/**
 * 扫雷 MineSweeper Run
 * 
 * by Ltabsyy & Jws
 * 
 * https://github.com/Ltabsyy/MineSweeper
 **/
#define LimHeight 256//最大高度，限制行数
#define LimWidth 384//最大宽度，限制列数
#define LimLength 72//最大枚举长度限制
#define LimDictionary 16777216//最大字典大小
#define LimName 32//限制自制地图名称长度
//内存占用至多约为 75*H*W+L*D Byte，默认约1.3MB

// 外部窗口形态
int sideLength = 32;//外部窗口方块边长
#define heightOfBlock sideLength
#define widthOfBlock sideLength//锁定纵横比
#define heightOfBar 2*sideLength
#define widthOfBorder sideLength/4
#define xOfChar widthOfBlock*10/32
#define yOfChar heightOfBlock/32
#define heightOfChar heightOfBlock

// 控制栏
int IsPosInRectangle(int x, int y, int x1, int y1, int x2, int y2);
void DrawControlBar(int select);
int ChoiceControlBar(COORD mousePos, DWORD dwButtonState);
void DrawProgressBar(int progress);

// 鼠标操作栏
void DrawSettingsBar(int select);
int ChoiceSettingsBar(COORD mousePos, DWORD dwButtonState);
void DrawSetDifficulty(int select);
int ChoiceSetDifficulty(COORD mousePos, DWORD dwButtonState);
void DrawSetSummonCheckMode(int select);
int ChoiceSetSummonCheckMode(COORD mousePos, DWORD dwButtonState);
void DrawSetChosenBoard(int select);
int ChoiceSetChosenBoard(COORD mousePos, DWORD dwButtonState);
void DrawSetOperateMode(int select);
int ChoiceSetOperateMode(COORD mousePos, DWORD dwButtonState);
void DrawSetCursor(int select);
int ChoiceSetCursor(COORD mousePos, DWORD dwButtonState);

// 地图生成和显示
struct Information
{
	//yOfMap = yOfMapEnd-heightOfMapShown+1
	//yOfMapEnd-heightOfBoard+1 = yOfMap+rs0
	//yOfCommand = yOfMapEnd+4
	int yOfMap, yOfInformation, yOfCommand;//输出定位
	int remainedMine;
	int t0, t1, t2;//显示用时
	int unsolved3BV, total3BV;//地图3BV
	int showInformation;//0第一次翻开，1刷新信息
	int clock0, clock1;//毫秒用时
}game;
void SummonBoard(int seed, int r0, int c0, int summonMode, int iterateMode);//地图生成
int Place(int n);//计算某数所占位数，用于对齐坐标轴
void PrintBlank(int n);//打印n个空格
void PrintCell(int board, int isShown, int bkcolor);//打印地图方块内字符
void ShowBlock(int r, int c, int mode);//根据背景颜色和模式显示地图方块
void ShowBoard(int mode);//在控制台显示地图，0前台，1后台
//void ShowPartBoard(int r1, int c1, int r2, int c2, int mode);//在控制台显示部分地图
void ReShowBoard(int r, int c, int mode, int yOfMap);//地图重绘
void FreshCursor(int r, int c, int yOfMap);//刷新光标
//void ShowPageBoard(int mode, char move);
void ShowBoardFast(int yOfMap);
void PrintSolvingMap(int mode);
void ShowInformation();//更新时间并显示游戏信息

// 外部窗口显示
int dx = 0, dy = 0;//地图偏移
int screenHeight, screenWidth;
int windowHeight, windowWidth, xFace;
int xRemainedMine, xRemainedMineNumber, xTime, xTimeNumber, x3BV, x3BVNumber, x3BVps, x3BVpsNumber;
int mouseR = -1, mouseC = -1;//悬浮高亮
void DrawMine(int r, int c);//绘制地图地雷
void DrawMineA(int x0, int y0, int r);//绘制地雷图形
void DrawFlag(int r, int c);//绘制地图旗帜
void DrawBlock(int r, int c, int board, int isShown, int highlight);//绘制方块
void DrawLineA(int x0, int y0, int r, int angle);//绘制时钟指针
void DrawClock(int x0, int y0, int r, int time);//绘制时钟
void DrawFace(int mode);//绘制笑脸
void DrawBoard(int mode);//绘制总外部窗口
void DrawSolution();//在外部窗口绘制方案矩阵
//void DrawMouse(int x, int y);//绘制鼠标
void UpdateWindowSize();//根据当前方块边长更新窗口大小
void InitWindow(int mode);
void ResizeWindow(char mode);//调整显示大小
int IsMousePosOutside();//鼠标在窗口边界外
void GetWindowOperation(char* operation, int* r, int* c);
int CloseWindow(int isWinning);

// 后台计算
int Difficulty(int height, int width, int mine);//根据地图信息判断难度
int IsAroundZeroChain(int r0, int c0);
void OpenZeroChain(int r0, int c0);//翻开0连锁翻开
int RemainedMineWin();//判断真实的剩余雷数为0，非显示的剩余雷数
int NumberOfNotShown();//%的数量
int NumberOfSign();//#的数量
int NumberOfNotShownAround(int r0, int c0);//某坐标附近%的数量
int NumberOfSignAround(int r0, int c0);//某坐标附近#的数量
int WASDMove(int* r, int* c, WORD wVirtualKeyCode);

// 方案思考
void LookMap();//从实时游戏获取地图
void TranslateMap(int rs0, int cs0);//从键盘输入获取地图
void Solve(int showAnswer);//生成方案矩阵并修改显示方式矩阵，1显示1解，2不确定翻开
int NumberTeamType(int number1, int number2, int numberOfMine1, int numberOfMine2, int numberOfNotShown1, int numberOfNotShown2);
int Think();
int ThinkSelect();
int DeepThink();
int ThinkMine(int numberOfThought);
void SummonMineRateForNotShown();
int NumberOfNumberAround(int r0, int c0);
void ShownModeBak(int mode);//备份显示方式矩阵，1备份，0恢复
int IsSolvableIsland();
int IsSolvableMap(int seed, int r0, int c0);
void ShowSolution(int yOfMap);//在原地图显示方案矩阵

// 深度思考
struct NumberCheck//一个数字对周围未知方块提供的信息
{
	int numberOfMine;
	int r1, c1;//左上角坐标
	int r2, c2;//右下角坐标
	int numberOfPossibility;//可能性总数
	int p;//当前处在可能性
	//三维数组存储所有可能性，二维数组仅分配最大3x3空间，减少比较次数
	int*** mineCheck;//0跳过，1非雷，2雷
};
struct NumberCheckChain//校验表
{
	//数字位置为全局变量thinkChain 2值
	struct NumberCheck* numberCheck;//数字校验表
	int numberOfThought;//数字数
	int t;//当前校验位置，类似栈顶指针，以实现剪枝和回溯
};
struct LocalResult//单个未知链
{
	int** thinkChain;//未知链
	char** dictionary;//全部可能，用char存储节省空间
	int* sumDictionary;//未知方块为雷可能数
	int* sumMineOfPossibility;//某可能的链中雷数
	int numberOfThought, numberOfPossibility;//未知方块数，正确可能数
	int minNumberOfMine, maxNumberOfMine;//雷数范围
};
struct GlobalResult//全局未知链
{
	struct LocalResult* thinkResult;//未知链数组
	int numberOfThinkChain;//未知链数
	int minNumberOfPossibleMine, maxNumberOfPossibleMine;//全局链中雷数范围
}results;
struct NumberCheck SummonMineCheck(struct NumberCheck nc);
void RefreshMineCheck(struct NumberCheckChain ncc);
int MergeCheck(struct NumberCheck nc);//检查并合并入全局numberCheck，0失败，1成功
int ThinkNumber(int numberOfThought);//选取并思考一个未知链
void SaveResult(int numberOfThought, int numberOfPossibility, int minNumberOfMine, int maxNumberOfMine);
void ClearResults();
void SortResult(struct LocalResult* result);
void ExcludeResult(struct LocalResult* result, int mine);
int WholeThink();//对全局未知链分块思考并合并思考

// 历史记录
struct Record
{
	//存储数据
	int numberOfMine;
	int heightOfBoard, widthOfBoard;
	int summonMode, iterateMode;
	int seed, r0, c0;
	int sTime, msTime;
	int solved3BV, total3BV;
	int isHelped;
	//计算数据
	float speed;
	int difficulty;
	float score;
};
struct Records
{
	//存储数据
	struct Record* record;
	int numberOfRecords;
	//计算数据
	int minimumTime[5];//难度0-4时间纪录
	float fastestSpeed[5];//3BV/s纪录
	float highestScore[5];//SS纪录
	int count[6];//局数
	int countEffective[6];//有效局数，5实际无用
	int totalTime;//所有记录总时间
	int totalSolved3BV;//总翻开数
	int maxOpenNumber;//最大翻开数字
	int gamerLevel;//玩家等级
};
int IsEffectiveRecord(struct Record record);
void PrintRecords(struct Records records, int mode);
struct Records ReadRecords();
void WriteRecords(struct Records records);
struct Records AddRecord(struct Records records, struct Record newRecord);
struct Records DeleteRecord(struct Records records, int deleteNumber);
struct Records DeleteRecords(struct Records records, int mode);
int MaxOpenNumber(struct Records records);
int GamerLevel(struct Records records);

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
void PrintMap(struct Map map, int showNumber);
void PrintMaps(struct Maps maps);
struct Maps ReadMaps();
void WriteMaps(struct Maps maps);
struct Maps AddMaps(struct Maps maps, struct Map newMap);
struct Maps DeleteMap(struct Maps maps, int deleteNumber);
struct Maps DeleteMaps(struct Maps maps);
struct Map EditMap(struct Map map);

// 操作记录
/*struct Operation
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
	int summonMode;
	int seed, r0, c0;
	int time;//终局时整数时间，避免毫秒时间显示偏差
	int isWin;//胜局时显示3BV全解
	//int sideLength;
	struct Operation* head;//头节点，顺序存储便于回放
	struct Operation* tail;//尾节点，避免插入时搜索尾部
}operationRecord;
void AddOperation(struct Operations* operations, int mstime, char operation, int r, int c);
struct Operations AddOperations(int seed, int r0, int c0);
void ClearOperations(struct Operations operations);
void WriteOperations(struct Operations operations);
struct Operations ReadOperations(char* fileName);
void PlayOperations(struct Operations operations);*/

// 功能模块
void RCScan(char* operation, int* r, int* c, int yOfCommand);//@#rc指令输入模块
void QuestionMark(char operation, int ro, int co);//问号模块
void Solution();//地图求解模块
void Bench(int seedMin, int seedMax, int r0, int c0, int showStep, int showSolution, int showInformation);//Bench模块
int BBBV(int seed, int r0, int c0, int mode);//Bechtel'sBoardBenchmarkValue，计算地图3BV
double SS(int h, int w, int n, double time, int solved3BV, int total3BV);//计算标准分数SS，代表发挥水平
int SearchSeed(int seedMin, int r0, int c0, int difficulty);//可解和筛选种子搜索模块
void MapSearch(int seedMin, int seedMax, int r0, int c0);//地图搜索模块
void** MatrixMemory(void** matrix, int rm, int cm, int sizeOfElements, int mode);//矩阵内存操作
void ReallocMemory(int height, int width, int dic, int length);//全局矩阵内存重分配
struct Records RecordsEditer(struct Records records);//RecordsEditer模块
int CustomMapsEditer();
void Error();//错误

// 全局矩阵
int** isMine = 0;//雷
int** numberOfMineAround = 0;//数字
int** board = 0;//后台总板，数字0~8雷9
int** isShown = 0;//显示方式矩阵，0未翻开，1已翻开，2已标记
int** zeroChain = 0;//0链
int** questionMark = 0;//问号矩阵，1交界线，2待翻开，3待标记
int** isShownBak = 0;//显示方式矩阵备份
int** isShownBuf = 0;//显示缓冲，上次显示方式矩阵
int** zeroCache = 0;//0块缓存
char** mapShown = 0;//键盘输入的地图
char** map = 0;//获取的地图
int** numberShown = 0;//获取的数字，非数字为9
int** solution = 0;//方案，1翻开，2标记
int** isThought = 0;//1未知方块，2附近数字，3枚举后放弃，4过长放弃
int** numberTeam = 0;//数对，1数对之一独占区，2数对之二独占区，3共享区
int** thinkChain = 0;//未知链
int** numberCheckBase = 0;//校验基底
int** numberCheck = 0;
int** mineCheck = 0;
float** isMineRate = 0;
//int** isMineRateA = 0;
//int** isMineRateB = 0;
int* possibility = 0;
int* sumDictionary = 0;
char** dictionary = 0;//字典，用char存储节省空间

// 全局变量及初始化
int numberOfMine = 10;//雷数量
int heightOfBoard = 10;//界面高度
int widthOfBoard = 10;//界面宽度
int summonCheckMode = 2;//0不校验，1非雷，2必空，3可解，4筛选
int showTime = 1;//显示用时，0不显示，1显示
int show3BV = 0;//显示3BV和3BV/s
int backgroundColor = 0x07;//背景颜色，深色模式0x07，浅色模式0xf0
int operateMode = 3;//操作模式，0@#rc，1wasd23，2鼠标点击，3Window
int solveMode = 1;//0简单模式，1游戏模式，2分析模式
//int lengthOfThinkChain = 19;//未知链长度
int lengthOfThinkNumberCheck = 15;
int lengthOfThinkMineCheck = 32;
int dictionaryCapacity = 32768;//全字典大小
int remainedMineJudgeLocalNC = 1;//剩余雷数判断
int remainedMineJudgeLocalMC = 0;
int remainedMineJudgeGlobal = 0;
int refreshCycle = 50;//刷新周期，默认50ms，一般鼠标8ms，游戏鼠标1ms
int fastShow = 1;//启用快速显示，仅刷新地图更新部分
int fastSign = 0;//#数字快速标记周围
int newCursor = 2;//1><光标，2淡黄色高亮光标，3淡黄色高亮行列
int visibleCursor = 0;//1显示控制台光标，0隐藏控制台光标
int keepCursor = 0;//保持光标显示
//int dynamicMemory = 1;//启用动态内存分配
char cursorLeft[2] = ">";//><光标左
char cursorRight[2] = "<";//><光标右
int chosenSolvable = 1;//筛选地图可解性
int chosen3BVMin[6] = {1, 1, 1, 1, 1, 0};//筛选地图3BV，3BV为2时，仅需1次点击
int chosen3BVMax[6] = {8, 9, 49, 149, 1239, 2147483647};
int chosenNumber = 0;//筛选地图指定包含数字
int islandPrediction = 0;//可解地图判断基于岛上可解的预判加速
int heightOfMapShown, widthOfMapShown, rs0, cs0;//控制台地图显示
int minNumberOfPossibleMine, maxNumberOfPossibleMine;
int numberOfAbandonedThinkChain, countOfDictionaryOverflow, dictionaryNeeded;//Bench统计
int countLengthOfThinkChain[2*LimLength]={0};
//int diffResult;

// 是否调试(0:关 1:开，显示部分 2:开，显示全部)
int debug = 0;

// 控制台显示
void ColorStr(const char* content, int color)//输出彩色字符
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
	//printf("%s", content);
	fputs(content, stdout);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), backgroundColor);
}
void gotoxy(short x, short y)//以覆写代替清屏，加速Bench
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
void ColorNumber(int number, int color)//输出彩色数字
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
	printf("%d", number);
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
void GetSizeOfMap(int* heightOfWindow, int* widthOfWindow, int* heightOfFont, int* maxHeightOfMap, int* maxWidthOfMap)
{
	HANDLE hdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	CONSOLE_FONT_INFO cfi;
	//获取控制台窗口大小和字体大小
	GetConsoleScreenBufferInfo(hdout, &csbi);
	GetCurrentConsoleFont(hdout, 0, &cfi);
	*heightOfWindow = csbi.srWindow.Bottom-csbi.srWindow.Top+1;//30 50
	*widthOfWindow = csbi.dwSize.X;//120 189
	*heightOfFont = cfi.dwFontSize.Y;//16
	//计算最大地图大小
	*maxHeightOfMap = *heightOfWindow-3-4-1-2;//30-3-4-1-2=20 40，去除控制栏3，游戏信息栏4，留白1，列坐标轴2
	*maxWidthOfMap = (*widthOfWindow-1-3)/2;//(120-1-3)/2=58 92(.5)，去除回车1，行坐标3，空白一半
	if(*maxHeightOfMap > 40-3) *maxHeightOfMap += 3;//大于40时不显示控制栏
	// M 37 38 39 40 41 42 43
	// W 47(48 49 50)48 49 50
	//   37          38 39 40
	//全屏状态下调整字体大小后需重新开关全屏
	//if(*heightOfFont > 16) *maxHeightOfMap -= 3*(*heightOfFont-16);
}
void AdaptScreenBufferWidth()//自适应屏幕缓冲区宽度
{
	HANDLE hdout = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(hdout, &csbi);
	rs0 = Place(widthOfBoard-1);//行起始指针，为列坐标显示占用行数
	cs0 = Place(heightOfBoard-1) + 1;//列起始指针，为行坐标显示占用列数，含空格
	heightOfMapShown = heightOfBoard + rs0;//计算包含坐标的行数
	widthOfMapShown = 2*widthOfBoard + cs0 + 1;//计算包含坐标的每行字符数加回车
	//printf("%d %d\n", csbi.dwSize.X, csbi.dwSize.Y);
	if(csbi.dwSize.X < widthOfMapShown)
	{
		csbi.dwSize.X = widthOfMapShown;
		SetConsoleScreenBufferSize(hdout, csbi.dwSize);
	}
}

int main(/*int argc, char** argv*/)
{
	FILE* file;//文件读写
	int choiceMode = 0, choiceTemp = 0, choiceSet = 0, choiceSubSet = 0;//游戏功能的选择
	char operation, key;//游戏操作和设置的字符读取，用户按键
	int temp;//数据暂存
	int seed, r0, c0, seedMin, seedMax;//地图生成
	int r, c;//通用坐标
	int isOpenMine;//游戏胜利判断
	int ra, ca;//连锁反应判断
	int ro, co;//上一个操作坐标
	int difficulty;//当前难度
	float density;//雷密度，实际没有必要定义(doge)
	struct Records records;//历史记录
	struct Record newRecord;
	// 控制台鼠标操作
	HANDLE hdin = GetStdHandle(STD_INPUT_HANDLE);
	COORD mousePos = {0, 0};//鼠标坐标
	COORD mouseOpenPos = {0, 0};
	COORD mouseSignPos = {0, 0};//鼠标已操作坐标，屏蔽双击
	INPUT_RECORD rcd;
	DWORD rcdnum;
	int isReadyRefreshMouseOpenPos = 0;
	int isReadyRefreshMouseSignPos = 0;
	// main()局部变量及初始化
	int lastMap = 0;//有上一次地图
	int clickSpace = 1;//0鼠标点击屏蔽空格
	int touchLeftSign = 0;//鼠标点击模式左右键功能切换，用于触屏标记
	int touchMode = 0;//1鼠标点击空地切换左右键
	int touchOpen = 0;//触动翻开，允许鼠标点击使用标记键快速翻开
	int questionMarkMode = 0;//1问号模式
	int isHelped = 0;//记录一局游戏是否被实时求解指令帮助
	int benchSeedMin = 0;//Bench种子范围
	int benchSeedMax = 99;
	int benchShowStep = 1;//-1无帧，0结束帧，1中间帧，2帧暂停，3全调试
	int benchShowSolution = 1;//Bench在原地图显示方案矩阵
	int benchShowInformation = 0;//Bench显示置顶信息
	//很好，我已经看不清我定义些啥了(doge)
	if(debug == 2) printf("<Debug>\nHello! Administrator Ltabsyy or Jws!\n");
	/*读取缓存*/
	records = ReadRecords();//读取历史记录
	if(debug == 2) PrintRecords(records, 1);
	if((file = fopen("minesweeper-settings.txt", "r")))//读取设置
	{//内嵌括号减少warning，其实可以不加(doge)
		fscanf(file, "Map:%d*%d-%d\n", &heightOfBoard, &widthOfBoard, &numberOfMine);
		fscanf(file, "summonCheckMode=%d\n", &summonCheckMode);
		fscanf(file, "showTime=%d\n", &showTime);
		fscanf(file, "show3BV=%d\n", &show3BV);
		fscanf(file, "backgroundColor=0x%x\n", &backgroundColor);
		fscanf(file, "operateMode=%d\n", &operateMode);
		fscanf(file, "solveMode=%d\n", &solveMode);
		//fscanf(file, "lengthOfThinkChain=%d\n", &lengthOfThinkChain);
		fscanf(file, "lengthOfThinkNumberCheck=%d\n", &lengthOfThinkNumberCheck);
		fscanf(file, "lengthOfThinkMineCheck=%d\n", &lengthOfThinkMineCheck);
		fscanf(file, "dictionaryCapacity=%d\n", &dictionaryCapacity);
		fscanf(file, "remainedMineJudgeLocalNC=%d\n", &remainedMineJudgeLocalNC);
		fscanf(file, "remainedMineJudgeLocalMC=%d\n", &remainedMineJudgeLocalMC);
		fscanf(file, "remainedMineJudgeGlobal=%d\n", &remainedMineJudgeGlobal);
		fscanf(file, "refreshCycle=%d\n", &refreshCycle);
		fscanf(file, "fastShow=%d\n", &fastShow);
		fscanf(file, "fastSign=%d\n", &fastSign);
		fscanf(file, "newCursor=%d\n", &newCursor);
		fscanf(file, "visibleCursor=%d\n", &visibleCursor);
		fscanf(file, "keepCursor=%d\n", &keepCursor);
		fscanf(file, "clickSpace=%d\n", &clickSpace);
		fscanf(file, "touchMode=%d\n", &touchMode);
		fscanf(file, "touchOpen=%d\n", &touchOpen);
		fscanf(file, "benchSeedMin=%d\n", &benchSeedMin);
		fscanf(file, "benchSeedMax=%d\n", &benchSeedMax);
		fscanf(file, "benchShowStep=%d\n", &benchShowStep);
		fscanf(file, "benchShowSolution=%d\n", &benchShowSolution);
		fscanf(file, "benchShowInformation=%d\n", &benchShowInformation);
		fscanf(file, "cursorLeft=%c\n", &cursorLeft[0]);
		fscanf(file, "cursorRight=%c\n", &cursorRight[0]);
		fscanf(file, "chosenSolvable=%d\n", &chosenSolvable);
		fscanf(file, "chosen3BV:[%d,%d][%d,%d][%d,%d][%d,%d][%d,%d][%d,%d]\n",
			&chosen3BVMin[0], &chosen3BVMax[0], &chosen3BVMin[1], &chosen3BVMax[1],
			&chosen3BVMin[2], &chosen3BVMax[2], &chosen3BVMin[3], &chosen3BVMax[3],
			&chosen3BVMin[4], &chosen3BVMax[4], &chosen3BVMin[5], &chosen3BVMax[5]);
		fscanf(file, "chosenNumber=%d\n", &chosenNumber);
		fscanf(file, "islandPrediction=%d\n", &islandPrediction);
		fclose(file);
		if(benchShowStep == 3 && debug != 2) benchShowStep = 2;
		if(lengthOfThinkMineCheck < lengthOfThinkNumberCheck) lengthOfThinkMineCheck = lengthOfThinkNumberCheck;//保持NC<=MC
		if(dictionaryCapacity > LimDictionary) dictionaryCapacity = LimDictionary;
		if(debug == 2)
		{
			printf("[Debug]已获取设置：\n");
			printf("Map:%d*%d-%d\n", heightOfBoard, widthOfBoard, numberOfMine);
			printf("summonCheckMode=%d\n", summonCheckMode);
			printf("showTime=%d\n", showTime);
			printf("show3BV=%d\n", show3BV);
			printf("backgroundColor=0x%x\n", backgroundColor);
			printf("operateMode=%d\n", operateMode);
			printf("solveMode=%d\n", solveMode);
			//printf("lengthOfThinkChain=%d\n", lengthOfThinkChain);
			printf("lengthOfThinkNumberCheck=%d\n", lengthOfThinkNumberCheck);
			printf("lengthOfThinkMineCheck=%d\n", lengthOfThinkMineCheck);
			printf("dictionaryCapacity=%d\n", dictionaryCapacity);
			printf("remainedMineJudgeLocalNC=%d\n", remainedMineJudgeLocalNC);
			printf("remainedMineJudgeLocalMC=%d\n", remainedMineJudgeLocalMC);
			printf("remainedMineJudgeGlobal=%d\n", remainedMineJudgeGlobal);
			printf("refreshCycle=%d\n", refreshCycle);
			printf("fastShow=%d\n", fastShow);
			printf("fastSign=%d\n", fastSign);
			printf("newCursor=%d\n", newCursor);
			printf("visibleCursor=%d\n", visibleCursor);
			printf("keepCursor=%d\n", keepCursor);
			printf("clickSpace=%d\n", clickSpace);
			printf("touchMode=%d\n", touchMode);
			printf("touchOpen=%d\n", touchOpen);
			printf("benchSeedMin=%d\n", benchSeedMin);
			printf("benchSeedMax=%d\n", benchSeedMax);
			printf("benchShowStep=%d\n", benchShowStep);
			printf("benchShowSolution=%d\n", benchShowSolution);
			printf("benchShowInformation=%d\n", benchShowInformation);
			printf("cursorLeft=%c\n", cursorLeft[0]);
			printf("cursorRight=%c\n", cursorRight[0]);
			printf("chosenSolvable=%d\n", chosenSolvable);
			printf("chosen3BV:[%d,%d][%d,%d][%d,%d][%d,%d][%d,%d][%d,%d]\n",
				chosen3BVMin[0], chosen3BVMax[0], chosen3BVMin[1], chosen3BVMax[1],
				chosen3BVMin[2], chosen3BVMax[2], chosen3BVMin[3], chosen3BVMax[3],
				chosen3BVMin[4], chosen3BVMax[4], chosen3BVMin[5], chosen3BVMax[5]);
			printf("chosenNumber=%d\n", chosenNumber);
			printf("islandPrediction=%d\n", islandPrediction);
		}
	}
	if((file = fopen("minesweeper-lastmap.txt", "r")))//读取上一次地图
	{
		lastMap = 1;
		choiceMode = 1;
		fscanf(file, "Map:%d*%d-%d\n", &heightOfBoard, &widthOfBoard, &numberOfMine);
		fscanf(file, "seed=%d,%d,%d\n", &seed, &r0, &c0);
		fscanf(file, "summonCheckMode=%d\n", &summonCheckMode);
		fscanf(file, "time=%d\n", &(game.t2));
		fscanf(file, "pos=(%d,%d)\n", &ro, &co);
		fscanf(file, "isHelped=%d\n", &isHelped);
		//提前申请内存空间读入显示方式矩阵
		ReallocMemory(heightOfBoard, widthOfBoard, dictionaryCapacity, lengthOfThinkMineCheck);
		for(r=0; r<heightOfBoard; r++)
		{
			fscanf(file, "  ");
			for(c=0; c<widthOfBoard; c++)
			{
				fscanf(file, "%d ", &isShown[r][c]);
			}
			fscanf(file, "\n");
		}
		fclose(file);
		if(debug == 2)
		{
			printf("[Debug]已获取上一次地图：\n");
			printf("Map:%d*%d-%d\n", heightOfBoard, widthOfBoard, numberOfMine);
			printf("seed=%d,%d,%d\n", seed, r0, c0);
			printf("summonCheckMode=%d\n", summonCheckMode);
			printf("time=%d\n", game.t2);
			printf("pos=(%d,%d)\n", ro, co);
			printf("isHelped=%d\n", isHelped);
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
		}
	}
	/*初始设置*/
	SetConsoleTitle("MineSweeper Console");
	if(backgroundColor != 0x07) setbgcolor(backgroundColor);//默认浅色模式
	if(visibleCursor == 0) showCursor(0);
	difficulty = Difficulty(heightOfBoard, widthOfBoard, numberOfMine);
	if(debug == 2)
	{
		printf("[Debug]当前难度：%d\n", difficulty);
		system("pause");
	}
	if(lastMap == 0)//申请内存空间
	{
		ReallocMemory(heightOfBoard, widthOfBoard, dictionaryCapacity, lengthOfThinkMineCheck);
	}
	SetConsoleMouseMode(1);
	//if(argc == 2) choiceMode = 10;
	/*进入主循环*/
	while(choiceMode != 5)
	{
		//clrscr();//清屏
		/*printf("*******************************\n"
			   "**新游戏(1)**设置(2)**退出(3)**\n"
			   "*******************************\n");*/
		/*printf("*******************************\n");//宽31
		if(lastMap == 1) printf("(0)继续上一次游戏\n");
		printf("(1)新游戏\n"
			   "(2)地图求解\n"
			   "(3)设置\n"
			   "(4)Bench\n"
			   "(5)退出\n"
			   "*******************************\n");*/
		DrawControlBar(0);
		AdaptScreenBufferWidth();
		//printf(">");
		FlushConsoleInputBuffer(hdin);//Bench抗双击
		while(choiceMode == 0)
		{
			ReadConsoleInput(hdin, &rcd, 1, &rcdnum);
			if(rcd.EventType == MOUSE_EVENT)
			{
				mousePos = rcd.Event.MouseEvent.dwMousePosition;
				choiceTemp = ChoiceControlBar(mousePos, rcd.Event.MouseEvent.dwButtonState);
				if(choiceTemp != 0) choiceMode = choiceTemp;
			}
			else if(rcd.EventType == KEY_EVENT && rcd.Event.KeyEvent.bKeyDown == 1)
			{
				if(rcd.Event.KeyEvent.wVirtualKeyCode >= '1' && rcd.Event.KeyEvent.wVirtualKeyCode <= '9')
				{
					choiceMode = rcd.Event.KeyEvent.wVirtualKeyCode-'1'+1;//主键盘1-9
				}
				else if(rcd.Event.KeyEvent.wVirtualKeyCode >= 'a' && rcd.Event.KeyEvent.wVirtualKeyCode <= 'i')
				{
					choiceMode = rcd.Event.KeyEvent.wVirtualKeyCode-'a'+1;//小键盘1-9
				}
				/*else if(rcd.Event.KeyEvent.wVirtualKeyCode == 'V')
				{
					choiceMode = 10;
				}*/
			}
			showCursor(visibleCursor);//避免调整窗口大小恢复控制台光标
			api_sleep(refreshCycle);
		}
		if(choiceMode == 1)//游戏
		{
			/*重置*/
			clrscr();
			DrawControlBar(0);
			game.remainedMine = numberOfMine;
			game.t0 = 0;
			game.t1 = 0;
			game.showInformation = 0;//不刷新游戏信息
			isOpenMine = 0;
			if(lastMap == 1)
			{
				for(r=0; r<heightOfBoard; r++)
				{
					for(c=0; c<widthOfBoard; c++)
					{
						if(isShown[r][c] == 2) game.remainedMine--;
					}
				}
			}
			else//无上一次地图或自制地图
			{
				game.t2 = 0;
				isHelped = 0;
				if(lastMap == 2) isHelped = 1;
				for(r=0; r<heightOfBoard; r++)//确保第一次显示为全未翻开
				{
					for(c=0; c<widthOfBoard; c++)
					{
						isShown[r][c] = 0;
					}
				}
			}
			if(fastShow == 1)
			{
				for(r=0; r<heightOfBoard; r++)
				{
					for(c=0; c<widthOfBoard; c++)
					{
						if(lastMap == 1) isShownBuf[r][c] = isShown[r][c];
						else isShownBuf[r][c] = 0;//初始化显示缓冲为全未翻开
					}
				}
			}
			/*获取种子和生成位置*/
			if(operateMode == 3) InitWindow(0);//创建窗口
			if(lastMap != 1)
			{
				ShowBoard(0);
				printf("选择坐标[r:行][c:列]\n");
				printf(">@ ");
				if(operateMode < 2) SetConsoleMouseMode(0);
				if(operateMode == 0)
				{
					//scanf("%d%d", &r0, &c0);//通过键盘获取第一次选择的坐标
					operation = 0;
					r0 = 0;//该坐标会陆续穿过RCScan,FreshCursor,ReShowBoard,ShowBlock造成越界访问，6
					c0 = 0;
					while(operation == 0)
					{
						operation = '@';//防止@被删除
						RCScan(&operation, &r0, &c0, heightOfMapShown-1+3+2);
					}
				}
				else if(operateMode == 1 || operateMode == 2)
				{
					r0 = heightOfBoard/2;
					c0 = widthOfBoard/2;
					while(choiceMode == 1)
					{
						ReadConsoleInput(hdin, &rcd, 1, &rcdnum);
						if(rcd.EventType == MOUSE_EVENT && operateMode == 2)
						{
							mousePos = rcd.Event.MouseEvent.dwMousePosition;
							choiceTemp = ChoiceControlBar(mousePos, rcd.Event.MouseEvent.dwButtonState);
							if(choiceTemp != 0) choiceMode = choiceTemp;
							if(IsPosInRectangle((mousePos.X-cs0)/2, mousePos.Y-rs0-3, 0, 0, widthOfBoard-1, heightOfBoard-1))
							{
								r0 = mousePos.Y-rs0-3;
								c0 = (mousePos.X-cs0)/2;
								if(rcd.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
								{
									break;
								}
							}
						}
						else if(rcd.EventType == KEY_EVENT && rcd.Event.KeyEvent.bKeyDown == 1)
						{
							if(WASDMove(&r0, &c0, rcd.Event.KeyEvent.wVirtualKeyCode));
							else if(rcd.Event.KeyEvent.wVirtualKeyCode >= '3' && rcd.Event.KeyEvent.wVirtualKeyCode <= '5')
							{
								choiceMode = rcd.Event.KeyEvent.wVirtualKeyCode-'1'+1;//主键盘3-5
							}
							else if(rcd.Event.KeyEvent.wVirtualKeyCode >= 'c' && rcd.Event.KeyEvent.wVirtualKeyCode <= 'e')
							{
								choiceMode = rcd.Event.KeyEvent.wVirtualKeyCode-'a'+1;//小键盘3-5
							}
							else
							{
								break;
							}
						}
						gotoxy(0, heightOfMapShown+4);
						printf(">@ %d %d ", r0, c0);//覆写尾部
						FreshCursor(r0, c0, 3);
						//gotoxy(cs0+2*c0, heightOfMapShown+2-heightOfBoard+1+r0);
						showCursor(visibleCursor);
						api_sleep(refreshCycle);
					}
					gotoxy(0, heightOfMapShown+5);
				}
				else if(operateMode == 3)
				{
					operation = 0;
					r0 = heightOfBoard/2;
					c0 = widthOfBoard/2;
					while(operation == 0)
					{
						DrawBoard(0);
						GetWindowOperation(&operation, &r0, &c0);
						gotoxy(0, heightOfMapShown+4);
						printf(">@ %d %d ", r0, c0);//覆写尾部
						FreshCursor(r0, c0, 3);
						delay_ms(refreshCycle);
					}
				}
				seed = time(0);//当前时间戳作种子生成随机数
				if(debug == 2)
				{
					temp = 0;
					printf("[Debug]seed=%d,%d,%d\n", seed, r0, c0);
					SetConsoleMouseMode(0);
					printf("[0:生成/1:更改]\n>");
					showCursor(1);
					scanf("%d", &temp);
					if(temp == 1)
					{
						printf("[seed] [r0] [c0]\n>");
						scanf("%d%d%d", &seed, &r0, &c0);
					}
				}
			}
			/*生成地图*/
			if(lastMap == 0 && summonCheckMode > 2)//可解地图生成
			{
				seed = SearchSeed(seed, r0, c0, difficulty);
				if(debug == 1 || debug == 2) system("pause");
			}
			else if(lastMap != 2)
			{
				ShownModeBak(1);//有上一次地图时显示方式矩阵不清零
				SummonBoard(seed, r0, c0, summonCheckMode, 1);
				ShownModeBak(0);
			}
			/*准备开始游戏*/
			if(operateMode == 3)
			{
				for(r=0; r<heightOfBoard; r++)
				{
					for(c=0; c<widthOfBoard; c++)
					{
						solution[r][c] = 0;
					}
				}
			}
			r = r0;
			c = c0;
			if(lastMap == 0)
			{
				ro = r0;
				co = c0;
				//operationRecord = AddOperations(seed, r0, c0);
			}
			game.total3BV = BBBV(seed, r0, c0, 3);//地图3BV仅计算一次，自制地图防止BBBV生成地图
			game.showInformation = 1;
			isShown[r0][c0] = 1;//翻开第一个0
			if(operateMode >= 2) SetConsoleMouseMode(1);//确保win10控制台接收鼠标信息
			else SetConsoleMouseMode(0);
			FlushConsoleInputBuffer(hdin);//清除打开地图前的多次点击
			showCursor(visibleCursor);//防止调整窗口大小导致控制台光标显示
			game.t0 = time(0);
			clrscr();
			DrawControlBar(0);
			game.clock0 = clock();
			/*游戏循环*/
			while(choiceMode == 1)
			{
				/*计算*/
				if(isShown[r][c] == 1)//对翻开操作做出反应
				{
					if(board[r][c] == 0)//翻开0连锁翻开
					{
						OpenZeroChain(r, c);
					}
					else if(board[r][c] == 9)//寄
					{
						clrscr();
						DrawControlBar(0);
						ShowBoard(1);
						printf(":(\nGame Over!\n");
						isOpenMine = 1;
						//game.t1 = time(0);//时间在操作后的ShowInformation()处获取
						game.unsolved3BV = BBBV(seed, r0, c0, 0);//temp暂存未解3BV
						if(showTime == 1) printf("用时：%d ", game.t1-game.t0+game.t2);
						if(show3BV == 1) printf("3BV：%d/%d 3BV/s：%.2f ", game.total3BV-game.unsolved3BV,
							game.total3BV, (float)(game.total3BV-game.unsolved3BV)/(game.t1-game.t0+game.t2));
						printf("\n");
						remove("minesweeper-lastmap.txt");
						lastMap = 0;
						SetConsoleMouseMode(1);
						choiceMode = 0;
						break;
					}
				}
				if(RemainedMineWin()//正确标记所有雷则胜利
					|| NumberOfNotShown() == game.remainedMine)//未翻开的都是雷则胜利
				{
					if(NumberOfSign() > numberOfMine)//标记量不能超过雷数
					{
						//gotoxy(0, game.yOfCommand+1);
						//printf(":(\n标记过多！请重新标记。\n");
						//if(operateMode == 2 && operation == '#') api_sleep(refreshCycle);//防止鼠标出现右键菜单
						//system("pause");
						/*for(r=0; r<heightOfBoard; r++)//自动取消所有标记
						{
							for(c=0; c<widthOfBoard; c++)
							{
								if(isShown[r][c] == 2)
								{
									isShown[r][c] = 0;
									game.remainedMine++;
								}
							}
						}*/
						/*if(isShown[r][c] == 2)//自动取消上一次标记
						{
							isShown[r][c] = 0;
							game.remainedMine++;
						}*/
						//continue;
					}
					else
					{
						clrscr();
						DrawControlBar(0);
						ShowBoard(1);
						printf(":)\nYou Win!\n");
						//game.remainedMine = 0;
						//game.t1 = time(0);
						game.unsolved3BV = 0;
						if(showTime == 1) printf("用时：%d ", game.t1-game.t0+game.t2);
						if(show3BV == 1) printf("3BV：%d 3BV/s：%.2f ", game.total3BV, (float)game.total3BV/(game.t1-game.t0+game.t2));
						printf("\n");
						remove("minesweeper-lastmap.txt");
						lastMap = 0;
						SetConsoleMouseMode(1);
						choiceMode = 0;
						break;
					}
				}
				/*显示*/
				if(heightOfBoard > 40)
				{
					gotoxy(0, 0);//地图高度大于40时不显示控制栏
					game.yOfMap = 0;
				}
				else
				{
					DrawControlBar(0);
					//gotoxy(0, 3);
					game.yOfMap = 3;
				}
				if(debug == 1 || debug == 2)
				{
					//clrscr();
					//if(heightOfBoard > 10) show3BV = 0;
					//DrawControlBar(0);
					printf("<Debug>\n");
					printf("Map:%d*%d-%d\n", heightOfBoard, widthOfBoard, numberOfMine);
					printf("seed=%d,%d,%d\n", seed, r0, c0);
					ShowBoard(1);
					printf("NumberOfNotShown=%d \n", NumberOfNotShown());
					printf("\n");
					game.yOfMap += heightOfMapShown+6;
				}
				game.yOfInformation = game.yOfMap + heightOfMapShown;
				game.yOfCommand = game.yOfInformation + 3;
				if(fastShow == 1)
				{
					if(r == r0 && c == c0) ShowBoard(0);//第一次绘制坐标轴
					ShowBoardFast(game.yOfMap);
				}
				else
				{
					ShowBoard(0);
				}
				//打印剩余雷数，用时，3BV
				game.unsolved3BV = BBBV(seed, r0, c0, 2);
				ShowInformation();
				//显示当前操作模式提示
				printf("选择模式与坐标\n");
				if(operateMode == 0) printf("[@:翻开/#:标记] [r] [c]  \n");//宽22+2
				if(operateMode == 1) printf("[wasd:移动/2:翻开/3:标记]\n");//宽24
				if(operateMode == 2)
				{
					if(touchLeftSign == 1)
					{
						printf("[鼠标左键:标记/右键:翻开]\n");//宽24
					}
					else
					{
						printf("[鼠标左键:翻开/右键:标记]\n");//宽24
					}
				}
				if(operateMode == 3) printf("[请在MineSweeper Window进行游戏]\n");//宽31
				/*备份至文件*/
				if(lastMap != 2)//自制地图不备份
				{
					file = fopen("minesweeper-lastmap.txt", "w");
					fprintf(file, "Map:%d*%d-%d\n", heightOfBoard, widthOfBoard, numberOfMine);
					fprintf(file, "seed=%d,%d,%d\n", seed, r0, c0);
					fprintf(file, "summonCheckMode=%d\n", summonCheckMode);
					fprintf(file, "time=%d\n", game.t1-game.t0+game.t2);
					fprintf(file, "pos=(%d,%d)\n", ro, co);
					fprintf(file, "isHelped=%d\n", isHelped);
					for(r=0; r<heightOfBoard; r++)
					{
						fprintf(file, "  ");
						for(c=0; c<widthOfBoard; c++)
						{
							fprintf(file, "%d ", isShown[r][c]);
						}
						fprintf(file, "\n");
					}
					fclose(file);
				}
				/*输入和显示用时*/
				printf(">");
				operation = 0;
				r = ro;
				c = co;
				while(choiceMode == 1)
				{
					if(operateMode == 3)
					{
						DrawBoard(0);
						DrawSolution();
						GetWindowOperation(&operation, &r, &c);
						if(operation == '%')//重新生成地图
						{
							game.remainedMine = numberOfMine;
							game.t0 = 0;
							game.t1 = 0;
							game.t2 = 0;
							game.showInformation = 0;
							for(r=0; r<heightOfBoard; r++)
							{
								for(c=0; c<widthOfBoard; c++)
								{
									isShown[r][c] = 0;
									solution[r][c] = 0;
								}
							}
							operation = 0;
							r0 = heightOfBoard/2;
							c0 = widthOfBoard/2;
							while(operation == 0)
							{
								DrawBoard(0);
								GetWindowOperation(&operation, &r0, &c0);
								delay_ms(refreshCycle);
							}
							seed = time(0);
							if(summonCheckMode > 2)//可解地图生成
							{
								seed = SearchSeed(seed, r0, c0, difficulty);
								for(r=0; r<heightOfBoard; r++)
								{
									for(c=0; c<widthOfBoard; c++)
									{
										solution[r][c] = 0;
									}
								}
							}
							SummonBoard(seed, r0, c0, summonCheckMode, 1);
							game.total3BV = BBBV(seed, r0, c0, 3);
							game.unsolved3BV = game.total3BV;
							game.showInformation = 1;
							r = r0;
							c = c0;
							isShown[r][c] = 1;
							isHelped = 0;
							//ClearOperations(operationRecord);
							//operationRecord = AddOperations(seed, r0, c0);
							game.t0 = time(0);
							game.clock0 = clock();
						}
					}
					if(operateMode == 2)
					{
						isReadyRefreshMouseOpenPos = 1;
						isReadyRefreshMouseSignPos = 1;
						GetNumberOfConsoleInputEvents(hdin, &rcdnum);//检索在控制台输入缓冲区中未读取的输入事件记录个数
						if(rcdnum == 0)//无输入时刷新
						{
							ShowInformation();
							//gotoxy(1, game.yOfCommand);//操作指令位置
							gotoxy(cs0+2*c, game.yOfMap+rs0+r);//对应点位置
							showCursor(visibleCursor);//避免调整窗口大小恢复控制台光标
							api_sleep(refreshCycle);
							continue;
						}
						ReadConsoleInput(hdin, &rcd, 1, &rcdnum);
						if(rcd.EventType == MOUSE_EVENT)
						{
							mousePos = rcd.Event.MouseEvent.dwMousePosition;
							if(heightOfBoard > 40)
							{
								//地图高度大于40时不显示控制栏
							}
							else
							{
								choiceTemp = ChoiceControlBar(mousePos, rcd.Event.MouseEvent.dwButtonState);
							}
							if(choiceTemp != 0)
							{
								if(choiceTemp == 1)//停止当前游戏，再次点击进入新游戏
								{
									gotoxy(0, game.yOfInformation+2);
									printf("[已停止游戏，按1进入新游戏]\n");//宽26
									remove("minesweeper-lastmap.txt");
									lastMap = 0;
									choiceMode = 0;
								}
								else if(choiceTemp == 2)//执行实时求解指令!
								{
									operation = '!';
								}
								else if(choiceTemp == 4)//执行实时求解指令Tab
								{
									operation = '\t';
								}
								else if(choiceTemp == 5)
								{
									gotoxy(0, game.yOfInformation+2);
									printf("[已暂停游戏，按1继续游戏]\n");//宽24
									lastMap = 1;
									choiceMode = 5;
								}
								else//choiceTemp == 3，进入设置
								{
									choiceMode = choiceTemp;
								}
							}
							//X=cs0+2*c, Y=yOfMap+rs0+r
							//r=Y-yOfMap-rs0, c=(X-cs0)/2
							if(IsPosInRectangle((mousePos.X-cs0)/2, mousePos.Y-game.yOfMap-rs0, 0, 0, widthOfBoard-1, heightOfBoard-1))
							{
								if(clickSpace == 0 && (mousePos.X-cs0)%2 == 1) continue;//屏蔽空格
								r = mousePos.Y-game.yOfMap-rs0;
								c = (mousePos.X-cs0)/2;
								if((rcd.Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED && touchLeftSign == 0)//通过位运算处理同时按下左右键
									|| (rcd.Event.MouseEvent.dwButtonState & RIGHTMOST_BUTTON_PRESSED && touchLeftSign == 1))
								{
									//if(mousePos.X == mouseOperatedPos.X && mousePos.Y == mouseOperatedPos.Y)
									if(IsPosInRectangle(mousePos.X, mousePos.Y, mouseOpenPos.X-1, mouseOpenPos.Y, mouseOpenPos.X+1, mouseOpenPos.Y))
									{
										//屏蔽翻开时鼠标轻微移动产生的双击
										//存在标记错误时可一定程度防止用户手快翻开周围
										//刷新间隔足够仍可标记后立即翻开
									}
									else
									{
										operation = '@';
										mouseOpenPos = mousePos;
										//break;
									}
									isReadyRefreshMouseOpenPos = 0;
								}
								if((rcd.Event.MouseEvent.dwButtonState & RIGHTMOST_BUTTON_PRESSED && touchLeftSign == 0)
									|| (rcd.Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED && touchLeftSign == 1))
								{
									//if(mousePos.X == mouseOperatedPos.X && mousePos.Y == mouseOperatedPos.Y)
									if(IsPosInRectangle(mousePos.X, mousePos.Y, mouseSignPos.X-1, mouseSignPos.Y, mouseSignPos.X+1, mouseSignPos.Y))
									{
										//屏蔽标记时鼠标轻微移动产生的双击
									}
									else
									{
										operation = '#';
										mouseSignPos = mousePos;
										//break;
									}
									isReadyRefreshMouseSignPos = 0;
								}
							}
							else if(IsPosInRectangle(mousePos.X, mousePos.Y, 0, game.yOfInformation+2, 24, game.yOfInformation+2))
							{
								gotoxy(0, game.yOfInformation+2);
								if(touchLeftSign == 1) ColorStr("[鼠标左键:标记/右键:翻开]", 0xff-backgroundColor);
								else ColorStr("[鼠标左键:翻开/右键:标记]", 0xff-backgroundColor);
								if(rcd.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
								{
									//点击[鼠标左键:翻开/右键:标记]进行左右键切换
									if(touchLeftSign == 0) touchLeftSign = 1;
									else touchLeftSign = 0;
									break;//更新按钮[鼠标左键:标记/右键:翻开]
								}
							}
							else if(choiceMode == 1)//仅游戏时取消悬浮高亮
							{
								gotoxy(0, game.yOfInformation+2);
								if(touchLeftSign == 1) printf("[鼠标左键:标记/右键:翻开]");
								else printf("[鼠标左键:翻开/右键:标记]");
							}
						}
						else if(rcd.EventType == KEY_EVENT && rcd.Event.KeyEvent.bKeyDown == 1)//按下而非松开键盘时
						{
							if(rcd.Event.KeyEvent.wVirtualKeyCode == '1')//!
							{
								//operation = '!';//按1代替!(Shift+1)
								if(GetKeyState(VK_SHIFT) < 0)
								{
									operation = '!';
								}
							}
							if(rcd.Event.KeyEvent.wVirtualKeyCode == '\t')//VK_TAB
							{
								operation = '\t';
							}
							if(WASDMove(&r, &c, rcd.Event.KeyEvent.wVirtualKeyCode));
							else if(rcd.Event.KeyEvent.wVirtualKeyCode == '2'//主键盘2
								|| rcd.Event.KeyEvent.wVirtualKeyCode == 'b'//小键盘2
								|| rcd.Event.KeyEvent.wVirtualKeyCode == 'H')//主键盘H/h
							{
								operation = '@';
							}
							else if(rcd.Event.KeyEvent.wVirtualKeyCode == '3'//主键盘3
								|| rcd.Event.KeyEvent.wVirtualKeyCode == 'c'//小键盘3
								|| rcd.Event.KeyEvent.wVirtualKeyCode == 'U')//主键盘U/u
							{
								operation = '#';
							}
							else if(rcd.Event.KeyEvent.wVirtualKeyCode == 'K'
								|| rcd.Event.KeyEvent.wVirtualKeyCode == 'C')
							{
								SetConsoleMouseMode(0);
								if(rcd.Event.KeyEvent.wVirtualKeyCode == 'C') operateMode = 0;
								else operateMode = 1;
								break;
							}
							else if(rcd.Event.KeyEvent.wVirtualKeyCode == 'Q')
							{
								if(questionMarkMode == 0)//进入问号模式
								{
									questionMarkMode = 1;
									QuestionMark('i', 0, 0);
								}
								else//退出问号模式
								{
									questionMarkMode = 0;
									QuestionMark('c', 0, 0);
								}
							}
							else if(rcd.Event.KeyEvent.wVirtualKeyCode == 'E')
							{
								if(questionMarkMode == 1)//执行并退出问号模式
								{
									questionMarkMode = 0;
									QuestionMark('\t', game.yOfMap, 0);
								}
							}
						}
						//刷新防双击坐标
						if(isReadyRefreshMouseOpenPos == 1)
						{
							mouseOpenPos.X = 0;
							mouseOpenPos.Y = 0;
						}
						if(isReadyRefreshMouseSignPos == 1)
						{
							mouseSignPos.X = 0;
							mouseSignPos.Y = 0;
						}
					}
					if(operateMode < 2 && kbhit_console())//按键
					{
						key = getch_console();//接收字符但不显示
						if(key == '`')//清空已输入操作
						{
							operation = 0;
							r = ro;
							c = co;
						}
						if(key == '!' || key == '\t')
						{
							operation = key;
						}
						//操作模式切换
						if((key == '@' || key == '#' || key == '$') && operateMode == 1)
						{
							gotoxy(0, game.yOfCommand);
							printf(">        ");//覆写@ 128 93
							gotoxy(1, game.yOfCommand);
							operateMode = 0;
							break;//立即更新操作提示
						}
						if((key == 'w' || key == 'W' || key == 'a' || key == 'A' || key == 's' || key == 'S'
							|| key == 'd' || key == 'D') && operateMode == 0)
						{
							operateMode = 1;
							break;//立即更新操作提示
						}
						if((key == 'm' || key == 'M') && operateMode != 2)
						{
							SetConsoleMouseMode(1);
							operateMode = 2;
							break;
						}
						if((key == 'c' || key == 'C') && operateMode != 0)
						{
							operateMode = 0;
							break;
						}
						if(key == 'q' || key == 'Q')
						{
							if(questionMarkMode == 0)
							{
								questionMarkMode = 1;
								QuestionMark('i', 0, 0);
							}
							else
							{
								questionMarkMode = 0;
								QuestionMark('c', 0, 0);
							}
						}
						if(key == 'e' || key == 'E')
						{
							if(questionMarkMode == 1)
							{
								questionMarkMode = 0;
								QuestionMark('\t', game.yOfMap, 0);
							}
						}
						//操作识别
						if(operateMode == 0)
						{
							if(key == '@' || key == '#' || key == '$')
							{
								operation = key;
								//gotoxy(0, game.yOfCommand);
								//printf(">%c ", operation);
								//scanf("%d%d", &r, &c);
								RCScan(&operation, &r, &c, game.yOfCommand);
								if(operation == 'm')
								{
									SetConsoleMouseMode(1);
									operateMode = 2;
								}
								break;
							}
						}
						if(operateMode == 1)//wasd操作设计
						{
							if((key == 'w' || key == 'W') && r > 0) r--;//上移
							if((key == 'a' || key == 'A') && c > 0) c--;//左移
							if((key == 's' || key == 'S') && r < heightOfBoard-1) r++;//下移
							if((key == 'd' || key == 'D') && c < widthOfBoard-1) c++;//下移
							if(key == '2' || key == 'h' || key == 'H') operation = '@';//2翻开
							if(key == '3' || key == 'u' || key == 'U') operation = '#';//3标记
							//if(operation != 0) break;
						}
					}
					//实时求解指令
					if(operation == '!' || operation == '\t')
					{
						ShownModeBak(1);
						LookMap();
						gotoxy(0, game.yOfCommand+1);//输出debug信息
						Solve(0);//执行当前可知所有Solution
						//printf("                     ");//覆写[Loading...]8192/8192，21空格
						ShowSolution(game.yOfMap);
						isHelped = 1;//记录使用过实时求解指令，不参与纪录计算
						if(operation == '!')
						{
							ShownModeBak(0);
							operation = 0;
							r = ro;
							c = co;
						}
						else if(operation == '\t')
						{
							api_sleep(refreshCycle);
							for(r=0; r<heightOfBoard; r++)
							{
								for(c=0; c<widthOfBoard; c++)
								{
									if(solution[r][c] == 1)
									{
										if(board[r][c] == 0)//翻开0连锁翻开
										{
											OpenZeroChain(r,c);
										}
										//!指令的Solve()不会翻开雷
									}
									else if(solution[r][c] == 2)
									{
										game.remainedMine--;
									}
								}
							}
							r = ro;
							c = co;
							break;
						}
					}
					if(questionMarkMode == 1)
					{
						QuestionMark('s', game.yOfMap, 0);
					}
					//刷新
					game.clock1 = clock();
					//temp = BBBV(seed, r0, c0, 0);//刷新3BV
					ShowInformation();
					//AddOperation(&operationRecord, game.clock1-game.clock0, operation, r, c);
					if(operateMode > 0)
					{
						//显示操作对应指令
						gotoxy(0, game.yOfCommand);
						if(operation == 0) printf(">? %d %d  ", r, c);//覆写尾部，(10,10)到(9,9)需2个空格
						else printf(">%c %d %d", operation, r, c);
						//刷新光标
						FreshCursor(r, c, game.yOfMap);
						if(operation != 0)
						{
							if(operateMode == 3) delay_ms(0);
							//api_sleep(refreshCycle);//保持延迟统一，通过降低按键跟手性减弱轻浮感
							gotoxy(0, game.yOfCommand+1);
							break;
						}
					}
					if(operateMode == 3) delay_ms(0);
					api_sleep(refreshCycle);//每50ms刷新一次
				}
				ro = r;
				co = c;
				/*问号模式操作*/
				if(questionMarkMode == 1)
				{
					QuestionMark(operation, r, c);
					continue;
				}
				/*执行操作*/
				if(touchOpen == 1 && operateMode >= 2 && operation == '#' && isShown[r][c] == 1
					&& NumberOfSignAround(r, c) == numberOfMineAround[r][c])
				{
					operation = '@';//触动翻开，允许鼠标点击使用标记键快速翻开
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
						//鼠标点击空地切换左右键
						if(board[r][c] == 0 && operateMode >= 2 && touchMode == 1 && touchLeftSign == 0) touchLeftSign = 1;
					}
					else if(isShown[r][c] == 2)
					{
						game.remainedMine++;//取消标记，剩余雷数+1
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
						game.remainedMine--;
					}
					else if(isShown[r][c] == 2)//取消标记
					{
						isShown[r][c] = 0;
						game.remainedMine++;
					}
					else if(isShown[r][c] == 1)//#数字时快速标记周围
					{
						//鼠标点击空地切换左右键
						if(board[r][c] == 0 && operateMode >= 2 && touchMode == 1 && touchLeftSign == 1) touchLeftSign = 0;
						if(fastSign == 1)
						{
							if(numberOfMineAround[r][c] == NumberOfNotShownAround(r, c) + NumberOfSignAround(r, c))
							{
								for(ra=r-1; ra<=r+1; ra++)//周围(%>0且)%+#=数字时
								{
									for(ca=c-1; ca<=c+1; ca++)
									{
										if(ra>=0 && ra<heightOfBoard && ca>=0 && ca<widthOfBoard)//确认在范围内
										{
											if(isShown[ra][ca] == 0)//标记所有%
											{
												isShown[ra][ca] = 2;
												game.remainedMine--;
											}
										}
									}
								}
							}
							else if(NumberOfNotShownAround(r, c) == 0 || NumberOfSignAround(r, c) > numberOfMineAround[r][c])
							{
								for(ra=r-1; ra<=r+1; ra++)//周围方块全被标记时，%=0(且#>0)，周围标记数大于数字时
								{
									for(ca=c-1; ca<=c+1; ca++)
									{
										if(ra>=0 && ra<heightOfBoard && ca>=0 && ca<widthOfBoard)//确认在范围内
										{
											if(isShown[ra][ca] == 2)//一键取消标记
											{
												isShown[ra][ca] = 0;
												game.remainedMine++;
											}
										}
									}
								}
							}
						}
						else
						{
							//printf(":(\n该坐标已翻开！\n");
							//system("pause");
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
						clrscr();
					}
				}
				/*else
				{
					printf(":(\n未选择操作模式！\n");
				}*/
			}
			/*游戏结束*/
			if(choiceMode == 5)//暂停
			{
				game.t2 = game.t1-game.t0+game.t2;//更新时间
				choiceMode = 0;//返回主页
			}
			else if(choiceMode == 0)//完成地图退出
			{
				newRecord.numberOfMine = numberOfMine;
				newRecord.heightOfBoard = heightOfBoard;
				newRecord.widthOfBoard = widthOfBoard;
				newRecord.summonMode = summonCheckMode;
				newRecord.iterateMode = 1;
				newRecord.seed = seed;
				newRecord.r0 = r0;
				newRecord.c0 = c0;
				newRecord.sTime = game.t1-game.t0+game.t2;
				newRecord.msTime = game.clock1-game.clock0 + game.t2*1000;
				newRecord.solved3BV = game.total3BV-game.unsolved3BV;
				newRecord.total3BV = game.total3BV;
				newRecord.isHelped = isHelped;
				newRecord.difficulty = difficulty;
				records = AddRecord(records, newRecord);
				WriteRecords(records);
				/*operationRecord.time = newRecord.sTime;
				operationRecord.isWin = 1-isOpenMine;
				if(IsEffectiveRecord(newRecord)) WriteOperations(operationRecord);
				ClearOperations(operationRecord);*/
				if(operateMode == 3) choiceMode = CloseWindow(1-isOpenMine);
			}
		}
		else if(choiceMode == 2)//地图求解
		{
			clrscr();
			/*DrawControlBar(0);
			gotoxy(0, 1);
			printf("** 键盘模式鼠标全选含坐标地图，复制粘贴并回车|按 ` 回车退出 **\n");
			gotoxy(0, 3);*/
			SetConsoleMouseMode(0);
			showCursor(1);
			Solution();//Solution模块
			showCursor(visibleCursor);
			clrscr();
			DrawControlBar(0);
			SetConsoleMouseMode(1);
			choiceMode = 0;
		}
		else if(choiceMode == 3)//设置
		{
			clrscr();
			DrawControlBar(0);
			DrawSettingsBar(0);
			choiceSet = 0;
			while(choiceMode == 3 && choiceSet != 9)
			{
				ReadConsoleInput(hdin, &rcd, 1, &rcdnum);
				if(rcd.EventType == MOUSE_EVENT)
				{
					mousePos = rcd.Event.MouseEvent.dwMousePosition;
					choiceTemp = ChoiceControlBar(mousePos, rcd.Event.MouseEvent.dwButtonState);
					if(choiceTemp != 0) choiceMode = choiceTemp;
					choiceTemp = ChoiceSettingsBar(mousePos, rcd.Event.MouseEvent.dwButtonState);
					if(choiceTemp != 0) choiceSet = choiceTemp;
				}
				else if(rcd.EventType == KEY_EVENT && rcd.Event.KeyEvent.bKeyDown == 1)
				{
					if(rcd.Event.KeyEvent.wVirtualKeyCode >= '1' && rcd.Event.KeyEvent.wVirtualKeyCode <= '9')
					{
						choiceSet = rcd.Event.KeyEvent.wVirtualKeyCode-'1'+1;//主键盘1-9
					}
					else if(rcd.Event.KeyEvent.wVirtualKeyCode >= 'a' && rcd.Event.KeyEvent.wVirtualKeyCode <= 'i')
					{
						choiceSet = rcd.Event.KeyEvent.wVirtualKeyCode-'a'+1;//小键盘1-9
					}
					else if(rcd.Event.KeyEvent.wVirtualKeyCode == '0' || rcd.Event.KeyEvent.wVirtualKeyCode == '`')//主键盘或小键盘0
					{
						clrscr();
						DrawControlBar(0);
						choiceMode = 0;//不保存直接返回主页
					}
				}
				if(choiceSet == 1)//设置游戏难度
				{
					ReallocMemory(42, 88, dictionaryCapacity, lengthOfThinkMineCheck);//申请顶级难度内存空间
					while(rcd.EventType == MOUSE_EVENT && rcd.Event.MouseEvent.dwButtonState != 0)//设置游戏难度抗双击
					{
						ReadConsoleInput(hdin, &rcd, 1, &rcdnum);
						api_sleep(refreshCycle);
					}
					clrscr();
					DrawSetDifficulty(-1);
					difficulty = -1;
					while(difficulty == -1)
					{
						ReadConsoleInput(hdin, &rcd, 1, &rcdnum);
						if(rcd.EventType == MOUSE_EVENT)
						{
							mousePos = rcd.Event.MouseEvent.dwMousePosition;
							choiceTemp = ChoiceSetDifficulty(mousePos, rcd.Event.MouseEvent.dwButtonState);
							if(choiceTemp != -1) difficulty = choiceTemp;
						}
						else if(rcd.EventType == KEY_EVENT && rcd.Event.KeyEvent.bKeyDown == 1)
						{
							if(rcd.Event.KeyEvent.wVirtualKeyCode >= '0' && rcd.Event.KeyEvent.wVirtualKeyCode <= '5')
							{
								difficulty = rcd.Event.KeyEvent.wVirtualKeyCode-'0'+0;//主键盘0-5
							}
							else if(rcd.Event.KeyEvent.wVirtualKeyCode == '`')//小键盘0
							{
								difficulty = 0;
							}
							else if(rcd.Event.KeyEvent.wVirtualKeyCode >= 'a' && rcd.Event.KeyEvent.wVirtualKeyCode <= 'e')
							{
								difficulty = rcd.Event.KeyEvent.wVirtualKeyCode-'a'+1;//小键盘1-5
							}
						}
						showCursor(visibleCursor);
						api_sleep(refreshCycle);
					}
					while(rcd.EventType == MOUSE_EVENT && rcd.Event.MouseEvent.dwButtonState != 0)//设置游戏难度返回抗双击
					{
						ReadConsoleInput(hdin, &rcd, 1, &rcdnum);
						api_sleep(refreshCycle);
					}
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
						//show3BV = 0;//自动隐藏3BV
					}
					else//自定义
					{
						DrawSetDifficulty(7);
						SetConsoleMouseMode(0);
						showCursor(1);
						//设置界面高度
						gotoxy(0, 8);
						printf("[行数]>");
						scanf("%d", &heightOfBoard);
						if(heightOfBoard < 1) heightOfBoard = 1;
						if(heightOfBoard > LimHeight) heightOfBoard = LimHeight;
						gotoxy(13, 6);
						printf("%d", heightOfBoard);
						//设置界面宽度
						gotoxy(0, 9);
						printf("[列数]>");
						scanf("%d", &widthOfBoard);
						if(widthOfBoard < 1) widthOfBoard = 1;
						if(widthOfBoard > LimWidth) widthOfBoard = LimWidth;
						gotoxy(13, 6);
						printf("%d*%d", heightOfBoard, widthOfBoard);
						//界面校正
						/*if(widthOfBoard == LimWidth && heightOfBoard > 100)
						{
							printf(":(\n界面不足以容纳行坐标轴！\n");
							printf("(1)维持当前界面高度，界面宽度-1\n");
							printf("(2)维持当前界面宽度，界面高度设为100\n");
							printf("(3)坚持设置！\n");
							printf(">");
							scanf("%d", &temp);
							if(temp == 1) widthOfBoard--;
							else if(temp == 2) heightOfBoard = 100;
						}*/
						//设置雷数
						gotoxy(0, 10);
						printf("[雷数]>");
						scanf("%d", &numberOfMine);
						if(numberOfMine < 0) numberOfMine = 0;
						if(numberOfMine >= heightOfBoard * widthOfBoard)
						{
							numberOfMine = heightOfBoard * widthOfBoard;//雷数校正
							//summonCheckMode = 0;//起始点必为雷
						}
						/*else if(numberOfMine > heightOfBoard * widthOfBoard * 715/1000
							|| numberOfMine > heightOfBoard * widthOfBoard - 9)//调整地图生成校验
						{
							summonCheckMode = 1;//起始点难以为空
						}*/
						//更新密度
						/*density =(float) numberOfMine / heightOfBoard / widthOfBoard;
						if(density > 0.72) printf(":(\n当前密度%.2f，难以生成地图\n", density);
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
						}*/
						difficulty = Difficulty(heightOfBoard, widthOfBoard, numberOfMine);
						//重新申请矩阵内存空间
						ReallocMemory(heightOfBoard, widthOfBoard, dictionaryCapacity, lengthOfThinkMineCheck);
						//显示预览
						clrscr();
						DrawSetDifficulty(-1);
						gotoxy(13, 6);
						printf("%d*%d-%d", heightOfBoard, widthOfBoard, numberOfMine);
						SummonBoard(0, heightOfBoard/2, widthOfBoard/2, summonCheckMode, 1);
						gotoxy(0, 8);
						ShowBoard(1);
						//gotoxy(0, 0);//防止地图过高时界面下滑
						//gotoxy(62, 7);
						system("pause");
						showCursor(visibleCursor);
						SetConsoleMouseMode(1);
					}
					if(difficulty >= 0 && difficulty <= 4)
					{
						//除自定义难度，释放顶级难度内存空间，并申请新难度内存空间
						ReallocMemory(heightOfBoard, widthOfBoard, dictionaryCapacity, lengthOfThinkMineCheck);
					}
					clrscr();
					DrawControlBar(0);
					DrawSettingsBar(0);
					choiceSet = 0;
				}
				else if(choiceSet == 2)//设置地图生成校验
				{
					choiceSubSet = -1;
					DrawSetSummonCheckMode(-1);
					while(choiceMode == 3 && choiceSet == 2 && choiceSubSet == -1)
					{
						ReadConsoleInput(hdin, &rcd, 1, &rcdnum);
						if(rcd.EventType == MOUSE_EVENT)
						{
							mousePos = rcd.Event.MouseEvent.dwMousePosition;
							choiceTemp = ChoiceControlBar(mousePos, rcd.Event.MouseEvent.dwButtonState);
							if(choiceTemp != 0) choiceMode = choiceTemp;
							choiceTemp = ChoiceSettingsBar(mousePos, rcd.Event.MouseEvent.dwButtonState);
							if(choiceTemp != 0) choiceSet = choiceTemp;
							choiceTemp = ChoiceSetSummonCheckMode(mousePos, rcd.Event.MouseEvent.dwButtonState);
							if(choiceTemp != -1) choiceSubSet = choiceTemp;
						}
						else if(rcd.EventType == KEY_EVENT && rcd.Event.KeyEvent.bKeyDown == 1)
						{
							if(rcd.Event.KeyEvent.wVirtualKeyCode >= '0' && rcd.Event.KeyEvent.wVirtualKeyCode <= '5')
							{
								choiceSubSet = rcd.Event.KeyEvent.wVirtualKeyCode-'0'+0;//主键盘0-5
							}
							else if(rcd.Event.KeyEvent.wVirtualKeyCode == '`')//小键盘0
							{
								choiceSubSet = 0;
							}
							else if(rcd.Event.KeyEvent.wVirtualKeyCode >= 'a' && rcd.Event.KeyEvent.wVirtualKeyCode <= 'e')
							{
								choiceSubSet = rcd.Event.KeyEvent.wVirtualKeyCode-'a'+1;//小键盘1-5
							}
						}
						if(choiceSubSet != -1 && choiceSubSet != 5) summonCheckMode = choiceSubSet;
						else if(choiceSubSet == 5)//设置筛选
						{
							DrawSetChosenBoard(-1);
							temp = -1;
							while(choiceMode == 3 && choiceSet == 2 && choiceSubSet == 5 && temp == -1)
							{
								ReadConsoleInput(hdin, &rcd, 1, &rcdnum);
								if(rcd.EventType == MOUSE_EVENT)
								{
									mousePos = rcd.Event.MouseEvent.dwMousePosition;
									choiceTemp = ChoiceControlBar(mousePos, rcd.Event.MouseEvent.dwButtonState);
									if(choiceTemp != 0) choiceMode = choiceTemp;
									choiceTemp = ChoiceSettingsBar(mousePos, rcd.Event.MouseEvent.dwButtonState);
									if(choiceTemp != 0) choiceSet = choiceTemp;
									choiceTemp = ChoiceSetSummonCheckMode(mousePos, rcd.Event.MouseEvent.dwButtonState);
									if(choiceTemp != -1) choiceSubSet = choiceTemp;
									choiceTemp = ChoiceSetChosenBoard(mousePos, rcd.Event.MouseEvent.dwButtonState);
									if(choiceTemp != -1) temp = choiceTemp;
								}
								else if(rcd.EventType == KEY_EVENT && rcd.Event.KeyEvent.bKeyDown == 1)
								{
									if(rcd.Event.KeyEvent.wVirtualKeyCode >= '0' && rcd.Event.KeyEvent.wVirtualKeyCode <= '8')
									{
										temp = rcd.Event.KeyEvent.wVirtualKeyCode-'0'+0;//主键盘0-8
									}
									else if(rcd.Event.KeyEvent.wVirtualKeyCode == '`')//小键盘0
									{
										temp = 0;
									}
									else if(rcd.Event.KeyEvent.wVirtualKeyCode >= 'a' && rcd.Event.KeyEvent.wVirtualKeyCode <= 'h')
									{
										temp = rcd.Event.KeyEvent.wVirtualKeyCode-'a'+1;//小键盘1-8
									}
								}
								api_sleep(refreshCycle);
							}
							if(temp >= 0 && temp <= 5)
							{
								SetConsoleMouseMode(0);
								showCursor(1);
								fflush(stdin);
								printf("[最小值]>");
								scanf("%d", &chosen3BVMin[temp]);
								if(chosen3BVMin[temp] < 0) chosen3BVMin[temp] = 0;
								printf("[最大值]>");
								scanf("%d", &chosen3BVMax[temp]);
								if(chosen3BVMax[temp] < 0) chosen3BVMax[temp] = 0;
								if(chosen3BVMin[temp] > chosen3BVMax[temp]) chosen3BVMin[temp] = chosen3BVMax[temp];
								showCursor(visibleCursor);
								SetConsoleMouseMode(1);
							}
							else if(temp == 6)
							{
								if(chosenSolvable == 1) chosenSolvable = 0;
								else chosenSolvable = 1;
							}
							else if(temp == 7)
							{
								SetConsoleMouseMode(0);
								showCursor(1);
								fflush(stdin);
								printf("[数字]>");
								scanf("%d", &chosenNumber);
								if(chosenNumber < 0) chosenNumber = 0;
								if(chosenNumber > 9) chosenNumber = 9;
								showCursor(visibleCursor);
								SetConsoleMouseMode(1);
							}
							clrscr();
							DrawControlBar(0);
							DrawSettingsBar(0);
							DrawSetSummonCheckMode(-1);
							choiceSubSet = -1;
						}
						api_sleep(refreshCycle);
					}
					clrscr();
					DrawControlBar(0);
					DrawSettingsBar(0);
					choiceSet = 0;
				}
				else if(choiceSet == 3)//更改显示用时和3BV
				{
					if(showTime == 0 && show3BV == 0) showTime = 1;
					else if(showTime == 1 && show3BV == 0) show3BV = 1;
					else if(showTime == 1 && show3BV == 1) showTime = 0;
					else if(showTime == 0 && show3BV == 1) show3BV = 0;
					else//默认为showTime == 1 && show3BV == 0
					{
						showTime = 1;
						show3BV = 1;
					}
					clrscr();
					DrawControlBar(0);
					DrawSettingsBar(0);//立即刷新页面
					choiceSet = 0;
				}
				else if(choiceSet == 4)//浅色模式/深色模式
				{
					if(backgroundColor == 0x07)
					{
						//system("color f0");
						backgroundColor = 0xf0;
					}
					else
					{
						//system("color 07");
						backgroundColor = 0x07;
					}
					setbgcolor(backgroundColor);
					clrscr();
					DrawControlBar(0);
					DrawSettingsBar(0);
					choiceSet = 0;
				}
				else if(choiceSet == 5)//设置默认操作模式
				{
					choiceSubSet = -1;
					DrawSetOperateMode(-1);
					while(choiceMode == 3 && choiceSet == 5 && choiceSubSet == -1)
					{
						ReadConsoleInput(hdin, &rcd, 1, &rcdnum);
						if(rcd.EventType == MOUSE_EVENT)
						{
							mousePos = rcd.Event.MouseEvent.dwMousePosition;
							choiceTemp = ChoiceControlBar(mousePos, rcd.Event.MouseEvent.dwButtonState);
							if(choiceTemp != 0) choiceMode = choiceTemp;
							choiceTemp = ChoiceSettingsBar(mousePos, rcd.Event.MouseEvent.dwButtonState);
							if(choiceTemp != 0) choiceSet = choiceTemp;
							choiceTemp = ChoiceSetOperateMode(mousePos, rcd.Event.MouseEvent.dwButtonState);
							if(choiceTemp != -1) choiceSubSet = choiceTemp;
						}
						else if(rcd.EventType == KEY_EVENT && rcd.Event.KeyEvent.bKeyDown == 1)
						{
							if(rcd.Event.KeyEvent.wVirtualKeyCode >= '0' && rcd.Event.KeyEvent.wVirtualKeyCode <= '3')
							{
								choiceSubSet = rcd.Event.KeyEvent.wVirtualKeyCode-'0'+0;//主键盘0-3
							}
							else if(rcd.Event.KeyEvent.wVirtualKeyCode == '`')//小键盘0
							{
								choiceSubSet = 0;
							}
							else if(rcd.Event.KeyEvent.wVirtualKeyCode >= 'a' && rcd.Event.KeyEvent.wVirtualKeyCode <= 'c')
							{
								choiceSubSet = rcd.Event.KeyEvent.wVirtualKeyCode-'a'+1;//小键盘1-3
							}
						}
						api_sleep(refreshCycle);
					}
					if(choiceSubSet != -1) operateMode = choiceSubSet;
					/*if(operateMode == 1) visibleCursor = 1;//仅wasd23显示光标 
					else visibleCursor = 0;
					showCursor(visibleCursor);*/
					clrscr();
					DrawControlBar(0);
					DrawSettingsBar(0);
					choiceSet = 0;
				}
				else if(choiceSet == 6)//快速标记（#数字快速标记周围）
				{
					if(fastSign == 0) fastSign = 1;
					else fastSign = 0;
					clrscr();
					DrawControlBar(0);
					DrawSettingsBar(0);
					choiceSet = 0;
				}
				else if(choiceSet == 7)//设置光标
				{
					choiceSubSet = 0;
					DrawSetCursor(0);
					while(choiceMode == 3 && choiceSet == 7 && choiceSubSet == 0)
					{
						ReadConsoleInput(hdin, &rcd, 1, &rcdnum);
						if(rcd.EventType == MOUSE_EVENT)
						{
							mousePos = rcd.Event.MouseEvent.dwMousePosition;
							choiceTemp = ChoiceControlBar(mousePos, rcd.Event.MouseEvent.dwButtonState);
							if(choiceTemp != 0) choiceMode = choiceTemp;
							choiceTemp = ChoiceSettingsBar(mousePos, rcd.Event.MouseEvent.dwButtonState);
							if(choiceTemp != 0) choiceSet = choiceTemp;
							choiceTemp = ChoiceSetCursor(mousePos, rcd.Event.MouseEvent.dwButtonState);
							if(choiceTemp != 0) choiceSubSet = choiceTemp;
						}
						else if(rcd.EventType == KEY_EVENT && rcd.Event.KeyEvent.bKeyDown == 1)
						{
							if(rcd.Event.KeyEvent.wVirtualKeyCode >= '1' && rcd.Event.KeyEvent.wVirtualKeyCode <= '4')
							{
								choiceSubSet = rcd.Event.KeyEvent.wVirtualKeyCode-'1'+1;//主键盘1-4
							}
							else if(rcd.Event.KeyEvent.wVirtualKeyCode >= 'a' && rcd.Event.KeyEvent.wVirtualKeyCode <= 'd')
							{
								choiceSubSet = rcd.Event.KeyEvent.wVirtualKeyCode-'a'+1;//小键盘1-4
							}
							else if(rcd.Event.KeyEvent.wVirtualKeyCode == '5' || rcd.Event.KeyEvent.wVirtualKeyCode == 'e')
							{
								choiceSubSet = 5;//键盘按5退出
							}
						}
						if(choiceSubSet == 1)//控制台光标
						{
							if(visibleCursor == 1) visibleCursor = 0;
							else visibleCursor = 1;
							showCursor(visibleCursor);
							clrscr();
							DrawControlBar(0);
							DrawSettingsBar(0);
							DrawSetCursor(0);
							choiceSubSet = 0;
						}
						else if(choiceSubSet == 2)//自选光标
						{
							if(newCursor < 3) newCursor++;
							else newCursor = 0;
							//自动隐藏乱动的控制台光标
							if(newCursor == 0 && visibleCursor == 0)
							{
								visibleCursor = 1;
								showCursor(1);
								//printf("已自动显示控制台光标\n");
								//system("pause");
							}
							else if(newCursor > 0 && visibleCursor == 1)
							{
								visibleCursor = 0;
								showCursor(0);
								//printf("已自动隐藏控制台光标\n");
								//system("pause");
							}
							clrscr();
							DrawControlBar(0);
							DrawSettingsBar(0);
							DrawSetCursor(0);
							choiceSubSet = 0;
						}
						else if(choiceSubSet == 3)//自定义><光标字符
						{
							SetConsoleMouseMode(0);
							showCursor(1);
							fflush(stdin);
							printf("[左]>");
							cursorLeft[0] = getchar();
							getchar();
							printf("[右]>");
							cursorRight[0] = getchar();
							showCursor(visibleCursor);
							SetConsoleMouseMode(1);
						}
						else if(choiceSubSet == 4)//保持光标显示
						{
							if(keepCursor == 0) keepCursor = 1;
							else keepCursor = 0;
							clrscr();
							DrawControlBar(0);
							DrawSettingsBar(0);
							DrawSetCursor(0);
							choiceSubSet = 0;
						}
						api_sleep(refreshCycle);
					}
					clrscr();
					DrawControlBar(0);
					DrawSettingsBar(0);
					choiceSet = 0;
				}
				else if(choiceSet == 8)//调试选项
				{
					gotoxy(0, 14);
					printf("*******************************\n"//宽31
						   "调试选项仅提供给专业玩家和开发者使用。\n"
						   "除非你知道你在做什么，否则请退出设置。\n"
						   "(1)退出\n"
						   "(2)设置地图求解器\n"
						   "(3)以密度设置雷数\n"
						   "(4)设置背景颜色\n"
						   "(5)设置刷新周期\n");
					if(fastShow == 0) printf("(6)启用快速显示\n");
					else printf("(6)关闭快速显示\n");
					printf("(7)设置鼠标点击\n");
					printf("(8)设置Bench\n");
					printf("*******************************\n");
					SetConsoleMouseMode(0);
					//getchar();
					fflush(stdin);//清空不稳定的键盘缓冲
					printf(">");
					showCursor(1);
					operation = getchar();
					if(operation == '$')//调试
					{
						scanf("%d", &temp);
						if(temp == debug)
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
					else if(operation == '2')//设置地图求解器
					{
						printf("*******************************\n");//宽31
						printf("(1)设置求解模式     当前:%d\n", solveMode);
						printf("(2)设置枚举限制     当前:NC%dMC%d\n", lengthOfThinkNumberCheck, lengthOfThinkMineCheck);
						printf("(3)设置字典大小     当前:%d\n", dictionaryCapacity);
						printf("(4)设置剩余雷数判断 当前:%d%d%d\n", remainedMineJudgeLocalNC, remainedMineJudgeLocalMC, remainedMineJudgeGlobal);
						if(islandPrediction == 0) printf("(5)启用可解地图岛预判加速\n");
						else printf("(5)关闭可解地图岛预判加速\n");
						printf("(6)退出\n");
						printf("*******************************\n");
						printf(">");
						scanf("%d", &temp);
						if(temp == 1)//设置求解模式
						{
							printf("*******************************\n"//宽31
								   "(0)简单模式：有翻开解时立即停止\n"
								   "(1)游戏模式：先不枚举，无解则枚举\n"
								   "(2)分析模式：始终以设置的限制枚举\n"
								   "*******************************\n");
							printf("/set solveMode ");
							scanf("%d", &temp);
							if(temp >= 0 && temp <= 2) solveMode = temp;
						}
						else if(temp == 2)//设置枚举限制(NC/MC)
						{
							printf("*******************************\n"//宽31
								   "** 0 ------ 15 ---------- %d **\n"
								   "** 关      效率         最大 **\n"
								   "*******************************\n", 30);
							/*printf("/set lengthOfThinkChain ");
							scanf("%d", &temp);
							if(temp >= 0 && temp <= LimLength) lengthOfThinkChain = temp;*/
							printf("/set lengthOfThinkNumberCheck ");
							scanf("%d", &temp);
							if(temp >= 0 && temp <= 30) lengthOfThinkNumberCheck = temp;
							printf("*******************************\n"//宽31
								   "** %d ------- 19 -------- %d **\n"
								   "** 关        效率       最大 **\n"
								   "*******************************\n", lengthOfThinkNumberCheck, LimLength);
							printf("/set lengthOfThinkMineCheck ");
							scanf("%d", &temp);
							if(temp > LimLength) temp = LimLength;//修改文件可无视此限制(doge)
							if(temp >= lengthOfThinkNumberCheck) lengthOfThinkMineCheck = temp;
							else lengthOfThinkMineCheck = lengthOfThinkNumberCheck;
							ReallocMemory(heightOfBoard, widthOfBoard, dictionaryCapacity, lengthOfThinkMineCheck);
						}
						else if(temp == 3)//设置字典大小
						{
							printf("/set dictionaryCapacity ");
							scanf("%d", &temp);
							if(temp < 0) temp = 0;
							if(temp > LimDictionary) temp = LimDictionary;
							dictionaryCapacity = temp;
							ReallocMemory(heightOfBoard, widthOfBoard, dictionaryCapacity, lengthOfThinkMineCheck);
						}
						else if(temp == 4)//设置剩余雷数判断
						{
							printf("*******************************\n");//宽31
							if(remainedMineJudgeLocalNC == 1) printf("(1)关闭NC根据剩余雷数排除\n");
							else printf("(1)启用NC根据剩余雷数排除\n");
							if(remainedMineJudgeLocalMC == 0) printf("(2)启用MC根据剩余雷数排除\n");
							else printf("(2)关闭MC根据剩余雷数排除\n");
							if(remainedMineJudgeGlobal == 0) printf("(3)启用全局剩余雷数判断\n");
							else printf("(3)关闭全局剩余雷数判断\n");
							printf("(4)退出\n");
							printf("*******************************\n");
							printf(">");
							scanf("%d", &temp);
							if(temp == 1)
							{
								if(remainedMineJudgeLocalNC == 1) remainedMineJudgeLocalNC = 0;
								else remainedMineJudgeLocalNC = 1;
							}
							else if(temp == 2)
							{
								if(remainedMineJudgeLocalMC == 0) remainedMineJudgeLocalMC = 1;
								else remainedMineJudgeLocalMC = 0;
							}
							else if(temp == 3)
							{
								if(remainedMineJudgeGlobal == 0) remainedMineJudgeGlobal = 1;
								else remainedMineJudgeGlobal = 0;
							}
						}
						else if(temp == 5)//可解地图岛预判加速
						{
							if(islandPrediction == 0) islandPrediction = 1;
							else islandPrediction = 0;
						}
					}
					else if(operation == '3')//以密度设置雷数
					{
						printf(">");
						scanf("%f", &density);
						numberOfMine = density * heightOfBoard * widthOfBoard;
						difficulty = Difficulty(heightOfBoard, widthOfBoard, numberOfMine);
						printf("/set numberOfMine %d\n", numberOfMine);
						system("pause");
					}
					else if(operation == '4')//设置背景颜色
					{
						printf("*******************************\n"//宽31
							   "[背景颜色][字体颜色]\n"
							   "0 = 黑色       8 = 灰色\n"
							   "1 = 蓝色       9 = 淡蓝色\n"
							   "2 = 绿色       A = 淡绿色\n"
							   "3 = 浅绿色     B = 淡浅绿色\n"
							   "4 = 红色       C = 淡红色\n"
							   "5 = 紫色       D = 淡紫色\n"
							   "6 = 黄色       E = 淡黄色\n"
							   "7 = 白色       F = 亮白色\n");
						ColorStr("0 ", 0x00);
						ColorStr("1 ", 0x10);
						ColorStr("2 ", 0x20);
						ColorStr("3 ", 0x30);
						ColorStr("4 ", 0x40);
						ColorStr("5 ", 0x50);
						ColorStr("6 ", 0x60);
						ColorStr("7 ", 0x70);
						ColorStr("8 ", 0x80);
						ColorStr("9 ", 0x90);
						ColorStr("a ", 0xa0);
						ColorStr("b ", 0xb0);
						ColorStr("c ", 0xc0);
						ColorStr("d ", 0xd0);
						ColorStr("e ", 0xe0);
						ColorStr("f ", 0xf0);
						printf("\n");
						printf("默认深色模式为07，浅色模式为f0\n"
							   "*******************************\n");
						printf("当前颜色：0x%x\n", backgroundColor);
						printf("/set backgroundColor 0x");
						scanf("%x", &backgroundColor);
						setbgcolor(backgroundColor);
					}
					else if(operation == '5')//设置刷新周期
					{
						printf("当前刷新周期：%dms|当前刷新率：%dHz\n", refreshCycle, 1000/refreshCycle);
						printf("/set refreshCycle ");
						scanf("%d", &temp);
						if(temp < 1) refreshCycle = 1;
						else refreshCycle = temp;
					}
					else if(operation == '6')//快速显示
					{
						if(fastShow == 0) fastShow = 1;
						else fastShow = 0;
					}
					else if(operation == '8')//设置Bench
					{
						printf("*******************************\n");//宽31
						printf("(1)设置种子范围 当前:%d-%d\n", benchSeedMin, benchSeedMax);
						printf("(2)设置帧显示   当前:");
						if(benchShowStep == -1) printf("无帧");
						else if(benchShowStep == 0) printf("结束帧");
						else if(benchShowStep == 1) printf("中间帧");
						else if(benchShowStep == 2) printf("帧暂停");
						else if(benchShowStep == 3) printf("全调试");
						printf("\n");
						if(benchShowSolution == 1) printf("(3)隐藏方案     当前:显示\n");
						else printf("(3)显示方案     当前:隐藏\n");
						if(benchShowInformation == 1) printf("(4)隐藏置顶信息 当前:显示\n");
						else printf("(4)显示置顶信息 当前:隐藏\n");
						printf("(5)退出\n");
						printf("*******************************\n");
						printf(">");
						scanf("%d", &temp);
						if(temp == 1)
						{
							printf("[seedMin] [seedMax]>");
							scanf("%d%d", &benchSeedMin, &benchSeedMax);
							if(benchSeedMin > benchSeedMax)
							{
								temp = benchSeedMin;
								benchSeedMin = benchSeedMax;
								benchSeedMax = temp;
							}
						}
						else if(temp == 2)
						{
							printf("[-1:无帧/0:结束帧/1:中间帧/2:帧暂停]>");
							scanf("%d", &benchShowStep);
							if(benchShowStep < -1) benchShowStep = -1;
							if(benchShowStep > 3) benchShowStep = 3;
							if(benchShowStep == 3 && debug != 2) benchShowStep = 2;
						}
						else if(temp == 3)
						{
							if(benchShowSolution == 1) benchShowSolution = 0;
							else benchShowSolution = 1;
						}
						else if(temp == 4)
						{
							if(benchShowInformation == 1) benchShowInformation = 0;
							else benchShowInformation = 1;
						}
					}
					else if(operation == '7')//设置鼠标点击
					{
						printf("*******************************\n");//宽31
						if(clickSpace == 1) printf("(1)启用鼠标点击屏蔽空格\n");
						else printf("(1)关闭鼠标点击屏蔽空格\n");
						if(touchMode == 0) printf("(2)启用鼠标点击空地切换左右键\n");
						else printf("(2)关闭鼠标点击空地切换左右键\n");
						if(touchOpen == 0) printf("(3)启用触动翻开(鼠标点击可使用标记键快速翻开)\n");
						else printf("(3)关闭触动翻开(鼠标点击可使用标记键快速翻开)\n");
						printf("(4)退出\n");
						printf("*******************************\n");
						printf(">");
						scanf("%d", &temp);
						if(temp == 1)//鼠标点击屏蔽空格
						{
							if(clickSpace == 1) clickSpace = 0;
							else clickSpace = 1;
						}
						else if(temp == 2)//鼠标点击空地切换左右键
						{
							if(touchMode == 0) touchMode = 1;
							else touchMode = 0;
						}
						else if(temp == 3)//触动翻开
						{
							if(touchOpen == 0) touchOpen = 1;
							else touchOpen = 0;
						}
					}
					//showCursor(visibleCursor);
					SetConsoleMouseMode(1);
					clrscr();
					DrawControlBar(0);
					DrawSettingsBar(0);
					choiceSet = 0;
				}
				showCursor(visibleCursor);
				api_sleep(refreshCycle);
			}
			if(choiceSet == 9)//返回并保存
			{
				file = fopen("minesweeper-settings.txt", "w");
				fprintf(file, "Map:%d*%d-%d\n", heightOfBoard, widthOfBoard, numberOfMine);
				fprintf(file, "summonCheckMode=%d\n", summonCheckMode);
				fprintf(file, "showTime=%d\n", showTime);
				fprintf(file, "show3BV=%d\n", show3BV);
				fprintf(file, "backgroundColor=0x%x\n", backgroundColor);
				fprintf(file, "operateMode=%d\n", operateMode);
				fprintf(file, "solveMode=%d\n", solveMode);
				//fprintf(file, "lengthOfThinkChain=%d\n", lengthOfThinkChain);
				fprintf(file, "lengthOfThinkNumberCheck=%d\n", lengthOfThinkNumberCheck);
				fprintf(file, "lengthOfThinkMineCheck=%d\n", lengthOfThinkMineCheck);
				fprintf(file, "dictionaryCapacity=%d\n", dictionaryCapacity);
				fprintf(file, "remainedMineJudgeLocalNC=%d\n", remainedMineJudgeLocalNC);
				fprintf(file, "remainedMineJudgeLocalMC=%d\n", remainedMineJudgeLocalMC);
				fprintf(file, "remainedMineJudgeGlobal=%d\n", remainedMineJudgeGlobal);
				fprintf(file, "refreshCycle=%d\n", refreshCycle);
				fprintf(file, "fastShow=%d\n", fastShow);
				fprintf(file, "fastSign=%d\n", fastSign);
				fprintf(file, "newCursor=%d\n", newCursor);
				fprintf(file, "visibleCursor=%d\n", visibleCursor);
				fprintf(file, "keepCursor=%d\n", keepCursor);
				fprintf(file, "clickSpace=%d\n", clickSpace);
				fprintf(file, "touchMode=%d\n", touchMode);
				fprintf(file, "touchOpen=%d\n", touchOpen);
				fprintf(file, "benchSeedMin=%d\n", benchSeedMin);
				fprintf(file, "benchSeedMax=%d\n", benchSeedMax);
				fprintf(file, "benchShowStep=%d\n", benchShowStep);
				fprintf(file, "benchShowSolution=%d\n", benchShowSolution);
				fprintf(file, "benchShowInformation=%d\n", benchShowInformation);
				fprintf(file, "cursorLeft=%c\n", cursorLeft[0]);
				fprintf(file, "cursorRight=%c\n", cursorRight[0]);
				fprintf(file, "chosenSolvable=%d\n", chosenSolvable);
				fprintf(file, "chosen3BV:[%d,%d][%d,%d][%d,%d][%d,%d][%d,%d][%d,%d]\n",
					chosen3BVMin[0], chosen3BVMax[0], chosen3BVMin[1], chosen3BVMax[1],
					chosen3BVMin[2], chosen3BVMax[2], chosen3BVMin[3], chosen3BVMax[3],
					chosen3BVMin[4], chosen3BVMax[4], chosen3BVMin[5], chosen3BVMax[5]);
				fprintf(file, "chosenNumber=%d\n", chosenNumber);
				fprintf(file, "islandPrediction=%d\n", islandPrediction);
				fclose(file);
				clrscr();
				choiceMode = 0;
			}
		}
		else if(choiceMode == 4)//Bench
		{
			if(debug == 2)
			{
				SetConsoleMouseMode(0);
				clrscr();//清除设置转Bench的界面残留
				DrawControlBar(0);
				printf("[seedMin] [seedMax] [-1:无帧/0:结束帧/1:中间帧/2:帧暂停/3:全调试]\n>");
				scanf("%d%d%d", &seedMin, &seedMax, &temp);
				clrscr();//清除输入信息
				DrawControlBar(0);
				gotoxy(0, 1);
				printf("**                   请等待Bench结束. . .                   **\n");
				Bench(seedMin, seedMax, heightOfBoard/2, widthOfBoard/2, temp, benchShowSolution, 1);//Bench模块
				system("pause");//暂停观察
				SetConsoleMouseMode(1);//Bench可左键暂停，右键继续
			}
			else
			{
				clrscr();
				DrawControlBar(0);
				gotoxy(0, 1);
				printf("**                   请等待Bench结束. . .                   **\n");
				Bench(benchSeedMin, benchSeedMax, heightOfBoard/2, widthOfBoard/2, benchShowStep, benchShowSolution, benchShowInformation);//Bench模块
			}
			choiceMode = 0;
		}
		else if(choiceMode == 6)//地图搜索
		{
			SetConsoleMouseMode(0);
			printf("当前时间戳：%d\n", (int)time(0));
			printf("[seedMin] [seedMax] [r0] [c0]\n>");
			showCursor(1);
			scanf("%d%d%d%d", &seedMin, &seedMax, &r0, &c0);
			MapSearch(seedMin, seedMax, r0, c0);//MapSearch模块
			system("pause");
			showCursor(visibleCursor);
			SetConsoleMouseMode(1);
			clrscr();
			DrawControlBar(0);
			choiceMode = 0;
		}
		else if(choiceMode == 7)//记录编辑器
		{
			SetConsoleMouseMode(0);
			records = RecordsEditer(records);//RecordsEditer模块
			showCursor(visibleCursor);
			SetConsoleMouseMode(1);
			clrscr();
			DrawControlBar(0);
			choiceMode = 0;
		}
		else if(choiceMode == 8)//自定义种子游戏
		{
			SetConsoleMouseMode(0);
			printf("当前时间戳：%d\n", (int)time(0));
			printf("[seed] [r0] [c0]\n>");
			showCursor(1);
			//scanf("%d%d%d", &seed, &r0, &c0);//不能以逗号形式输入？
			while(scanf("%d", &seed) == 0) getchar();
			while(scanf("%d", &r0) == 0) getchar();
			while(scanf("%d", &c0) == 0) getchar();//同时兼容空格形式和逗号形式
			lastMap = 1;
			game.t2 = 0;
			ro = r0;
			co = c0;
			isHelped = 1;//无效记录
			for(r=0; r<heightOfBoard; r++)
			{
				for(c=0; c<widthOfBoard; c++)
				{
					isShown[r][c] = 0;
				}
			}
			//isShown[r0][c0] = 1;
			showCursor(visibleCursor);
			SetConsoleMouseMode(1);
			clrscr();
			DrawControlBar(0);
			choiceMode = 1;//进入游戏
		}
		/*else if(choiceMode == 10)//播放操作记录
		{
			SetConsoleMouseMode(0);
			if(argc == 2) operationRecord = ReadOperations(argv[1]);
			else operationRecord = ReadOperations(NULL);
			SetConsoleMouseMode(1);
			clrscr();
			DrawControlBar(0);
			PlayOperations(operationRecord);
			//system("pause");
			choiceMode = 0;
			argc = 1;
		}*/
		else if(choiceMode == 9)//自制地图编辑器
		{
			SetConsoleMouseMode(0);
			temp = CustomMapsEditer();
			if(temp == 1)//进入游戏
			{
				seed = -1;
				r0 = 0;
				c0 = 0;
				lastMap = 2;
				ro = heightOfBoard/2;
				co = widthOfBoard/2;
			}
			showCursor(visibleCursor);
			SetConsoleMouseMode(1);
			clrscr();
			DrawControlBar(0);
			choiceMode = temp;
		}
	}
	/*退出*/
	ReallocMemory(0, 0, 0, 0);//释放内存空间，可能并不必要(doge)
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

void DrawControlBar(int select)
{
	gotoxy(0, 0);
	printf("**************************************************************\n");//宽62
	printf("** [(1)新游戏] [(2)地图求解] [(3)设置] [(4)Bench] [(5)退出] **\n");
	printf("**************************************************************\n");
	//printf("** [(6)地图搜索] [(7)历史记录]  [(8)种子游戏] [(9)自制地图] **\n");
	//printf("**************************************************************\n");
	if(select == 1)
	{
		gotoxy(3, 1);
		ColorStr("[(1)新游戏]", 0xff-backgroundColor);
	}
	else if(select == 2)
	{
		gotoxy(15, 1);
		ColorStr("[(2)地图求解]", 0xff-backgroundColor);
	}
	else if(select == 3)
	{
		gotoxy(29, 1);
		ColorStr("[(3)设置]", 0xff-backgroundColor);
	}
	else if(select == 4)
	{
		gotoxy(39, 1);
		ColorStr("[(4)Bench]", 0xff-backgroundColor);
	}
	else if(select == 5)
	{
		gotoxy(50, 1);
		ColorStr("[(5)退出]", 0xff-backgroundColor);
	}
	gotoxy(0, 3);
}

int ChoiceControlBar(COORD mousePos, DWORD dwButtonState)
{
	int choiceMode = 0;
	static int lastChoice = 0;//仅在悬浮选择变化时刷新，消除闪烁
	if(IsPosInRectangle(mousePos.X, mousePos.Y, 3, 1, 13, 1))
	{
		if(lastChoice != 1)
		{
			DrawControlBar(1);
			lastChoice = 1;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceMode = 1;
			lastChoice = 0;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 15, 1, 27, 1))
	{
		if(lastChoice != 2)
		{
			DrawControlBar(2);
			lastChoice = 2;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceMode = 2;
			lastChoice = 0;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 29, 1, 37, 1))
	{
		if(lastChoice != 3)
		{
			DrawControlBar(3);
			lastChoice = 3;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceMode = 3;
			lastChoice = 0;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 39, 1, 48, 1))
	{
		if(lastChoice != 4)
		{
			DrawControlBar(4);
			lastChoice = 4;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceMode = 4;
			lastChoice = 0;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 50, 1, 58, 1))
	{
		if(lastChoice != 5)
		{
			DrawControlBar(5);
			lastChoice = 5;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceMode = 5;
			lastChoice = 0;
		}
	}
	else if(lastChoice != 0)
	{
		DrawControlBar(0);
		lastChoice = 0;
	}
	return choiceMode;
}

void DrawProgressBar(int progress)//在控制栏显示进度条
{
	int i;
	gotoxy(0, 0);
	printf("**************************************************************\n");//宽62
	//printf("** |====================================--------------| 72% **\n");
	printf("** |");
	for(i=0; i<progress/2; i++)
	{
		printf("=");
	}
	if(i!=0 && i!=50)
	{
		printf("\b>");
	}
	for(; i<100/2; i++)
	{
		printf("-");
	}
	printf("|%3d%% **\n", progress);
	printf("**************************************************************\n");
}

void DrawSettingsBar(int select)
{
	gotoxy(0, 3);
	printf("**************************************************************\n");//宽62
	printf("* [(1)设置游戏难度]      当前雷数:%d|界面大小:%d*%d|密度:%.2f\n", numberOfMine, heightOfBoard, widthOfBoard, (float)numberOfMine/heightOfBoard/widthOfBoard);
	
	if(summonCheckMode == 0) printf("* [(2)设置地图生成校验]  当前:关闭校验\n");
	else if(summonCheckMode == 1) printf("* [(2)设置地图生成校验]  当前:起始点必非雷\n");
	else if(summonCheckMode == 2) printf("* [(2)设置地图生成校验]  当前:起始点必为空\n");
	else if(summonCheckMode == 3) printf("* [(2)设置地图生成校验]  当前:地图可解\n");
	else if(summonCheckMode == 4) printf("* [(2)设置地图生成校验]  当前:地图经筛选\n");
	else printf("* [(2)设置地图生成校验]  当前:起始点必为空?\n");
	
	if(showTime == 0 && show3BV == 0) printf("* [(3)设置显示用时和3BV] 当前:不显示\n");
	else if(showTime == 1 && show3BV == 0) printf("* [(3)设置显示用时和3BV] 当前:仅显示用时\n");
	else if(showTime == 1 && show3BV == 1) printf("* [(3)设置显示用时和3BV] 当前:显示用时和3BV\n");
	else if(showTime == 0 && show3BV == 1) printf("* [(3)设置显示用时和3BV] 当前:仅显示3BV\n");
	else printf("* [(3)设置显示用时和3BV] 当前:仅显示用时?\n");
	
	if(backgroundColor == 0x07) printf("* [(4)浅色模式]          当前:深色模式\n");
	else if(backgroundColor == 0xf0) printf("* [(4)深色模式]          当前:浅色模式\n");
	else printf("* [(4)深色模式]          当前:自定义模式\n");
	
	if(operateMode == 0) printf("* [(5)设置默认操作模式]  当前:键盘@#rc\n");
	else if(operateMode == 1) printf("* [(5)设置默认操作模式]  当前:键盘wasd23\n");
	else if(operateMode == 2) printf("* [(5)设置默认操作模式]  当前:鼠标点击\n");
	else if(operateMode == 3) printf("* [(5)设置默认操作模式]  当前:Window\n");
	else printf("* [(5)设置默认操作模式]  当前:鼠标点击?\n");
	
	if(fastSign == 0) printf("* [(6)启用快速标记]      当前:禁用\n");
	else if(fastSign == 1) printf("* [(6)禁用快速标记]      当前:启用(标记数字快速标记周围)\n");
	else printf("* [(6)启用快速标记]      当前:禁用?\n");
	
	printf("* [(7)设置光标]\n");
	printf("* [(8)调试选项]\n");
	printf("* [(9)返回并保存]\n");
	printf("**************************************************************\n");
	if(select == 1)
	{
		gotoxy(2, 4);
		ColorStr("[(1)设置游戏难度]", 0xff-backgroundColor);
	}
	else if(select == 2)
	{
		gotoxy(2, 5);
		ColorStr("[(2)设置地图生成校验]", 0xff-backgroundColor);
	}
	else if(select == 3)
	{
		gotoxy(2, 6);
		if(showTime == 0 && show3BV == 0) ColorStr("[(3)启用实时显示用时 ]", 0xff-backgroundColor);
		else if(showTime == 1 && show3BV == 0) ColorStr("[(3)启用实时显示3BV  ]", 0xff-backgroundColor);
		else if(showTime == 1 && show3BV == 1) ColorStr("[(3)关闭实时显示用时 ]", 0xff-backgroundColor);
		else if(showTime == 0 && show3BV == 1) ColorStr("[(3)关闭实时显示3BV  ]", 0xff-backgroundColor);
		else ColorStr("[(3)设置显示用时和3BV]", 0xff-backgroundColor);
	}
	else if(select == 4)
	{
		gotoxy(2, 7);
		if(backgroundColor == 0x07) ColorStr("[(4)浅色模式]", 0xff-backgroundColor);
		else ColorStr("[(4)深色模式]", 0xff-backgroundColor);
	}
	else if(select == 5)
	{
		gotoxy(2, 8);
		ColorStr("[(5)设置默认操作模式]", 0xff-backgroundColor);
	}
	else if(select == 6)
	{
		gotoxy(2, 9);
		if(fastSign == 0) ColorStr("[(6)启用快速标记]", 0xff-backgroundColor);
		else ColorStr("[(6)禁用快速标记]", 0xff-backgroundColor);
	}
	else if(select == 7)
	{
		gotoxy(2, 10);
		ColorStr("[(7)设置光标]", 0xff-backgroundColor);
	}
	else if(select == 8)
	{
		gotoxy(2, 11);
		ColorStr("[(8)调试选项]", 0xff-backgroundColor);
	}
	else if(select == 9)
	{
		gotoxy(2, 12);
		ColorStr("[(9)返回并保存]", 0xff-backgroundColor);
	}
	gotoxy(0, 14);
}

int ChoiceSettingsBar(COORD mousePos, DWORD dwButtonState)
{
	int choiceSet = 0;
	static int lastChoice = 0;
	if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 4, 18, 4))
	{
		if(lastChoice != 1)
		{
			DrawSettingsBar(1);
			lastChoice = 1;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSet = 1;
			lastChoice = 0;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 5, 22, 5))
	{
		if(lastChoice != 2)
		{
			DrawSettingsBar(2);
			lastChoice = 2;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSet = 2;
			lastChoice = 0;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 6, 23, 6))
	{
		if(lastChoice != 3)
		{
			DrawSettingsBar(3);
			lastChoice = 3;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSet = 3;
			lastChoice = 0;//按下后刷新按钮
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 7, 14, 7))
	{
		if(lastChoice != 4)
		{
			DrawSettingsBar(4);
			lastChoice = 4;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSet = 4;
			lastChoice = 0;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 8, 22, 8))
	{
		if(lastChoice != 5)
		{
			DrawSettingsBar(5);
			lastChoice = 5;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSet = 5;
			lastChoice = 0;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 9, 18, 9))
	{
		if(lastChoice != 6)
		{
			DrawSettingsBar(6);
			lastChoice = 6;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSet = 6;
			lastChoice = 0;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 10, 14, 10))
	{
		if(lastChoice != 7)
		{
			DrawSettingsBar(7);
			lastChoice = 7;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSet = 7;
			lastChoice = 0;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 11, 14, 11))
	{
		if(lastChoice != 8)
		{
			DrawSettingsBar(8);
			lastChoice = 8;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSet = 8;
			lastChoice = 0;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 12, 16, 12))
	{
		if(lastChoice != 9)
		{
			DrawSettingsBar(9);
			lastChoice = 9;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSet = 9;
			lastChoice = 0;
		}
	}
	else if(lastChoice != 0)
	{
		DrawSettingsBar(0);
		lastChoice = 0;
	}
	return choiceSet;
}

void DrawSetDifficulty(int select)
{
	int r, c;
	int heightOfWindow, widthOfWindow, heightOfFont, maxHeightOfMap, maxWidthOfMap;
	GetSizeOfMap(&heightOfWindow, &widthOfWindow, &heightOfFont, &maxHeightOfMap, &maxWidthOfMap);
	gotoxy(0, 0);
	/*printf("**************************************************************\n");//宽62
	printf("** [(0)默认：10*10 - 10] ** 空间占用示意 ** 可使用全屏，    **\n");
	printf("** [(1)初级： 9*9  - 10] ****************** 可使用Ctrl+滚轮 **\n");
	printf("** [(2)中级：16*16 - 40] **              ** 调整字体大小。  **\n");
	printf("** [(3)高级：16*30 - 99] **              *********************\n");
	printf("** [(4)顶级：42*88 -715] **    [清屏]    ** 如界面下滑，    **\n");
	printf("** [(5)自定义***** - **] **              ** 不建议选择！    **\n");
	printf("**************************************************************\n");*/
	printf("**************************************************************\n");//宽62
	printf("** [(0)默认：10*10 - 10] ** 窗口大小：%3d*%2d 字体大小：%2d   **\n", widthOfWindow, heightOfWindow, heightOfFont);
	printf("** [(1)初级： 9*9  - 10] ** 最大地图：%2d*%3d 当前密度：%.2f **\n", maxHeightOfMap, maxWidthOfMap, (float)numberOfMine/heightOfBoard/widthOfBoard);
	printf("** [(2)中级：16*16 - 40] *************************************\n");
	printf("** [(3)高级：16*30 - 99] **              ** 可使用全屏，    **\n");
	printf("** [(4)顶级：42*88 -715] **    [清屏]    ** 可使用Ctrl+滚轮 **\n");
	printf("** [(5)自定义***** - **] **              ** 调整字体大小。  **\n");
	printf("**************************************************************\n");
	if(select == 0)
	{
		gotoxy(3, 1);
		ColorStr("[(0)默认：10*10 - 10]", 0xff-backgroundColor);
	}
	else if(select == 1)
	{
		gotoxy(3, 2);
		ColorStr("[(1)初级： 9*9  - 10]", 0xff-backgroundColor);
	}
	else if(select == 2)
	{
		gotoxy(3, 3);
		ColorStr("[(2)中级：16*16 - 40]", 0xff-backgroundColor);
	}
	else if(select == 3)
	{
		gotoxy(3, 4);
		ColorStr("[(3)高级：16*30 - 99]", 0xff-backgroundColor);
	}
	else if(select == 4)
	{
		gotoxy(3, 5);
		ColorStr("[(4)顶级：42*88 -715]", 0xff-backgroundColor);
	}
	else if(select == 5)
	{
		gotoxy(3, 6);
		ColorStr("[(5)自定义***** - **]", 0xff-backgroundColor);
	}
	else if(select == 6)
	{
		gotoxy(31, 5);
		ColorStr("[清屏]", 0xff-backgroundColor);
	}
	gotoxy(0, 8);
	if(select == 7)//清除预览地图
	{
		for(r=0; r<heightOfBoard+2; r++)
		{
			for(c=0; c<widthOfBoard+1; c++)//顶级预览地图显示内容178列
			{
				printf("  ");
			}
			printf("\n");
		}
		gotoxy(0, 0);
	}
}

int ChoiceSetDifficulty(COORD mousePos, DWORD dwButtonState)
{
	int choiceDifficulty = -1;
	static int lastChoice = -1;
	if(IsPosInRectangle(mousePos.X, mousePos.Y, 3, 1, 23, 1))
	{
		if(lastChoice != 0)
		{
			if(heightOfBoard > 10 || widthOfBoard > 10)
			{
				DrawSetDifficulty(7);
			}
			//预览默认地图
			numberOfMine = 10;
			heightOfBoard = 10;
			widthOfBoard = 10;
			DrawSetDifficulty(0);
			SummonBoard(0, 5, 5, summonCheckMode, 1);
			ShowBoard(1);
			lastChoice = 0;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceDifficulty = 0;
			lastChoice = -1;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 3, 2, 23, 2))
	{
		if(lastChoice != 1)
		{
			if(heightOfBoard > 9 || widthOfBoard > 9)
			{
				DrawSetDifficulty(7);
			}
			//预览初级地图
			numberOfMine = 10;
			heightOfBoard = 9;
			widthOfBoard = 9;
			DrawSetDifficulty(1);
			SummonBoard(0, 4, 4, summonCheckMode, 1);
			ShowBoard(1);
			lastChoice = 1;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceDifficulty = 1;
			lastChoice = -1;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 3, 3, 23, 3))
	{
		if(lastChoice != 2)
		{
			if(heightOfBoard > 16 || widthOfBoard > 16)
			{
				DrawSetDifficulty(7);
			}
			//预览中级地图
			numberOfMine = 40;
			heightOfBoard = 16;
			widthOfBoard = 16;
			DrawSetDifficulty(2);
			SummonBoard(0, 8, 8, summonCheckMode, 1);
			ShowBoard(1);
			lastChoice = 2;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceDifficulty = 2;
			lastChoice = -1;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 3, 4, 23, 4))
	{
		if(lastChoice != 3)
		{
			if(heightOfBoard > 16 || widthOfBoard > 30)
			{
				DrawSetDifficulty(7);
			}
			//预览高级地图
			numberOfMine = 99;
			heightOfBoard = 16;
			widthOfBoard = 30;
			DrawSetDifficulty(3);
			SummonBoard(0, 8, 15, summonCheckMode, 6);//选用含有6的第6次迭代
			ShowBoard(1);
			lastChoice = 3;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceDifficulty = 3;
			lastChoice = -1;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 3, 5, 23, 5))
	{
		if(lastChoice != 4)
		{
			//预览顶级地图
			numberOfMine = 715;
			heightOfBoard = 42;
			widthOfBoard = 88;
			DrawSetDifficulty(4);
			SummonBoard(0, 21, 44, summonCheckMode, 1);
			ShowBoard(1);
			gotoxy(0, 0);//防止界面下滑
			lastChoice = 4;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceDifficulty = 4;
			lastChoice = -1;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 3, 6, 23, 6))
	{
		if(lastChoice != 5)
		{
			DrawSetDifficulty(5);
			lastChoice = 5;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)//自定义
		{
			choiceDifficulty = 5;
			lastChoice = -1;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 31, 5, 36, 5))
	{
		if(lastChoice != 6)
		{
			DrawSetDifficulty(6);
			lastChoice = 6;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			//choiceDifficulty = -1;
			clrscr();//清屏
			lastChoice = -1;//按下后刷新防止空屏
		}
	}
	else// if(lastChoice != -1)//始终刷新
	{
		DrawSetDifficulty(-1);//刷新窗口大小
		lastChoice = -1;//防止清屏移开后回来按钮不反色
	}
	return choiceDifficulty;
}

void DrawSetSummonCheckMode(int select)
{
	gotoxy(0, 14);
	printf("*******************************\n"//宽31
		   "* [(0)关闭校验]\n"
		   "* [(1)起始点必非雷]\n"
		   "* [(2)起始点必为空]\n"
		   "* [(3)地图可解]（生成时间可能较长）\n"
		   "* [(4)地图经筛选]（易生成重复地图）\n"
		   "* [(5)设置筛选]\n"
		   "*******************************\n");
	if(select == 0)
	{
		gotoxy(2, 15);
		ColorStr("[(0)关闭校验]", 0xff-backgroundColor);
	}
	else if(select == 1)
	{
		gotoxy(2, 16);
		ColorStr("[(1)起始点必非雷]", 0xff-backgroundColor);
	}
	else if(select == 2)
	{
		gotoxy(2, 17);
		ColorStr("[(2)起始点必为空]", 0xff-backgroundColor);
	}
	else if(select == 3)
	{
		gotoxy(2, 18);
		ColorStr("[(3)地图可解]", 0xff-backgroundColor);
	}
	else if(select == 4)
	{
		gotoxy(2, 19);
		ColorStr("[(4)地图经筛选]", 0xff-backgroundColor);
	}
	else if(select == 5)
	{
		gotoxy(2, 20);
		ColorStr("[(5)设置筛选]", 0xff-backgroundColor);
	}
	gotoxy(0, 22);
}

int ChoiceSetSummonCheckMode(COORD mousePos, DWORD dwButtonState)
{
	int choiceSummonCheckMode = -1;
	static int lastChoice = -1;
	if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 15, 14, 15))
	{
		if(lastChoice != 0)
		{
			DrawSetSummonCheckMode(0);
			lastChoice = 0;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSummonCheckMode = 0;
			//lastChoice = -1;//按下后操作栏立即消失
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 16, 18, 16))
	{
		if(lastChoice != 1)
		{
			DrawSetSummonCheckMode(1);
			lastChoice = 1;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSummonCheckMode = 1;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 17, 18, 17))
	{
		if(lastChoice != 2)
		{
			DrawSetSummonCheckMode(2);
			lastChoice = 2;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSummonCheckMode = 2;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 18, 14, 18))
	{
		if(lastChoice != 3)
		{
			DrawSetSummonCheckMode(3);
			lastChoice = 3;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSummonCheckMode = 3;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 19, 16, 19))
	{
		if(lastChoice != 4)
		{
			DrawSetSummonCheckMode(4);
			lastChoice = 4;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSummonCheckMode = 4;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 20, 14, 20))
	{
		if(lastChoice != 5)
		{
			DrawSetSummonCheckMode(5);
			lastChoice = 5;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSummonCheckMode = 5;
		}
	}
	else if(lastChoice != -1)
	{
		DrawSetSummonCheckMode(-1);
		lastChoice = -1;
	}
	return choiceSummonCheckMode;
}

void DrawSetChosenBoard(int select)
{
	gotoxy(0, 22);
	printf("*******************************\n");//宽31
	printf("* [(0)设置默认难度3BV]   当前:[%d,%d]\n", chosen3BVMin[0], chosen3BVMax[0]);
	printf("* [(1)设置初级难度3BV]   当前:[%d,%d]\n", chosen3BVMin[1], chosen3BVMax[1]);
	printf("* [(2)设置中级难度3BV]   当前:[%d,%d]\n", chosen3BVMin[2], chosen3BVMax[2]);
	printf("* [(3)设置高级难度3BV]   当前:[%d,%d]\n", chosen3BVMin[3], chosen3BVMax[3]);
	printf("* [(4)设置顶级难度3BV]   当前:[%d,%d]\n", chosen3BVMin[4], chosen3BVMax[4]);
	printf("* [(5)设置自定义难度3BV] 当前:[%d,%d]\n", chosen3BVMin[5], chosen3BVMax[5]);
	if(chosenSolvable == 1) printf("* [(6)忽略地图可解性]    当前:确保\n");
	else printf("* [(6)确保地图可解性]    当前:忽略\n");
	printf("* [(7)指定包含数字]      当前:%d\n", chosenNumber);
	printf("* [(8)退出]\n");
	printf("*******************************\n");
	if(select == 0)
	{
		gotoxy(2, 23);
		ColorStr("[(0)设置默认难度3BV]", 0xff-backgroundColor);
	}
	else if(select == 1)
	{
		gotoxy(2, 24);
		ColorStr("[(1)设置初级难度3BV]", 0xff-backgroundColor);
	}
	else if(select == 2)
	{
		gotoxy(2, 25);
		ColorStr("[(2)设置中级难度3BV]", 0xff-backgroundColor);
	}
	else if(select == 3)
	{
		gotoxy(2, 26);
		ColorStr("[(3)设置高级难度3BV]", 0xff-backgroundColor);
	}
	else if(select == 4)
	{
		gotoxy(2, 27);
		ColorStr("[(4)设置顶级难度3BV]", 0xff-backgroundColor);
	}
	else if(select == 5)
	{
		gotoxy(2, 28);
		ColorStr("[(5)设置自定义难度3BV]", 0xff-backgroundColor);
	}
	else if(select == 6)
	{
		gotoxy(2, 29);
		if(chosenSolvable == 1) ColorStr("[(6)忽略地图可解性]", 0xff-backgroundColor);
		else ColorStr("[(6)确保地图可解性]", 0xff-backgroundColor);
	}
	else if(select == 7)
	{
		gotoxy(2, 30);
		ColorStr("[(7)指定包含数字]", 0xff-backgroundColor);
	}
	else if(select == 8)
	{
		gotoxy(2, 31);
		ColorStr("[(8)退出]", 0xff-backgroundColor);
	}
	gotoxy(0, 33);
}

int ChoiceSetChosenBoard(COORD mousePos, DWORD dwButtonState)
{
	int choiceSetChosenBoard = -1;
	static int lastChoice = -1;
	if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 23, 21, 23))
	{
		if(lastChoice != 0)
		{
			DrawSetChosenBoard(0);
			lastChoice = 0;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSetChosenBoard = 0;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 24, 21, 24))
	{
		if(lastChoice != 1)
		{
			DrawSetChosenBoard(1);
			lastChoice = 1;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSetChosenBoard = 1;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 25, 21, 25))
	{
		if(lastChoice != 2)
		{
			DrawSetChosenBoard(2);
			lastChoice = 2;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSetChosenBoard = 2;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 26, 21, 26))
	{
		if(lastChoice != 3)
		{
			DrawSetChosenBoard(3);
			lastChoice = 3;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSetChosenBoard = 3;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 27, 21, 27))
	{
		if(lastChoice != 4)
		{
			DrawSetChosenBoard(4);
			lastChoice = 4;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSetChosenBoard = 4;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 28, 21, 28))
	{
		if(lastChoice != 5)
		{
			DrawSetChosenBoard(5);
			lastChoice = 5;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSetChosenBoard = 5;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 29, 21, 29))
	{
		if(lastChoice != 6)
		{
			DrawSetChosenBoard(6);
			lastChoice = 6;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSetChosenBoard = 6;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 30, 21, 30))
	{
		if(lastChoice != 7)
		{
			DrawSetChosenBoard(7);
			lastChoice = 7;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSetChosenBoard = 7;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 31, 21, 31))
	{
		if(lastChoice != 8)
		{
			DrawSetChosenBoard(8);
			lastChoice = 8;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceSetChosenBoard = 8;
		}
	}
	else if(lastChoice != -1)
	{
		DrawSetChosenBoard(-1);
		lastChoice = -1;
	}
	return choiceSetChosenBoard;
}

void DrawSetOperateMode(int select)
{
	gotoxy(0, 14);
	printf("**************************************************************\n");
	printf("* 键盘操作时可根据按键自动切换@#rc/wasd23操作模式！\n");
	printf("* 键盘操作可按M切换到鼠标操作，鼠标操作可按K切换到键盘操作。\n");
	printf("* [(0)@#rc操作模式]\n");
	printf("* 输入[@:翻开/#:标记] [r:行坐标] [c:列坐标]，\n");
	printf("* 并按回车后执行，如@ 0 0指翻开(0,0)。\n");
	printf("* [(1)wasd23操作模式]\n");
	printf("* 通过wasd移动光标，按2翻开，按3标记，\n");
	printf("* 无小键盘可用hu代替23。\n");
	printf("* [(2)鼠标点击操作模式]\n");
	printf("* 可通过鼠标，wasd，方向键移动光标。\n");
	printf("* 兼容wasd23，但无法使用@#rc和鼠标全选复制地图。\n");
	printf("* [(3)Window操作模式]\n");
	printf("* 在独立图形界面窗口进行游戏，控制台内操作均屏蔽，\n");
	printf("* 游戏结束后按鼠标右键返回控制台。\n");
	printf("**************************************************************\n");
	if(select == 0)
	{
		gotoxy(2, 17);
		ColorStr("[(0)@#rc操作模式]", 0xff-backgroundColor);
	}
	else if(select == 1)
	{
		gotoxy(2, 20);
		ColorStr("[(1)wasd23操作模式]", 0xff-backgroundColor);
	}
	else if(select == 2)
	{
		gotoxy(2, 23);
		ColorStr("[(2)鼠标点击操作模式]", 0xff-backgroundColor);
	}
	else if(select == 3)
	{
		gotoxy(2, 26);
		ColorStr("[(3)Window操作模式]", 0xff-backgroundColor);
	}
	gotoxy(0, 27);
}

int ChoiceSetOperateMode(COORD mousePos, DWORD dwButtonState)
{
	int choiceOperateMode = -1;
	static int lastChoice = -1;
	if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 17, 18, 17))
	{
		if(lastChoice != 0)
		{
			DrawSetOperateMode(0);
			lastChoice = 0;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceOperateMode = 0;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 20, 20, 20))
	{
		if(lastChoice != 1)
		{
			DrawSetOperateMode(1);
			lastChoice = 1;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceOperateMode = 1;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 23, 22, 23))
	{
		if(lastChoice != 2)
		{
			DrawSetOperateMode(2);
			lastChoice = 2;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceOperateMode = 2;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 26, 20, 26))
	{
		if(lastChoice != 3)
		{
			DrawSetOperateMode(3);
			lastChoice = 3;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceOperateMode = 3;
		}
	}
	else if(lastChoice != -1)
	{
		DrawSetOperateMode(-1);
		lastChoice = -1;
	}
	return choiceOperateMode;
}

void DrawSetCursor(int select)
{
	gotoxy(0, 14);
	printf("*******************************\n");//宽31
	if(visibleCursor == 1) printf("* [(1)隐藏控制台光标]   当前:显示\n");
	else if(visibleCursor == 0) printf("* [(1)显示控制台光标]   当前:隐藏\n");
	else printf("* [(1)隐藏控制台光标]   当前:显示?\n");
	
	if(newCursor == 0) printf("* [(2)设置自选光标形态] 当前:未启用\n");
	else if(newCursor == 1) printf("* [(2)设置自选光标形态] 当前:><光标\n");
	else if(newCursor == 2) printf("* [(2)设置自选光标形态] 当前:淡黄色高亮光标\n");
	else if(newCursor == 3) printf("* [(2)设置自选光标形态] 当前:淡黄色高亮行列\n");
	else printf("* [(2)设置自选光标形态] 当前:未启用?\n");
	
	printf("* [(3)自定义><光标字符] 当前:%c%%%c\n", cursorLeft[0], cursorRight[0]);
	
	if(keepCursor == 0) printf("* [(4)保持光标显示]     当前:操作后消失\n");
	else if(keepCursor == 1) printf("* [(4)不保持光标显示]   当前:保持\n");
	else printf("* [(4)保持光标显示]     当前:操作后消失?\n");
	
	printf("* [(5)退出]\n");
	printf("*******************************\n");
	if(select == 1)
	{
		gotoxy(2, 15);
		if(visibleCursor == 0) ColorStr("[(1)显示控制台光标]", 0xff-backgroundColor);
		else ColorStr("[(1)隐藏控制台光标]", 0xff-backgroundColor);
	}
	else if(select == 2)
	{
		gotoxy(2, 16);
		ColorStr("[(2)设置自选光标形态]", 0xff-backgroundColor);
	}
	else if(select == 3)
	{
		gotoxy(2, 17);
		ColorStr("[(3)自定义><光标字符]", 0xff-backgroundColor);
	}
	else if(select == 4)
	{
		gotoxy(2, 18);
		if(keepCursor == 1) ColorStr("[(4)不保持光标显示]", 0xff-backgroundColor);
		else ColorStr("[(4)保持光标显示]", 0xff-backgroundColor);
	}
	else if(select == 5)
	{
		gotoxy(2, 19);
		ColorStr("[(5)退出]", 0xff-backgroundColor);
	}
	gotoxy(0, 21);
}

int ChoiceSetCursor(COORD mousePos, DWORD dwButtonState)
{
	int choiceCursor = 0;
	static int lastChoice = 0;
	if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 15, 20, 15))
	{
		if(lastChoice != 1)
		{
			DrawSetCursor(1);
			lastChoice = 1;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceCursor = 1;
			lastChoice = 0;//按下后立即刷新信息
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 16, 22, 16))
	{
		if(lastChoice != 2)
		{
			DrawSetCursor(2);
			lastChoice = 2;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceCursor = 2;
			lastChoice = 0;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 17, 22, 17))
	{
		if(lastChoice != 3)
		{
			DrawSetCursor(3);
			lastChoice = 3;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceCursor = 3;
			lastChoice = 0;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 18, 20, 18))
	{
		if(lastChoice != 4)
		{
			DrawSetCursor(4);
			lastChoice = 4;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceCursor = 4;
			lastChoice = 0;
		}
	}
	else if(IsPosInRectangle(mousePos.X, mousePos.Y, 2, 19, 10, 19))
	{
		if(lastChoice != 5)
		{
			DrawSetCursor(5);
			lastChoice = 5;
		}
		if(dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			choiceCursor = 5;
			lastChoice = 0;
		}
	}
	else if(lastChoice != 0)
	{
		DrawSetCursor(0);
		lastChoice = 0;
	}
	return choiceCursor;
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

void PrintBlank(int n)//打印n个空格
{
	for(; n > 0; n--)
	{
		putchar(' ');
	}
}

void PrintCell(int board, int isShown, int bkcolor)//打印地图方块内字符
{
	/*----------
	  9 2: #
	  0 2: _#
	  n 2: n#(0<n<9)
	  9 0: *
	  n 0: % (n!=9 使用0 0)
	  9 1: @
	  0 1:   (空格)
	  n 1: n (0<n<9)
	----------*/
	if(isShown == 2)
	{
		if(board == 9)
		{
			ColorStr("#", 0x0c + bkcolor);
			putchar(' ');
			//ColorStr("↑", 0x0c + bkcolor);
			//ColorStr("◎", 0x0c + bkcolor);
		}
		else//错误标记
		{
			if(board == 0) ColorStr("_", 0x04 + bkcolor);
			else ColorNumber(board, 0x04 + bkcolor);
			ColorStr("#", 0x0c + bkcolor);
		}
	}
	else if(isShown == 0)
	{
		if(board == 9)
		{
			ColorStr("* ", 0x0c + bkcolor);
			//ColorStr("⊕", 0x0c + bkcolor);
			//ColorStr("●", 0x0c + bkcolor);
		}
		else
		{
			putchar('%');
			putchar(' ');
			//printf("■");
			//printf("□");
		}
	}
	else
	{
		if(board == 9)
		{
			ColorStr("@", 0x04 + bkcolor);
			putchar(' ');
			//ColorStr("※", 0x04 + bkcolor);
			//ColorStr("●", 0x04 + bkcolor);
		}
		else if(board == 0)
		{
			putchar(' ');
			putchar(' ');
			//printf("　");//我超，全角空格？！
		}
		else if(board == 1)
		{
			ColorStr("1 ", 0x01 + bkcolor);
			//ColorStr("①", 0x01 + bkcolor);
			//ColorStr("１", 0x01 + bkcolor);
		}
		else if(board == 2)
		{
			ColorStr("2 ", 0x02 + bkcolor);
			//ColorStr("②", 0x02 + bkcolor);
			//ColorStr("２", 0x02 + bkcolor);
		}
		else if(board == 3)
		{
			ColorStr("3 ", 0x04 + bkcolor);
			//ColorStr("③", 0x04 + bkcolor);
			//ColorStr("３", 0x04 + bkcolor);
		}
		else if(board == 4)
		{
			ColorStr("4 ", 0x04 + bkcolor);
			//ColorStr("④", 0x04 + bkcolor);
			//ColorStr("４", 0x04 + bkcolor);
		}
		else if(board == 5)
		{
			ColorStr("5 ", 0x04 + bkcolor);
			//ColorStr("⑤", 0x04 + bkcolor);
			//ColorStr("５", 0x04 + bkcolor);
		}
		else if(board == 6)
		{
			ColorStr("6", 0x04 + bkcolor);
			putchar(' ');//浅色模式678显示淡黄色底纹
			//ColorStr("⑥", 0x04 + bkcolor);
			//ColorStr("６", 0x04 + bkcolor);
		}
		else if(board == 7)
		{
			ColorStr("7", 0x04 + bkcolor);
			putchar(' ');
			//ColorStr("⑦", 0x04 + bkcolor);
			//ColorStr("７", 0x04 + bkcolor);
		}
		else if(board == 8)
		{
			ColorStr("8", 0x04 + bkcolor);
			putchar(' ');
			//ColorStr("⑧", 0x04 + bkcolor);
			//ColorStr("８", 0x04 + bkcolor);
		}
	}
}

void ShowBlock(int r, int c, int mode)//根据背景颜色和模式显示地图方块
{
	if(backgroundColor == 0xf0)//浅色模式
	{
		if(mode == 1)//后台
		{
			if(isShown[r][c] == 2)//错误标记
			{
				PrintCell(board[r][c], 2, 0xf0);
			}
			else if(board[r][c] == 0)
			{
				PrintCell(0, 1, 0xf0);
			}
			else if(board[r][c] == 9)
			{
				PrintCell(9, isShown[r][c], 0xf0);
			}
			else
			{
				PrintCell(board[r][c], 1, 0xf0);
			}
		}
		else if(mode == 0)//前台
		{
			if(isShown[r][c] == 2)//浅色模式标记加灰色底纹
			{
				PrintCell(9, 2, 0x70);
			}
			else if(isShown[r][c] == 0)
			{
				PrintCell(0, 0, 0xf0);
			}
			else//isShown[r][c] == 1
			{
				if(board[r][c] > 5)//浅色模式678显示淡黄色底纹
				{
					PrintCell(board[r][c], 1, 0xe0);
				}
				else
				{
					PrintCell(board[r][c], 1, 0xf0);
				}
			}
		}
		else if(mode == 2)//浅色模式前台高亮
		{
			if(isShown[r][c] == 2)//#%空格加淡黄色底纹
			{
				//PrintCell(9, 2, 0xe0);//底纹不覆盖空格
				ColorStr("# ", 0xec);//底纹覆盖空格
			}
			else if(isShown[r][c] == 0)
			{
				//PrintCell(0, 0, 0xe0);
				//ColorStr("%", 0xe0);//PrintCell%采用背景颜色
				//printf(" ");//底纹不覆盖空格
				ColorStr("% ", 0xe0);
			}
			else//isShown[r][c] == 1
			{
				if(board[r][c] == 0)
				{
					//ColorStr(" ", 0xe0);//PrintCell空格采用背景颜色
					//printf(" ");//底纹不覆盖全部空格
					ColorStr("  ", 0xe0);
				}
				else if(board[r][c] > 5)
				{
					ColorNumber(board[r][c], 0xe4);//PrintCell 6-8不覆盖空格颜色
					ColorStr(" ", 0xe0);
				}
				else
				{
					PrintCell(board[r][c], 1, 0xe0);
				}
			}
		}
	}
	else//深色模式和自定义模式
	{
		if(mode == 1)//后台
		{
			if(isShown[r][c] == 2)//错误标记
			{
				PrintCell(board[r][c], 2, backgroundColor/16*16);
			}
			else if(board[r][c] == 0)
			{
				PrintCell(0, 1, backgroundColor/16*16);
			}
			else if(board[r][c] == 9)
			{
				PrintCell(9, isShown[r][c], backgroundColor/16*16);
			}
			else if(board[r][c] < 6)
			{
				PrintCell(board[r][c], 1, backgroundColor/16*16);
			}
			else//6-8
			{
				PrintCell(board[r][c], 1, backgroundColor/16*16 + 0x0e - 0x04);
			}
		}
		else if(mode == 0)//前台
		{
			if(isShown[r][c] == 2)
			{
				PrintCell(9, 2, backgroundColor/16*16);
			}
			else if(isShown[r][c] == 0)
			{
				PrintCell(0, 0, backgroundColor/16*16);
			}
			else//isShown[r][c] == 1
			{
				if(board[r][c] > 5)//深色模式678以淡黄色显示
				{
					PrintCell(board[r][c], 1, backgroundColor/16*16 + 0x0e - 0x04);
				}
				else
				{
					PrintCell(board[r][c], 1, backgroundColor/16*16);
				}
			}
		}
		else if(mode == 2)//深色模式前台高亮
		{
			if(isShown[r][c] == 2)//以淡黄色显示#%
			{
				PrintCell(9, 2, backgroundColor/16*16 + 0x0e - 0x0c);
			}
			else if(isShown[r][c] == 0)
			{
				ColorStr("% ", backgroundColor/16*16 + 0x0e);
			}
			else//isShown[r][c] == 1
			{
				if(board[r][c] == 0)//以淡黄色_显示空格
				{
					ColorStr("_ ", backgroundColor/16*16 + 0x0e);
				}
				else if(board[r][c] > 5)//深色模式678以淡黄色底纹和红色显示
				{
					PrintCell(board[r][c], 1, 0xe0);
				}
				else
				{
					//PrintCell(board[r][c], isShown[r][c], backgroundColor/16*16);
					ColorNumber(board[r][c], backgroundColor/16*16 + 0x0e);
					printf(" ");
				}
			}
		}
	}
}

void ShowBoard(int mode)//在控制台显示地图，0前台，1后台
{
	int r, c, i, j, n;
	for(i=Place(widthOfBoard-1); i>0; i--)//列坐标
	{
		PrintBlank(Place(heightOfBoard-1)+1);
		for(c=0; c<widthOfBoard; c++)
		{
			n = c;
			for(j=1; j<i; j++)
			{
				n /= 10;
			}
			if(i != 1 && n == 0)
			{
				printf("  ");
			}
			else
			{
				printf("%d ", n%10);
			}
		}
		printf("\n");
	}
	for(r=0; r<heightOfBoard; r++)
	{
		PrintBlank(Place(heightOfBoard-1)-Place(r));//行坐标
		printf("%d ", r);
		for(c=0; c<widthOfBoard; c++)
		{
			ShowBlock(r, c, mode);
			//ShowBlock(r, c, 2);
		}
		printf("\n");
	}
	//ShowPartBoard(0, 0, heightOfBoard-1, widthOfBoard-1, mode);
}
/*
void ShowPartBoard(int r1, int c1, int r2, int c2, int mode)//在控制台显示部分地图，用于超大地图显示
{
	int r, c, i, j, n;
	for(i=Place(c2); i>0; i--)//列坐标
	{
		PrintBlank(Place(r2)+1);
		for(c=c1; c<=c2; c++)
		{
			n = c;
			for(j=1; j<i; j++)
			{
				n /= 10;
			}
			if(i != 1 && n == 0)
			{
				printf("  ");
			}
			else
			{
				printf("%d ", n%10);
			}
		}
		printf("\n");
	}
	for(r=r1; r<=r2; r++)
	{
		PrintBlank(Place(r2)-Place(r));//行坐标
		printf("%d ", r);
		for(c=c1; c<=c2; c++)
		{
			ShowBlock(r, c, mode);
		}
		printf("\n");
	}
}
*/
void ReShowBoard(int r, int c, int mode, int yOfMap)//地图的行/列/点重绘
{
	int i, j, n;
	if(r != -1 && c == -1)//重绘行
	{
		gotoxy(0, 0);//防止行重绘不显示控制栏
		gotoxy(0, yOfMap+rs0+r);
		PrintBlank(Place(heightOfBoard-1)-Place(r));//重绘行坐标
		if(mode == 2)
		{
			if(backgroundColor == 0xf0)
			{
				ColorNumber(r, 0xe0);
			}
			else
			{
				ColorNumber(r, backgroundColor/16*16 + 0x0e);
			}
			printf(" ");
		}
		else
		{
			printf("%d ", r);
		}
		for(c=0; c<widthOfBoard; c++)//重绘该行方块
		{
			ShowBlock(r, c, mode);
		}
		printf("\n");
	}
	else if(r == -1 && c != -1)//重绘列
	{
		for(i=Place(widthOfBoard-1); i>0; i--)//重绘列坐标
		{
			n = c;
			for(j=1; j<i; j++)
			{
				n /= 10;
			}
			gotoxy(cs0+2*c, yOfMap+Place(widthOfBoard-1)-i);
			if(i != 1 && n == 0)
			{
				printf("  ");
			}
			else
			{
				n %= 10;
				if(mode == 2)
				{
					if(backgroundColor == 0xf0)
					{
						ColorNumber(n, 0xe0);
					}
					else
					{
						ColorNumber(n, backgroundColor/16*16 + 0x0e);
					}
					printf(" ");
				}
				else
				{
					printf("%d ", n);
				}
			}
		}
		for(r=0; r<heightOfBoard; r++)//重绘该列方块
		{
			gotoxy(cs0+2*c, yOfMap+rs0+r);
			ShowBlock(r, c, mode);
		}
	}
	else if(r != -1 && c != -1)//重绘点
	{
		gotoxy(cs0+2*c, yOfMap+rs0+r);
		ShowBlock(r, c, mode);
	}
	else//(-1,-1)全部重绘
	{
		gotoxy(0, yOfMap);
		ShowBoard(mode);
	}
}

void FreshCursor(int r, int c, int yOfMap)//刷新光标
{
	static int ra = 0, ca = 0;//初始化为左上角，防止淡黄色高亮光标闪退
	if(newCursor > 0)
	{
		if(newCursor > 1)//绘制淡黄色高亮光标
		{
			if(ra != r || ca != c)//浅用ra,ca实现抗闪烁(doge)
			{
				//ReShowBoard(-1, -1, 0, yOfMap);//重绘全图
				ReShowBoard(ra, ca, 0, yOfMap);
				ReShowBoard(r, c, 2, yOfMap);
				if(newCursor == 3)//淡黄色高亮行列
				{
					if(ra != r) ReShowBoard(ra, -1, 0, yOfMap);
					if(ca != c) ReShowBoard(-1, ca, 0, yOfMap);
					ReShowBoard(r, -1, 2, yOfMap);
					ReShowBoard(-1, c, 2, yOfMap);
				}
				ra = r;
				ca = c;
			}
			else if(keepCursor == 1)//ra == r && ca == c
			{
				ReShowBoard(r, c, 2, yOfMap);
				if(newCursor == 3)
				{
					if(ra != r) ReShowBoard(ra, -1, 0, yOfMap);
					if(ca != c) ReShowBoard(-1, ca, 0, yOfMap);
					ReShowBoard(r, -1, 2, yOfMap);
					ReShowBoard(-1, c, 2, yOfMap);
				}
			}
		}
		else if(newCursor == 1)//绘制><光标
		{
			//去除拖影
			// % %3%4% %
			// %1%>@<%2%
			// % %5%6% %
			/*gotoxy(cs0+2*c-3, yOfMap+rs0+r);
			printf(" ");//去除左拖影
			gotoxy(cs0+2*c+3, yOfMap+rs0+r);
			printf(" ");//去除右拖影
			gotoxy(cs0+2*c-1, yOfMap+rs0+r-1);
			printf(" ");//去除左上拖影
			gotoxy(cs0+2*c+1, yOfMap+rs0+r-1);
			printf(" ");//去除右上拖影
			gotoxy(cs0+2*c-1, yOfMap+rs0+r+1);
			printf(" ");//去除左下拖影
			gotoxy(cs0+2*c+1, yOfMap+rs0+r+1);
			printf(" ");//去除右下拖影*/
			/*for(ra=r-2; ra<=r+2; ra++)
			{
				for(ca=c-2; ca<=c+3; ca++)//向右偏移
				{
					if(ra>=0 && ra<heightOfBoard && ca>=0 && ca<=widthOfBoard)//确认在范围内并清除右边界拖影
					{
						gotoxy(cs0+2*ca-1, yOfMap+rs0+ra);
						printf(" ");//去除左侧拖影
					}
				}
			}*/
			if(ra != r || ca != c)
			{
				//去除拖影
				gotoxy(cs0+2*ca-1, yOfMap+rs0+ra);
				printf(" ");
				gotoxy(cs0+2*ca+1, yOfMap+rs0+ra);
				printf(" ");
				if(ca == -1) ReShowBoard(-1, -1, 0, yOfMap);//@#rc仅输入行坐标时光标位于坐标轴，需重绘全图
				//绘制><
				gotoxy(cs0+2*c-1, yOfMap+rs0+r);
				if(backgroundColor == 0xf0) ColorStr(cursorLeft, 0xfd);
				else ColorStr(cursorLeft, backgroundColor/16*16 + 0x0e);
				gotoxy(cs0+2*c+1, yOfMap+rs0+r);
				if(backgroundColor == 0xf0) ColorStr(cursorRight, 0xfd);
				else ColorStr(cursorRight, backgroundColor/16*16 + 0x0e);
				ra = r;
				ca = c;
			}
			else if(keepCursor == 1)
			{
				//if(ca == -1) ReShowBoard(-1, -1, 0, yOfMap);//@#rc无需保持光标显示
				gotoxy(cs0+2*c-1, yOfMap+rs0+r);
				if(backgroundColor == 0xf0) ColorStr(cursorLeft, 0xfd);
				else ColorStr(cursorLeft, backgroundColor/16*16 + 0x0e);
				gotoxy(cs0+2*c+1, yOfMap+rs0+r);
				if(backgroundColor == 0xf0) ColorStr(cursorRight, 0xfd);
				else ColorStr(cursorRight, backgroundColor/16*16 + 0x0e);
			}
		}
	}
	//光标移动至对应点
	gotoxy(cs0+2*c, yOfMap+rs0+r);//当实际位置为该位置左部，光标显示在该位置
}
/*
void ShowPageBoard(int mode, char move)
{
	//页面范围[rPage,rPage+heightOfPage-1]
	static int rPage = 0, cPage = 0;
	const int heightOfPage = 16, widthOfPage = 30;
	//移动页面
	if(move != 0)
	{
		if(move == 'w' || move == 'W')
		{
			if(rPage-5 > 0) rPage -= 5;
			else if(rPage > 0) rPage = 0;
		}
		else if(move == 'a' || move == 'A')
		{
			if(cPage-5 > 0) cPage -= 5;
			else if(cPage > 0) cPage = 0;
		}
		else if(move == 's' || move == 'S')
		{
			if(rPage+heightOfPage-1+5 < heightOfBoard-1) rPage += 5;
			else if(rPage+heightOfPage-1 < heightOfBoard-1) rPage = heightOfBoard - heightOfPage;
		}
		else if(move == 'd' || move == 'D')
		{
			if(cPage+widthOfPage-1+5 < widthOfBoard-1) cPage += 5;
			else if(cPage+widthOfPage-1 < widthOfBoard-1) cPage = widthOfBoard - widthOfPage;
		}
	}
	//显示页面
	ShowPartBoard(rPage, cPage, rPage+heightOfPage-1, cPage+widthOfPage-1, mode);
}
*/
void ShowBoardFast(int yOfMap)
{
	int r, c;
	//比较并刷新不同
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(isShown[r][c] != isShownBuf[r][c])
			{
				ReShowBoard(r, c, 0, yOfMap);
				isShownBuf[r][c] = isShown[r][c];
			}
		}
	}
}

void PrintSolvingMap(int mode)//1为枚举可能性
{
	int r, c, i, j, n, p = 0;
	for(i=Place(widthOfBoard-1); i>0; i--)//列坐标
	{
		PrintBlank(Place(heightOfBoard-1)+1);
		for(c=0; c<widthOfBoard; c++)
		{
			n = c;
			for(j=1; j<i; j++)
			{
				n /= 10;
			}
			if(i != 1 && n == 0)
			{
				printf("  ");
			}
			else
			{
				printf("%d ", n%10);
			}
		}
		printf("\n");
	}
	for(r=0; r<heightOfBoard; r++)
	{
		PrintBlank(Place(heightOfBoard-1)-Place(r));//行坐标
		printf("%d ", r);
		for(c=0; c<widthOfBoard; c++)
		{
			if(mode != 0 && thinkChain[r][c] == 1 && possibility[p] == 1)
			{
				ColorStr("#", backgroundColor/16*16 + 0x0c);
				ColorStr("?", backgroundColor/16*16 + 0x0d);
				p++;
			}
			else if(mode != 0 && thinkChain[r][c] == 1 && possibility[p] == 0)
			{
				ColorStr("@", backgroundColor/16*16 + 0x0e);
				ColorStr("?", backgroundColor/16*16 + 0x0d);
				p++;
			}
			else if(map[r][c]>='1' && map[r][c]<='9')
			{
				PrintCell(map[r][c]-'1'+1, 1, backgroundColor/16*16);
			}
			else if(map[r][c] == '#')
			{
				PrintCell(9, 2, backgroundColor/16*16);
			}
			else if(map[r][c] == '@')
			{
				PrintCell(9, 1, backgroundColor/16*16 + 0x0e - 0x04);
			}
			/*else if(map[r][c] == '?')
			{
				ColorStr("? ", backgroundColor/16*16 + 0x0d);
			}*/
			else
			{
				printf("%c ", map[r][c]);
			}
		}
		printf("\n");
	}
}

void ShowInformation()//更新时间并显示游戏信息
{
	int t, solved3BV = game.total3BV - game.unsolved3BV;
	gotoxy(0, game.yOfInformation);
	printf("剩余雷数: %d ", game.remainedMine);//从行首写防止文字重影
	game.t1 = time(0);//刷新用时
	t = game.t1 - game.t0 + game.t2;
	if(showTime == 1) printf("用时：%d ", t);
	if(show3BV == 1) printf("3BV：%d/%d 3BV/s：%.2f ", solved3BV, game.total3BV, (float)solved3BV/t);
	printf("\n");
}

void DrawMine(int r, int c)//绘制地图地雷
{
	float x = c*widthOfBlock+widthOfBorder+dx;
	float y = r*heightOfBlock+heightOfBar+widthOfBorder+dy;
	//setcolor(LIGHTRED);
	//xyprintf(x+xOfChar, y+yOfChar, "@");
	setfillcolor(BLACK);
	ege_fillellipse(x+7.4/32*widthOfBlock, y+7.4/32*heightOfBlock, 0.6*widthOfBlock, 0.6*heightOfBlock);
	setcolor(BLACK);
	setlinewidth(sideLength/16);
	ege_line(x+5.0/32*widthOfBlock, y+17.0/32*heightOfBlock, x+29.0/32*widthOfBlock, y+17.0/32*heightOfBlock);
	ege_line(x+17.0/32*widthOfBlock, y+5.0/32*heightOfBlock, x+17.0/32*widthOfBlock, y+29.0/32*heightOfBlock);
	ege_line(x+9.0/32*widthOfBlock, y+9.0/32*heightOfBlock, x+25.0/32*widthOfBlock, y+25.0/32*heightOfBlock);
	ege_line(x+9.0/32*widthOfBlock, y+25.0/32*heightOfBlock, x+25.0/32*widthOfBlock, y+9.0/32*heightOfBlock);
	setfillcolor(WHITE);
	ege_fillrect(x+12.0/32*widthOfBlock, y+12.0/32*heightOfBlock, 4.0/32*widthOfBlock, 4.0/32*heightOfBlock);
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
	float x = c*widthOfBlock+widthOfBorder+dx;
	float y = r*heightOfBlock+heightOfBar+widthOfBorder+dy;
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
		{x+6.0f/32*widthOfBlock, y+11.0f/32*heightOfBlock},
		{x+17.0f/32*widthOfBlock, y+6.0f/32*heightOfBlock},
		{x+17.0f/32*widthOfBlock, y+16.0f/32*heightOfBlock}
	};
	ege_fillpoly(3, polyPoints);
}

void DrawBlock(int r, int c, int board, int isShown, int highlight)//绘制方块
{
	float x = c*widthOfBlock+widthOfBorder+dx;
	float y = r*heightOfBlock+heightOfBar+widthOfBorder+dy;
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
		ege_fillrect(x+widthOfBlock*2.0/32, y+heightOfBlock*2.0/32, widthOfBlock*28/32, heightOfBlock*28/32);
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
			ege_fillrect(x+widthOfBlock*2.0/32, y+heightOfBlock*2.0/32, widthOfBlock*28/32, heightOfBlock*28/32);
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
	DrawLineA(x0, y0, r/2, 270+hour%12*30+minute/12*6);//每12分钟跳一格
	//转轴
	setfillcolor(RED);
	ege_fillellipse(x0-r/10, y0-r/10, r/5, r/5);
	// 输出文字时间
	//xyprintf(x0+r+r/5, y0+r-r/5, "%2d:%2d:%2d", hour, minute, second);
}

void DrawFace(int mode)//绘制笑脸
{
	static int clickClock = 0;
	float h = heightOfBlock*3/2;
	float w = widthOfBlock*3/2;
	float x = xFace;//(widthOfBlock*widthOfBoard-w)/2+widthOfBorder
	float y = (heightOfBar-h)/2;
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
	//按下形态维持
	if(mode == 1) clickClock = clock();
	else if(clock() - clickClock < 200) mode = 1;
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

void DrawBoard(int mode)//绘制总外部窗口
{
	int r, c;
	int rc1, cc1, rc2, cc2;
	int highlight;
	setfillcolor(LIGHTGRAY);
	ege_fillrect(0, 0, windowWidth, heightOfBar);//清除旧顶栏减少锯齿感
	ege_point polyPoints1[6] =
	{
		{(float)0+dx, (float)heightOfBar+dy},
		{(float)widthOfBlock*widthOfBoard+widthOfBorder*2+dx, (float)heightOfBar+dy},
		{(float)widthOfBlock*widthOfBoard+widthOfBorder+dx, (float)heightOfBar+widthOfBorder+dy},
		{(float)widthOfBorder+dx, (float)heightOfBar+widthOfBorder+dy},
		{(float)widthOfBorder+dx, (float)heightOfBar+heightOfBlock*heightOfBoard+widthOfBorder+dy},
		{(float)0+dx, (float)heightOfBar+heightOfBlock*heightOfBoard+widthOfBorder*2+dy}
	};
	ege_point polyPoints2[6] =
	{
		{(float)widthOfBlock*widthOfBoard+widthOfBorder*2+dx, (float)heightOfBar+dy},
		{(float)widthOfBlock*widthOfBoard+widthOfBorder+dx, (float)heightOfBar+widthOfBorder+dy},
		{(float)widthOfBlock*widthOfBoard+widthOfBorder+dx, (float)heightOfBar+heightOfBlock*heightOfBoard+widthOfBorder+dy},
		{(float)widthOfBorder+dx, (float)heightOfBar+heightOfBlock*heightOfBoard+widthOfBorder+dy},
		{(float)0+dx, (float)heightOfBar+heightOfBlock*heightOfBoard+widthOfBorder*2+dy},
		{(float)widthOfBlock*widthOfBoard+widthOfBorder*2+dx, (float)heightOfBar+heightOfBlock*heightOfBoard+widthOfBorder*2+dy}
	};
	setfillcolor(GRAY);
	ege_fillpoly(6, polyPoints1);
	setfillcolor(WHITE);
	ege_fillpoly(6, polyPoints2);
	//裁剪优化
	//for(rc1=0; (rc1+1)*heightOfBlock+heightOfBar+widthOfBorder+dy < 0; rc1++);
	//for(cc1=0; (cc1+1)*widthOfBlock+widthOfBorder+dx < 0; cc1++);
	//for(rc2=heightOfBoard-1; (rc2-1)*heightOfBlock+heightOfBar+widthOfBorder+dy > screenHeight; rc2--);
	//for(cc2=widthOfBoard-1; (cc2-1)*widthOfBlock+widthOfBorder+dx > screenWidth; cc2--);
	//for(x=0; k*x+b < 0; x++); -> if(b >= 0) x = 0; else x = (-b+k-1)/k;
	if(heightOfBlock+heightOfBar+widthOfBorder+dy >= 0) rc1 = 0;
	else rc1 = (-(heightOfBlock+heightOfBar+widthOfBorder+dy)+heightOfBlock-1)/heightOfBlock;
	if(widthOfBlock+widthOfBorder+dx >= 0) cc1 = 0;
	else cc1 = (-(widthOfBlock+widthOfBorder+dx)+widthOfBlock-1)/widthOfBlock;
	//for(x=M; k*x+b > 0; x--); -> if(k*M+b <= 0) x = M; else if(b <= 0) x = -b/k; else x = (-b-k+1)/k;
	if((heightOfBoard-2)*heightOfBlock+heightOfBar+widthOfBorder+dy <= screenHeight) rc2 = heightOfBoard-1;
	else if(-heightOfBlock+heightOfBar+widthOfBorder+dy <= screenHeight)
	{
		rc2 = -(-heightOfBlock+heightOfBar+widthOfBorder+dy-screenHeight)/heightOfBlock;
	}
	else rc2 = (-(-heightOfBlock+heightOfBar+widthOfBorder+dy-screenHeight)-heightOfBlock+1)/heightOfBlock;
	if((widthOfBoard-2)*widthOfBlock+widthOfBorder+dx <= screenWidth) cc2 = widthOfBoard-1;
	else if(-widthOfBlock+widthOfBorder+dx <= screenWidth)
	{
		cc2 = -(-widthOfBlock+widthOfBorder+dx-screenWidth)/widthOfBlock;
	}
	else cc2 = (-(-widthOfBlock+widthOfBorder+dx-screenWidth)-widthOfBlock+1)/widthOfBlock;
	for(r=rc1; r<=rc2; r++)
	{
		for(c=cc1; c<=cc2; c++)
		{
			highlight = 0;
			if(newCursor > 1)//淡黄色高亮光标
			{
				if(r == mouseR && c == mouseC) highlight = 1;
				if(newCursor == 3)
				{
					if(r == mouseR || c == mouseC) highlight = 1;
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
	DrawMineA(xRemainedMine, heightOfBar/2, 20*heightOfBar/64*4/3);
	setcolor(RED);
	setfontbkcolor(BLACK);
	xyprintf(xRemainedMineNumber, (heightOfBar-heightOfChar)/2, " %d ", game.remainedMine);
	//用时
	if(showTime == 1)
	{
		DrawClock(xTime, heightOfBar/2, 20*heightOfBar/64, time(0));//按真实时间走的钟(doge)
		setcolor(RED);
		setfontbkcolor(BLACK);
		xyprintf(xTimeNumber, (heightOfBar-heightOfChar)/2, " %d ", game.t1-game.t0+game.t2);
	}
	//3BV
	if(show3BV == 1 && game.showInformation == 1 && widthOfBoard > 27)
	{
		setbkmode(TRANSPARENT);
		setcolor(BLACK);
		//setfontbkcolor(LIGHTGRAY);
		xyprintf(x3BV, (heightOfBar-heightOfChar)/2, "3BV");
		xyprintf(x3BVps, (heightOfBar-heightOfChar)/2, "3BV/s");
		setbkmode(OPAQUE);
		setcolor(RED);
		setfontbkcolor(BLACK);
		xyprintf(x3BVNumber, (heightOfBar-heightOfChar)/2, " %d/%d ", game.total3BV-game.unsolved3BV, game.total3BV);
		xyprintf(x3BVpsNumber, (heightOfBar-heightOfChar)/2, " %.2f ", (float)(game.total3BV-game.unsolved3BV)/(game.t1-game.t0+game.t2));
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
				//填充式
				//if(solution[r][c] == 1) setfillcolor(LIGHTGREEN);
				//else if(solution[r][c] == 2) setfillcolor(PINK);
				//else if(solution[r][c] == 3) setfillcolor(LIGHTYELLOW);
				//ege_fillrect(x+widthOfBlock*2/32, y+heightOfBlock*2/32, widthOfBlock*28/32, heightOfBlock*28/32);
				//线框式
				if(solution[r][c] == 1) setcolor(DEEPSKYBLUE);
				else if(solution[r][c] == 2) setcolor(ORANGERED);
				else if(solution[r][c] == 3) setcolor(DEEPSKYBLUE);
				setlinewidth(sideLength/8-sideLength/8%2);//防止奇数线宽边缘模糊
				ege_rectangle(x+widthOfBlock*6/32, y+heightOfBlock*6/32, widthOfBlock*20/32, heightOfBlock*20/32);
				if(solution[r][c] == 3)
				{
					setcolor(PURPLE);
					xyprintf(x+xOfChar, y+yOfChar, "?");
				}
			}
		}
	}
}
/*
void DrawMouse(int x, int y)//绘制鼠标
{
	int x0 = widthOfBorder+dx;
	int y0 = heightOfBar+widthOfBorder+dy;
	float k = 0.8*sideLength/32;
	//鼠标位置指示
	//setcolor(BLACK);
	//setlinewidth(2);
	//ege_line(x0+x-sideLength/4, y0+y, x0+x+sideLength/4, y0+y);
	//ege_line(x0+x, y0+y-sideLength/4, x0+x, y0+y+sideLength/4);
	//setlinewidth(sideLength/16);
	//ege_line(x0+x+sideLength/4, y0+y+sideLength/4, x0+x+sideLength*5/8, y0+y+sideLength*5/8);
	//ege_line(x0+x+sideLength/4, y0+y+sideLength/4, x0+x+sideLength/2, y0+y+sideLength*3/8);
	//ege_line(x0+x+sideLength/4, y0+y+sideLength/4, x0+x+sideLength*3/8, y0+y+sideLength/2);
	//鼠标指针
	ege_point polyPoints[8] = {
		{x0+x+0*k, y0+y+0*k}, {x0+x+0*k, y0+y+27*k}, {x0+x+6*k, y0+y+22*k}, {x0+x+11*k, y0+y+31*k},
		{x0+x+16*k, y0+y+28*k}, {x0+x+11*k, y0+y+20*k}, {x0+x+20*k, y0+y+20*k}, {x0+x+0*k, y0+y+0*k}
	};
	setfillcolor(DEEPSKYBLUE);
	ege_fillpoly(8, polyPoints);
	setcolor(BLACK);
	setlinewidth(2*k);
	ege_drawpoly(8, polyPoints);
}
*/
void UpdateWindowSize()//根据当前方块边长更新窗口大小
{
	//限制不超过屏幕大小1.5*1
	if(widthOfBoard < 10)
	{
		if(widthOfBlock*10+widthOfBorder*2 > screenWidth*3/2) windowWidth = screenWidth*3/2;
		else windowWidth = widthOfBlock*10+widthOfBorder*2;
	}
	else
	{
		if(widthOfBlock*widthOfBoard+widthOfBorder*2 > screenWidth*3/2) windowWidth = screenWidth*3/2;
		else windowWidth = widthOfBlock*widthOfBoard+widthOfBorder*2;
	}
	if(heightOfBar+heightOfBlock*heightOfBoard+widthOfBorder*2 > screenHeight) windowHeight = screenHeight;
	else windowHeight = heightOfBar+heightOfBlock*heightOfBoard+widthOfBorder*2;
	//计算笑脸横坐标
	if(widthOfBoard > 53 && widthOfBlock*widthOfBoard/2+widthOfBorder+widthOfBlock*3/4 > screenWidth)//笑脸右界超出屏幕
	{
		xFace = 28*widthOfBlock;
	}
	else if(widthOfBoard < 10)
	{
		xFace = widthOfBlock*5+widthOfBorder-widthOfBlock*3/4;
	}
	else
	{
		xFace = widthOfBlock*widthOfBoard/2+widthOfBorder-widthOfBlock*3/4;
	}
	//剩余雷数
	if(widthOfBoard > 12)
	{
		xRemainedMine = 2*widthOfBlock;
		xRemainedMineNumber = 3*widthOfBlock+xOfChar;
	}
	else
	{
		xRemainedMine = 1*widthOfBlock;
		xRemainedMineNumber = 2*widthOfBlock;
	}
	//用时
	if(widthOfBoard > 23)
	{
		xTime = 7*widthOfBlock;
		xTimeNumber = 8*widthOfBlock+xOfChar;
	}
	else if(widthOfBoard > 12)
	{
		xTime = (widthOfBoard+6)*widthOfBlock/2;
		xTimeNumber = (widthOfBoard+8)*widthOfBlock/2+xOfChar;
	}
	else if(widthOfBoard > 10)
	{
		xTime = (widthOfBoard+4)*widthOfBlock/2;
		xTimeNumber = (widthOfBoard+6)*widthOfBlock/2+xOfChar;
	}
	else
	{
		xTime = 7*widthOfBlock;
		xTimeNumber = 8*widthOfBlock;
	}
	//3BV
	if(widthOfBoard > 53)
	{
		x3BV = 12*widthOfBlock+xOfChar;
		x3BVps = 20*widthOfBlock+xOfChar;
		x3BVNumber = 14*widthOfBlock+xOfChar;
		x3BVpsNumber = 23*widthOfBlock+xOfChar;
	}
	else if(widthOfBoard > 27)
	{
		x3BV = (widthOfBoard+3)*widthOfBlock/2+xOfChar;
		x3BVps = (widthOfBoard+17)*widthOfBlock/2+xOfChar;
		x3BVNumber = (widthOfBoard+7)*widthOfBlock/2+xOfChar;
		x3BVpsNumber = (widthOfBoard+22)*widthOfBlock/2+xOfChar;
	}
}

void InitWindow(int mode)//创建窗口
{
	static int fastInit = 0;
	if(mode == 1)//传入参数准备执行快速初始化
	{
		fastInit = 1;
		return;
	}
	if(fastInit == 0)
	{
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
		else if(screenHeight >= 1440) sideLength = 44;
		else if(screenHeight >= 1080) sideLength = 32;
		else sideLength = 24;
		while(widthOfBlock*widthOfBoard+widthOfBorder*2 > screenWidth
			|| heightOfBar+heightOfBlock*(heightOfBoard+4)+widthOfBorder*2 > screenHeight)
		{
			if(sideLength > 16) sideLength -= 4;
			else sideLength -= 1;
		}
		if(sideLength < 4) sideLength = 4;
		UpdateWindowSize();//保存窗口大小
		setcaption("MineSweeper Window");
		SetProcessDPIAware();//避免Windows缩放造成模糊
		initgraph(windowWidth, windowHeight, INIT_RENDERMANUAL);
		setbkcolor(LIGHTGRAY);
		setfont(heightOfChar, 0, "Consolas");
		setbkmode(TRANSPARENT);//默认设置为无背景字体
		ege_enable_aa(true);
	}
	fastInit = 0;
	//flushmouse();//避免上一局鼠标消息选择起始点
	if(widthOfBoard < 10) dx = widthOfBlock*(10-widthOfBoard)/2;
	else dx = 0;
	dy = 0;//偏移回正
	GetWindowOperation(NULL, NULL, NULL);
	//showmouse(0);//隐藏鼠标指针
}

void ResizeWindow(char mode)//调整显示大小
{
	int oldSideLength = sideLength;
	//调整方块边长
	if(mode == '+')//4-16时每格调整1，16-64时4，64+时16
	{
		if(sideLength >= 64) sideLength += 16;
		else if(sideLength >= 16) sideLength += 4;
		else sideLength += 1;
	}
	else if(mode == '-')
	{
		if(sideLength > 64) sideLength -= 16;
		else if(sideLength > 16) sideLength -= 4;
		else if(sideLength > 4) sideLength -= 1;
	}
	//调整窗口大小
	UpdateWindowSize();
	resizewindow(windowWidth, windowHeight);
	setfont(heightOfChar, 0, "Consolas");//更新字体大小
	//调整地图偏移
	dx = dx*sideLength/oldSideLength;//维持左上角不变
	dy = dy*sideLength/oldSideLength;
}

int IsMousePosOutside()//鼠标在窗口边界外
{
	//EGE无法区分鼠标静止和鼠标在窗口边界外，调用WindowsAPI
	HWND hwnd = getHWnd();//获取绘图窗口句柄
	RECT rect;
	POINT point;
	GetWindowRect(hwnd, &rect);//获取窗口四角坐标
	GetCursorPos(&point);//获取鼠标屏幕坐标
	return (point.x < rect.left || point.x > rect.right || point.y < rect.top || point.y > rect.bottom);
	//ScreenToClient(hwnd, &point);//转换为窗口坐标
	//窗口大小rect.right-rect.left+1, rect.bottom-rect.top+1
	//return (point.x <= 0 || point.x > rect.right-rect.left || point.y <= 0 || point.y > rect.bottom-rect.top);
}

void GetWindowOperation(char* operation, int* r, int* c)
{
	int xm, ym, xn, yn, xo, yo, clock0, clock1;
	static int isOpening, isSigning, ro, co;//拖动操作
	int isMouseInBoard = 0;
	mouse_msg mouseMsg;
	key_msg keyMsg;
	if(operation == NULL || r == NULL || c == NULL)//重置键鼠消息
	{
		isOpening = 0;
		isSigning = 0;
		flushmouse();
		flushkey();
		return;
	}
	if(IsMousePosOutside())
	{
		mouseR = -1;
		mouseC = -1;
	}
	while(mousemsg())//使用while代替if避免堆积消息产生延迟
	{
		mouseMsg = getmouse();
		//鼠标位置分析
		xm = mouseMsg.x;
		ym = mouseMsg.y;
		//if(game.showInformation == 1)
		//{
			//AddOperation(&operationRecord, clock()-game.clock0, 'm',
				//(ym-dy-heightOfBar-widthOfBorder)*64/sideLength, (xm-dx-widthOfBorder)*64/sideLength);//坐标拟合到64边长
		//}
		mouseR = (ym-dy-heightOfBar-widthOfBorder)/heightOfBlock;
		mouseC = (xm-dx-widthOfBorder)/widthOfBlock;
		if(IsPosInRectangle(xm-dx, ym-dy, widthOfBorder, heightOfBar+widthOfBorder,
			widthOfBlock*widthOfBoard+widthOfBorder-1, heightOfBar+heightOfBlock*heightOfBoard+widthOfBorder-1))
		{
			*r = mouseR;
			*c = mouseC;
			isMouseInBoard = 1;
		}
		else
		{
			mouseR = -1;
			mouseC = -1;
		}
		//鼠标操作处理
		if(mouseMsg.is_up())
		{
			if(mouseMsg.is_left()) isOpening = 0;//鼠标左键抬起
			if(mouseMsg.is_right()) isSigning = 0;//鼠标右键抬起
		}
		if(isOpening == 1 && keystate(key_control))
		{
			isOpening = 0;//Ctrl截止拖动翻开
		}
		if(mouseMsg.is_down())
		{
			if(mouseMsg.is_left())//鼠标左键按下
			{
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
							DrawBoard(0);
							dx -= xn-xm;
							dy -= yn-ym;
							clock0 = clock();
						}
						if(mouseMsg.is_left() && mouseMsg.is_up()) break;
						//delay_ms(refreshCycle);
					}
					dx += xn-xm;
					dy += yn-ym;
					if(dx > -widthOfBlock/8 && dx < widthOfBlock/8) dx = 0;//弱趋向回正
					if(dy > -heightOfBlock/8 && dy < heightOfBlock/8) dy = 0;
					cleardevice();//避免重影
					DrawBoard(0);//避免闪烁
				}
				else
				{
					if(IsPosInRectangle(xm, ym, xFace, heightOfBar/2-heightOfBlock*3/4,
						xFace+widthOfBlock*3/2, heightOfBar/2+heightOfBlock*3/4))
					{
						*operation = '%';
					}
					else if(isMouseInBoard == 1)
					{
						*operation = '@';
						DrawFace(1);
						isOpening = 1;
						ro = *r;
						co = *c;
						break;
					}
				}
			}
			if(mouseMsg.is_right())//鼠标右键按下
			{
				if(isMouseInBoard == 1)
				{
					*operation = '#';
					DrawFace(1);
					isSigning = 1;
					ro = *r;
					co = *c;
					break;
				}
			}
		}
		if(mouseMsg.is_move() && (isOpening == 1 || isSigning == 1))
		{
			//mousepos(&xm, &ym);//最新位置
			//xm = mouseMsg.x;
			//ym = mouseMsg.y;//缓冲位置
			if(isMouseInBoard == 1)
			{
				//DrawFace(1);
				if(ro != *r || co != *c)//移动到其他方块
				{
					xn = widthOfBorder+(*c)*widthOfBlock+widthOfBlock/2+dx-xm;
					yn = heightOfBar+widthOfBorder+(*r)*heightOfBlock+heightOfBlock/2+dy-ym;
					if(xn < 0) xn *= -1;
					if(yn < 0) yn *= -1;
					xo = widthOfBorder+co*widthOfBlock-widthOfBlock/4+dx;
					yo = heightOfBar+widthOfBorder+ro*heightOfBlock-heightOfBlock/4+dy;
					if(2*(heightOfBlock*xn+widthOfBlock*yn) <= heightOfBlock*widthOfBlock//必须移动到方块内菱形位置
						&& !IsPosInRectangle(xm, ym, xo, yo, xo+widthOfBlock*3/2, yo+heightOfBlock*3/2))//移动距离必须超过3/4个方块
					{
						if(isOpening == 1) *operation = '@';
						if(isSigning == 1) *operation = '#';
						DrawFace(1);
						ro = *r;
						co = *c;
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
		if(mouseMsg.is_wheel() && keystate(key_control))
		{
			if(mouseMsg.wheel > 0) ResizeWindow('+');
			else ResizeWindow('-');
			cleardevice();//地图偏移时避免重影
			DrawBoard(0);//避免闪烁
		}
	}
	while(kbmsg())//鼠标专门处理，避免无尽滚动，随后处理纯键盘操作输入
	{
		keyMsg = getkey();
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
	}
}

int CloseWindow(int isWinning)
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
	DrawBoard(1);
	if(isWinning == 1)
	{
		DrawFace(3);
		setcolor(BLACK);//显示阴影
		xyprintf(widthOfBlock+sideLength/16, heightOfBar+widthOfBorder+sideLength/16, "You Win!");
		setcolor(YELLOW);
		xyprintf(widthOfBlock, heightOfBar+widthOfBorder, "You Win!");
	}
	else
	{
		DrawFace(2);
		setcolor(BLACK);
		xyprintf(widthOfBlock+sideLength/16, heightOfBar+widthOfBorder+sideLength/16, "Game Over!");
		setcolor(RED);
		xyprintf(widthOfBlock, heightOfBar+widthOfBorder, "Game Over!");
	}
	delay_ms(1000);
	flushmouse();
	while(newGame == -1)
	{
		cleardevice();//清除旧游戏结束文字减少锯齿感
		DrawBoard(1);//刷新界面
		if(isWinning == 1)
		{
			DrawFace(3);
			setcolor(BLACK);
			xyprintf(widthOfBlock+sideLength/16, heightOfBar+widthOfBorder+sideLength/16, "You Win!");
			setcolor(YELLOW);
			xyprintf(widthOfBlock, heightOfBar+widthOfBorder, "You Win!");
		}
		else
		{
			DrawFace(2);
			setcolor(BLACK);
			xyprintf(widthOfBlock+sideLength/16, heightOfBar+widthOfBorder+sideLength/16, "Game Over!");
			setcolor(RED);
			xyprintf(widthOfBlock, heightOfBar+widthOfBorder, "Game Over!");
		}
		setfont(heightOfChar/2, 0, "黑体");
		setcolor(BLACK);
		//xyprintf(widthOfBlock+sideLength/32, heightOfBar+widthOfBorder+heightOfBlock+sideLength/32, "请按键盘任意键关闭窗口");
		xyprintf(widthOfBlock+sideLength/32, heightOfBar+widthOfBorder+heightOfBlock+sideLength/32, "左键新游戏，右键关闭窗口");
		setcolor(RED);
		//xyprintf(widthOfBlock, heightOfBar+widthOfBorder+heightOfBlock, "请按键盘任意键关闭窗口");
		xyprintf(widthOfBlock, heightOfBar+widthOfBorder+heightOfBlock, "左键新游戏，右键关闭窗口");
		//xyprintf(widthOfBlock, heightOfBar+heightOfBlock*3/2, "请勿按右上角退出！");
		//xyprintf(widthOfBlock, heightOfBar+heightOfBlock, "Press any key to continue . . .");
		setfont(heightOfChar, 0, "Consolas");
		//getch();
		//delay_ms(2000);
		//break;
		if(IsMousePosOutside())
		{
			mouseR = -1;
			mouseC = -1;
		}
		while(mousemsg())
		{
			mouseMsg = getmouse();
			xm = mouseMsg.x;
			ym = mouseMsg.y;
			if(IsPosInRectangle(xm-dx, ym-dy, widthOfBorder, heightOfBar+widthOfBorder,
				widthOfBlock*widthOfBoard+widthOfBorder-1, heightOfBar+heightOfBlock*heightOfBoard+widthOfBorder-1))
			{
				mouseR = (ym-dy-heightOfBar-widthOfBorder)/heightOfBlock;
				mouseC = (xm-dx-widthOfBorder)/widthOfBlock;
			}
			else
			{
				mouseR = -1;
				mouseC = -1;
			}
			if(mouseMsg.is_left() && mouseMsg.is_down())
			{
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
							DrawBoard(1);
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
					DrawBoard(1);
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
				if(mouseMsg.wheel > 0) ResizeWindow('+');
				else ResizeWindow('-');
				cleardevice();
				DrawBoard(1);
				if(isWinning == 1) DrawFace(3);
				else DrawFace(2);
			}
		}
		delay_ms(refreshCycle);
	}
	cleardevice();
	if(newGame == 1) InitWindow(1);
	else closegraph();
	return newGame;
}

int Difficulty(int height, int width, int mine)//根据地图信息判断难度
{
	if(height == 10 && width == 10 && mine == 10) return 0;
	else if(height == 9 && width == 9 && mine == 10) return 1;
	//else if(height == 8 && width == 8 && mine == 10) return 1;//竞速初级，四分中级
	else if(height == 16 && width == 16 && mine == 40) return 2;
	else if(height == 16 && width == 30 && mine == 99) return 3;
	else if(height == 30 && width == 16 && mine == 99) return 3;//竖向高级难度
	else if(height == 42 && width == 88 && mine == 715) return 4;
	else if(height == 88 && width == 42 && mine == 715) return 4;//竖向顶级难度
	//else if(height == 42 && width == 88 && mine == 717) return 4;
	//else if(height == 88 && width == 42 && mine == 717) return 4;//未来可能调整的顶级难度
	else return 5;
}

int IsAroundZeroChain(int r0, int c0)
{
	/*int r, c;
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
	return 0;*/
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

int RemainedMineWin()//判断真实的剩余雷数为0
{
	int r, c;
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(isMine[r][c] == 1 && isShown[r][c] == 0)
			{
				return 0;
			}
		}
	}
	return 1;
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

int NumberOfSign()//标记的数量(#)
{
	int r, c, n = 0;
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

int NumberOfNotShownAround(int r0, int c0)
{
	int r, c, n = 0;
	for(r=r0-1; r<=r0+1; r++)
	{
		for(c=c0-1; c<=c0+1; c++)
		{
			if(r>=0 && r<heightOfBoard && c>=0 && c<widthOfBoard)//确认在范围内
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

int WASDMove(int* r, int* c, WORD wVirtualKeyCode)
{
	if(wVirtualKeyCode == 'W' || wVirtualKeyCode == VK_UP)//W/w/方向键上
	{
		if((*r) > 0)//*r > 0
		{
			(*r)--;//*与++优先级一致，具有右结合性，无法取消括号
			return 1;
		}
	}
	else if(wVirtualKeyCode == 'A' || wVirtualKeyCode == VK_LEFT)
	{
		if((*c) > 0)
		{
			(*c)--;
			return 1;
		}
	}
	else if(wVirtualKeyCode == 'S' || wVirtualKeyCode == VK_DOWN)
	{
		if((*r) < heightOfBoard-1)
		{
			(*r)++;
			return 1;
		}
	}
	else if(wVirtualKeyCode == 'D' || wVirtualKeyCode == VK_RIGHT)
	{
		if((*c) < widthOfBoard-1)
		{
			(*c)++;
			return 1;
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
			numberShown[r][c] = 9;
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

void TranslateMap(int rs0, int cs0)//将输入翻译为地图、显示方式矩阵、已知数字
{
	int r, c, rs, cs;
	//获取地图
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
		PrintSolvingMap(0);
	}
	//识别显示方式矩阵
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(map[r][c] == '%' || map[r][c] == '?')
			{
				isShown[r][c] = 0;
			}
			else if(map[r][c] == '#')
			{
				isShown[r][c] = 2;
			}
			else//可正确识别@为非雷，0识别为@
			{
				isShown[r][c] = 1;
			}
		}
	}
	/*if(debug == 2)
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
	}*/
	//获取已知数字
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			numberShown[r][c] = 9;
			if(isShown[r][c] == 1)
			{
				if(map[r][c] == ' ' || map[r][c] == '_')
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
	/*if(debug == 2)
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
	}*/
}

void Solve(int showAnswer)//程序核心部分(doge)
{
	int r, c, ra, ca, isThinking;
	int isSolving = 1, isFound = 0, isFoundOpen = 0, isSigned = 0;
	int numberOfNotShownAround, numberOfSignAround;
	float minMineRateNotZero;
	/*重置*/
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			solution[r][c] = 0;//0无 1@ 2#
			isThought[r][c] = 0;//0无，1交界线处未知方块，2交界线处非0数字，3处理后不确定的1，4因过长放弃的1
			isMineRate[r][c] = 0;
			//isMineRateA[r][c] = 0;
			//isMineRateB[r][c] = 0;
		}
	}
	minNumberOfPossibleMine = 0;
	maxNumberOfPossibleMine = 0;
	//diffResult = 0;
	/*寻找突破点*/
	while(isSolving == 1)
	{
		isSolving = 0;
		/*执行策略*/
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				if(isShown[r][c] == 1 && numberShown[r][c] != 0)//寻找数字
				{
					numberOfNotShownAround = NumberOfNotShownAround(r, c);//暂存以避免重复计算
					if(numberOfNotShownAround != 0)
					{
						numberOfSignAround = NumberOfSignAround(r, c);
						if(numberShown[r][c] == numberOfNotShownAround + numberOfSignAround)
						{
							/*策略一
							当某块的数字与未知方块的个数相同时，那么未知方块都是雷*/
							if(debug == 2) printf("[Debug]找到1类点(%d,%d)->#", r, c);
							//标记周围未知方块
							for(ra=r-1; ra<=r+1; ra++)
							{
								for(ca=c-1; ca<=c+1; ca++)
								{
									if(ra>=0 && ra<heightOfBoard && ca>=0 && ca<widthOfBoard)//确认在范围内
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
							isSolving = 1;
							//if(diffResult < 1) diffResult = 1;
						}
						else if(numberShown[r][c] == numberOfSignAround)
						{
							/*策略二
							当某块的数字与周围旗子的个数相同时，那么未知方块都不是雷*/
							if(debug == 2) printf("[Debug]找到2类点(%d,%d)->@", r, c);
							//翻开周围未知方块
							for(ra=r-1; ra<=r+1; ra++)
							{
								for(ca=c-1; ca<=c+1; ca++)
								{
									if(ra>=0 && ra<heightOfBoard && ca>=0 && ca<widthOfBoard)//确认在范围内
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
							isSolving = 1;
							//if(diffResult < 1) diffResult = 1;
						}
					}
				}
			}
		}
		if(isSolving == 0)//策略一二无法进行
		{
			if(isSigned == 0)
			{
				/*标记交界线处未知方块*/
				for(r=0; r<heightOfBoard; r++)
				{
					for(c=0; c<widthOfBoard; c++)
					{
						if(isShown[r][c] == 0 && isThought[r][c] == 0 && NumberOfNumberAround(r, c) != 0)
						{
							isThought[r][c] = 1;
							map[r][c] = '?';
						}
					}
				}
				isSigned = 1;//仅标记一次
			}
			/*检查待推理方块存在*/
			isThinking = 0;
			for(r=0; r<heightOfBoard; r++)
			{
				for(c=0; c<widthOfBoard; c++)
				{
					if(isThought[r][c] == 1)
					{
						isThinking = 1;
						break;
					}
				}
				if(isThinking == 1) break;
			}
			if(isThinking == 1)
			{
				/*策略三
				逻辑推理*/
				//if(debug == 2) printf("[Debug]正在深入思考. . . \n");
				isSolving = Think();
				//if(isSolving == 1 && diffResult < 2) diffResult = 2;
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
						isFound = 1;
					}
					else if(solution[r][c] == 1)
					{
						map[r][c] = '@';
						isShown[r][c] = 1;
						isThought[r][c] = 0;
						isFound = 1;
						isFoundOpen = 1;
					}
				}
			}
			if(debug == 2)
			{
				printf("[Debug]已更新地图：\n");
				PrintSolvingMap(0);
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
			if(solveMode == 0 && isFoundOpen == 1) break;//简单模式有翻开解立即停止
		}
	}//策略一二三无法进行
	/*复杂局面分析*/
	if((isFoundOpen == 0 || solveMode == 2) && isThinking == 1)//游戏模式已有翻开解不枚举
	{
		// 初始化全局结果缓存
		if(remainedMineJudgeGlobal == 1)
		{
			results.numberOfThinkChain = 0;
			results.minNumberOfPossibleMine = 0;
			results.maxNumberOfPossibleMine = 0;
		}
		// 逐个枚举全局所有未知链
		isSolving = 1;
		while(isSolving == 1)
		{
			/*策略四
			枚举判断*/
			isSolving = DeepThink();//0放弃，1尝试下一未知链
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
							isFound = 1;
						}
						else if(solution[r][c] == 1)
						{
							map[r][c] = '@';
							isShown[r][c] = 1;
							isThought[r][c] = 0;
							isFound = 1;
							isFoundOpen = 1;
						}
					}
				}
				if(debug == 2)
				{
					printf("[Debug]已更新地图：\n");
					PrintSolvingMap(0);
				}
				if(solveMode != 2 && isFoundOpen == 1)
				{
					if(remainedMineJudgeGlobal == 1) ClearResults();
					break;//非分析模式有翻开解立即停止
				}
			}
		}//全部未知链枚举完毕，可根据多块枚举的整体结果进行剩余雷数判断
		if(isFoundOpen == 0 || solveMode == 2)//仅枚举后进行
		{
			if(remainedMineJudgeGlobal == 1)
			{
				/*策略五
				全局剩余雷数判断*/
				isFoundOpen = WholeThink();
				if(isFoundOpen == 1) isFound = 1;
				ClearResults();
			}
		}
	}
	if(isFoundOpen == 0 || solveMode == 2)//游戏模式有翻开解不分析雷率
	{
		/*策略五
		雷率计算*/
		SummonMineRateForNotShown();
		minMineRateNotZero = 1;
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				if(isShown[r][c] == 2)
				{
					isMineRate[r][c] = 1;
					//isMineRateA[r][c] = 1;
					//isMineRateB[r][c] = 1;
				}
				if(isMineRate[r][c] != 0 && isMineRate[r][c] < minMineRateNotZero)
				{
					minMineRateNotZero = isMineRate[r][c];
				}
				//枚举后根据剩余雷数判断
				if(isMineRate[r][c] == 0 && isShown[r][c] != 1)
				{
					solution[r][c] = 1;
					isShown[r][c] = 1;
					isFound = 1;
					isFoundOpen = 1;
					if(debug == 2) printf("[Debug]找到7类点@(%d,%d)\n", r, c);
					//if(diffResult < 4) diffResult = 4;
				}
				if(isMineRate[r][c] == 1 && isShown[r][c] != 2)
				{
					solution[r][c] = 2;
					isShown[r][c] = 2;
					isFound = 1;
					if(debug == 2) printf("[Debug]找到8类点#(%d,%d)\n", r, c);
					//if(diffResult < 4) diffResult = 4;
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
					if(isMineRate[r][c] == 0) printf("     ");
					else if(isMineRate[r][c] == 1) printf("Mine ");
					else printf("%.2f ", isMineRate[r][c]);
				}
				printf("\n");
			}
			printf("[Debug]最低非0雷率%.2f\n", minMineRateNotZero);
		}
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
					//solution[r][c] = 1;
					solution[r][c] = 3;//不确定翻开
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
				else if(solution[r][c] == 3)
				{
					printf("@?");
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
					else if(solution[r][c] == 3)
					{
						printf("@? %d %d\n", r, c);
					}
				}
			}
		}
	}
	/*输出*/
	//return isFoundOpen;
	if(showAnswer == 1)//输出文字
	{
		if(isFoundOpen == 1)//优先翻开左上角
		{
			for(r=0; r<heightOfBoard; r++)
			{
				for(c=0; c<widthOfBoard; c++)
				{
					if(solution[r][c] == 1) break;
				}
				if(solution[r][c] == 1) break;
			}
		}
		else//随机标记确定点和翻开不确定点
		{
			while(1)//随机选择
			{
				r = rand() % heightOfBoard;
				c = rand() % widthOfBoard;
				if(solution[r][c] != 0)
				{
					break;
				}
			}
		}
		if(solution[r][c] == 1)
		{
			printf(":)\n>@ %d %d\n", r, c);
		}
		else if(solution[r][c] == 2)
		{
			printf(":)\n># %d %d\n", r, c);
		}
		else if(solution[r][c] == 3)
		{
			printf(":(\n>@ %d %d\n", r, c);
		}
	}
	else if(showAnswer == 2 && isFound == 0)//翻开不确定点
	{
		while(1)//随机选择
		{
			r = rand() % heightOfBoard;
			c = rand() % widthOfBoard;
			if(solution[r][c] != 0)
			{
				break;
			}
		}
		/*for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				if(solution[r][c] == 3) break;
			}
			if(solution[r][c] == 3) break;
		}*/
		isShown[r][c] = 1;
	}
	/*else if(showAnswer == 2 && isFound == 1 && isFoundOpen == 0)
	{
		//选择到标记解，与旧版本和文字输出统一随机数，实际并不必要
		while(1)//随机选择
		{
			r = rand() % heightOfBoard;
			c = rand() % widthOfBoard;
			if(solution[r][c] != 0)
			{
				break;
			}
		}//我愿称为抽象行为(doge)
	}*/
}

int NumberTeamType(int number1, int number2, int numberOfMine1, int numberOfMine2, int numberOfNotShown1, int numberOfNotShown2)//数对分类显示调试信息
{
	if(numberOfNotShown2 == 0)//数对之二独占区全已知，1234类数对
	{
		if(number1 == number2)//相等数对，12类数对
		{
			if(numberOfMine1 == numberOfMine2)//已知雷数相等
			{
				return 1;
			}
			if(numberOfMine2 - numberOfMine1 == numberOfNotShown1)//已知雷数之差等于数对之一独占区未知量
			{
				return 2;
			}
		}
		else//不相等数对，34类数对
		{
			if(number2 - number1 == numberOfMine2 - numberOfMine1)//数差等于已知雷数差
			{
				return 3;
			}
			if(number1 - number2 == numberOfMine1 - numberOfMine2 + numberOfNotShown1)//数差等于已知雷数差加数对之一独占区未知量
			{
				return 4;
			}
		}
	}
	else//数对之二独占区不全已知，5678类数对
	{
		if(number1 == number2)//相等数对，56类数对
		{
			if(numberOfMine1 - numberOfMine2 == numberOfNotShown2)//数对之一和数对之二独占区已知雷数之差等于数对之二独占区未知量
			{
				return 5;
			}
			if(numberOfMine2 - numberOfMine1 == numberOfNotShown1)//数对之二和数对之一独占区已知雷数之差等于数对之一独占区未知量
			{
				return 6;
			}
		}
		else//不相等数对，78类数对，实际是所有数对的通式(doge)
		{
			if(number2 - number1 == numberOfMine2 - numberOfMine1 + numberOfNotShown2)//数对之二与数对之一之差等于已知雷数差加数对之二独占区未知量
			{
				return 7;
			}
			if(number1 - number2 == numberOfMine1 - numberOfMine2 + numberOfNotShown1)//数对之一与数对之二之差等于已知雷数差加数对之一独占区未知量
			{
				return 8;
			}
		}
	}
	return 0;
}

int Think()
{
	int r, c, r1, c1, r2, c2;//循环变量
	int numberOfMine1, numberOfMine2, numberOfNotShown1, numberOfNotShown2;//数对变量
	int isFound = 0;
	/*标记未知方块附近数字，作为数对之一*/
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(isThought[r][c] == 2)//重置数对之一
			{
				isThought[r][c] = 0;
			}
			if(isShown[r][c] == 1 && numberShown[r][c] != 9 && NumberOfNotShownAround(r, c) != 0)
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
				if(isThought[r][c] == 0) printf("  ");
				else printf("%d ", isThought[r][c]);
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
							if(r2>=0 && r2<heightOfBoard && c2>=0 && c2<widthOfBoard)//确认在范围内
							{
								if(isShown[r2][c2] == 1 && numberShown[r2][c2] != 9 && numberShown[r2][c2] != 0)//已找到数对之二
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
											if(r>=0 && r<heightOfBoard && c>=0 && c<widthOfBoard)//确认在范围内
											{
												numberTeam[r][c] += 1;
											}
										}
									}
									for(r=r2-1; r<=r2+1; r++)
									{
										for(c=c2-1; c<=c2+1; c++)
										{
											if(r>=0 && r<heightOfBoard && c>=0 && c<widthOfBoard)//确认在范围内
											{
												numberTeam[r][c] += 2;
											}
										}
									}
									numberTeam[r1][c1] = 9;
									numberTeam[r2][c2] = 9;
									//数对之一独占区1，数对之二独占区2，共享区3
									/*求独占区雷数和未知数*/
									numberOfMine1 = 0;
									numberOfMine2 = 0;
									numberOfNotShown1 = 0;
									numberOfNotShown2 = 0;
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
										printf("numberShown1=%d\n", numberShown[r1][c1]);
										printf("numberShown2=%d\n", numberShown[r2][c2]);
										printf("numberOfMine1=%d\n", numberOfMine1);
										printf("numberOfMine2=%d\n", numberOfMine2);
										printf("numberOfNotShown1=%d\n", numberOfNotShown1);
										printf("numberOfNotShown2=%d\n", numberOfNotShown2);
										for(r=0; r<heightOfBoard; r++)
										{
											printf("  ");
											for(c=0; c<widthOfBoard; c++)
											{
												if(numberTeam[r][c] == 0) printf("  ");
												else printf("%d ", numberTeam[r][c]);
											}
											printf("\n");
										}
									}*/
									if(numberOfNotShown1 != 0)
									{
										if(numberShown[r2][c2] - numberShown[r1][c1] == numberOfMine2 - numberOfMine1 + numberOfNotShown2)//数对之二与数对之一之差等于已知雷数差加数对之二独占区未知量
										{
											if(debug == 2)
											{
												printf("[Debug]找到");
												printf("%d", NumberTeamType(numberShown[r1][c1], numberShown[r2][c2], numberOfMine1, numberOfMine2, numberOfNotShown1, numberOfNotShown2));
												printf("类数对(%d,%d)(%d,%d)->@", r1, c1, r2, c2);
											}
											//翻开数对之一独占区未知方块
											for(r=r1-1; r<=r1+1; r++)
											{
												for(c=c1-1; c<=c1+1; c++)
												{
													if(r>=0 && r<heightOfBoard && c>=0 && c<widthOfBoard)//确认在范围内
													{
														if(numberTeam[r][c] == 1 && isThought[r][c] == 1)
														{
															solution[r][c] = 1;
															if(debug == 2) printf("(%d,%d)", r, c);
															isFound = 1;
														}
													}
												}
											}
											if(debug == 2) printf("\n");
											//return 1;
										}
										if(numberShown[r1][c1] - numberShown[r2][c2] == numberOfMine1 - numberOfMine2 + numberOfNotShown1)//数对之一与数对之二之差等于已知雷数差加数对之一独占区未知量
										{
											if(debug == 2)
											{
												printf("[Debug]找到");
												printf("%d", NumberTeamType(numberShown[r1][c1], numberShown[r2][c2], numberOfMine1, numberOfMine2, numberOfNotShown1, numberOfNotShown2));
												printf("类数对(%d,%d)(%d,%d)->#", r1, c1, r2, c2);
											}
											//标记数对之一独占区未知方块
											for(r=r1-1; r<=r1+1; r++)
											{
												for(c=c1-1; c<=c1+1; c++)
												{
													if(r>=0 && r<heightOfBoard && c>=0 && c<widthOfBoard)//确认在范围内
													{
														if(numberTeam[r][c] == 1 && isThought[r][c] == 1)
														{
															solution[r][c] = 2;
															if(debug == 2) printf("(%d,%d)", r, c);
															isFound = 1;
														}
													}
												}
											}
											if(debug == 2) printf("\n");
											//return 1;
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
	return isFound;
}

int ThinkSelect()//生成未知链
{
	int r, c, r1, c1, r2, c2;//循环变量
	int isFoundThinkChain, isRising, isAroundThinkChain;//未知链生成
	//int rc1, cc1, rc2, cc2;//未知链框架
	//每次更新的时候，旧算法总是突然体现出卓越性能，而在平常，性能则平平无奇
	int numberOfThought;//未知链长度
	/*生成准备*/
	/*for(r=0; r<heightOfBoard; r++)//重置
	{
		for(c=0; c<widthOfBoard; c++)
		{
			thinkChain[r][c] = 0;
		}
	}
	isFoundThinkChain = 0;
	for(r=0; r<heightOfBoard; r++)//找到生成点
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(isThought[r][c] == 1)
			{
				thinkChain[r][c] = 1;
				isFoundThinkChain = 1;
				break;
			}
		}
		if(isFoundThinkChain == 1) break;
	}*/
	isFoundThinkChain = 0;
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			thinkChain[r][c] = 0;//重置未知链
			if(isThought[r][c] == 1 && isFoundThinkChain == 0)
			{
				thinkChain[r][c] = 1;
				/*rc1 = r;
				cc1 = c;
				rc2 = r;
				cc2 = c;*/
				isFoundThinkChain = 1;//找到生成点
			}
		}
	}
	if(isFoundThinkChain == 0) return 0;//退出
	/*生成未知链*/
	/*if(rc1 > 0) rc1--;//初始化未知链框架
	if(cc1 > 0) cc1--;
	if(rc2+1 < heightOfBoard) rc2++;
	if(cc2+1 < widthOfBoard) cc2++;*/
	isRising = 1;
	while(isRising == 1)//向周围生长
	{
		isRising = 0;
		for(r2=0; r2<heightOfBoard; r2++)//标记周围数字用于校验
		{
			for(c2=0; c2<widthOfBoard; c2++)
			{
				if(isShown[r2][c2] == 1 && numberShown[r2][c2] != 9 && numberShown[r2][c2] != 0)
				{
					//isAroundThinkChain = 0;
					for(r=r2-1; r<=r2+1; r++)
					{
						for(c=c2-1; c<=c2+1; c++)
						{
							if(r>=0 && r<heightOfBoard && c>=0 && c<widthOfBoard)//确认在范围内
							{
								if(thinkChain[r][c] == 1)
								{
									thinkChain[r2][c2] = 2;//isAroundThinkChain = 1;
								}
							}
						}
					}
					/*if(isAroundThinkChain == 1)
					{
						thinkChain[r2][c2] = 2;
					}*/
				}
			}
		}
		/*if(rc1 > 0) rc1--;//调整未知链框架框住生长点
		if(cc1 > 0) cc1--;
		if(rc2+1 < heightOfBoard) rc2++;
		if(cc2+1 < widthOfBoard) cc2++;*/
		for(r1=0; r1<heightOfBoard; r1++)//3*3范围有数字则生长
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
							if(r>=0 && r<heightOfBoard && c>=0 && c<widthOfBoard)//确认在范围内
							{
								//if(thinkChain[r][c] == 1 || thinkChain[r][c] == 2)//3*3范围有生成点或数字则生长
								if(thinkChain[r][c] == 2)
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
		/*if(isRising == 1)//调整未知链框架框住数字
		{
			if(rc1 > 0) rc1--;
			if(cc1 > 0) cc1--;
			if(rc2+1 < heightOfBoard) rc2++;
			if(cc2+1 < widthOfBoard) cc2++;
		}*/
	}
	if(debug == 2)
	{
		printf("[Debug]已选择未知链：\n");
		for(r=0; r<heightOfBoard; r++)
		{
			printf("  ");
			for(c=0; c<widthOfBoard; c++)
			{
				if(thinkChain[r][c] == 0) printf("  ");
				else printf("%d ", thinkChain[r][c]);
			}
			printf("\n");
		}
	}
	/*计算长度*/
	numberOfThought = 0;
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
	return numberOfThought;
}

int DeepThink()//枚举
{
	int r, c, r1, c1, i;//循环变量
	int numberOfThought;//枚举准备
	int realNumberOfPossibility, numberOfPossibility, temp;//枚举判断
	int minNumberOfMine, maxNumberOfMine;
	/*选取未知链*/
	numberOfThought = ThinkSelect();
	if(numberOfThought == 0) return 0;
	if(debug == 2) printf("[Debug]未知链长度%d\n", numberOfThought);
	if(numberOfThought < 2*LimLength) countLengthOfThinkChain[numberOfThought]++;
	if(numberOfThought > lengthOfThinkMineCheck)
	{
		//int上限2147483647=2^31-1，最多支持长度30的未知链
		//0关 19效率 30最大
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
		//system("pause");
		return 1;
	}
	for(i=0; i<numberOfThought; i++)
	{
		sumDictionary[i] = 0;//初始化和字典，全字典无需初始化
	}
	if(numberOfThought <= lengthOfThinkNumberCheck)
	{
		if(debug == 2) printf("[Debug]准备枚举雷(NC)\n");
		realNumberOfPossibility = ThinkMine(numberOfThought);
	}
	else
	{
		if(debug == 2) printf("[Debug]准备枚举数字(MC)\n");
		//debug = 2;
		//clrscr();
		realNumberOfPossibility = ThinkNumber(numberOfThought);
		//debug = 0;
	}
	if(realNumberOfPossibility > dictionaryNeeded) dictionaryNeeded = realNumberOfPossibility;
	/*if(realNumberOfPossibility == 0)
	{
		//printf("%d解为0个", numberOfThought);
		for(r1=0; r1<heightOfBoard; r1++)//处理该链可解部分
		{
			for(c1=0; c1<widthOfBoard; c1++)
			{
				if(thinkChain[r1][c1] == 1)
				{
					isThought[r1][c1] = 3;//放弃该链
				}
			}
		}
		return 1;
		//system("pause");
	}*/
	/*结果分析*/
	/*if(realNumberOfPossibility == 1)//输出唯一可能
	{
		for(c=0; c<numberOfThought; c++)
		{
			possibility[c] = 0;
		}
		r = temp;
		c = 0;
		while(r > 0)
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
		c = 0;
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
	else*/
	{
		/*处理字典*/
		if(realNumberOfPossibility > dictionaryCapacity)
		{
			if(debug == 1 || debug == 2) printf("[Debug]字典大小不足%d！\n", realNumberOfPossibility);
			numberOfPossibility = dictionaryCapacity;
			countOfDictionaryOverflow++;//Bench统计
		}
		else
		{
			numberOfPossibility = realNumberOfPossibility;
		}
		if(debug == 2)
		{
			printf("[Debug]已生成字典，共%d种可能\n", numberOfPossibility);
			if(numberOfPossibility <= 1024)
			{
				for(i=0; i<numberOfPossibility; i++)
				{
					printf("%3d: ", i);
					for(c=0; c<numberOfThought; c++)
					{
						printf("%d ", dictionary[i][c]);
					}
					printf("\n");
				}
			}
			printf("Sum: ");
			for(c=0; c<numberOfThought; c++)
			{
				printf("%d ", sumDictionary[c]);
			}
			printf("\n");
		}
		minNumberOfMine = numberOfThought;
		maxNumberOfMine = 0;
		for(i=0; i<numberOfPossibility; i++)
		{
			temp = 0;
			for(c=0; c<numberOfThought; c++)
			{
				temp += dictionary[i][c];
			}
			if(temp < minNumberOfMine) minNumberOfMine = temp;
			if(temp > maxNumberOfMine) maxNumberOfMine = temp;
		}
		if(debug == 2)
		{
			printf("[Debug]字典雷数范围[%d,%d]\n", minNumberOfMine, maxNumberOfMine);
		}
		c = 0;
		for(r1=0; r1<heightOfBoard; r1++)//处理该链可解部分
		{
			for(c1=0; c1<widthOfBoard; c1++)
			{
				if(thinkChain[r1][c1] == 1)
				{
					if(sumDictionary[c] == realNumberOfPossibility)
					{
						solution[r1][c1] = 2;
						minNumberOfMine--;
						maxNumberOfMine--;
						if(debug == 2) printf("[Debug]找到5类点#(%d,%d)\n", r1, c1);
						//if(diffResult < 3) diffResult = 3;
					}
					else if(sumDictionary[c] == 0)
					{
						solution[r1][c1] = 1;
						if(debug == 2) printf("[Debug]找到6类点@(%d,%d)\n", r1, c1);
						//if(diffResult < 3) diffResult = 3;
					}
					else
					{
						isMineRate[r1][c1] =(float) sumDictionary[c] / realNumberOfPossibility;
						//isMineRateA[r1][c1] = sumDictionary[c];
						//isMineRateB[r1][c1] = realNumberOfPossibility;
						isThought[r1][c1] = 3;//放弃该链
					}
					c++;
				}
			}
		}
		minNumberOfPossibleMine += minNumberOfMine;
		maxNumberOfPossibleMine += maxNumberOfMine;
		if(debug == 2)
		{
			printf("[Debug]不确定雷数范围[%d,%d]\n", minNumberOfMine, maxNumberOfMine);
			printf("[Debug]已更新全局不确定雷数范围[%d,%d]\n", minNumberOfPossibleMine, maxNumberOfPossibleMine);
		}
	}
	if(remainedMineJudgeGlobal == 1)
	{
		SaveResult(numberOfThought, numberOfPossibility, minNumberOfMine, maxNumberOfMine);
	}
	return 1;//该链枚举完毕，尝试下一未知链
}

int ThinkMine(int numberOfThought)//枚举
{
	int r, c, r1, c1, r2, c2, i;//循环变量
	int numberOfPossibility, remainedMine, remainedNotShown;//枚举准备
	int isWrong, realNumberOfPossibility, temp;//枚举判断
	/*numberOfPossibility = 1;
	for(i=0; i<numberOfThought; i++)//总可能数
	{
		numberOfPossibility *= 2;//妈的，gmon.out给我滚啊
	}*/
	numberOfPossibility = 1 << numberOfThought;
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			numberCheckBase[r][c] = 0;
		}
	}
	for(r1=0; r1<heightOfBoard; r1++)//计算校验基底
	{
		for(c1=0; c1<widthOfBoard; c1++)
		{
			if(isShown[r1][c1] == 2)
			{
				for(r2=r1-1; r2<=r1+1; r2++)
				{
					for(c2=c1-1; c2<=c1+1; c2++)
					{
						if(r2>=0 && r2<heightOfBoard && c2>=0 && c2<widthOfBoard)//确认在范围内
						{
							numberCheckBase[r2][c2]++;
						}
					}
				}
			}
		}
	}
	if(debug == 2)
	{
		printf("[Debug]%d个未知点共%d种可能性\n", numberOfThought, numberOfPossibility);
		/*printf("[Debug]已生成校验基底：\n");
		for(r=0; r<heightOfBoard; r++)
		{
			printf("  ");
			for(c=0; c<widthOfBoard; c++)
			{
				printf("%d ", numberCheckBase[r][c]);
			}
			printf("\n");
		}*/
	}
	/*枚举验证*/
	if(remainedMineJudgeLocalNC == 1)
	{
		remainedMine = numberOfMine - NumberOfSign();//剩余雷数
		remainedNotShown = NumberOfNotShown() - numberOfThought;//不含未知链的剩余%数
	}
	realNumberOfPossibility = 0;
	for(i=0; i<numberOfPossibility; i++)//遍历可能性
	{
		/*进度条*/
		if(numberOfPossibility > 262144 && (i+1)%131072 == 0)
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
				//numberCheck[r][c] = 0;
				numberCheck[r][c] = numberCheckBase[r][c];
			}
		}
		/*生成可能性*/
		r = i;//r既是row又是remainder
		c = 0;//c既是column又是count，一变量多用了属于是(doge)
		while(r > 0)
		{
			possibility[c] = r%2;
			r /= 2;
			c++;
		}
		/*根据雷数排除*/
		if(remainedMineJudgeLocalNC == 1)
		{
			r = 0;
			for(c=0; c<numberOfThought; c++)
			{
				r += possibility[c];
			}
			if(r > remainedMine//未知链中雷数大于剩余雷数
				|| remainedNotShown < remainedMine - r)//不含未知链的剩余%数小于剩余雷数减未知链中雷数
			{
				//if(numberOfPossibility > 262144) printf(">>>");//进度条加速
				continue;
			}
		}
		/*绘制地图数字*/
		isWrong = 0;
		c = 0;
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
								if(r2>=0 && r2<heightOfBoard && c2>=0 && c2<widthOfBoard)//确认在范围内
								{
									numberCheck[r2][c2]++;
									if(numberCheck[r2][c2] > numberShown[r2][c2]) isWrong = 1;
								}
							}
						}
					}
					c++;
				}
				if(isWrong == 1) break;//提前排除该可能性
			}
			if(isWrong == 1) break;
		}
		/*if(debug == 2)
		{
			printf("\r[Debug]假设%d：\n", i);
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
		if(isWrong == 0)
		{
			/*数字校验*/
			isWrong = 0;
			for(r=0; r<heightOfBoard; r++)
			{
				for(c=0; c<widthOfBoard; c++)
				{
					if(thinkChain[r][c] == 2)//仅校验未知链附近数字
					{
						if(numberShown[r][c] != numberCheck[r][c])
						{
							isWrong = 1;
						}
					}
				}
			}
			if(isWrong == 0)
			{
				for(c=0; c<numberOfThought; c++)//写入字典
				{
					if(realNumberOfPossibility < dictionaryCapacity) dictionary[realNumberOfPossibility][c] = possibility[c];
					sumDictionary[c] += possibility[c];
				}
				realNumberOfPossibility++;
				temp = i;
				if(debug == 2)
				{
					printf("\r[Debug]已找到第%d种可能性%d：", realNumberOfPossibility, temp);//覆盖进度条
					for(c=0; c<numberOfThought; c++)
					{
						printf("%d", possibility[c]);
					}
					printf("\n");
					PrintSolvingMap(1);
				}
			}
		}
	}
	if(numberOfPossibility > 262144)//进度条
	{
		//printf("\n");
		printf("\r                     \r");//覆写[Loading...]8192/8192，21空格
	}
	return realNumberOfPossibility;
}

void SummonMineRateForNotShown()
{
	int r, c, realNumberOfNotShown;
	int minRemainedMine, maxRemainedMine;
	float aveRemainedMine, isMineRateOfNotShown;
	/*计算平均剩余雷数和未枚举的未知方块数*/
	r = numberOfMine - NumberOfSign();//暂存未标记雷数，避免重复计算
	minRemainedMine = r - maxNumberOfPossibleMine;
	maxRemainedMine = r - minNumberOfPossibleMine;
	aveRemainedMine = r;
	realNumberOfNotShown = NumberOfNotShown();
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(isThought[r][c] == 3)
			{
				aveRemainedMine -= isMineRate[r][c];
				//aveRemainedMine -=(float) isMineRateA[r][c]/isMineRateB[r][c];
				realNumberOfNotShown--;
			}
		}
	}
	if(maxRemainedMine <= 0)
	{
		isMineRateOfNotShown = 0;
	}
	else if(minRemainedMine >= realNumberOfNotShown)
	{
		isMineRateOfNotShown = 1;
	}
	else if(aveRemainedMine > 0)//含min == max
	{
		isMineRateOfNotShown = aveRemainedMine / realNumberOfNotShown;
	}
	else//雷率期望<=0
	{
		isMineRateOfNotShown = 0.01;//趋向翻开内部
		//if(minRemainedMine < 0) isMineRateOfNotShown =(float) maxRemainedMine / realNumberOfNotShown;
		//else isMineRateOfNotShown =(float) minRemainedMine / realNumberOfNotShown;
	}
	if(debug == 2)
	{
		printf("<Debug>\n");
		printf("aveRemainedMine=%.2f\n", aveRemainedMine);
		printf("minRemainedMine=%d\n", minRemainedMine);
		printf("maxRemainedMine=%d\n", maxRemainedMine);
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

int NumberOfNumberAround(int r0, int c0)//周围已显示数字的量，用来确定交界线
{
	int r, c, n = 0;
	for(r=r0-1; r<=r0+1; r++)
	{
		for(c=c0-1; c<=c0+1; c++)
		{
			if(r>=0 && r<heightOfBoard && c>=0 && c<widthOfBoard)//确认在范围内
			{
				if(isShown[r][c] == 1 && numberShown[r][c] != 9)//isShown1存在@numberShown9
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

int IsSolvableIsland()//判断当前地图岛上可解性
{
	int r, c, temp, isOpenMine = 0;
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			solution[r][c] = 0;
			if(board[r][c] == 0 && isShown[r][c] == 0)
			{
				isShown[r][c] = 1;//翻开全图0链
				OpenZeroChain(r, c);
			}
		}
	}
	while(1)
	{
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				if(solution[r][c] == 3)
				{
					//isShown[r][c] = 1;//不排除雷率解
					isOpenMine = 1;//不确定解必炸，排除雷率解
				}
				else if(solution[r][c] == 1)
				{
					if(board[r][c] == 0)
					{
						OpenZeroChain(r, c);
					}
					else if(board[r][c] == 9)
					{
						isOpenMine = 1;
					}
				}
			}
		}
		if(isOpenMine == 1) break;
		if(RemainedMineWin()) break;
		LookMap();
		temp = debug;
		debug = 0;//不显示求解信息
		Solve(0);
		debug = temp;
	}
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			isShown[r][c] = 0;
		}
	}
	if(isOpenMine == 1)
	{
		if(debug == 1 || debug == 2) printf("[Debug]该地图岛上不可解\n");
		return 0;
	}
	else
	{
		if(debug == 1 || debug == 2) printf("[Debug]该地图岛上可解\n");
		return 1;
	}
}

int IsSolvableMap(int seed, int r0, int c0)
{
	int r, c, temp, isOpenMine = 0;
	ShownModeBak(1);
	SummonBoard(seed, r0, c0, summonCheckMode, 1);
	if(islandPrediction == 1 && IsSolvableIsland() == 0) return 0;//预判加速
	isShown[r0][c0] = 1;
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			solution[r][c] = 0;
		}
	}
	OpenZeroChain(r0, c0);
	if(debug == 1 || debug == 2) printf("[Debug]正在分析seed=%d,%d,%d\n", seed, r0, c0);
	while(1)
	{
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				if(solution[r][c] == 3)
				{
					//isShown[r][c] = 1;//不排除雷率解
					isOpenMine = 1;//不确定解必炸，排除雷率解
				}
				else if(solution[r][c] == 1)
				{
					if(board[r][c] == 0)
					{
						OpenZeroChain(r, c);
					}
					else if(board[r][c] == 9)
					{
						isOpenMine = 1;
					}
				}
			}
		}
		if(isOpenMine == 1) break;
		if(RemainedMineWin()) break;
		LookMap();
		temp = debug;
		debug = 0;//不显示求解信息
		Solve(0);
		debug = temp;
	}
	if(debug == 2)
	{
		ShowBoard(1);
	}
	ShownModeBak(0);
	if(isOpenMine == 1)
	{
		if(debug == 1 || debug == 2) printf("[Debug]该地图不可解\n");
		return 0;
	}
	else
	{
		if(debug == 1 || debug == 2) printf("[Debug]该地图可解\n");
		return 1;
	}
}

void ShowSolution(int yOfMap)//在原地图显示方案矩阵
{
	int r, c;
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(solution[r][c] == 1)//调用PrinrCell显示@
			{
				gotoxy(cs0+2*c, yOfMap+rs0+r);
				if(backgroundColor == 0xf0)
				{
					PrintCell(9, 1, 0xe0);
				}
				else
				{
					PrintCell(9, 1, backgroundColor/16*16 + 0x0e - 0x04);
				}
			}
			else if(solution[r][c] == 2)//调用PrinrCell显示#
			{
				gotoxy(cs0+2*c, yOfMap+rs0+r);
				if(backgroundColor == 0xf0)
				{
					PrintCell(9, 2, 0xe0);
				}
				else
				{
					PrintCell(9, 2, backgroundColor/16*16 + 0x0e - 0x0c);
				}
			}
			else if(solution[r][c] == 3)
			{
				gotoxy(cs0+2*c, yOfMap+rs0+r);
				if(backgroundColor == 0xf0)//调用PrinrCell显示@
				{
					PrintCell(9, 1, 0xe0);
				}
				else
				{
					PrintCell(9, 1, backgroundColor/16*16 + 0x0e - 0x04);
				}
				gotoxy(cs0+2*c+1, yOfMap+rs0+r);
				ColorStr("?", backgroundColor/16*16 + 0x0d);
			}
		}
	}
}

struct NumberCheck SummonMineCheck(struct NumberCheck nc)
{
	int i, r, c, p, numberOfUnknownAround, numberOfPossibility;
	int*** mineCheckTemp;
	int** newMineCheck;
	//计算周围未知方块数
	numberOfUnknownAround = 0;
	for(r=nc.r1; r<=nc.r2; r++)
	{
		for(c=nc.c1; c<=nc.c2; c++)
		{
			if(thinkChain[r][c] == 1)
			{
				numberOfUnknownAround++;
			}
			if(isShown[r][c] == 2)
			{
				nc.numberOfMine--;
			}
		}
	}
	//计算枚举总可能性数
	/*numberOfPossibility = 1;
	for(i=0; i<numberOfUnknownAround; i++)
	{
		numberOfPossibility *= 2;
	}*/
	numberOfPossibility = 1 << numberOfUnknownAround;
	/*if(debug == 2)
	{
		printf("[Debug]准备枚举在(%d,%d)(%d,%d)间对%d个雷在%d个未知方块的%d种情况\n",
			nc.r1, nc.c1, nc.r2, nc.c2, nc.numberOfMine, numberOfUnknownAround, numberOfPossibility);
	}*/
	for(i=0; i<numberOfPossibility; i++)
	{
		//计算i二进制对应1数
		for(p=0; p<numberOfUnknownAround; p++)
		{
			possibility[p] = 0;
		}
		r = i;
		p = 0;
		while(r > 0)
		{
			possibility[p] = r%2;//0非雷，1雷
			r /= 2;
			p++;
		}
		c = 0;
		for(p=0; p<numberOfUnknownAround; p++)
		{
			c += possibility[p];
		}
		if(c == nc.numberOfMine)//存储该情况
		{
			// 分配存储空间，且初始化为0
			newMineCheck =(int**) MatrixMemory(NULL, nc.r2-nc.r1+1, nc.c2-nc.c1+1, sizeof(int), 1);
			p = 0;
			for(r=nc.r1; r<=nc.r2; r++)
			{
				for(c=nc.c1; c<=nc.c2; c++)
				{
					if(thinkChain[r][c] == 1)//遍历未知方块
					{
						newMineCheck[r-nc.r1][c-nc.c1] = possibility[p]+1;//0跳过，1非雷，2雷
						p++;
					}
				}
			}
			/*if(debug == 2)
			{
				printf("[Debug]找到情况%d\n", i);
				for(r=nc.r1; r<=nc.r2; r++)
				{
					for(c=nc.c1; c<=nc.c2; c++)
					{
						printf("%d ", newMineCheck[r-nc.r1][c-nc.c1]);
					}
					printf("\n");
				}
			}*/
			//加入校验表
			mineCheckTemp =(int***) calloc(nc.numberOfPossibility+1, sizeof(int**));
			for(p=0; p<nc.numberOfPossibility; p++)
			{
				mineCheckTemp[p] = nc.mineCheck[p];
			}
			mineCheckTemp[nc.numberOfPossibility] = newMineCheck;
			if(nc.numberOfPossibility != 0) free(nc.mineCheck);
			nc.mineCheck = mineCheckTemp;
			nc.numberOfPossibility++;
		}
	}
	/*if(debug == 2)
	{
		printf("[Debug]共%d种情况\n", nc.numberOfPossibility);
	}*/
	return nc;
}

void RefreshMineCheck(struct NumberCheckChain ncc)
{
	int r, c, i;
	struct NumberCheck nc;
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			mineCheck[r][c] = 0;
		}
	}
	for(i=0; i<ncc.t; i++)
	{
		nc = ncc.numberCheck[i];
		for(r=nc.r1; r<=nc.r2; r++)
		{
			for(c=nc.c1; c<=nc.c2; c++)
			{
				if(nc.mineCheck[nc.p][r-nc.r1][c-nc.c1] != 0)
				{
					mineCheck[r][c] = nc.mineCheck[nc.p][r-nc.r1][c-nc.c1];
				}
			}
		}
	}
	/*if(debug == 2)
	{
		printf("[Debug]已刷新雷校验矩阵：\n");
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				printf("%d ", mineCheck[r][c]);
			}
			printf("\n");
		}
	}*/
}

int MergeCheck(struct NumberCheck nc)//检查并合并入全局numberCheck，0失败，1成功
{
	int r, c;
	for(r=nc.r1; r<=nc.r2; r++)
	{
		for(c=nc.c1; c<=nc.c2; c++)
		{
			if(mineCheck[r][c] != 0 && nc.mineCheck[nc.p][r-nc.r1][c-nc.c1] != 0)
			{
				if(mineCheck[r][c] != nc.mineCheck[nc.p][r-nc.r1][c-nc.c1])
				{
					return 0;
				}
			}
		}
	}
	for(r=nc.r1; r<=nc.r2; r++)
	{
		for(c=nc.c1; c<=nc.c2; c++)
		{
			if(nc.mineCheck[nc.p][r-nc.r1][c-nc.c1] != 0)
			{
				mineCheck[r][c] = nc.mineCheck[nc.p][r-nc.r1][c-nc.c1];
			}
		}
	}
	/*if(debug == 2)
	{
		printf("[Debug]已合并雷校验矩阵：\n");
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				printf("%d ", mineCheck[r][c]);
			}
			printf("\n");
		}
	}*/
	return 1;
}

int ThinkNumber(int numberOfThought)//选取并思考一个未知链
{
	int r, c, r1, c1, i, p;//循环变量
	int realNumberOfPossibility, remainedMine, remainedNotShown;
	long long int numberOfPossibility, temp;//不用unsigned以观察溢出
	int exp2, exp3/*, exp5*/;
	struct NumberCheckChain ncc;
	struct NumberCheck* numberCheckTemp;
	struct NumberCheck newNumberCheck;
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			mineCheck[r][c] = 0;
		}
	}
	/*根据未知链生成校验表*/
	ncc.numberOfThought = 0;
	ncc.t = 0;
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(thinkChain[r][c] == 2)//遍历数字
			{
				// 新增校验信息
				newNumberCheck.numberOfMine = numberShown[r][c];
				newNumberCheck.numberOfPossibility = 0;
				newNumberCheck.p = 0;
				//定位边界坐标
				if(r == 0) newNumberCheck.r1 = r;
				else newNumberCheck.r1 = r-1;
				if(c == 0) newNumberCheck.c1 = c;
				else newNumberCheck.c1 = c-1;
				if(r == heightOfBoard-1) newNumberCheck.r2 = r;
				else newNumberCheck.r2 = r+1;
				if(c == widthOfBoard-1) newNumberCheck.c2 = c;
				else newNumberCheck.c2 = c+1;
				//枚举周边未知方块可能情况
				newNumberCheck = SummonMineCheck(newNumberCheck);
				// 加入校验表
				numberCheckTemp =(struct NumberCheck*) calloc(ncc.numberOfThought+1, sizeof(struct NumberCheck));
				for(i=0; i<ncc.numberOfThought; i++)
				{
					numberCheckTemp[i] = ncc.numberCheck[i];
				}
				numberCheckTemp[ncc.numberOfThought] = newNumberCheck;
				if(ncc.numberOfThought != 0) free(ncc.numberCheck);
				ncc.numberCheck = numberCheckTemp;
				ncc.numberOfThought++;
			}
		}
	}
	if(debug == 2)
	{
		printf("[Debug]已生成校验表，共");
		//输出编号
		numberOfPossibility = 1;
		exp2 = 0;
		exp3 = 0;
		//exp5 = 0;
		for(i=0; i<ncc.numberOfThought; i++)
		{
			numberOfPossibility *= ncc.numberCheck[i].numberOfPossibility;
			while(numberOfPossibility % 2 == 0)
			{
				numberOfPossibility /= 2;
				exp2++;
			}
			while(numberOfPossibility % 3 == 0)
			{
				numberOfPossibility /= 3;
				exp3++;
			}
			/*while(numberOfPossibility % 5 == 0)
			{
				numberOfPossibility /= 5;
				exp5++;
			}*/
		}
		if(exp2 > 0)//输出因数2指数，避免long long int溢出
		{
			printf("2");
			if(exp2 > 1) printf("^%d", exp2);
			printf("*");
		}
		if(exp3 > 0)
		{
			printf("3");
			if(exp3 > 1) printf("^%d", exp3);
			printf("*");
		}
		/*if(exp5 > 0)
		{
			printf("5");
			if(exp5 > 1) printf("^%d", exp5);
			printf("*");
		}*/
		printf("%lld", numberOfPossibility);
		printf("种可能性：");
		for(i=0; i<ncc.numberOfThought; i++)
		{
			printf("%d ", ncc.numberCheck[i].numberOfPossibility);
		}
		printf("\n");
		/*printf("t=%d\n", ncc.t);
		printf("p=");
		for(i=0; i<ncc.numberOfThought; i++)
		{
			printf("%d ", ncc.numberCheck[i].p);
		}
		printf("\n");*/
	}
	/*枚举验证*/
	if(remainedMineJudgeLocalMC == 1)
	{
		remainedMine = numberOfMine - NumberOfSign();//剩余雷数
		remainedNotShown = NumberOfNotShown() - numberOfThought;//不含未知链的剩余%数
	}
	realNumberOfPossibility = 0;
	while(1)
	{
		/*if(debug == 2)
		{
			printf("[Debug]尝试合并(%d,%d)", ncc.t, ncc.numberCheck[ncc.t].p);
			for(i=0; i<ncc.numberOfThought; i++)
			{
				printf("%d ", ncc.numberCheck[i].p);
			}
		}*/
		if(MergeCheck(ncc.numberCheck[ncc.t]) == 0)//合并失败
		{
			//if(debug == 2) printf(" 合并失败\n");
			if(ncc.numberCheck[ncc.t].p + 1 < ncc.numberCheck[ncc.t].numberOfPossibility)
			{
				ncc.numberCheck[ncc.t].p++;//切换下一个情况再次尝试
			}
			else
			{
				ncc.numberCheck[ncc.t].p = 0;
				ncc.t--;//回溯
				while(ncc.t >= 0 && ncc.numberCheck[ncc.t].p + 1 == ncc.numberCheck[ncc.t].numberOfPossibility)
				{
					ncc.numberCheck[ncc.t].p = 0;
					ncc.t--;
				}
				//if(ncc.t < 0) printf("合并失败准备跳出\n");
				if(ncc.t < 0) break;
				RefreshMineCheck(ncc);//重新合并0~t-1
				ncc.numberCheck[ncc.t].p++;
			}
		}
		else//合并成功
		{
			/*printf("\r[Loading...]");
			for(i=0; i<ncc.numberOfThought; i++)
			{
				if(i == ncc.t)
				{
					ColorNumber(ncc.numberCheck[i].p, 0x0e);
				}
				else if(ncc.numberCheck[i].p == ncc.numberCheck[i].numberOfPossibility-1)
				{
					ColorNumber(ncc.numberCheck[i].p, 0x02);
				}
				else
				{
					printf("%d", ncc.numberCheck[i].p);
				}
				printf(" ");
			}*/
			/*if(debug == 2)
			{
				printf(" 合并成功\n");
				for(i=0; i<ncc.numberOfThought; i++)
				{
					printf("%d ", ncc.numberCheck[i].p);
				}
				printf("\n");
			}*/
			if(ncc.t == ncc.numberOfThought-1)//得到一个完整解
			{
				//转译解
				c = 0;
				for(r1=0; r1<heightOfBoard; r1++)
				{
					for(c1=0; c1<widthOfBoard; c1++)
					{
						if(thinkChain[r1][c1] == 1)
						{
							possibility[c] = mineCheck[r1][c1]-1;
							c++;
						}
					}
				}
				//根据雷数排除
				if(remainedMineJudgeLocalMC == 1)
				{
					r = 0;
					for(c=0; c<numberOfThought; c++)
					{
						r += possibility[c];
					}
					if(r > remainedMine//未知链中雷数大于剩余雷数
						|| remainedNotShown < remainedMine - r)//不含未知链的剩余%数小于剩余雷数减未知链中雷数
					{
						//遍历下一种情况
						while(ncc.t >= 0 && ncc.numberCheck[ncc.t].p + 1 == ncc.numberCheck[ncc.t].numberOfPossibility)
						{
							ncc.numberCheck[ncc.t].p = 0;
							ncc.t--;
						}
						//if(ncc.t < 0) printf("合并成功准备跳出\n");
						if(ncc.t < 0) break;
						RefreshMineCheck(ncc);
						ncc.numberCheck[ncc.t].p++;
						continue;
					}
				}
				//存储解
				for(c=0; c<numberOfThought; c++)//写入字典
				{
					if(realNumberOfPossibility < dictionaryCapacity) dictionary[realNumberOfPossibility][c] = possibility[c];
					sumDictionary[c] += possibility[c];
				}
				realNumberOfPossibility++;
				if(debug == 2)
				{
					//printf("[Debug]找到解：\n");
					//PrintSolvingMap(1);
					if(realNumberOfPossibility <= 1024 || realNumberOfPossibility%1024 == 0)//跳跃输出
					{
						if(realNumberOfPossibility > 1024) printf("\r");//覆写输出
						printf("[Debug]已找到第%d种可能性", realNumberOfPossibility);
						//输出编号
						temp = 0;
						for(i=0; i<ncc.numberOfThought; i++)
						{
							//a1b2b3+a2b3+a3=((a1)b2+a2)b3+a3
							//t <= (max-p)/n -> t*n+p <= max
							if(temp <= (9223372036854775807-ncc.numberCheck[i].p)/ncc.numberCheck[i].numberOfPossibility)
							{
								temp *= ncc.numberCheck[i].numberOfPossibility;
								temp += ncc.numberCheck[i].p;
							}
							else break;
						}
						if(i == ncc.numberOfThought) printf("%lld", temp); 
						else
						{
							/*for(c=0; c < ncc.numberOfThought-i; c++)
							{
								printf("(");
							}*/
							printf("%lld...", temp); 
							/*for(; i<ncc.numberOfThought; i++)
							{
								printf(")*%d+%d", ncc.numberCheck[i].numberOfPossibility, ncc.numberCheck[i].p);
							}*/
						}
						printf("：");
						for(i=0; i<ncc.numberOfThought; i++)
						{
							printf("%d ", ncc.numberCheck[i].p);
						}
					}
					if(realNumberOfPossibility <= 1024) printf("\n");
				}
				//遍历下一种情况
				while(ncc.t >= 0 && ncc.numberCheck[ncc.t].p + 1 == ncc.numberCheck[ncc.t].numberOfPossibility)
				{
					ncc.numberCheck[ncc.t].p = 0;
					ncc.t--;
				}
				//if(ncc.t < 0) printf("合并成功准备跳出\n");
				if(ncc.t < 0) break;
				RefreshMineCheck(ncc);
				ncc.numberCheck[ncc.t].p++;
			}
			else
			{
				ncc.t++;//继续合并
			}
		}
	}
	if(debug == 2 && realNumberOfPossibility > 1024) printf("\n");
	/*printf("\r            ");
	for(i=0; i<ncc.numberOfThought; i++)
	{
		printf("   ");
	}
	printf("\r");*/
	/*if(debug == 2)
	{
		printf("[Debug]准备释放空间\n");
		printf("t=%d\n", ncc.t);
		printf("p=");
		for(i=0; i<ncc.numberOfThought; i++)
		{
			printf("%d ", ncc.numberCheck[i].p);
		}
		printf("\n");
	}*/
	for(i=0; i<ncc.numberOfThought; i++)
	{
		for(p=0; p<ncc.numberCheck[i].numberOfPossibility; p++)
		{
			MatrixMemory((void**)ncc.numberCheck[i].mineCheck[p], ncc.numberCheck[i].r2-ncc.numberCheck[i].r1+1, 0, 0, 0);
		}
	}
	free(ncc.numberCheck);
	/*if(debug == 2)
	{
		printf("释放完毕\n");
		printf("[Debug]共有解%d\n", realNumberOfPossibility);
		system("pause");
	}*/
	/*if(realNumberOfPossibility == 0)
	{
		debug = 2;
		PrintSolvingMap(0);
		realNumberOfPossibility = ThinkNumber(numberOfThought);
		for(i=0; i<ncc.numberOfThought; i++)
		{
			printf("%d ", ncc.numberCheck[i].numberOfPossibility);
		}
		printf("\n");
		//system("pause");
	}*/
	return realNumberOfPossibility;
}

void SaveResult(int numberOfThought, int numberOfPossibility, int minNumberOfMine, int maxNumberOfMine)
{
	int r, c, i;
	int* reduceList;//缩链表
	int* reduceMap;//缩链映射
	struct LocalResult result;
	struct LocalResult* resultsTemp;
	// 记录入结果缓存
	result.numberOfThought = numberOfThought;
	result.numberOfPossibility = numberOfPossibility;
	result.minNumberOfMine = minNumberOfMine;
	result.maxNumberOfMine = maxNumberOfMine;
	// 计算缩链映射
	reduceList =(int*) calloc(numberOfThought, sizeof(int));
	for(c=0; c<numberOfThought; c++)
	{
		if(sumDictionary[c] == 0 || sumDictionary[c] == numberOfPossibility)
		{
			reduceList[c] = 1;
			result.numberOfThought--;
			/*if(sumDictionary[c] == numberOfPossibility)
			{
				result.minNumberOfMine--;
				result.maxNumberOfMine--;
			}*///已计算
		}
	}
	reduceMap =(int*) calloc(result.numberOfThought, sizeof(int));\
	//A 想写B，但是大脑抽风写成了A的样子(doge)
	/*----------------
	c    0 1 2 3 4 5 6 7 8 9
	List 0 0 1 0 0 1 0 0 1 0
	r    0 0 1 1 1 2 2 2 3 3
	c+r  0 1 3 4 5 7 8
	Map  0 1 3 4 6 7 9
	----------------*/
	/*r = 0;
	for(c=0; c+r<numberOfThought; c++)
	{
		if(reduceList[c+r] == 1) r++;
		reduceMap[c] = c+r;
		if(reduceList[c+r] == 1) reduceMap[c]++;
	}*/
	//B
	c = 0;
	for(i=0; i<numberOfThought; i++)
	{
		if(reduceList[i] == 0)
		{
			reduceMap[c] = i;
			c++;
		}
	}
	numberOfThought = result.numberOfThought;
	//记录未知方块为雷可能数(和字典)
	result.sumDictionary =(int*) calloc(numberOfThought, sizeof(int));
	for(c=0; c<numberOfThought; c++)
	{
		result.sumDictionary[c] = sumDictionary[reduceMap[c]];
	}
	//记录某可能的链中雷数
	result.sumMineOfPossibility =(int*) calloc(numberOfPossibility, sizeof(int));
	for(i=0; i<numberOfPossibility; i++)
	{
		//result.sumMineOfPossibility[i] = 0;
		for(c=0; c<numberOfThought; c++)
		{
			result.sumMineOfPossibility[i] += dictionary[i][reduceMap[c]];
		}
	}
	//记录未知链
	result.thinkChain =(int**) MatrixMemory(NULL, heightOfBoard, widthOfBoard, sizeof(int), 1);
	i = 0;
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			if(thinkChain[r][c] == 1)
			{
				if(reduceList[i] == 1) result.thinkChain[r][c] = 0;//缩链
				else result.thinkChain[r][c] = 1;
				i++;
			}
			else
			{
				result.thinkChain[r][c] = thinkChain[r][c];
			}
		}
	}
	//记录全部可能(全字典)
	result.dictionary =(char**) MatrixMemory(NULL, numberOfPossibility, numberOfThought, sizeof(char), 1);
	for(i=0; i<numberOfPossibility; i++)
	{
		for(c=0; c<numberOfThought; c++)
		{
			result.dictionary[i][c] = dictionary[i][reduceMap[c]];
		}
	}
	free(reduceList);//原未知链长度必非0
	if(numberOfThought != 0) free(reduceMap);
	//存入整体结果
	resultsTemp =(struct LocalResult*) calloc(results.numberOfThinkChain+1, sizeof(struct LocalResult));
	for(i=0; i<results.numberOfThinkChain; i++)
	{
		resultsTemp[i] = results.thinkResult[i];
	}
	resultsTemp[results.numberOfThinkChain] = result;
	if(results.numberOfThinkChain != 0) free(results.thinkResult);
	results.thinkResult = resultsTemp;
	results.numberOfThinkChain++;
	results.minNumberOfPossibleMine += minNumberOfMine;
	results.maxNumberOfPossibleMine += maxNumberOfMine;
}

void ClearResults()//释放结果缓存
{
	int i;
	struct LocalResult result;
	if(results.numberOfThinkChain != 0)
	{
		for(i=0; i<results.numberOfThinkChain; i++)
		{
			result = results.thinkResult[i];
			MatrixMemory((void**)result.thinkChain, heightOfBoard, widthOfBoard, sizeof(int), 0);
			MatrixMemory((void**)result.dictionary, result.numberOfPossibility, result.numberOfThought, sizeof(char), 0);
			free(result.sumDictionary);
			free(result.sumMineOfPossibility);
		}
		free(results.thinkResult);
	}
}

void SortResult(struct LocalResult* result)//计数排序，考虑同数区分和顺序
{
	int i, min, max;
	int* count;
	int* temp1;
	char** temp2;
	unsigned int numberOfPossibility = result->numberOfPossibility;//避免多次结构体指针访问，以及莫名奇妙的警告(doge)
	//计算数据范围
	min = result->sumMineOfPossibility[0];
	max = result->sumMineOfPossibility[0];
	for(i=0; i<numberOfPossibility; i++)
	{
		if(result->sumMineOfPossibility[i] < min) min = result->sumMineOfPossibility[i];
		if(result->sumMineOfPossibility[i] > max) max = result->sumMineOfPossibility[i];
	}
	//统计数据出现次数
	count =(int*) calloc(max-min+1, sizeof(int));
	for(i=0; i<numberOfPossibility; i++)
	{
		count[result->sumMineOfPossibility[i]-min]++;
	}
	//转换为待存位置
	//1 2 3 4 5
	//1 3 6 10 15
	//0 1 3 6 10 (15)
	for(i=1; i<max-min+1; i++)
	{
		count[i] += count[i-1];
	}
	for(i=max-min; i>0; i--)
	{
		count[i] = count[i-1];
	}
	count[0] = 0;
	//数据转存
	temp1 =(int*) calloc(numberOfPossibility, sizeof(int));
	temp2 =(char**) calloc(numberOfPossibility, sizeof(char*));
	for(i=0; i<numberOfPossibility; i++)
	{
		temp1[count[result->sumMineOfPossibility[i]-min]] = result->sumMineOfPossibility[i];
		temp2[count[result->sumMineOfPossibility[i]-min]] = result->dictionary[i];
		count[result->sumMineOfPossibility[i]-min]++;
	}
	free(count);
	free(result->sumMineOfPossibility);
	free(result->dictionary);
	result->sumMineOfPossibility = temp1;
	result->dictionary = temp2;
}

void ExcludeResult(struct LocalResult* result, int mine)//排除雷数
{
	int r, c, i;
	int length = 0;
	for(i=0; i < result->numberOfPossibility; i++)
	{
		if(debug == 2 && result->numberOfPossibility > 1024 && i%1024 == 0)
		{
			printf("\r[Loading...]%d/%d ", i, result->numberOfPossibility);
		}
		if(result->sumMineOfPossibility[i] == mine)
		{
			length++;
			//和字典减
			for(c=0; c < result->numberOfThought; c++)
			{
				result->sumDictionary[c] -= result->dictionary[i][c];
			}
		}
		else if(length > 0)//连块排除
		{
			i -= length;
			//全字典移行
			for(c=0; c<length; c++)
			{
				free(result->dictionary[i+c]);
			}
			for(r=i; r < result->numberOfPossibility-length; r++)
			{
				result->dictionary[r] = result->dictionary[r+length];
				result->sumMineOfPossibility[r] = result->sumMineOfPossibility[r+length];//一维内存暂不释放
			}
			result->numberOfPossibility -= length;
			length = 0;
		}
	}
	if(length > 0)//连块排除收尾处理
	{
		i -= length;
		//全字典移行
		for(c=0; c<length; c++)
		{
			free(result->dictionary[i+c]);
		}
		for(r=i; r < result->numberOfPossibility-length; r++)
		{
			result->dictionary[r] = result->dictionary[r+length];
			result->sumMineOfPossibility[r] = result->sumMineOfPossibility[r+length];//一维内存暂不释放
		}
		result->numberOfPossibility -= length;
		length = 0;
	}
	/*for(i=0; i < result->numberOfPossibility; )
	{
		if(debug == 2 && i%16 == 0) printf("\r[Loading...]%d/%d ", i, result->numberOfPossibility);
		if(result->sumMineOfPossibility[i] == mine)
		{
			//和字典减
			for(c=0; c < result->numberOfThought; c++)
			{
				result->sumDictionary[c] -= result->dictionary[i][c];
			}
			//全字典移行
			free(result->dictionary[i]);
			for(r=i; r < result->numberOfPossibility-1; r++)
			{
				result->dictionary[r] = result->dictionary[r+1];
				result->sumMineOfPossibility[r] = result->sumMineOfPossibility[r+1];//一维内存暂不释放
			}
			result->numberOfPossibility--;
		}
		else
		{
			i++;
		}
	}*/
	//if(debug == 2) printf("\n[Debug]已排除雷数%d\n", mine);
	if(debug == 2) printf("\n");
	//重新计算雷数范围
	if(mine == result->minNumberOfMine || mine == result->maxNumberOfMine)
	{
		result->minNumberOfMine = result->numberOfThought;
		result->maxNumberOfMine = 0;
		for(i=0; i < result->numberOfPossibility; i++)
		{
			if(result->sumMineOfPossibility[i] < result->minNumberOfMine)
			{
				result->minNumberOfMine = result->sumMineOfPossibility[i];
			}
			if(result->sumMineOfPossibility[i] > result->maxNumberOfMine)
			{
				result->maxNumberOfMine = result->sumMineOfPossibility[i];
			}
		}
		//更新全局雷数范围
		results.minNumberOfPossibleMine = 0;
		results.maxNumberOfPossibleMine = 0;
		for(i=0; i<results.numberOfThinkChain; i++)
		{
			results.minNumberOfPossibleMine += results.thinkResult[i].minNumberOfMine;
			results.maxNumberOfPossibleMine += results.thinkResult[i].maxNumberOfMine;
		}
		if(debug == 2)
		{
			printf("[Debug]已更新未知链雷数范围[%d,%d]\n", result->minNumberOfMine, result->maxNumberOfMine);
			printf("[Debug]已更新全局雷数范围[%d,%d]\n", results.minNumberOfPossibleMine, results.maxNumberOfPossibleMine);
		}
	}
}

int WholeThink()
{
	int r, c, r1, c1, i;
	int remainedMine, remainedNotShown;
	int isSolving = 1, isFoundOpen = 0;
	struct LocalResult result;//避免多次结构体访问
	remainedMine = numberOfMine - NumberOfSign();//剩余雷数
	remainedNotShown = NumberOfNotShown();//不含全部未知链的剩余%数
	for(i=0; i<results.numberOfThinkChain; i++)
	{
		remainedNotShown -= results.thinkResult[i].numberOfThought;
	}
	if(debug == 2)
	{
		printf("[Debug]已获取全局可能性\n");
		printf("未知链数：%d\n", results.numberOfThinkChain);
		printf("全局未知链雷数范围[%d,%d]\n", results.minNumberOfPossibleMine, results.maxNumberOfPossibleMine);
		printf("剩余雷数：%d 剩余%%数：%d\n", remainedMine, remainedNotShown);
		printf("约束雷数范围：[%d,%d]\n", remainedMine - remainedNotShown, remainedMine);
		for(i=0; i<results.numberOfThinkChain; i++)
		{
			result = results.thinkResult[i];
			printf("\n未知链%d：\n", i);
			printf("长度：%d，雷数范围[%d,%d]\n", result.numberOfThought, result.minNumberOfMine, result.maxNumberOfMine);
			printf("可能数：%d\n", result.numberOfPossibility);
			printf("和字典：");
			for(c=0; c<result.numberOfThought; c++)
			{
				printf("%d ", result.sumDictionary[c]);
			}
			printf("\n");
			printf("雷数字典：");
			for(r=0; r<result.numberOfPossibility; r++)
			{
				if(r == 1024)
				{
					printf("...");
					break;
				}
				printf("%d ", result.sumMineOfPossibility[r]);
			}
			printf("\n");
			/*if(result.numberOfPossibility > 32768)
			{
				FILE* file = fopen("number.txt", "w");
				for(r=0; r<result.numberOfPossibility; r++)
				{
					fprintf(file, "%d ", result.sumMineOfPossibility[r]);
				}
			}*/
		}
	}
	// 是否执行
	if(results.maxNumberOfPossibleMine > remainedMine//最大雷数大于剩余雷数
		|| results.minNumberOfPossibleMine < remainedMine - remainedNotShown)//最小雷数小于剩余方块全为雷的剩余雷数
	{
		/*for(i=0; i<results.numberOfThinkChain; i++)
		{
			if(results.thinkResult[i].numberOfPossibility > dictionaryCapacity)//存在溢出字典
			{
				if(debug == 2) printf("[Debug]不可执行全局剩余雷数判断\n");
			}
		}*/
		if(debug == 2) printf("[Debug]可执行全局剩余雷数判断\n");
		//排序
		for(i=0; i<results.numberOfThinkChain; i++)
		{
			result = results.thinkResult[i];
			if(result.minNumberOfMine == result.maxNumberOfMine//跳过雷数确定链
				|| result.numberOfPossibility == dictionaryCapacity)//跳过字典溢出链
			{
				continue;
			}
			if(result.numberOfPossibility > 1024)
			{
				SortResult(&(results.thinkResult[i]));
				if(debug == 2)
				{
					result = results.thinkResult[i];
					printf("[Debug]已排序未知链%d可能性\n", i);
					printf("长度：%d，雷数范围[%d,%d]\n", result.numberOfThought, result.minNumberOfMine, result.maxNumberOfMine);
					printf("可能数：%d\n", result.numberOfPossibility);
					printf("和字典：");
					for(c=0; c<result.numberOfThought; c++)
					{
						printf("%d ", result.sumDictionary[c]);
					}
					printf("\n");
					printf("雷数字典：");
					for(r=0; r<result.numberOfPossibility; r++)
					{
						printf("%d", result.sumMineOfPossibility[r]);
						if(r+1 < result.numberOfPossibility && result.sumMineOfPossibility[r+1] == result.sumMineOfPossibility[r])
						{
							c = r;
							while(r+1 < result.numberOfPossibility && result.sumMineOfPossibility[r+1] == result.sumMineOfPossibility[r])
							{
								r++;
							}
							printf("*%d", r-c+1);
						}
						printf(" ");
					}
					printf("\n");
				}
			}
		}
		// 根据雷数信息排除
		while(isSolving == 1)
		{
			isSolving = 0;
			for(i=0; i<results.numberOfThinkChain; i++)
			{
				result = results.thinkResult[i];
				if(result.minNumberOfMine == result.maxNumberOfMine//跳过雷数确定链
					|| result.numberOfPossibility == dictionaryCapacity)//跳过字典溢出链
				{
					continue;
				}
				//if(debug == 2) printf("[Debug]正在探查未知链%d\n", i);
				//选择雷数
				c = 0;
				while(c < result.maxNumberOfMine)
				{
					result = results.thinkResult[i];
					for(r=0; r<result.numberOfPossibility; r++)
					{
						if(result.sumMineOfPossibility[r] > c)
						{
							c = result.sumMineOfPossibility[r];
							break;
						}
					}
					//if(debug == 2) printf("[Debug]正在探查雷数%d", c);
					if(results.minNumberOfPossibleMine - result.minNumberOfMine + c <= remainedMine
						&& results.maxNumberOfPossibleMine - result.maxNumberOfMine + c >= remainedMine - remainedNotShown)
					{
						//合法雷数
						//if(debug == 2) printf("：合法\n");
					}
					else
					{
						//if(debug == 2) printf("：非法\n");
						if(debug == 2) printf("[Debug]已找到未知链%d非法雷数%d\n", i, c);
						ExcludeResult(&(results.thinkResult[i]), c);//修改原始数据
						result = results.thinkResult[i];//更新数据
						isSolving = 1;
					}
				}
			}
		}
		// 寻找解
		if(debug == 2)
		{
			printf("[Debug]已更新全局可能性\n");
			printf("未知链数：%d\n", results.numberOfThinkChain);
			printf("全局未知链雷数范围[%d,%d]\n", results.minNumberOfPossibleMine, results.maxNumberOfPossibleMine);
			printf("剩余雷数：%d 剩余%%数：%d\n", remainedMine, remainedNotShown);
			printf("约束雷数范围：[%d,%d]\n", remainedMine - remainedNotShown, remainedMine);
			for(i=0; i<results.numberOfThinkChain; i++)
			{
				result = results.thinkResult[i];
				printf("\n未知链%d：\n", i);
				printf("长度：%d，雷数范围[%d,%d]\n", result.numberOfThought, result.minNumberOfMine, result.maxNumberOfMine);
				printf("可能数：%d\n", result.numberOfPossibility);
				printf("和字典：");
				for(c=0; c<result.numberOfThought; c++)
				{
					printf("%d ", result.sumDictionary[c]);
				}
				printf("\n");
				printf("雷数字典：");
				for(r=0; r<result.numberOfPossibility; r++)
				{
					if(r == 1024)
					{
						printf("...");
						break;
					}
					printf("%d ", result.sumMineOfPossibility[r]);
				}
				printf("\n");
			}
		}
		for(i=0; i<results.numberOfThinkChain; i++)
		{
			result = results.thinkResult[i];
			if(result.numberOfPossibility == dictionaryCapacity)//无需跳过雷数确定链
			{
				continue;
			}
			//if(debug == 2) printf("[Debug]正在检索未知链%d\n", i);
			c = 0;
			for(r1=0; r1<heightOfBoard; r1++)
			{
				for(c1=0; c1<widthOfBoard; c1++)
				{
					if(result.thinkChain[r1][c1] == 1)
					{
						if(result.sumDictionary[c] == 0)
						{
							solution[r1][c1] = 1;
							isShown[r1][c1] = 1;
							isFoundOpen = 1;
							if(debug == 2) printf("[Debug]找到10类点@(%d,%d)\n", r1, c1);
							//if(diffResult < 5) diffResult = 5;
						}
						else if(result.sumDictionary[c] == result.numberOfPossibility)
						{
							solution[r1][c1] = 2;
							isShown[r1][c1] = 2;
							if(debug == 2) printf("[Debug]找到9类点#(%d,%d)\n", r1, c1);
							//if(diffResult < 5) diffResult = 5;
						}
						else
						{
							isMineRate[r1][c1] =(float) result.sumDictionary[c] / result.numberOfPossibility;
							//isMineRateA[r1][c1] = result.sumDictionary[c];
							//isMineRateB[r1][c1] = result.numberOfPossibility;
						}
						c++;
					}
				}
			}
		}
		minNumberOfPossibleMine = results.minNumberOfPossibleMine;
		maxNumberOfPossibleMine = results.maxNumberOfPossibleMine;
	}
	else//剩余区域空间不产生雷数限制不执行
	{
		//不执行，空间释放
		if(debug == 2) printf("[Debug]不可执行全局剩余雷数判断\n");
	}
	// 整合拼接（仅根据雷数信息拼接）
	//转为雷数表
	/*int** mineList =(int**) calloc(results.numberOfThinkChain, sizeof(int*));
	for(i=0; i<results.numberOfThinkChain; i++)
	{
		result = results.thinkResult[i];
		if(result.numberOfPossibility == dictionaryCapacity)//存在字典溢出链直接退出
		{
			break;
		}
		//计算存在雷数数
		c = 0;
		for(r1=result.minNumberOfMine; r1<=result.maxNumberOfMine; r1++)
		{
			c1 = 0;
			for(r=0; r<result.numberOfPossibility; r++)
			{
				if(result.sumMineOfPossibility[r] == r1)
				{
					c1 = 1;
					break;
				}
			}
			if(c1 == 1) c++;
		}
		mineList[i] =(int*) calloc(c, sizeof(int));
		c = 0;
		for(r1=result.minNumberOfMine; r1<=result.maxNumberOfMine; r1++)
		{
			c1 = 0;
			for(r=0; r<result.numberOfPossibility; r++)
			{
				if(result.sumMineOfPossibility[r] == r1)
				{
					c1 = 1;
					break;
				}
			}
			if(c1 == 1)
			{
				mineList[i][c] = r1;
				c++;
			}
		}
	}*/
	// 剩余区域空间判断
	/*if(debug == 2)
	{
		printf("[Debug]已追踪：\n");
		for(r=0; r<heightOfBoard; r++)
		{
			printf("  ");
			for(c=0; c<widthOfBoard; c++)
			{
				if(isThought[r][c] == 0) printf("  ");
				else printf("%d ", isThought[r][c]);
			}
			printf("\n");
		}
	}
	if(remainedMine - results.minNumberOfPossibleMine <= 0)
	{
		//剩余区域空间均为空
	}
	if(remainedMine - results.maxNumberOfPossibleMine >= remainedNotShown)
	{
		//剩余区域空间均为雷
	}*/
	return isFoundOpen;
}

int IsEffectiveRecord(struct Record record)
{
	if(record.solved3BV == record.total3BV && record.difficulty != 5 && record.isHelped == 0 && record.summonMode != 4)
	{
		return 1;//胜利记录、非自定义难度记录、未被实时求解指令帮助过的记录
	}
	else
	{
		return 0;//未胜利记录、自定义难度记录、被实时求解指令帮助过的记录
	}
}

void PrintRecords(struct Records records, int mode)
{
	struct Record* record = records.record;
	int numberOfRecords = records.numberOfRecords, i = 0;
	const char* difficultyName[5] = {"默认", "初级", "中级", "高级", "顶级"};//显示难度名称
	const char* levelName[13] = {
		"无境界",
		"凝气境", "淬体境", "煅骨境", "元丹境", "破海境", "凌空境",
		"天境", "仙境", "神境", "圣境", "帝境", "至尊境"
	};//显示玩家等级称号
	//printf("[Records Editer]\n");
	printf("**************************************************************\n");//宽62
	for(; i<numberOfRecords; i++)
	{
		if(i == 1 && mode == 0 && numberOfRecords > 1024+2)
		{
			i = numberOfRecords-1024;
			printf("...\n");
		}
		if(IsEffectiveRecord(record[i]))
		{
			ColorStr("有效", backgroundColor/16*16 + 0x02);
		}
		else
		{
			ColorStr("无效", backgroundColor/16*16 + 0x04);
		}
		printf("记录%d:", i);
		printf("Map:%d*%d-%d,", record[i].heightOfBoard, record[i].widthOfBoard, record[i].numberOfMine);
		printf("%d,%d,%d,", record[i].seed, record[i].r0, record[i].c0);
		printf("%d,%d ", record[i].summonMode, record[i].iterateMode);
		printf("time=%d(%d) ", record[i].sTime, record[i].msTime);
		printf("3BV:%d/%d ", record[i].solved3BV, record[i].total3BV);
		printf("3BV/s=%.2f ", record[i].speed);
		printf("SS=%.2f ", record[i].score);
		printf("isHelped=%d\n", record[i].isHelped);
	}
	printf("**************************************************************\n");
	printf("* 已读取记录%d条\n", numberOfRecords);
	for(i=0; i<5; i++)
	{
		printf("* %s", difficultyName[i]);
		printf("时间纪录:%4d | 3BV/s纪录:%5.2f", records.minimumTime[i], records.fastestSpeed[i]);//最大9999秒，99.99速
		printf(" | SS纪录:%7.2f", records.highestScore[i]);//最大9999.99
		//printf(" | 局数:%6d | 胜利局数:%5d", records.count[i], records.countEffective[i]);//确保100万局和10万局内显示
		printf("\n");
	}
	printf("* 总时间：%d(%.2f小时) 总翻开数：%d\n", records.totalTime, (float)records.totalTime/3600, records.totalSolved3BV);
	printf("* 最大翻开数字：%d\n", records.maxOpenNumber);//(Ltabsyy: 8)
	printf("* 玩家等级：Lv.%d(%s)\n", records.gamerLevel, levelName[records.gamerLevel]);
	printf("*******************************\n");
}

struct Records ReadRecords()
{
	FILE* file;
	struct Records records;
	struct Record* record;
	char line[128]={0};//文件行缓冲
	int numberOfRecords = 0, i;
	//初始化
	records.numberOfRecords = 0;
	for(i=0; i<5; i++)
	{
		records.minimumTime[i] = -1;
		records.fastestSpeed[i] = 0;
		records.highestScore[i] = 0;
		records.count[i] = 0;
		records.countEffective[i] = 0;
	}
	records.totalTime = 0;
	records.totalSolved3BV = 0;
	records.maxOpenNumber = 0;
	records.gamerLevel = 0;
	if((file = fopen("minesweeper-records.csv", "r")))
	{
		fscanf(file, "heightOfBoard,widthOfBoard,numberOfMine,"
			"seed,r0,c0,summonMode,iterateMode,sTime,msTime,solved3BV,total3BV,isHelped\n");
		while(fgets(line, 128, file) != NULL)
		{
			numberOfRecords++;//计算记录数
		}
		record =(struct Record*) calloc(numberOfRecords, sizeof(struct Record));//分配内存
		rewind(file);//返回到文件头
		fscanf(file, "heightOfBoard,widthOfBoard,numberOfMine,"
			"seed,r0,c0,summonMode,iterateMode,sTime,msTime,solved3BV,total3BV,isHelped\n");
		for(i=0; fgets(line, 128, file) != NULL; i++)
		{
			sscanf(line, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
				&(record[i].heightOfBoard), &(record[i].widthOfBoard), &(record[i].numberOfMine),
				&(record[i].seed), &(record[i].r0), &(record[i].c0),
				&(record[i].summonMode), &(record[i].iterateMode),
				&(record[i].sTime), &(record[i].msTime),
				&(record[i].solved3BV), &(record[i].total3BV),
				&(record[i].isHelped));
		}
		fclose(file);
		records.numberOfRecords = numberOfRecords;
		records.record = record;
		//计算纪录
		for(i=0; i<numberOfRecords; i++)
		{
			//计算3BV/s
			record[i].speed =(float) record[i].solved3BV / record[i].sTime;
			//计算难度
			record[i].difficulty = Difficulty(record[i].heightOfBoard, record[i].widthOfBoard, record[i].numberOfMine);
			//计算分数(发挥水平)
			record[i].score = SS(record[i].heightOfBoard, record[i].widthOfBoard, record[i].numberOfMine,
								 record[i].sTime, record[i].solved3BV, record[i].total3BV);
			//计算总时间和总翻开数
			records.totalTime += record[i].sTime;
			records.totalSolved3BV += record[i].solved3BV;
			records.count[record[i].difficulty]++;
			//计算纪录
			if(IsEffectiveRecord(record[i]))
			{
				if(record[i].sTime < records.minimumTime[record[i].difficulty] || records.minimumTime[record[i].difficulty] == -1)
				{
					records.minimumTime[record[i].difficulty] = record[i].sTime;
				}
				if(record[i].speed > records.fastestSpeed[record[i].difficulty])
				{
					records.fastestSpeed[record[i].difficulty] = record[i].speed;
				}
				if(record[i].score > records.highestScore[record[i].difficulty])
				{
					records.highestScore[record[i].difficulty] = record[i].score;
				}
				records.countEffective[record[i].difficulty]++;
			}
		}
		//计算玩家等级
		records.gamerLevel = GamerLevel(records);
		//计算最大翻开数字
		//records.maxOpenNumber = MaxOpenNumber(records);//可暂不计算提升启动速度
	}
	return records;//值传递而不是指针传递
}

void WriteRecords(struct Records records)
{
	FILE* file;
	struct Record* record = records.record;
	int numberOfRecords = records.numberOfRecords, i;
	file = fopen("minesweeper-records.csv", "w");
	fprintf(file, "heightOfBoard,widthOfBoard,numberOfMine,"
		"seed,r0,c0,summonMode,iterateMode,sTime,msTime,solved3BV,total3BV,isHelped");
	for(i=0; i<numberOfRecords; i++)
	{
		fprintf(file, "\n%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
			record[i].heightOfBoard, record[i].widthOfBoard, record[i].numberOfMine,
			record[i].seed, record[i].r0, record[i].c0,
			record[i].summonMode, record[i].iterateMode,
			record[i].sTime, record[i].msTime,
			record[i].solved3BV, record[i].total3BV,
			record[i].isHelped);
	}
	fclose(file);
	if(numberOfRecords == 0)//删除文件
	{
		remove("minesweeper-records.csv");
	}
}

struct Records AddRecord(struct Records records, struct Record newRecord)
{
	int i;
	int numberOfRecords = records.numberOfRecords + 1;
	struct Record* record =(struct Record*) calloc(numberOfRecords, sizeof(struct Record));
	newRecord.speed =(float) newRecord.solved3BV / newRecord.sTime;
	newRecord.score = SS(newRecord.heightOfBoard, newRecord.widthOfBoard, newRecord.numberOfMine,
						 newRecord.sTime, newRecord.solved3BV, newRecord.total3BV);
	//读取record数据
	for(i=0; i<numberOfRecords-1; i++)
	{
		record[i] = records.record[i];
	}
	record[numberOfRecords-1] = newRecord;
	//更新records数据
	if(numberOfRecords != 1) free(records.record);//records非空时释放
	records.record = record;
	records.numberOfRecords++;
	records.totalTime += newRecord.sTime;
	records.totalSolved3BV += newRecord.solved3BV;
	if(IsEffectiveRecord(newRecord))
	{
		if(newRecord.sTime < records.minimumTime[newRecord.difficulty] || records.minimumTime[newRecord.difficulty] == -1)
		{
			records.minimumTime[newRecord.difficulty] = newRecord.sTime;
			printf("恭喜！你更新了当前难度的时间纪录！\n");
			if(debug == 1 || debug == 2)
			{
				printf("[Debug]已更新%d难度时间纪录为%d\n", newRecord.difficulty, newRecord.sTime);
			}
		}
		if(newRecord.speed > records.fastestSpeed[newRecord.difficulty])
		{
			records.fastestSpeed[newRecord.difficulty] = newRecord.speed;
			printf("恭喜！你更新了当前难度的3BV/s纪录！\n");
			if(debug == 1 || debug == 2)
			{
				printf("[Debug]已更新%d难度3BV/s纪录为%.2f\n", newRecord.difficulty, newRecord.speed);
			}
		}
		if(newRecord.score > records.highestScore[newRecord.difficulty])
		{
			records.highestScore[newRecord.difficulty] = newRecord.score;
			printf("恭喜！你更新了当前难度的SS纪录！\n");
			if(debug == 1 || debug == 2)
			{
				printf("[Debug]已更新%d难度SS纪录为%.2f\n", newRecord.difficulty, newRecord.score);
			}
		}
	}
	return records;
}

struct Records DeleteRecord(struct Records records, int deleteNumber)
{
	int i;
	int numberOfRecords = records.numberOfRecords - 1;
	struct Record* record;
	if(deleteNumber >= 0 && deleteNumber < records.numberOfRecords)
	{
		record =(struct Record*) calloc(numberOfRecords, sizeof(struct Record));
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

struct Records DeleteRecords(struct Records records, int mode)
{
	int i, j, numberOfRecords;
	int* deleteList;//删除表
	struct Record* record;
	if(mode == 0 || mode == 1)
	{
		//标记待删除记录
		deleteList =(int*) calloc(records.numberOfRecords, sizeof(int));
		if(mode == 0)//删除无效记录
		{
			for(i=0; i<records.numberOfRecords; i++)
			{
				if(IsEffectiveRecord(records.record[i]) == 0)
				{
					deleteList[i] = 1;
				}
			}
		}
		else if(mode == 1)//仅保留纪录
		{
			for(i=0; i<records.numberOfRecords; i++)
			{
				if(IsEffectiveRecord(records.record[i]) == 0//无效记录
					|| (records.record[i].sTime != records.minimumTime[records.record[i].difficulty]
						&& records.record[i].speed != records.fastestSpeed[records.record[i].difficulty]))//不是纪录
				{
					deleteList[i] = 1;
				}
			}
		}
		//转移records数据
		numberOfRecords = 0;
		for(i=0; i<records.numberOfRecords; i++)
		{
			if(deleteList[i] == 0) numberOfRecords++;//一般玩家大部分记录无效，故从0计算保留数
		}
		record =(struct Record*) calloc(numberOfRecords, sizeof(struct Record));
		j = 0;
		for(i=0; i<records.numberOfRecords; i++)
		{
			if(deleteList[i] == 0)
			{
				record[j] = records.record[i];
				j++;
			}
		}
		free(deleteList);
		free(records.record);
		records.record = record;
		records.numberOfRecords = numberOfRecords;
	}
	else if(mode == 2)//删除全部记录
	{
		records.numberOfRecords = 0;
		for(i=0; i<5; i++)
		{
			records.minimumTime[i] = -1;
			records.fastestSpeed[i] = 0;
		}
		free(records.record);
	}
	return records;
}

int MaxOpenNumber(struct Records records)//计算有效记录最大翻开数字
{
	int i, r, c, maxOpenNumber = 0;
	int numberOfRecords = records.numberOfRecords;
	struct Record* record = records.record;
	struct Record temp;
	//确保内存空间足够
	temp.numberOfMine = numberOfMine;
	temp.heightOfBoard = heightOfBoard;
	temp.widthOfBoard = widthOfBoard;
	ReallocMemory(42, 88, dictionaryCapacity, lengthOfThinkMineCheck);
	if(debug == 2)
	{
		printf("[Debug]即将计算最大翻开数字\n");
		system("pause");
	}
	for(i=0; i<numberOfRecords; i++)
	{
		if(IsEffectiveRecord(record[i]))
		{
			numberOfMine = record[i].numberOfMine;
			heightOfBoard = record[i].heightOfBoard;
			widthOfBoard = record[i].widthOfBoard;
			SummonBoard(record[i].seed, record[i].r0, record[i].c0, record[i].summonMode, record[i].iterateMode);
			//ShowBoard(1);
			for(r=0; r<heightOfBoard; r++)
			{
				for(c=0; c<widthOfBoard; c++)
				{
					if(board[r][c] != 9 && board[r][c] > maxOpenNumber)
					{
						maxOpenNumber = board[r][c];
					}
				}
			}
			if(maxOpenNumber == 8) break;
		}
	}
	if(debug == 2)
	{
		printf("[Debug]最大翻开数字：%d\n", maxOpenNumber);
		system("pause");//现代CPU这么快么？
	}
	//恢复内存空间
	numberOfMine = temp.numberOfMine;
	heightOfBoard = temp.heightOfBoard;
	widthOfBoard = temp.widthOfBoard;
	ReallocMemory(heightOfBoard, widthOfBoard, dictionaryCapacity, lengthOfThinkMineCheck);
	return maxOpenNumber;
}

int GamerLevel(struct Records records)//计算玩家等级并显示称号
{
	int level = 0;//"无境界":初始
	//完成任意一条件获取
	if(records.minimumTime[0] != -1 || records.minimumTime[1] != -1)
	{
		level = 1;//"凝气境":赢1场默认或初级地图
	}
	if((records.minimumTime[0] != -1 && records.minimumTime[0] <= 26)//25.60秒内赢默认地图
		|| (records.minimumTime[1] != -1 && records.minimumTime[1] <= 30)//29.44秒内赢初级地图
		|| records.minimumTime[2] != -1)//赢1场中级地图
	{
		level = 2;//"淬体境"
	}
	if((records.minimumTime[0] != -1 && records.minimumTime[0] <= 19)//18.10秒内赢默认地图
		|| (records.minimumTime[1] != -1 && records.minimumTime[1] <= 21)//20.82秒内赢初级地图
		|| (records.minimumTime[2] != -1 && records.minimumTime[2] <= 129)//128.76秒内赢中级地图
		|| records.minimumTime[3] != -1)//赢1场高级地图
	{
		level = 3;//"煅骨境"
	}
	if((records.minimumTime[0] != -1 && records.minimumTime[0] <= 13)//默认基准12.80秒
		|| (records.minimumTime[1] != -1 && records.minimumTime[1] <= 15)//初级基准14.72秒
		|| (records.minimumTime[2] != -1 && records.minimumTime[2] <= 92)//91.04秒内赢中级地图
		|| (records.minimumTime[3] != -1 && records.minimumTime[3] <= 344))//343.14秒内赢高级地图
	{
		level = 4;//"元丹境"
	}
	//完成所有条件获取
	if(records.minimumTime[0] != -1 && records.minimumTime[1] != -1
		&& records.minimumTime[2] != -1 && records.minimumTime[3] != -1)
	{
		if(records.minimumTime[0] <= 10//9.05秒内赢默认地图
			&& records.minimumTime[1] <= 11//10.41秒内赢初级地图
			&& records.minimumTime[2] <= 65//中级基准64.38秒
			&& records.minimumTime[3] <= 243)//242.64秒内赢高级地图
		{
			level = 5;//"破海境"
			if(records.minimumTime[0] <= 7//6.40秒内赢默认地图
				&& records.minimumTime[1] <= 8//7.36秒内赢初级地图
				&& records.minimumTime[2] <= 46//45.52秒内赢中级地图
				&& records.minimumTime[3] <= 172//高级基准171.57秒
				&& records.minimumTime[4] != -1)//赢1场顶级地图
			{
				level = 6;//"凌空境"
				if(records.minimumTime[0] <= 5//4.53秒内赢默认地图
					&& records.minimumTime[1] <= 6//5.21秒内赢初级地图
					&& records.minimumTime[2] <= 33//32.19秒内赢中级地图
					&& records.minimumTime[3] <= 122//121.34秒内赢高级地图
					&& records.minimumTime[4] <= 2478)//2477.04秒内赢顶级地图
				{
					level = 7;//"天境"
					if(records.minimumTime[0] <= 4//3.20秒内赢默认地图
						&& records.minimumTime[1] <= 4//3.68秒内赢初级地图
						&& records.minimumTime[2] <= 23//22.77秒内赢中级地图
						&& records.minimumTime[3] <= 86//85.79秒内赢高级地图
						&& records.minimumTime[4] <= 1752)//1751.27秒内赢顶级地图
					{
						level = 8;//"仙境"
						if(records.minimumTime[0] <= 3//2.26秒内赢默认地图
							&& records.minimumTime[1] <= 3//2.60秒内赢初级地图
							&& records.minimumTime[2] <= 17//16.10秒内赢中级地图
							&& records.minimumTime[3] <= 61//60.67秒内赢高级地图
							&& records.minimumTime[4] <= 1239)//顶级基准1238.52秒
						{
							level = 9;//"神境"(Ltabsyy: 1 2 17 49 553)
							if(records.minimumTime[0] <= 2//1.60秒内赢默认地图
								&& records.minimumTime[1] <= 2//1.84秒内赢初级地图
								&& records.minimumTime[2] <= 12//11.38秒内赢中级地图
								&& records.minimumTime[3] <= 43//42.89秒内赢高级地图
								&& records.minimumTime[4] <= 876)//875.90秒内赢顶级地图
							{
								level = 10;//"圣境"
								if(records.minimumTime[0] <= 2//1.13秒内赢默认地图
									&& records.minimumTime[1] <= 2//1.30秒内赢初级地图
									&& records.minimumTime[2] <= 9//8.05秒内赢中级地图
									&& records.minimumTime[3] <= 31//30.33秒内赢高级地图
									&& records.minimumTime[4] <= 620)//619.26秒内赢顶级地图
								{
									level = 11;//"帝境"
									if(records.minimumTime[0] <= 1//0.80秒内赢默认地图
										&& records.minimumTime[1] <= 1//0.92秒内赢初级地图
										&& records.minimumTime[2] <= 6//5.69秒内赢中级地图
										&& records.minimumTime[3] <= 22//21.45秒内赢高级地图
										&& records.minimumTime[4] <= 438)//437.95秒内赢顶级地图
									{
										level = 12;//"至尊境"(纪录 0.09 5.80 25.10)
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return level;
}

void PrintMap(struct Map map, int showNumber)
{
	int r, c, ra, ca, i, j, n;
	struct Record temp;
	//备份难度
	temp.numberOfMine = numberOfMine;
	temp.heightOfBoard = heightOfBoard;
	temp.widthOfBoard = widthOfBoard;
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
			else// if(showNumber == 1)
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
	//调用3BV和岛可解性计算
	numberOfMine = map.numberOfMine;
	heightOfBoard = map.height;
	widthOfBoard = map.width;
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			isMine[r][c] = map.isMine[r][c];
			board[r][c] = map.board[r][c];
		}
	}
	if(show3BV == 1)
	{
		printf("3BV=%d \n", BBBV(-1, 0, 0, 0));
		printf("IsSolvableIsland=%d\n", IsSolvableIsland());
	}
	if(showNumber == 0)
	{
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				if(map.board[r][c] != 0 && map.board[r][c] != 9)
				{
					map.board[r][c] = 0;
				}
			}
		}
	}
	//ShowBoard(1);
	//集成输出以处理无数字显示
	for(i=Place(map.width-1); i>0; i--)//列坐标
	{
		PrintBlank(Place(map.height-1)+1);
		for(c=0; c<map.width; c++)
		{
			n = c;
			for(j=1; j<i; j++)
			{
				n /= 10;
			}
			if(i != 1 && n == 0)
			{
				printf("  ");
			}
			else
			{
				printf("%d ", n%10);
			}
		}
		printf("\n");
	}
	for(r=0; r<map.height; r++)
	{
		PrintBlank(Place(map.height-1)-Place(r));//行坐标
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
	//恢复难度
	numberOfMine = temp.numberOfMine;
	heightOfBoard = temp.heightOfBoard;
	widthOfBoard = temp.widthOfBoard;
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
	struct Record temp;
	//备份难度
	temp.numberOfMine = numberOfMine;
	temp.heightOfBoard = heightOfBoard;
	temp.widthOfBoard = widthOfBoard;
	numberOfMine = map.numberOfMine;
	heightOfBoard = map.height;
	widthOfBoard = map.width;
	if(fastShow == 1) boardBuf =(int**) MatrixMemory(NULL, map.height, map.width, sizeof(int), 1);
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
		if(fastShow == 0)
		{
			gotoxy(0, 0);
			PrintMap(map, showNumber);
		}
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
					else// if(showNumber == 1)
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
			printf("Map:%d*%d-%d \n", map.height, map.width, map.numberOfMine);//雷数减少时覆写尾部
			numberOfMine = map.numberOfMine;
			for(r=0; r<heightOfBoard; r++)
			{
				for(c=0; c<widthOfBoard; c++)
				{
					isMine[r][c] = map.isMine[r][c];
					board[r][c] = map.board[r][c];
				}
			}
			if(show3BV == 1)
			{
				printf("3BV=%d \n", BBBV(-1, 0, 0, 0));
				printf("IsSolvableIsland=%d\n", IsSolvableIsland());
			}
			if(showNumber == 0)
			{
				for(r=0; r<heightOfBoard; r++)
				{
					for(c=0; c<widthOfBoard; c++)
					{
						if(map.board[r][c] != 0 && map.board[r][c] != 9)
						{
							map.board[r][c] = 0;
						}
					}
				}
			}
			for(r=0; r<map.height; r++)
			{
				for(c=0; c<map.width; c++)
				{
					if(map.board[r][c] != boardBuf[r][c])
					{
						gotoxy(cs0+2*c, 3+2*show3BV+rs0+r);//yOfMap=3
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
			gotoxy(0, 3+2*show3BV+rs0+map.height);
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
					if(IsPosInRectangle((mousePos.X-cs0)/2, mousePos.Y-rs0-3-2*show3BV, 0, 0, map.width-1, map.height-1))
					{
						r = mousePos.Y-rs0-3-2*show3BV;
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
				showCursor(visibleCursor);
				//api_sleep(refreshCycle);//此处无需延时，因为ReadConsoleInput()阻塞
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
	//恢复难度
	numberOfMine = temp.numberOfMine;
	heightOfBoard = temp.heightOfBoard;
	widthOfBoard = temp.widthOfBoard;
	return map;
}
/*
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
	else
	{
		//如果上一次操作为?且最近三次坐标相同，则不保存上一次
		//1 @ 0 0, (2 ? 0 0), 3 ? 0 0, 4 ? 1 1
		if(operations->tail->operation == '?'
			&& operations->tail->r == r && operations->tail->c == c
			&& operations->tail->prev != NULL
			&& operations->tail->prev->r == r && operations->tail->prev->c == c)
		{
			operations->tail->prev->next = newNode;
			newNode->prev = operations->tail->prev;
			free(operations->tail);
			operations->tail = newNode;
		}
		//如果上一次操作和新操作均为鼠标坐标且时间相同，则不保存这一次
		//如果上一次操作和新操作均为鼠标坐标且相同，则不保存这一次
		else if(operation == 'm' && operations->tail->operation == 'm'
			&& (operations->tail->mstime == mstime || (operations->tail->r == r && operations->tail->c == c)))
		{
			free(newNode);
		}
		else
		{
			operations->tail->next = newNode;
			operations->tail = newNode;
		}
	}
}

struct Operations AddOperations(int seed, int r0, int c0)
{
	struct Operations operations;
	operations.numberOfMine = numberOfMine;
	operations.heightOfBoard = heightOfBoard;
	operations.widthOfBoard = widthOfBoard;
	operations.summonMode = summonCheckMode;
	operations.seed = seed;
	operations.r0 = r0;
	operations.c0 = c0;
	operations.time = 0;
	operations.isWin = 2;
	operations.head =(struct Operation*) malloc(sizeof(struct Operation));
	operations.head->mstime = 0;
	operations.head->operation = '@';
	operations.head->r = r0;
	operations.head->c = c0;
	operations.head->next = NULL;
	operations.head->prev = NULL;
	operations.tail = operations.head;
	return operations;
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

void WriteOperations(struct Operations operations)
{
	FILE* file;
	char fileName[128];//minesweeper-operations\minesweeper-operations_42x88-715_12345.678_2147483647.txt
	struct Operation* p;
	mkdir("minesweeper-operations");//创建文件夹
	sprintf(fileName, "minesweeper-operations\\minesweeper-operations_%dx%d-%d_%.3f_%d.txt",
			operations.heightOfBoard, operations.widthOfBoard, operations.numberOfMine,
			(float)(operations.tail->mstime-operations.head->mstime)/1000, operations.seed);
	file = fopen(fileName, "w");
	fprintf(file, "Map:%d*%d-%d\n", operations.heightOfBoard, operations.widthOfBoard, operations.numberOfMine);
	fprintf(file, "seed=%d,%d,%d\n", operations.seed, operations.r0, operations.c0);
	fprintf(file, "summonCheckMode=%d\n", operations.summonMode);
	fprintf(file, "time=%d\n", operations.time);
	fprintf(file, "isWin=%d\n", operations.isWin);
	//fprintf(file, "sideLength=16\n");
	for(p=operations.head; p!=NULL; p=p->next)
	{
		fprintf(file, "\n%d %c %d %d", p->mstime, p->operation, p->r, p->c);
	}
	fclose(file);
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
		showCursor(1);
		fileName = InputFileName();
		showCursor(visibleCursor);
	}
	if((file = fopen(fileName, "r")))
	{
		fscanf(file, "Map:%d*%d-%d\n", &(operations.heightOfBoard), &(operations.widthOfBoard), &(operations.numberOfMine));
		fscanf(file, "seed=%d,%d,%d\n", &(operations.seed), &(operations.r0), &(operations.c0));
		fscanf(file, "summonCheckMode=%d\n", &(operations.summonMode));
		fscanf(file, "time=%d\n", &(operations.time));
		fscanf(file, "isWin=%d\n", &(operations.isWin));
		//fscanf(file, "sideLength=16\n");
		while(fscanf(file, "\n%d %c %d %d", &mstime, &operation, &r, &c) != EOF)
		{
			//if(fscanf(file, "\n") == EOF) break;
			//if(fgetc(file) == EOF) break;
			//fscanf(file, "%d %c %d %d", &mstime, &operation, &r, &c);
			AddOperation(&operations, mstime, operation, r, c);
			//printf("\n%d %c %d %d", operations.tail->mstime, operations.tail->operation, operations.tail->r, operations.tail->c);
		}
		fclose(file);
	}
	if(debug == 2)
	{
		struct Operation* p;
		for(p=operations.head; p!=NULL; p=p->next)
		{
			printf("\n%d %c %d %d", p->mstime, p->operation, p->r, p->c);
		}
		system("pause");
	}
	return operations;
}

void PlayOperations(struct Operations operations)
{
	//const int sideLength = 16;
	int clock0, r, c, ra, ca;
	char operation;
	int hasMouse = 0;
	int xm, ym;
	struct Record temp;
	struct Operation* p;
	temp.numberOfMine = numberOfMine;
	temp.heightOfBoard = heightOfBoard;
	temp.widthOfBoard = widthOfBoard;
	temp.summonMode = summonCheckMode;
	//temp.isHelped = keepCursor;//保持光标显示
	numberOfMine = operations.numberOfMine;
	heightOfBoard = operations.heightOfBoard;
	widthOfBoard = operations.widthOfBoard;
	summonCheckMode = operations.summonMode;
	//keepCursor = 1;
	ReallocMemory(heightOfBoard, widthOfBoard, dictionaryCapacity, lengthOfThinkMineCheck);
	AdaptScreenBufferWidth();
	SummonBoard(operations.seed, operations.r0, operations.c0, summonCheckMode, 1);
	game.total3BV = BBBV(operations.seed, operations.r0, operations.c0, 1);
	game.unsolved3BV = game.total3BV;
	game.remainedMine = numberOfMine;
	if(fastShow == 1)
	{
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				isShownBuf[r][c] = 0;//初始化显示缓冲为全未翻开
			}
		}
		gotoxy(0, 3);
		ShowBoard(0);
	}
	if(operateMode == 3)
	{
		InitWindow(0);
		for(p=operations.head; p!=NULL; p=p->next)
		{
			if(p->operation == 'm')
			{
				hasMouse = 1;
				break;
			}
		}
		for(operation = 0; operation == 0; )
		{
			DrawBoard(0);
			GetWindowOperation(&operation, &r, &c);
			delay_ms(refreshCycle);
		}
	}
	game.t0 = time(0);
	game.t2 = 0;
	clock0 = clock();
	for(p=operations.head; p!=NULL; p=p->next)
	{
		while(clock()-clock0 < p->mstime) api_sleep(1);
		operation = p->operation;
		r = p->r;
		c = p->c;
		//执行操作
		if(operation == '@')
		{
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
								}
							}
						}
					}
				}
			}
			else
			{
				if(isShown[r][c] == 2) game.remainedMine++;//取消标记，剩余雷数+1
				isShown[r][c] = 1;
				if(board[r][c] == 0) OpenZeroChain(r, c);
			}
			game.unsolved3BV = BBBV(operations.seed, operations.r0, operations.c0, 0);
		}
		else if(operation == '#')
		{
			if(isShown[r][c] == 0)
			{
				isShown[r][c] = 2;
				game.remainedMine--;
			}
			else if(isShown[r][c] == 2)
			{
				isShown[r][c] = 0;
				game.remainedMine++;
			}
			else if(isShown[r][c] == 1 && fastSign == 1)//快速标记
			{
				if(numberOfMineAround[r][c] == NumberOfNotShownAround(r, c) + NumberOfSignAround(r, c))
				{
					for(ra=r-1; ra<=r+1; ra++)
					{
						for(ca=c-1; ca<=c+1; ca++)
						{
							if(ra>=0 && ra<heightOfBoard && ca>=0 && ca<widthOfBoard)//确认在范围内
							{
								if(isShown[ra][ca] == 0)
								{
									isShown[ra][ca] = 2;
									game.remainedMine--;
								}
							}
						}
					}
				}
				else if(NumberOfNotShownAround(r, c) == 0 || NumberOfSignAround(r, c) > numberOfMineAround[r][c])
				{
					for(ra=r-1; ra<=r+1; ra++)
					{
						for(ca=c-1; ca<=c+1; ca++)
						{
							if(ra>=0 && ra<heightOfBoard && ca>=0 && ca<widthOfBoard)//确认在范围内
							{
								if(isShown[ra][ca] == 2)
								{
									isShown[ra][ca] = 0;
									game.remainedMine++;
								}
							}
						}
					}
				}
			}
		}
		if(fastShow == 1)
		{
			ShowBoardFast(3);
		}
		else
		{
			gotoxy(0, 3);
			ShowBoard(0);
		}
		ShowInformation();
		if(operateMode == 3)
		{
			if(operation != 'm')//悬浮高亮
			{
				mouseR = r;
				mouseC = c;
			}
			DrawBoard(0);
			if(operation == '@' || operation == '#') DrawFace(1);
			if(hasMouse == 1)//鼠标显示
			{
				if(operation == 'm')
				{
					xm = c*sideLength/64;
					ym = r*sideLength/64;//坐标拟合到64边长
				}
			}
			else
			{
				xm = widthOfBlock/2+c*widthOfBlock;
				ym = heightOfBlock/2+r*heightOfBlock;
			}
			DrawMouse(xm, ym);
			delay_ms(0);
		}
		if(operation == 'm')
		{
			//SetCursorPos(c, r);
		}
		else
		{
			FreshCursor(r, c, 3);
			//SetCursorPos((cs0+2*c)*sideLength/2+sideLength/4+87, (rs0+r)*sideLength+sideLength/2+110);
		}
	}
	clrscr();
	DrawControlBar(0);
	if(operations.isWin == 0)//败局
	{
		ShowBoard(1);
		printf(":(\nGame Over!\n");
		printf("剩余雷数: %d ", game.remainedMine);
		if(showTime == 1) printf("用时：%d ", operations.time);
		if(show3BV == 1) printf("3BV：%d/%d 3BV/s：%.2f ", game.total3BV-game.unsolved3BV, game.total3BV, (float)(game.total3BV-game.unsolved3BV)/operations.time);
	}
	else if(operations.isWin == 1)//胜局
	{
		ShowBoard(1);
		printf(":)\nYou Win!\n");
		game.unsolved3BV = 0;
		if(showTime == 1) printf("用时：%d ", operations.time);
		if(show3BV == 1) printf("3BV：%d 3BV/s：%.2f ", game.total3BV, (float)game.total3BV/operations.time);
	}
	else//未完成局
	{
		ShowBoard(0);
		printf("剩余雷数: %d ", game.remainedMine);
		if(showTime == 1) printf("用时：%d ", operations.time);
		if(show3BV == 1) printf("3BV：%d/%d 3BV/s：%.2f ", game.total3BV-game.unsolved3BV, game.total3BV, (float)(game.total3BV-game.unsolved3BV)/operations.time);
	}
	if(operateMode == 3)
	{
		if(operations.isWin != 2)
		{
			CloseWindow(operations.isWin);
		}
		else
		{
			DrawBoard(0);
			setfont(heightOfChar/2, 0, "黑体");
			setcolor(BLACK);
			xyprintf(widthOfBlock+sideLength/32, heightOfBar+widthOfBorder+heightOfBlock+sideLength/32, "请按键盘任意键关闭窗口");
			setcolor(RED);
			xyprintf(widthOfBlock, heightOfBar+widthOfBorder+heightOfBlock, "请按键盘任意键关闭窗口");
			setfont(heightOfChar, 0, "Consolas");
			delay_ms(0);
			getch();
			closegraph();
		}
	}
	numberOfMine = temp.numberOfMine;
	heightOfBoard = temp.heightOfBoard;
	widthOfBoard = temp.widthOfBoard;
	summonCheckMode = temp.summonMode;
	//keepCursor = temp.isHelped;
	ReallocMemory(heightOfBoard, widthOfBoard, dictionaryCapacity, lengthOfThinkMineCheck);
}
*/
void RCScan(char* operation, int* r, int* c, int yOfCommand)//@#rc指令输入模块
{
	int p = 0;
	char key;
	gotoxy(0, yOfCommand);
	printf(">%c _     \r>%c ", *operation, *operation);//覆写>@ 128 92
	while(p != 4)
	{
		if(kbhit_console())
		{
			key = getch_console();//接收字符但不显示
			gotoxy(0, yOfCommand);
			//切为鼠标
			if(key == 'm' || key == 'M')
			{
				*operation = 'm';
				break;
			}
			//wasd处理
			if(p == 3 && (key == 'w' || key == 'W' || key == 'a' || key == 'A'
				|| key == 's' || key == 'S' || key == 'd' || key == 'D'))
			{
				//wasd改变坐标
				if(key == 'w' || key == 'a' || key == 's' || key == 'd')
				{
					WASDMove(r, c, key-'a'+'A');//小写转大写
				}
				else if(key == 'W' || key == 'A' || key == 'S' || key == 'D')
				{
					WASDMove(r, c, key);
				}
				//立即显示完整坐标
				printf("\r>%c %d %d \b", *operation, *r, *c);//\b覆写并回移光标
				continue;
			}
			if((p == 1 || p == 2) && (key == 'w' || key == 'W' || key == 's' || key == 'S'))
			{
				//仅输入r时ws改变r
				if(key == 'w' || key == 's')
				{
					WASDMove(r, c, key-'a'+'A');
				}
				else if(key == 'W' || key == 'S')
				{
					WASDMove(r, c, key);
				}
				p = 2;//改变后可立即输入c
				printf("\r>%c %d _ \b\b", *operation, *r);
				continue;
			}
			//退格符处理
			if(key == '\b')
			{
				if(p == 1 || p == 2)
				{
					*r = 0;
					p = 0;
					//gotoxy(0, yOfCommand);
					printf("\r>%c _   \b\b\b\b", *operation);
					//gotoxy(3, yOfCommand);
				}
				else if(p == 3)
				{
					*c = 0;
					p = 2;
					//gotoxy(0, yOfCommand);
					printf("\r>%c %d _  \b\b\b", *operation, *r);
					//gotoxy(4+Place(*r), yOfCommand);
				}
				else if(p == 0)//清空已输入操作
				{
					*operation = 0;
					//gotoxy(0, yOfCommand);
					printf("\r>   \b\b\b");
					//gotoxy(1, yOfCommand);
					break;
				}
				continue;
			}
			//输入坐标
			if(p == 0)//未输入r
			{
				*r = 0;
				*c = 0;
				if(key >= '0' && key <= '9')
				{
					*r = key-'0'+0;
					p = 1;
				}
			}
			else if(p == 1)//正在输入r
			{
				if(key >= '0' && key <= '9')
				{
					*r *= 10;
					*r += key-'0'+0;
					if(*r >= heightOfBoard)
					{
						*r = 0;
						p = 0;
					}
				}
				else//按空格转到c
				{
					p = 2;
				}
			}
			else if(p == 2)//未输入c
			{
				if(key >= '0' && key <= '9')
				{
					*c = key-'0'+0;
					p = 3;
				}
			}
			else if(p == 3)//正在输入c
			{
				if(key >= '0' && key <= '9')
				{
					*c *= 10;
					*c += key-'0'+0;
					if(*c >= widthOfBoard)
					{
						*c = 0;
						p = 2;
					}
				}
				else//按任意键结束
				{
					p = 4;
				}
			}
			//显示坐标
			//gotoxy(0, yOfCommand);
			if(p == 0)
			{
				printf("\r>%c _  \b\b\b", *operation);
			}
			else if(p == 1)
			{
				//printf(">%c %d", *operation, *r);
				printf("\r>%c ", *operation);
				ColorNumber(*r, 0xff-backgroundColor);
				printf("_ \b\b");
			}
			else if(p == 2)
			{
				printf("\r>%c %d _  \b\b\b", *operation, *r);
			}
			else if(p == 3)
			{
				//printf(">%c %d %d", *operation, *r, *c);
				printf("\r>%c %d ", *operation, *r);
				ColorNumber(*c, 0xff-backgroundColor);
				printf("_ \b\b");
			}
			else if(p == 4)
			{
				printf("\r>%c %d %d \b", *operation, *r, *c);
			}
		}
		else
		{
			if(game.showInformation == 1)//刷新信息
			{
				ShowInformation();
				//gotoxy(cs0+2*(*c), yOfCommand-4-heightOfBoard+1+(*r));//光标移到坐标对应位置
				if(p == 1 || p == 2)
				{
					FreshCursor(*r, -1, yOfCommand-4-heightOfMapShown+1);
				}
				else
				{
					FreshCursor(*r, *c, yOfCommand-4-heightOfMapShown+1);
				}
			}
			else//第一次翻开
			{
				//gotoxy(cs0+2*(*c), yOfCommand-2-heightOfBoard+1+(*r));
				if(p == 1 || p == 2)
				{
					FreshCursor(*r, -1, yOfCommand-2-heightOfMapShown+1);
				}
				else
				{
					FreshCursor(*r, *c, yOfCommand-2-heightOfMapShown+1);
				}
				//yOfCommand = heightOfMapShown-1+3+2
			}
		}
		//api_sleep(refreshCycle);
	}
}

void QuestionMark(char operation, int ro, int co)//问号模块
{
	int r, c, ra, ca;
	if(operation == 'i')//初始化
	{
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				questionMark[r][c] = 0;
			}
		}
		//以?标记交界线
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				if(isShown[r][c] == 1 && board[r][c] != 0 && NumberOfNotShownAround(r, c) != 0)
				{
					for(ra=r-1; ra<=r+1; ra++)
					{
						for(ca=c-1; ca<=c+1; ca++)
						{
							if(ra>=0 && ra<heightOfBoard && ca>=0 && ca<widthOfBoard)//确认在范围内
							{
								if(isShown[ra][ca] == 0)
								{
									questionMark[ra][ca] = 1;
								}
							}
						}
					}
				}
			}
		}
	}
	else if(operation == 's')//显示
	{
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				if(questionMark[r][c] != 0)
				{
					gotoxy(cs0+2*c, ro+rs0+r);//ro为yOfMap
					if(questionMark[r][c] == 1)
					{
						printf("?");
						//ColorStr("?", backgroundColor/16*16 + 0x0d);
					}
					else if(questionMark[r][c] == 2)
					{
						ColorStr("@", backgroundColor/16*16 + 0x0e);
						ColorStr("?", backgroundColor/16*16 + 0x0d);
					}
					else if(questionMark[r][c] == 3)
					{
						ColorStr("#", backgroundColor/16*16 + 0x0c);
						ColorStr("?", backgroundColor/16*16 + 0x0d);
					}
				}
			}
		}
	}
	else if(operation == 'c')//清除
	{
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				questionMark[r][c] = 0;
			}
		}
	}
	else if(operation == '@' && isShown[ro][co] == 0)//翻开问号
	{
		if(questionMark[ro][co] != 2) questionMark[ro][co] = 2;
		else questionMark[ro][co] = 0;
	}
	else if(operation == '#' && isShown[ro][co] == 0)//标记问号
	{
		if(questionMark[ro][co] != 3) questionMark[ro][co] = 3;
		else questionMark[ro][co] = 0;
	}
	else if(operation == '\t')//执行
	{
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				if(questionMark[r][c] != 0)
				{
					/*if(questionMark[r][c] == 2 && isShown[r][c] == 0)
					{
						isShown[r][c] = 1;
					}
					else */if(questionMark[r][c] == 3 && isShown[r][c] == 0)
					{
						isShown[r][c] = 2;
					}
					ReShowBoard(r, c, 0, ro);//ro为yOfMap
					questionMark[r][c] = 0;
				}
			}
		}
	}
}

void Solution()//地图求解模块
{
	int r, c, yOfMap;
	gotoxy(0, 0);
	//printf("MineSweeper Solution\n");
	printf("**************************************************************\n");//宽62
	printf("** 地图求解器使用说明：\n");
	printf("** (1)在本界面，对文本地图求解\n");
	printf("** 在键盘模式，鼠标从含坐标地图左上角到右下角整个选中，\n");
	printf("** 按Ctrl+C或右键复制，在此处Ctrl+V或右键粘贴，回车。\n");
	printf("** 游戏若为鼠标模式，按K切换到键盘模式，按M切回鼠标模式。\n");
	printf("** 需要在设置内选择正确的难度！\n");
	printf("** (2)在实时游戏，对当前局面求解\n");
	printf("** 按!或[(2)地图求解]，在原地图显示当前局面所有方案。\n");
	printf("** 按Tab或[(4)Bench]，将显示并立即执行所有方案。\n");
	printf("** 使用后该局游戏将不会打破纪录！\n");
	printf("** (3)按 ` 并回车退出\n");
	printf("**************************************************************\n");
	printf("当前雷数:%d|当前界面大小:%d*%d\n", numberOfMine, heightOfBoard, widthOfBoard);
	yOfMap = 14;
	srand(time(0));
	if(debug == 2)
	{
		printf("<Debug>\n");
		printf("pointer ->(%d,%d)\n", rs0, cs0);
		printf("heightOfMapShown=%d\n", heightOfMapShown);
		printf("widthOfMapShown=%d\n", widthOfMapShown);
		yOfMap += 4;
	}
	/*扫雷*/
	//getchar();//吃回车
	while(1)
	{
		/*输入*/
		printf("请粘贴最新地图：\n");
		yOfMap++;
		fflush(stdin);//清除不稳定的回车
		for(r=0; r<heightOfMapShown; r++)
		{
			for(c=0; c<widthOfMapShown; c++)//默认为空格
			{
				mapShown[r][c] = ' ';
			}
			//gets(mapShown[r]);//从键盘获取该行地图
			fgets(mapShown[r], 2*LimWidth+5, stdin);
			if(mapShown[r][0] == '`') break;
		}
		if(r < heightOfMapShown && mapShown[r][0] == '`')//不访问未申请空间
		{
			break;
		}
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
					//if(mapShown[r][c] == '\n') break;//确保对齐
				}
				//printf("\n");//fgets不丢弃回车
			}
		}
		TranslateMap(rs0, cs0);
		/*扫雷策略和输出*/
		Solve(1);
		if(debug == 2)
		{
			system("pause");
			clrscr();
			yOfMap = 0;
		}
		gotoxy(0, yOfMap);
		PrintSolvingMap(0);//以彩色重绘地图
		ShowSolution(yOfMap);
		yOfMap += heightOfMapShown+2;
		if(debug == 2) yOfMap -= 2;
		gotoxy(0, yOfMap);
	}
}

void Bench(int seedMin, int seedMax, int r0, int c0, int showStep, int showSolution, int showInformation)//Bench模块
{
	int seed, r, c, isOpenMine, temp;
	int t0, t1, countOfWin = 0, countOfSolution = 0, countOfStep = 0;//Bench统计
	//int isWinningOfSeed[LimDictionary]={0};
	//int BBBVOfSeed[LimDictionary][2]={0};
	//int* isWinningOfSeed =(int*) calloc(seedMax-seedMin+1, sizeof(int));
	int** BBBVOfSeed =(int**) MatrixMemory(NULL, seedMax-seedMin+1, 2, sizeof(int), 1);
	int countOf3BV = 0, countOfSolved3BV = 0;
	int maxPlaceOfBBBV = 0, i;//根据最大3BV位数补全3BV矩阵空格
	int clock0, clock1;
	/*初始化*/
	numberOfAbandonedThinkChain = 0;
	countOfDictionaryOverflow = 0;
	dictionaryNeeded = 0;
	for(i=0; i<2*LimLength; i++)
	{
		countLengthOfThinkChain[i] = 0;
	}
	for(seed=seedMin; seed<=seedMax; seed++)
	{
		temp = BBBV(seed, r0, c0, 1);
		BBBVOfSeed[seed-seedMin][1] = temp;
		countOf3BV += temp;
		if(Place(temp) > maxPlaceOfBBBV) maxPlaceOfBBBV = Place(temp);//计算最大3BV位数
	}
	countOfSolved3BV = countOf3BV;
	t0 = time(0);//计时
	clock0 = clock();
	/*开跑！*/
	for(seed=seedMin; seed<=seedMax; seed++)
	{
		isOpenMine = 0;
		SummonBoard(seed, r0, c0, summonCheckMode, 1);
		isShown[r0][c0] = 1;
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				solution[r][c] = 0;
			}
		}
		if(showStep == 1 && fastShow == 1)//中间帧快速显示准备
		{
			for(r=0; r<heightOfBoard; r++)
			{
				for(c=0; c<widthOfBoard; c++)
				{
					isShownBuf[r][c] = 0;//初始化显示缓冲为全未翻开
				}
			}
			gotoxy(0, 3+showInformation);
			ShowBoard(0);
		}
		OpenZeroChain(r0, c0);//第一次打开0链
		//if(debug == 2) ShowBoard(0);//仅显示开局和结束
		while(1)
		{
			if(showStep > 2)
			{
				clrscr();
				DrawProgressBar(100 * (seed-1-seedMin+1) / (seedMax-seedMin+1));
			}
			gotoxy(0, 3);//覆写
			if(showInformation == 1)
			{
				if(showStep > 0)
				{
					//1行Bench置顶信息
					printf("** Map:%d*%d-%d | ", heightOfBoard, widthOfBoard, numberOfMine);
					printf("seed=%d,%d,%d | ", seed, r0, c0);
					printf("Frame %d | ", countOfStep);
					t1 = time(0);//显示Ave FPS（步数/时间）
					printf("FPS %.1f \n", (float)countOfStep/(t1-t0));//覆写掉帧和inf字符
					//if(debug == 1 || debug == 2) ShowBoard(1);
				}
			}
			/*全局已翻开点检查*/
			for(r=0; r<heightOfBoard; r++)
			{
				for(c=0; c<widthOfBoard; c++)
				{
					//if(isShown[r][c] == 1)
					if(solution[r][c] == 1 || (solution[r][c] == 3 && isShown[r][c] == 1))
					{
						if(board[r][c] == 0)
						{
							OpenZeroChain(r, c);
						}
						else if(board[r][c] == 9)
						{
							isOpenMine = 1;
						}
					}
				}
			}
			/*胜负判断*/
			if(isOpenMine == 1) break;
			if(RemainedMineWin())
			{
				countOfWin++;
				//isWinningOfSeed[seed-seedMin]++;
				break;
			}
			/*地图求解*/
			if(showStep > 1) ShowBoard(0);
			else if(showStep == 1)
			{
				if(fastShow == 0) ShowBoard(0);
				else//中间帧快速显示
				{
					if(showSolution == 1)//快速显示抹除不确定翻开
					{
						for(r=0; r<heightOfBoard; r++)
						{
							for(c=0; c<widthOfBoard; c++)
							{
								if(solution[r][c] == 3 && isShown[r][c] == 0)
								{
									ReShowBoard(r, c, 0, 3+showInformation);
								}
							}
						}
					}
					ShowBoardFast(3+showInformation);
				}
			}
			LookMap();
			if(showStep > 2)
			{
				printf("\n");//隔开Loading显示调试信息
				Solve(2);
			}
			else
			{
				temp = debug;
				debug = 0;//Bench时不显示多余信息
				if(showStep > 0)//控制仅显示结束帧时Loading位置相同，无帧不控制
				{
					gotoxy(0, heightOfMapShown+3+showInformation);
				}
				Solve(2);
				debug = temp;
			}
			/*if(diffResult > 2)
			{
				gotoxy(0, 3+showInformation);
				ShowBoard(0);
				ShowSolution(3+showInformation);
				gotoxy(0, heightOfMapShown+3+showInformation);
				temp = RealRemainder();
				for(r=0; r<heightOfBoard; r++)
				{
					for(c=0; c<widthOfBoard; c++)
					{
						if(solution[r][c] == 2) temp++;
					}
				}
				printf("Remainder %d diffResult %d \n", temp, diffResult);
				system("pause");
			}*/
			if(showStep > 0 && showSolution == 1)//Bench仅显示方案矩阵
			{
				ShowSolution(3+showInformation);//yOfMap=3+showInformation
			}
			//gotoxy(0, heightOfMapShown+3+showInformation);
			//printf("                     ");//覆写[Loading...]8192/8192，21空格
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
			if(showStep > 1)
			{
				gotoxy(0, heightOfMapShown+3+showInformation);
				system("pause");//显示在行首
			}
			//showCursor(visibleCursor);//Bench不可避免调整窗口大小恢复控制台光标，影响性能
		}
		if(showInformation == 1)
		{
			if(showStep <= 0)
			{
				printf("** Map:%d*%d-%d | ", heightOfBoard, widthOfBoard, numberOfMine);
				printf("seed=%d,%d,%d | ", seed, r0, c0);
				printf("Frame %d | ", countOfStep);
				t1 = time(0);
				printf("FPS %.1f \n", (float)countOfStep/(t1-t0));
			}
		}
		if(showStep != -1) ShowBoard(1);
		temp = BBBV(seed, r0, c0, 0);
		BBBVOfSeed[seed-seedMin][0] = BBBVOfSeed[seed-seedMin][1] - temp;
		countOfSolved3BV -= temp;//减未解3BV计算已解3BV
		if(showTime == 1) DrawProgressBar(100 * (seed-seedMin+1) / (seedMax-seedMin+1));//进度条
		if(showStep > 1)
		{
			gotoxy(0, heightOfMapShown+3+showInformation);//防止显示进度条后光标位置变化
			system("pause");
		}
	}
	/*输出结果*/
	clock1 = clock();
	t1 = time(0);
	//clrscr();
	gotoxy(0, 3);
	if(showInformation == 1)
	{
		printf("** Map:%d*%d-%d | ", heightOfBoard, widthOfBoard, numberOfMine);
		printf("seed=%d,%d,%d | ", seedMax, r0, c0);
		printf("Frame %d | ", countOfStep);
		printf("FPS %.1f \n", (float)countOfStep/(t1-t0));
	}
	ShowBoard(1);
	if(debug == 1 || debug == 2)
	{
		printf("胜利数：%d 地图数：%d\n", countOfWin, seedMax-seedMin+1);
		temp = seedMax-seedMin+1;
		printf("参考标准差：%f", sqrt((float)countOfWin*(temp-countOfWin)/temp));//胜利数参考标准差
		printf("(%f%%)\n", sqrt((float)countOfWin*(temp-countOfWin)/temp/temp/temp)*100);//胜率参考标准差
		printf("方案数：%d 步数：%d 步均方案数：%.2f\n", countOfSolution, countOfStep, (float)countOfSolution/countOfStep);
		printf("用时：%d 3BV：%d/%d 3BV/s：%.2f\n", t1-t0, countOfSolved3BV, countOf3BV, (float)countOfSolved3BV/(t1-t0));
		printf("毫秒用时：%d\n", clock1-clock0);
		printf("放弃链数：%d 字典溢出：%d 字典需求：%d\n", numberOfAbandonedThinkChain, countOfDictionaryOverflow, dictionaryNeeded);
		printf("未知链长度统计：");
		for(i=0; i<2*LimLength; i++)
		{
			if(i%10 == 0)
			{
				r = 0;
				for(c=i; c<=i+9 && c<2*LimLength; c++)
				{
					r += countLengthOfThinkChain[c];
				}
				if(r == 0)
				{
					i += 9;
					continue;//跳过空行
				}
				printf("\n%3d-", i);
				if(i+9 < 2*LimLength) printf("%3d: ", i+9);
				else printf("127: ");
			}
			printf("%d ", countLengthOfThinkChain[i]);
		}
		printf("\n");
		/*printf("胜利矩阵：\n");
		for(seed=seedMin; seed<=seedMax; seed++)
		{
			if(isWinningOfSeed[seed-seedMin] == 1)
			{
				//printf("%2d ", seed);
				PrintBlank(Place(seedMax)-Place(seed));
				printf("%d ", seed);
			}
			else
			{
				//printf("   ");
				PrintBlank(Place(seedMax)+1);
			}
			if((seed-seedMin+1)%10 == 0) printf("\n");
		}
		if((seedMax-seedMin+1)%10 != 0) printf("\n");
		printf("已解3BV矩阵：\n");
		for(seed=seedMin; seed<=seedMax; seed++)
		{
			if(BBBVOfSeed[seed-seedMin][0] == BBBVOfSeed[seed-seedMin][1])
			{
				PrintBlank(maxPlaceOfBBBV-Place(BBBVOfSeed[seed-seedMin][1]));
				printf("%d ", BBBVOfSeed[seed-seedMin][1]);
				PrintBlank(maxPlaceOfBBBV+1);
			}
			else
			{
				PrintBlank(maxPlaceOfBBBV-Place(BBBVOfSeed[seed-seedMin][0]));
				printf("%d/", BBBVOfSeed[seed-seedMin][0]);
				PrintBlank(maxPlaceOfBBBV-Place(BBBVOfSeed[seed-seedMin][1]));
				printf("%d ", BBBVOfSeed[seed-seedMin][1]);
			}
			if((seed-seedMin+1)%10 == 0) printf("\n");
		}
		if((seedMax-seedMin+1)%10 != 0) printf("\n");*/
		printf("胜利+已解3BV矩阵：");
		for(seed=seedMin; seed<=seedMax; seed++)
		{
			if((seed-seedMin)%100 == 0)
			{
				printf("\n");
				PrintBlank(2*Place(seedMax)+3);
				for(c=0; c<10; c++)
				{
					PrintBlank(2*maxPlaceOfBBBV);
					printf("%d ", c);
				}
			}
			if((seed-seedMin)%10 == 0)
			{
				printf("\n");
				PrintBlank(Place(seedMax)-Place(seed));
				printf("%d-", seed);
				if(seed+9 <= seedMax)
				{
					PrintBlank(Place(seedMax)-Place(seed+9));
					printf("%d", seed+9);
				}
				else
				{
					printf("%d", seedMax);
				}
				printf(": ");
			}
			if(BBBVOfSeed[seed-seedMin][0] == BBBVOfSeed[seed-seedMin][1])
			{
				PrintBlank(maxPlaceOfBBBV-Place(BBBVOfSeed[seed-seedMin][1]));
				ColorNumber(BBBVOfSeed[seed-seedMin][1], backgroundColor/16*16 + 0x02);
				PrintBlank(maxPlaceOfBBBV+2);
			}
			else
			{
				PrintBlank(maxPlaceOfBBBV-Place(BBBVOfSeed[seed-seedMin][0]));
				ColorNumber(BBBVOfSeed[seed-seedMin][0], backgroundColor/16*16 + 0x04);
				/*if(BBBVOfSeed[seed-seedMin][0] < BBBVOfSeed[seed-seedMin][1]/5)
				{
					ColorNumber(BBBVOfSeed[seed-seedMin][0], backgroundColor/16*16 + 0x04);
				}
				else if(BBBVOfSeed[seed-seedMin][0] < BBBVOfSeed[seed-seedMin][1]*4/5)
				{
					ColorNumber(BBBVOfSeed[seed-seedMin][0], backgroundColor/16*16 + 0x0c);
				}
				else
				{
					ColorNumber(BBBVOfSeed[seed-seedMin][0], backgroundColor/16*16 + 0x06);
				}*/
				printf("/");
				PrintBlank(maxPlaceOfBBBV-Place(BBBVOfSeed[seed-seedMin][1]));
				ColorNumber(BBBVOfSeed[seed-seedMin][1], backgroundColor/16*16 + 0x04);
				printf(" ");
			}
		}
		printf("\n");
	}
	else
	{
		if(seedMax-seedMin+1 == 100) printf("胜率：%d%% ", countOfWin*100/(seedMax-seedMin+1));
		else printf("胜率：%d%%(%d/%d) ", countOfWin*100/(seedMax-seedMin+1), countOfWin, seedMax-seedMin+1);
		if(showTime == 1) printf("用时：%d ", t1-t0);
		if(show3BV == 1) printf("3BV：%d/%d 3BV/s：%.2f", countOfSolved3BV, countOf3BV, (float)countOfSolved3BV/(t1-t0));
		printf("\n");
	}
	//if(debug == 2) printf("CPU Clock: %d\n", clock1-clock0);
}

int BBBV(int seed, int r0, int c0, int mode)//计算地图3BV
{
	int bbbv = 0, r, c, z, temp;
	int* zeroChainCache;//缓存0链是否被打开
	static int numberOfZeroChain = 0;
	if(mode != 2) ShownModeBak(1);
	if(mode == 1)//1计算全部3BV，0计算未解3BV，3计算全部3BV并缓存，2根据缓存计算未解3BV
	{
		SummonBoard(seed, r0, c0, summonCheckMode, 1);
	}
	else if(mode == 0)//防止标记阻碍0链打开
	{
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				if(isShown[r][c] == 2)
				{
					isShown[r][c] = 0;
				}
			}
		}
	}
	else if(mode == 3)
	{
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				isShown[r][c] = 0;
			}
		}
	}
	if(mode == 1 || mode == 0)
	{
		/*计算0链数量*/
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
	}
	else if(mode == 2)//根据0块缓存计算未解3BV
	{
		zeroChainCache =(int*) calloc(numberOfZeroChain, sizeof(int));
		//快速计算剩余0链和数字
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				if(board[r][c] == 0)//点到0:未解3BV+1，区分0链
				{
					if(zeroChainCache[zeroCache[r][c]-2] == 0)
					{
						zeroChainCache[zeroCache[r][c]-2] = 1;
						if(isShown[r][c] != 1) bbbv++;
					}
				}
				//点到0块缓存数字:不变
				else if(board[r][c] != 9 && isShown[r][c] != 1 && zeroCache[r][c] == 1)
				{
					bbbv++;//点到其他数字:已解3BV+1
				}
			}
		}
		free(zeroChainCache);
	}
	else if(mode == 3)
	{
		for(r=0; r<heightOfBoard; r++)
		{
			for(c=0; c<widthOfBoard; c++)
			{
				zeroCache[r][c] = 1;//一般数字1，0链附近数字0，0链序号2开始
			}
		}
		z = 0;
		while(1)
		{
			temp = 0;
			//打开一个0链
			for(r=0; r<heightOfBoard; r++)
			{
				for(c=0; c<widthOfBoard; c++)
				{
					if(board[r][c] == 0 && isShown[r][c] != 1)
					{
						isShown[r][c] = 1;
						OpenZeroChain(r, c);
						bbbv++;
						temp = 1;
					}
					if(temp == 1) break;
				}
				if(temp == 1) break;
			}
			if(temp == 1)
			{
				for(r=0; r<heightOfBoard; r++)
				{
					for(c=0; c<widthOfBoard; c++)
					{
						if(isShown[r][c] == 1 && zeroCache[r][c] == 1)//翻开方块计入0块缓存
						{
							if(board[r][c] == 0) zeroCache[r][c] = z+2;//bbbv+1
							else zeroCache[r][c] = 0;
						}
					}
				}
				z++;
			}
			else break;
		}
		//生成0链缓存
		numberOfZeroChain = bbbv;//当前3BV即为0链数量
		/*if(debug == 2)
		{
			printf("[Debug]已生成0块缓存：\n");
			for(r=0; r<heightOfBoard; r++)
			{
				printf("  ");
				for(c=0; c<widthOfBoard; c++)
				{
					printf("%2d", zeroCache[r][c]);
				}
				printf("\n");
			}
			system("pause");
		}*/
	}
	if(mode != 2)
	{
		/*计算未显示数字数量*/
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
	}
	return bbbv;
}

double SS(int h, int w, int n, double time, int solved3BV, int total3BV)//计算记录标准分数SS
{
	//计算相对难度值RDV = A(e^(n/h/w)-1)*(h^B)*(w^C)
	const double A = 8.14593564;
	const double B = 0.528462037;
	const double C = 1.172055106;
	double densityWeight = expm1(1.0*n/(h*w));
	if(h > w)//约定长边为w
	{
		int temp = h;
		h = w;
		w = temp;
	}
	double rdv = A * densityWeight * pow(h, B) * pow(w, C);
	double completionWeight = pow(1.0*solved3BV/total3BV, 0.7);
	double qg = pow(time, 1.7) / solved3BV;
	return rdv * completionWeight / qg;
}

int SearchSeed(int seedMin, int r0, int c0, int difficulty)//可解和筛选种子搜索模块
{
	int seed = seedMin;
	int isOpenMine, bbbv, r, c;
	if(summonCheckMode == 4)//生成可解速通地图
	{
		isOpenMine = 1;
		while(isOpenMine == 1)
		{
			bbbv = BBBV(seed, r0, c0, 1);//生成地图并计算3BV
			if(bbbv >= chosen3BVMin[difficulty] && bbbv <= chosen3BVMax[difficulty])//筛选地图3BV
			{
				for(r=0; r<heightOfBoard; r++)//筛选地图指定包含数字
				{
					for(c=0; c<widthOfBoard; c++)
					{
						if(board[r][c] == chosenNumber) isOpenMine = 0;
					}
				}
				if(isOpenMine == 0 && chosenSolvable == 1)//筛选地图可解性
				{
					isOpenMine = 1-IsSolvableMap(seed, r0, c0);
				}
			}
			if(isOpenMine == 1) seed++;
		}
	}
	else
	{
		while(IsSolvableMap(seed, r0, c0) == 0) seed++;//可解地图保持种子不变
	}
	return seed;
}

void MapSearch(int seedMin, int seedMax, int r0, int c0)//地图搜索模块
{
	int seed, bbbv, temp;
	int t0, t1, t2 = 0;
	int choice = 1;
	int r, c;
	for(r=0; r<heightOfBoard; r++)
	{
		for(c=0; c<widthOfBoard; c++)
		{
			isShown[r][c] = 0;//清零显示方式矩阵
		}
	}
	if(debug == 2)
	{
		printf("*******************************\n");//宽31
		printf("(1)普通搜索\n");
		printf("(2)3BV筛选搜索\n");
		printf("(3)3BV汇总统计\n");
		printf("(4)可解种子搜索效率\n");
		printf("(5)指定包含数字\n");
		printf("(6)地图生成效率\n");
		printf("(7)退出\n");
		printf("*******************************\n");
		printf(">");
		scanf("%d", &choice);
	}
	if(choice == 1)
	{
		for(seed=seedMin; seed<=seedMax; seed++)
		{
			//SummonBoard(seed, r0, c0, summonCheckMode, 1);
			bbbv = BBBV(seed, r0, c0, 1);
			printf("Map:%d*%d-%d\n", heightOfBoard, widthOfBoard, numberOfMine);
			printf("seed=%d,%d,%d\n", seed, r0, c0);
			printf("3BV=%d\n", bbbv);
			temp = debug;
			debug = 0;//不显示重复信息
			printf("IsSolvableMap=%d\n", IsSolvableMap(seed, r0, c0));
			debug = temp;
			ShowBoard(1);
			//system("pause");
		}
	}
	else if(choice == 2)//3BV筛选搜索
	{
		int bbbvMin = 0, bbbvMax = 2;//3BV为2只需1次点击
		printf("[3bvMin] [3bvMax]>");
		scanf("%d%d", &bbbvMin, &bbbvMax);
		printf("是否遍历起始点\n");
		printf("[1/0]>");
		scanf("%d", &temp);
		for(seed=seedMin; seed<=seedMax; seed++)
		{
			if(temp == 1)
			{
				for(r0=0; r0<heightOfBoard; r0++)
				{
					for(c0=0; c0<widthOfBoard; c0++)//遍历所有起始点
					{
						bbbv = BBBV(seed, r0, c0, 1);
						if(bbbv >= bbbvMin && bbbv <= bbbvMax)
						{
							printf("Map:%d*%d-%d\n", heightOfBoard, widthOfBoard, numberOfMine);
							printf("seed=%d,%d,%d\n", seed, r0, c0);
							printf("3BV=%d\n", bbbv);
							temp = debug;
							debug = 0;//不显示重复信息
							printf("IsSolvableMap=%d\n", IsSolvableMap(seed, r0, c0));
							debug = temp;
							ShowBoard(1);
							//system("pause");
						}
					}
				}
			}
			else
			{
				bbbv = BBBV(seed, r0, c0, 1);
				if(bbbv >= bbbvMin && bbbv <= bbbvMax)
				{
					printf("Map:%d*%d-%d\n", heightOfBoard, widthOfBoard, numberOfMine);
					printf("seed=%d,%d,%d\n", seed, r0, c0);
					printf("3BV=%d\n", bbbv);
					temp = debug;
					debug = 0;//不显示重复信息
					printf("IsSolvableMap=%d\n", IsSolvableMap(seed, r0, c0));
					debug = temp;
					ShowBoard(1);
					//system("pause");
				}
			}
		}
	}
	else if(choice == 3)//3BV汇总统计
	{
		int* bbbvCount =(int*) calloc(heightOfBoard*widthOfBoard, sizeof(int));
		int i, j;
		t0 = time(0);
		for(seed=seedMin; seed<=seedMax; seed++)
		{
			bbbv = BBBV(seed, r0, c0, 1);
			bbbvCount[bbbv]++;
			t1 = time(0);
			if(t2 != t1)
			{
				printf("\rseed=%d 用时：%d", seed, t1-t0);
				t2 = t1;
			}
		}
		printf("\rseed=%d 用时：%d 平均用时：%.2f ms\n", seedMax, t1-t0, (float)(t1-t0)/(seedMax-seedMin+1)*1000);
		/*for(i=1; i<heightOfBoard*widthOfBoard; i++)
		{
			bbbvCount[i] += bbbvCount[i-1];//总数
		}*/
		printf("Map:%d*%d-%d", heightOfBoard, widthOfBoard, numberOfMine);
		printf("=%d", heightOfBoard*widthOfBoard-numberOfMine);
		for(i=0; i<heightOfBoard*widthOfBoard; i++)
		{
			if(i%10 == 0)
			{
				temp = 0;
				for(j=i; j<=i+9 && j<heightOfBoard*widthOfBoard; j++)
				{
					temp += bbbvCount[j];
				}
				if(temp == 0)
				{
					i += 9;
					continue;//跳过空行
				}
				printf("\n%d-", i);
				if(i+9 < heightOfBoard*widthOfBoard) printf("%d: ", i+9);
				else printf("%d: ", heightOfBoard*widthOfBoard);
			}
			printf("%d ", bbbvCount[i]);
		}
		printf("\n");
	}
	else if(choice == 4)//可解种子搜索效率
	{
		int count = 0;
		t0 = time(0);
		showCursor(visibleCursor);
		for(seed=seedMin; seed<=seedMax; seed++)
		{
			temp = debug;
			debug = 0;//不显示地图信息
			if(IsSolvableMap(seed, r0, c0)) count++;
			debug = temp;
			t1 = time(0);
			printf("\rseed=%d 可解数：%d 用时：%d", seed, count, t1-t0);
		}
		printf(" 平均用时：%.2f\n", (float)(t1-t0)/count);
		showCursor(1);
	}
	else if(choice == 5)//指定包含数字
	{
		int n, isSolvable;
		int count = 0, countSolvable = 0;
		printf("[n]>");
		scanf("%d", &n);
		for(seed=seedMin; seed<=seedMax; seed++)
		{
			SummonBoard(seed, r0, c0, summonCheckMode, 1);
			temp = 0;
			for(r=0; r<heightOfBoard; r++)
			{
				for(c=0; c<widthOfBoard; c++)
				{
					if(board[r][c] == n) temp = 1;
				}
			}
			if(temp == 1)
			{
				bbbv = BBBV(seed, r0, c0, 1);
				printf("Map:%d*%d-%d\n", heightOfBoard, widthOfBoard, numberOfMine);
				printf("seed=%d,%d,%d\n", seed, r0, c0);
				printf("3BV=%d\n", bbbv);
				temp = debug;
				debug = 0;//不显示重复信息
				isSolvable = IsSolvableMap(seed, r0, c0);
				printf("IsSolvableMap=%d\n", isSolvable);
				debug = temp;
				ShowBoard(1);
				//system("pause");
				count++;
				countSolvable += isSolvable;
				/*if(isSolvable == 1)
				{
					printf("Map:%d*%d-%d\n", heightOfBoard, widthOfBoard, numberOfMine);
					printf("seed=%d,%d,%d\n", seed, r0, c0);
					printf("3BV=%d\n", bbbv);
					printf("IsSolvableMap=%d\n", isSolvable);
					ShowBoard(1);
				}*/
			}
		}
		printf("地图数：%d 可解数：%d\n", count, countSolvable);
	}
	else if(choice == 6)//地图生成效率
	{
		t0 = time(0);
		showCursor(visibleCursor);
		for(seed=seedMin; seed<=seedMax; seed++)
		{
			SummonBoard(seed, r0, c0, summonCheckMode, 1);
			t1 = time(0);
			if(t2 != t1)
			{
				printf("\rseed=%d 用时：%d", seed, t1-t0);
				t2 = t1;
			}
		}
		printf("\rseed=%d 用时：%d", seedMax, t1-t0);
		printf(" 平均用时：%.2f ms\n", (float)(t1-t0)/(seedMax-seedMin+1)*1000);
		showCursor(1);
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

void ReallocMemory(int height, int width, int dic, int length)//全局矩阵内存重分配
{
	static int lastHeight = 0, lastWidth = 0, lastDic = 0, lastLength = 0;
	if(height != lastHeight || width != lastWidth)//重分配H*W矩阵
	{
		if(lastHeight != 0 && lastWidth != 0)//释放内存
		{
			isMine =(int**) MatrixMemory((void**)isMine, lastHeight, lastWidth, sizeof(int), 0);
			numberOfMineAround =(int**) MatrixMemory((void**)numberOfMineAround, lastHeight, lastWidth, sizeof(int), 0);
			board =(int**) MatrixMemory((void**)board, lastHeight, lastWidth, sizeof(int), 0);
			isShown =(int**) MatrixMemory((void**)isShown, lastHeight, lastWidth, sizeof(int), 0);
			zeroChain =(int**) MatrixMemory((void**)zeroChain, lastHeight, lastWidth, sizeof(int), 0);
			questionMark =(int**) MatrixMemory((void**)questionMark, lastHeight, lastWidth, sizeof(int), 0);
			isShownBak =(int**) MatrixMemory((void**)isShownBak, lastHeight, lastWidth, sizeof(int), 0);
			isShownBuf =(int**) MatrixMemory((void**)isShownBuf, lastHeight, lastWidth, sizeof(int), 0);
			zeroCache =(int**) MatrixMemory((void**)zeroCache, lastHeight, lastWidth, sizeof(int), 0);
			mapShown =(char**) MatrixMemory((void**)mapShown, lastHeight+2, 2*lastWidth+5, sizeof(char), 0);
			map =(char**) MatrixMemory((void**)map, lastHeight, lastWidth, sizeof(char), 0);
			numberShown =(int**) MatrixMemory((void**)numberShown, lastHeight, lastWidth, sizeof(int), 0);
			solution =(int**) MatrixMemory((void**)solution, lastHeight, lastWidth, sizeof(int), 0);
			isThought =(int**) MatrixMemory((void**)isThought, lastHeight, lastWidth, sizeof(int), 0);
			numberTeam =(int**) MatrixMemory((void**)numberTeam, lastHeight, lastWidth, sizeof(int), 0);
			thinkChain =(int**) MatrixMemory((void**)thinkChain, lastHeight, lastWidth, sizeof(int), 0);
			numberCheckBase =(int**) MatrixMemory((void**)numberCheckBase, lastHeight, lastWidth, sizeof(int), 0);
			numberCheck =(int**) MatrixMemory((void**)numberCheck, lastHeight, lastWidth, sizeof(int), 0);
			mineCheck =(int**) MatrixMemory((void**)mineCheck, lastHeight, lastWidth, sizeof(int), 0);
			isMineRate =(float**) MatrixMemory((void**)isMineRate, lastHeight, lastWidth, sizeof(float), 0);
			//isMineRateA =(int**) MatrixMemory((void**)isMineRateA, lastHeight, lastWidth, sizeof(int), 0);
			//isMineRateB =(int**) MatrixMemory((void**)isMineRateB, lastHeight, lastWidth, sizeof(int), 0);
			isMine = 0;//清除指针残留值
			numberOfMineAround = 0;
			board = 0;
			isShown = 0;
			zeroChain = 0;
			questionMark = 0;
			isShownBak = 0;
			isShownBuf = 0;
			zeroCache = 0;
			mapShown = 0;
			map = 0;
			numberShown = 0;
			solution = 0;
			isThought = 0;
			numberTeam = 0;
			thinkChain = 0;
			numberCheckBase = 0;
			numberCheck = 0;
			mineCheck = 0;
			isMineRate = 0;
			//isMineRateA = 0;
			//isMineRateB = 0;
		}
		if(height != 0 && width != 0)//申请内存
		{
			isMine =(int**) MatrixMemory((void**)isMine, height, width, sizeof(int), 1);
			numberOfMineAround =(int**) MatrixMemory((void**)numberOfMineAround, height, width, sizeof(int), 1);
			board =(int**) MatrixMemory((void**)board, height, width, sizeof(int), 1);
			isShown =(int**) MatrixMemory((void**)isShown, height, width, sizeof(int), 1);
			zeroChain =(int**) MatrixMemory((void**)zeroChain, height, width, sizeof(int), 1);
			questionMark =(int**) MatrixMemory((void**)questionMark, height, width, sizeof(int), 1);
			isShownBak =(int**) MatrixMemory((void**)isShownBak, height, width, sizeof(int), 1);
			isShownBuf =(int**) MatrixMemory((void**)isShownBuf, height, width, sizeof(int), 1);
			zeroCache =(int**) MatrixMemory((void**)zeroCache, height, width, sizeof(int), 1);
			mapShown =(char**) MatrixMemory((void**)mapShown, height+2, 2*width+5, sizeof(char), 1);
			map =(char**) MatrixMemory((void**)map, height, width, sizeof(char), 1);
			numberShown =(int**) MatrixMemory((void**)numberShown, height, width, sizeof(int), 1);
			solution =(int**) MatrixMemory((void**)solution, height, width, sizeof(int), 1);
			isThought =(int**) MatrixMemory((void**)isThought, height, width, sizeof(int), 1);
			numberTeam =(int**) MatrixMemory((void**)numberTeam, height, width, sizeof(int), 1);
			thinkChain =(int**) MatrixMemory((void**)thinkChain, height, width, sizeof(int), 1);
			numberCheckBase =(int**) MatrixMemory((void**)numberCheckBase, height, width, sizeof(int), 1);
			numberCheck =(int**) MatrixMemory((void**)numberCheck, height, width, sizeof(int), 1);
			mineCheck =(int**) MatrixMemory((void**)mineCheck, height, width, sizeof(int), 1);
			isMineRate =(float**) MatrixMemory((void**)isMineRate, height, width, sizeof(float), 1);
			//isMineRateA =(int**) MatrixMemory((void**)isMineRateA, height, width, sizeof(int), 1);
			//isMineRateB =(int**) MatrixMemory((void**)isMineRateB, height, width, sizeof(int), 1);
		}
		lastHeight = height;
		lastWidth = width;
	}
	if(dic != lastDic || length != lastLength)//重分配D*L矩阵
	{
		if(lastDic != 0 && lastLength != 0)//释放内存
		{
			free(possibility);
			free(sumDictionary);
			dictionary =(char**) MatrixMemory((void**)dictionary, lastDic, lastLength, sizeof(char), 0);
			possibility = 0;//清除指针残留值
			sumDictionary = 0;
			dictionary = 0;
		}
		if(dic != 0 && length != 0)//申请内存
		{
			possibility =(int*) calloc(length, sizeof(int));
			sumDictionary =(int*) calloc(length, sizeof(int));
			dictionary =(char**) MatrixMemory((void**)dictionary, dic, length, sizeof(char), 1);
		}
		lastDic = dic;
		lastLength = length;
	}
}

struct Records RecordsEditer(struct Records records)//记录编辑器模块
{
	int choice, i, showAll = 0;
	struct Record newRecord;
	records.maxOpenNumber = MaxOpenNumber(records);//计算最大翻开数字
	while(1)
	{
		clrscr();
		PrintRecords(records, showAll);
		//printf("[Records Editer]\n");
		if(showAll == 0 && records.numberOfRecords > 1024) printf("(0)查看全部记录\n");
		printf("(1)删除一条记录\n");
		printf("(2)删除无效记录\n");
		printf("(3)仅保留纪录\n");
		printf("(4)删除全部记录\n");
		printf("(5)保存并重新读取\n");
		printf("(6)保存并退出\n");
		printf("(7)重新读取\n");
		printf("(8)直接退出\n");
		printf("(9)插入一条记录\n");
		printf("*******************************\n");//宽31
		choice = -1;
		printf(">");
		showCursor(1);
		scanf("%d", &choice);
		if(choice == 0)
		{
			showAll = 1;
		}
		else if(choice == 1)//删除一条记录
		{
			printf("[序号]>");
			scanf("%d", &i);
			if(IsEffectiveRecord(records.record[i]))//是有效记录
			{
				if(records.record[i].sTime == records.minimumTime[records.record[i].difficulty]
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
		else if(choice == 2)//删除无效记录
		{
			records = DeleteRecords(records, 0);
		}
		else if(choice == 3)//仅保留纪录
		{
			records = DeleteRecords(records, 1);
		}
		else if(choice == 4)//删除全部记录
		{
			records = DeleteRecords(records, 2);
		}
		else if(choice == 5)//保存并重新读取
		{
			WriteRecords(records);
			records = ReadRecords();
			records.maxOpenNumber = MaxOpenNumber(records);//计算最大翻开数字
		}
		else if(choice == 6)//保存并退出
		{
			WriteRecords(records);
			break;
		}
		else if(choice == 7)//重新读取
		{
			records = ReadRecords();
			records.maxOpenNumber = MaxOpenNumber(records);//计算最大翻开数字
		}
		else if(choice == 8)//退出
		{
			break;
		}
		else if(choice == 9)//插入一条记录
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
			scanf("%d", &(newRecord.summonMode));
			if(newRecord.summonMode < 0) newRecord.summonMode = 0;
			if(newRecord.summonMode > 3) newRecord.summonMode = 3;
			newRecord.iterateMode = 1;
			printf("地图种子(例如%d)，起始点位置\n", (int)time(0));
			printf("[seed] [r0] [c0]>");
			scanf("%d%d%d", &(newRecord.seed), &(newRecord.r0), &(newRecord.c0));
			if(newRecord.r0 < 0) newRecord.r0 = 0;
			if(newRecord.r0 >= newRecord.heightOfBoard) newRecord.r0 = newRecord.heightOfBoard-1;
			if(newRecord.c0 < 0) newRecord.c0 = 0;
			if(newRecord.c0 >= newRecord.widthOfBoard) newRecord.c0 = newRecord.widthOfBoard-1;
			printf("[用时]>");
			scanf("%d", &(newRecord.sTime));
			if(newRecord.sTime < 0) newRecord.sTime = 0;
			newRecord.msTime = newRecord.sTime*1000;
			//计算地图3BV
			numberOfMine = newRecord.numberOfMine;
			heightOfBoard = newRecord.heightOfBoard;
			widthOfBoard = newRecord.widthOfBoard;
			summonCheckMode = newRecord.summonMode;
			newRecord.total3BV = BBBV(newRecord.seed, newRecord.r0, newRecord.c0, 1);
			printf("当前地图3BV：%d\n", newRecord.total3BV);
			printf("[已解3BV]>");
			scanf("%d", &(newRecord.solved3BV));
			if(newRecord.solved3BV < 0) newRecord.solved3BV = 0;
			if(newRecord.solved3BV > newRecord.total3BV) newRecord.solved3BV = newRecord.total3BV;
			if(debug == 1 || debug == 2)
			{
				printf("是否被实时求解指令帮助\n");
				printf("[1/0]>");
				scanf("%d", &(newRecord.isHelped));
				if(newRecord.isHelped != 0) newRecord.isHelped = 1;
			}
			else
			{
				newRecord.isHelped = 1;//插入无效记录
			}
			newRecord.speed =(float) newRecord.solved3BV / newRecord.sTime;
			newRecord.score = SS(newRecord.heightOfBoard, newRecord.widthOfBoard, newRecord.numberOfMine,
								 newRecord.sTime, newRecord.solved3BV, newRecord.total3BV);
			records = AddRecord(records, newRecord);
		}
		else
		{
			Error();
		}
	}
	return records;
}

int CustomMapsEditer()//地图绘制器模块
{
	int choice, i, r, c;
	struct Maps maps = ReadMaps();
	struct Map newMap;
	struct Record temp;
	//备份难度并设置为最大地图大小
	temp.numberOfMine = numberOfMine;
	temp.heightOfBoard = heightOfBoard;
	temp.widthOfBoard = widthOfBoard;
	heightOfBoard = 0;
	widthOfBoard = 0;
	for(i=0; i<maps.numberOfMaps; i++)
	{
		if(maps.map[i].height > heightOfBoard) heightOfBoard = maps.map[i].height;
		if(maps.map[i].width > widthOfBoard) widthOfBoard = maps.map[i].width;
	}
	ReallocMemory(heightOfBoard, widthOfBoard, dictionaryCapacity, lengthOfThinkMineCheck);
	while(1)
	{
		clrscr();
		PrintMaps(maps);
		//printf("[Map Editer]\n");
		printf("(1)新建地图 (2)编辑地图\n");
		printf("(3)删除地图 (4)重命名地图\n");
		printf("(5)删除所有 (6)复制地图\n");
		printf("(7)重新读取 (8)进入游戏\n");
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
			//更新最大地图大小
			if(newMap.height > heightOfBoard || newMap.width > widthOfBoard)
			{
				if(newMap.height > heightOfBoard) heightOfBoard = newMap.height;
				if(newMap.width > widthOfBoard) widthOfBoard = newMap.width;
				ReallocMemory(heightOfBoard, widthOfBoard, dictionaryCapacity, lengthOfThinkMineCheck);
			}
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
			printf("[序号]>");
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
			return 1;//返回并进入游戏
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
	//恢复难度
	numberOfMine = temp.numberOfMine;
	heightOfBoard = temp.heightOfBoard;
	widthOfBoard = temp.widthOfBoard;
	ReallocMemory(heightOfBoard, widthOfBoard, dictionaryCapacity, lengthOfThinkMineCheck);
	return 0;
}

void Error()//错误
{
	int c;
	//system("color 1f");
	setbgcolor(0x1f);
	printf(":(\n"
		"你的设备遇到问题，需要重启。\n"
		"我们只收集某些错误信息，然后你可以重新启动。\n\n");
	for(c=0; c<=10; c++)
	{
		printf("\r%3d%% 完成", c*10);
		api_sleep(200);
	}
	api_sleep(4*200);
	printf("\n");
	//getchar();
	fflush(stdin);//清空输入缓冲区，多字符仅蓝屏一次(doge)
	//system("color 07");
	setbgcolor(backgroundColor);
	clrscr();
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
MineSweeper Run 2.2
——新增 继续上一次游戏
——新增 保存上一次设置
——新增 地图求解也在原地图显示全部方案
——修复 游戏Debug关闭时相关字符不消失
——修复 低难度游戏Bench锁定校验时字符不消失
MineSweeper Run 2.3
——新增 调试选项可启用鼠标点击
——新增 游戏时可按M/K在鼠标和键盘操作间切换
——新增 浅色模式不再是调试选项
——新增 调试选项可设置任意背景颜色
——新增 调试选项可设置隐藏光标
//——优化 鼠标点击时!(Shift+1)可直接按1
——优化 鼠标点击时wasd可使用方向键
MineSweeper Run 2.4
——新增 鼠标点击不再是调试选项
——优化 现在按K无缝切换键盘操作
——优化 鼠标点击时支持!指令(Shift+1)
——修复 上一次游戏地图与设置不同时显示错误
——修复 游戏Debug时检测鼠标位置偏移
——修复 启用过鼠标操作后切换到键盘操作无法全选复制地图
——修复 鼠标右击胜利时出现右键菜单
——修复 启用过鼠标操作后地图求解右击不能粘贴
——修复 鼠标模式Debug不能更改种子
MineSweeper Run 2.5
——新增 命令行窗口命名为MineSweeper Console
——优化 定义功能模块函数以缩减main函数
——修复 鼠标点击偶发的双击
——修复 鼠标模式无操作时用时显示停滞
MineSweeper Run 2.6
——新增 现在默认鼠标点击操作模式
——新增 全新界面
——新增 现在主页，设置菜单，设置难度支持鼠标点击
——优化 删除部分不必要的调试
——优化 现在打开程序直接继续上一次游戏
——修复 继续上一次游戏会丢失标记
MineSweeper Run 3.0
——新增 现在设置内除调试选项均支持鼠标点击
——新增 游戏时点击新游戏即可停止游戏，再次点击可立即重开
——新增 调试选项可启用鼠标点击屏蔽空格
——优化 默认刷新周期从100ms提高到50ms
——优化 移除大写WASD移动5格功能
——修复 鼠标翻开偶发的双击
MineSweeper Run 3.1
——新增 全新难度选择界面
——新增 调试选项可启用游戏结束时显示3BV和3BV/s
——优化 自定义难度时地图生成校验会自动更改
——优化 地图高度大于40时游戏不显示控制栏
——修复 自定义背景时不显示地图
MineSweeper Run 3.2
——新增 3BV和3BV/s现在可以游戏实时显示
——修复 浅色模式678淡黄色底纹显示过多
——修复 难度设置键盘操作无效
——修复 鼠标模式无操作时光标不停留在原来位置
MineSweeper Run 3.3
——新增 Bench进度条，3BV
——新增 显示3BV不再是调试选项
——优化 @#rc输入坐标体验（'_'光标，阴影显示正在输入坐标，退格键一键删除整个坐标）
——修复 Debug时Bench的Loading覆盖位置错误
——修复 显示用时时@#rc显示位置错误
MineSweeper Run 3.4
——新增 鼠标点击模式支持左右键功能切换，用于触屏标记
——新增 历史记录
——优化 游戏重新支持一定的调试
——优化 设置为顶级难度时自动隐藏3BV
——修复 翻开标记后剩余雷数显示错误
  # MineSweeper Records Editer
  Ltabsyy实现对历史记录文件的编辑
MineSweeper Records Editer 1.1
——新增 仅保留纪录
MineSweeper Run 3.5
——新增 Bench调试时显示已解3BV矩阵
——新增 RecordsEditer模块，可在主页按7使用
——优化 Debug的地图搜索功能独立为MapSearch模块，可在主页按6使用
——优化 游戏剩余雷数，用时，3BV信息在一行显示，设置为顶级难度时不再自动隐藏3BV
——优化 重排版部分if内条件以减少代码行数
——优化 现在切键盘操作处无需按任意键
——修复 进入调试选项有时会立即退出
MineSweeper Run 3.6
——新增 MapSearch模块显示地图是否可解
——新增 可速解地图生成
——新增 调试选项可启用动态内存分配
——优化 鼠标右键长按拖动时可以正常连续标记
——优化 现在地图生成时显示方式矩阵必然清零
——优化 现在非鼠标点击模式可全选复制第一次全未翻开的地图
——优化 第一次翻开时也可按键盘选择控制栏3-5，5为返回主页
——优化 减少鼠标操作栏的判断次数
——修复 wasd23模式第一次翻开无法使用
——修复 第一次翻开或未完成地图时，选择控制栏其他功能会增加历史记录
MineSweeper Run 3.7
——优化 统一代码书写风格
——优化 选择可速解地图时提示易生成重复地图
——修复 可解地图生成后不翻开起始点
——修复 无历史记录时游戏结束闪退
——修复 动态内存分配时地图求解输入后闪退
MineSweeper Run 3.8
——新增 RecordsEditer模块可显示玩家等级称号
——优化 现在RecordsEditer纪录显示在记录后
——优化 程序初始化效率
——优化 地图求解输入的稳定性
——修复 Bench调试的已解3BV矩阵不能对齐
——修复 自定义背景时不在原地图显示方案矩阵
——修复 修改难度后不保存有时不能在原地图正确显示方案
——修复 启动后不修改难度可能不显示打破纪录
MineSweeper Run 3.9
——新增 调试选项可启用淡黄色高亮光标
——优化 RecordsEditer界面
——优化 @#rc输入坐标体验（输入过程坐标对应位置显示控制台光标，支持wasd改变坐标）
——修复 鼠标从(10,10)移动到(9,9)时坐标可能显示错误
——修复 被实时求解指令帮助过后重启游戏可打破纪录
——修复 游戏调试不显示控制栏和3BV
MineSweeper Run 4.0
——新增 RecordsEditer可显示所有记录总时间，总翻开数
——优化 游戏时按下退出后显示暂停
——优化 对设置的缓存文件条目重排序
——优化 标记过多时只自动取消上一个标记
——修复 @#rc第一次翻开时光标位置错误
——修复 键盘模式重启后不能复制地图
MineSweeper Run 4.1
——新增 RecordsEditer可显示有效记录最大翻开数字
——新增 BenchDebug2功能（可自定义种子范围，不显示中间帧，每帧暂停）
——新增 难度选择界面显示窗口大小，字体大小，最大地图大小，当前密度
——优化 BenchDebug2体验（可左键暂停右键继续，Bench结束暂停）
——优化 游戏后记录的总时间和总翻开数自动更新
——优化 简化难度与有效记录判断代码
——优化 显示记录时显示是否有效
——优化 消除鼠标操作栏闪烁
——优化 RecordsEditer界面
——修复 Bench不显示胜利局面
MineSweeper Run 4.2
——新增 Bench调试显示帧序号，Debug2可显示Solve调试信息
——新增 主页按8可进入自定种子游戏
——优化 现在最大翻开数字不在启动时计算，提升启动速度
——优化 读取历史记录时纪录计算效率
——修复 重新读取记录后设置难度或进入游戏闪退
  # MineSweeper Map Painter
  Ltabsyy实现对自制地图文件的编辑
MineSweeper Run 4.3
——新增 自制地图的编辑，可在主页按9使用
——新增 自制地图的游戏
——修复 暂停后继续的地图重置和上一次地图时间错误
MineSweeper Map Painter 1.1
——优化 功能与函数跟随Run 4.3升级
MineSweeper Run 4.4
——新增 调试选项可启用淡黄色高亮行列
——优化 不再使用bool
——优化 #数字一键标记周围升级为快速标记（可一键取消所有标记，仅%+#=数字时标记）
——优化 确保自制地图游戏为无效记录
——优化 淡黄色高亮光标抗闪烁
——修复 Debug2重新读取记录时闪烁
——修复 ><光标覆盖行坐标轴十位，对角移动易出现拖影
——修复 快速标记不改变剩余雷数
——修复 重新读取记录后最大翻开数字为0
MineSweeper Run 4.5
——新增 快速标记，设置光标不再是调试选项
——优化 现在默认隐藏控制台光标，显示淡黄色高亮光标
——修复 淡黄色高亮光标可能引起闪退
——修复 玩家等级4级计算错误
MineSweeper Run 4.6
——新增 游戏结束时错误标记显示为数字和#
——新增 更详细的地图求解器提示信息
——优化 现在Bench总是显示一行置顶信息，在原地图显示方案矩阵
——优化 Bench范围不再被字典大小限制
——优化 减少Bench置顶信息的刷新次数
——优化 现在控制台光标在游戏开始时检查隐藏，防止调整窗口大小导致光标显示
——优化 现在深色模式淡黄色高亮光标的空地以_显示，678以红色加淡黄色底纹显示
——优化 地图前台显示方式的判断效率
——优化 现在游戏时点击地图求解按钮与按!执行实时求解指令等效
——优化 现在游戏时点击Bench按钮与按Tab执行实时求解指令等效
——优化 蓝屏体验，多字符仅蓝屏一次
——修复 Bench帧率为无穷大时闪退
——修复 Bench不显示中间帧时列坐标轴闪烁
——修复 RecordsEditer和自制地图界面输入字符后不断闪烁
MineSweeper Run 4.7
——新增 调试选项可设置Bench
——优化 通过连续操作减少逻辑推理的次数
——优化 逻辑推理和枚举的准备效率
——优化 修改可速解地图标准
——优化 屏蔽打开地图前的多次点击
——优化 第一次翻开也支持自选光标
——优化 @#rc输入坐标体验（支持自选光标，仅输入行坐标时高亮整行）
——优化 ><光标抗闪烁，且不再会出现拖影
——修复 自定种子游戏不刷新显示方式矩阵
——修复 @#rc输入坐标时用时显示停滞
MineSweeper Run 4.8
——新增 调试选项可自定义><光标字符
——优化 通过提前校验加速枚举判断
——优化 通过校验基底加速枚举判断
——优化 错误标记空地现在显示为_#
——优化 @#rc输入坐标体验（仅输入行坐标时可用ws改变坐标，中途按M立即切为鼠标）
——优化 鼠标点击，wasd23可按C直切@#rc
——优化 地图求解现在可识别?和_字符
——优化 微调部分设置界面
——修复 Bench鼠标双击
——修复 Bench仅显示结束帧时Loading位置错误
——修复 @#rc进入新游戏闪退
MineSweeper Run 4.9
——新增 地图求解器简单模式（有翻开解立即停止）
——新增 自定义><光标字符不再是调试选项
——新增 调试选项可启用鼠标点击空地切换左右键
——新增 鼠标切换左右键处悬浮高亮
——优化 地图求解器整体效率（游戏模式有解时不再分析雷率，游戏模式仅有标记解时仍枚举，交界线仅标记一次）
——优化 默认枚举限制提高到21
——优化 枚举进度条由地图求解器自行覆写
——修复 雷率可能为-0或负数
——修复 先鼠标点击设置后按键盘进入调试选项时显示错误
——修复 设置光标处键盘按键为控制栏
MineSweeper Run 4.10
——新增 地图求解键盘输入后及调试信息可将地图彩色显示
——优化 默认枚举限制回退到19
——优化 地图求解以@?和:(区分不确定翻开
——优化 地图求解器收尾效率
——优化 更快更精细的分割未知链
——修复 低雷率点可能被错误标记
MineSweeper Run 4.11
——新增 游戏时可按Q进入问号模式
——新增 Bench统计字典大小不足次数
——优化 逻辑推理调试信息
——优化 枚举后不再进行简单判断和逻辑推理
——优化 雷率解不再视为可解地图
——修复 错误标记时已解3BV计算错误
——修复 Bench最小种子不为0时闪退
MineSweeper Run 4.12
——新增 枚举判断的剪枝和回溯算法(MC)
——新增 Bench记录字典需求
——优化 字典改用char存储
——优化 最大枚举限制由30提高到64，字典大小由1K提高到32K
——优化 可设置NC/MC作用长度范围，默认为15和19
MineSweeper Run 4.13
——优化 默认MC长度提高到32
——优化 标记过多时不再自动取消标记和显示提示
——修复 Bench帧暂停不暂停
——修复 执行标记问号导致标记过多时卡死
MineSweeper Run 4.14
——新增 Bench调试显示参考标准差
——新增 重构动态内存分配，可根据枚举长度动态分配字典内存
——新增 Bench调试显示未知链长度统计
——优化 以和字典代替全字典进行枚举判断
——优化 历史记录超过1024时仅输出最近记录
——优化 MC可能性超过1024时调试信息覆写式输出
——优化 可能性超过1024时调试信息不输出全字典
——优化 保持MC长度为枚举长度(NC<=MC)
——优化 可设置操作后保持光标显示
——优化 融合显示胜利矩阵和已解3BV矩阵
——优化 Bench非调试时置顶信息默认隐藏，可设置开关
——优化 调整设置Bench界面
——优化 对设置的缓存文件条目重排序
——修复 帧暂停时设置方案显示会变为中间帧
MineSweeper Run 4.15
——新增 调试选项可设置字典大小
——新增 MC根据剩余雷数排除
——新增 调试选项可设置局部NC/MC根据剩余雷数排除，默认NC开MC关
——优化 调整设置地图求解器界面
——优化 查看全部记录时也能正常操作
MineSweeper Run 4.16
——新增 调试选项可启用快速显示（通过显示缓冲仅刷新更新部分，减少顶级地图按键延迟）
——优化 调试选项归纳设置鼠标点击屏蔽空格和点击空地切换左右键
MineSweeper Run 4.17
——新增 调试选项可启用全局剩余雷数判断（根据多块枚举的整体结果进行剩余雷数判断）
——优化 MC可能性超过1024时调试信息跳跃式输出
——修复 字典溢出时雷率可能大于1
MineSweeper Run 4.18
——新增 地图搜索Debug2支持3BV筛选搜索、3BV汇总统计、可解种子搜索效率
——优化 通过反复探查强化全局剩余雷数判断
——优化 简化全局剩余雷数判断调试信息
——优化 通过连块排除加速全局剩余雷数判断
——优化 调整地图求解Debug2的连续显示
MineSweeper Run 5.0
——优化 控制台光标在键盘输入时必然显示
——优化 删除所有自制地图效率
——优化 减少调整窗口大小恢复控制台光标的可能
——优化 可解地图生成效率
——优化 Bench效率
——优化 结果缓存全字典也改用char存储
MineSweeper Run 5.1
——新增 Bench无帧
——优化 最大枚举限制由64提高到72，最大字典大小由8M提高到16M
——优化 Bench中间帧支持快速显示
——优化 设置游戏难度抗双击
——修复 鼠标按住右键时左键不能翻开，按住左键时右键不能快速标记
MineSweeper Run 5.2
——新增 地图搜索Debug2支持指定包含数字
——新增 自适应屏幕缓冲区宽度
——优化 可速解地图不再视为有效记录
——修复 Bench无帧不显示置顶信息
——修复 可解种子搜索效率必然显示控制台光标
——修复 键盘不可选择可速解地图
——修复 RecordsEditer界面输入字符后不断闪烁
MineSweeper Run 5.3
——新增 可速解地图改为筛选地图
——新增 设置界面按0不保存直接返回主页
——优化 Bench地图数不为100时胜率显示地图数和胜利数
——优化 子设置代码
——修复 自定义种子游戏不能以逗号形式输入
——修复 动态内存分配时设置难度闪退
——修复 Bench中间帧快速显示不确定翻开滞留
MineSweeper Run 5.4
——优化 设置筛选输入后自动调整为合理值
——修复 3BV汇总统计显示可能遗漏
——修复 键盘选择设置难度需鼠标移动后响应
——修复 顶级难度鼠标悬浮预览可能不能正确覆写
MineSweeper Run 5.5
——优化 通过排序加速可能性超过1024的全局剩余雷数判断
——优化 操作后立即响应
——优化 上一次地图保持地图生成校验
——优化 将竖向高级和顶级视为有效记录
——修复 鼠标点击后不显示操作提示
MineSweeper Run 5.6
——新增 调试选项可启用触动翻开（允许鼠标点击使用标记键快速翻开）
——优化 自制地图编辑支持快速显示
——修复 设置默认操作模式时直接点击新游戏闪退
——修复 自制地图布雷时雷数可能显示错误
——修复 自制地图编辑必然显示控制台光标
——修复 自定义或以密度设置雷数的难度可能不能正常筛选和显示打破纪录
MineSweeper Run 5.7
——优化 MC调试信息可能性显示
——优化 通过0块缓存加速未解3BV计算
——修复 标记0链及附近方块时已解3BV减少
MineSweeper Run 5.8
——新增 地图搜索Debug2支持地图生成效率
——新增 3BV汇总统计显示用时
——优化 全局剩余雷数判断的排序效率
——优化 通过前置校验加速起始点必为空的地图生成
——优化 通过0链框架加速0链生成和打开
——优化 自制地图游戏时间从第一次点击后开始
——修复 自制地图游戏时3BV计算错误
——修复 键盘模式编辑自制地图后直接返回主页
——修复 自制地图游戏动态内存分配可能闪退
MineSweeper Run 5.9
——优化 现在玩家等级9级及以上更难达到
——优化 现在默认启用动态内存分配
MineSweeper Run 5.10
——优化 找到8后停止计算最大翻开数字
——优化 计算最大翻开数字调用代码
——优化 独立可解和筛选种子搜索模块
MineSweeper Run 5.11
——优化 现在默认启用快速显示
——优化 彻底移除非动态内存分支
——修复 Bench无帧界面可能上下跳跃
——修复 地图搜索时显示方式矩阵可能异常
MineSweeper Run 5.12
——优化 通过单向边界检查加速0链判断
——优化 地图大小上限由128*93提升到256*384
——优化 自制地图游戏3BV计算也使用加速算法
——优化 实时求解指令Tab的执行效率
——优化 上一次地图快速显示的第一次显示
——修复 删除历史记录和自制地图时可能出现的内存错误
MineSweeper Map Painter 1.2
——优化 功能与函数跟随Run 5.12升级
MineSweeper Run 5.13
——新增 3BV筛选搜索显示地图可解性
——新增 自制地图显示3BV和岛上可解性
——新增 调试选项可启用可解地图岛预判加速
——新增 Bench调试显示毫秒用时
——优化 集成式仅标记雷可赢判断
——优化 记录统计显示
——优化 简单判断和逻辑推理效率
——优化 记录批量删除效率
——修复 自制地图编辑时调整窗口大小显示控制台光标
MineSweeper Run 5.14
——优化 独立空格打印函数
——优化 枚举总可能数计算效率
——优化 调整地图生成算法（集成自动调整地图生成校验、快速生成必然地图）
——优化 重新设计9级及以上玩家等级称号
——优化 移除部分不必要的调试
——修复 键盘设置光标不能按5退出
——修复 地图列数超过100时列坐标中部0不显示
MineSweeper Run 5.15
——新增 对密度>=0.25或面积小于64或大于3696的地图使用第二代雷场生成算法
——新增 使用csv文件保存历史记录
——新增 记录显示3BV/s和毫秒用时
——优化 彩色字符显示效率
——优化 重新设计8级及以下玩家等级称号
——优化 统一游戏信息和专用未解3BV内存
——优化 独立游戏信息显示函数
MineSweeper Run 5.16
——新增 标准分数SS纪录
——新增 全面使用第二代雷场生成算法
——新增 历史记录仅显示最近1024条时显示第1条和省略号
——优化 更精确的记录毫秒用时和秒用时
//——新增 调试选项可启用保存有效记录的操作记录
//——新增 主页按V或拖动文件至程序图标播放操作记录
//——新增 调试选项可启用屏蔽鼠标点击翻开标记
//——新增 Window操作模式
//——新增 组合雷率计算（根据多块枚举的结果组合进行雷率计算）
//——新增 内外雷率扰动（根据内部雷分布组合数对应枚举结果雷数扰动交界线雷率）
//——新增 触雷直接重开和超时间纪录且低速直接重开
//——新增 调试选项可启用统一标记（鼠标点击拖动标记根据起始操作统一标记/取消标记）
//——优化 现在地图求解可选择从外部文件读取地图，界面支持鼠标点击
//——优化 重新设计自定义难度设置，以密度设置雷数不再是调试选项
//——优化 雷率由浮点计算转为整数计算
//——修复 未操作的Tab后不使用快速显示
--------------------------------*/
