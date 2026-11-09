#include <stdint.h>
#include <stdio.h>

// 读取大端法 DeepSeek设计 Ltabsyy调整
int ReadBE_uint8(FILE *fp, uint8_t *out)
{
	return fread(out, 1, 1, fp) == 1; // 实际就是读取一个字节
}

int ReadBE_uint16(FILE *fp, uint16_t *out)
{
	unsigned char buf[2];
	if(fread(buf, 1, 2, fp) != 2) return 0;
	*out = ((uint16_t)buf[0] << 8) | (uint16_t)buf[1];
	return 1;
}

int ReadBE_uint32(FILE *fp, uint32_t *out)
{
	unsigned char buf[4];
	if(fread(buf, 1, 4, fp) != 4) return 0;
	*out = ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16)
		 | ((uint32_t)buf[2] << 8)  | (uint32_t)buf[3];
	return 1;
}

int ReadBE_uint64(FILE *fp, uint64_t *out)
{
	unsigned char buf[8];
	if(fread(buf, 1, 8, fp) != 8) return 0;
	*out = ((uint64_t)buf[0] << 56) | ((uint64_t)buf[1] << 48)
		 | ((uint64_t)buf[2] << 40) | ((uint64_t)buf[3] << 32)
		 | ((uint64_t)buf[4] << 24) | ((uint64_t)buf[5] << 16)
		 | ((uint64_t)buf[6] << 8)  | (uint64_t)buf[7];
	return 1;
}

int ReadBE_int16(FILE *fp, int16_t *out)
{
	uint16_t u;
	if(!ReadBE_uint16(fp, &u)) return 0; // 复用 uint16 转换，补码下安全
	*out = (int16_t)u;   // 主流平台（补码）行为正确
	return 1;
}

int main(int argc, char** argv)
{
	uint8_t val8;
	uint16_t val16;
	uint32_t val32;
	uint64_t val64;
	int16_t sval16;
	char s[256]={0};
	int isTranscoding = 0, rows, cols;
	if(argc != 2)
	{
		printf("请用此程序打开evf文件");
		getchar();
		return 0;
	}
	FILE* file = fopen(argv[1], "rb");
	if(!file)
	{
		printf("evf文件打开失败！");
		getchar();
		return 0;
	}
	printf("%s读取成功！\n", argv[1]);
	//读取版本号
	ReadBE_uint8(file, &val8);
	printf("版本号：%u\n", val8);
	//读取摘要
	ReadBE_uint8(file, &val8);
	printf("摘要：%u\n", val8);
	if(val8 & 0x80)
	{
		printf("扫完 ");
	}
	else
	{
		printf("未扫完 ");
	}
	if(val8 & 0x40)
	{
		printf("正式 ");
	}
	else
	{
		printf("非正式 ");
	}
	if(val8 & 0x20)
	{
		printf("公平 ");
	}
	else
	{
		printf("非公平 ");
	}
	if(val8 & 0x10)
	{
		printf("盲扫");
	}
	else
	{
		printf("非盲扫 ");
	}
	if(val8 & 0x08)
	{
		printf("转码 ");
		isTranscoding = 1;
	}
	else
	{
		printf("非转码 ");
	}
	putchar('\n');
	//游戏设置
	ReadBE_uint8(file, &val8);
	printf("游戏设置：%u\n", val8);
	if(val8 & 0x80)
	{
		printf("开问号 ");
	}
	else
	{
		printf("关问号 ");
	}
	if(val8 & 0x40)
	{
		printf("鼠标不超边框 ");
	}
	else
	{
		printf("鼠标能超边框 ");
	}
	if(val8 & 0x20)
	{
		printf("触雷自动重开 ");
	}
	else
	{
		printf("触雷不自动重开 ");
	}
	putchar('\n');
	//行数
	ReadBE_uint8(file, &val8);
	printf("行数：%u\n", val8);
	rows = val8;
	//列数
	ReadBE_uint8(file, &val8);
	printf("列数：%u\n", val8);
	cols = val8;
	//雷数
	ReadBE_uint16(file, &val16);
	printf("雷数：%u\n", val16);
	//方格尺寸
	ReadBE_uint8(file, &val8);
	printf("方格尺寸：%u\n", val8);
	//游戏模式
	ReadBE_uint16(file, &val16);
	printf("游戏模式：%u\n", val16);
	//3BV数值
	ReadBE_uint16(file, &val16);
	printf("3BV数值：%u\n", val16);
	//用时
	ReadBE_uint32(file, &val32);
	printf("用时：%u\n", val32);
	//国家或地区的名称或代码
	ReadBE_uint16(file, &val16);
	printf("国家或地区的名称或代码：%c%c\n", val16 >> 8, val16 & 0xff);
	//开始时间戳
	ReadBE_uint64(file, &val64);
	printf("开始时间戳：%llu\n", val64);
	//结束时间戳
	ReadBE_uint64(file, &val64);
	printf("结束时间戳：%llu\n", val64);
	//录像的最初来源
	for(int i=0; (s[i]=fgetc(file))!=0; i++);
	puts(s);
	if(isTranscoding)//仅转码录像
	{
		//转码软件的名称
		for(int i=0; (s[i]=fgetc(file))!=0; i++);
		puts(s);
		//录像的用户标识、比赛标识、唯一性标识的可能的编码方式
		for(int i=0; (s[i]=fgetc(file))!=0; i++);
		puts(s);
	}
	//用户标识
	for(int i=0; (s[i]=fgetc(file))!=0; i++);
	puts(s);
	//比赛标识
	for(int i=0; (s[i]=fgetc(file))!=0; i++);
	puts(s);
	//唯一性标识
	for(int i=0; (s[i]=fgetc(file))!=0; i++);
	puts(s);
	//uuid 的占用字节数
	ReadBE_uint16(file, &val16);
	printf("uuid 的占用字节数：%u\n", val16);
	for(int i=0; i<val16; i++) putchar(fgetc(file)?'*':'_');
	putchar('\n');
	//局面
	int n = rows*cols/8+!!(rows*cols%8);
	printf("局面：\n");
	for(int i=0; i<n; i++)
	{
		printf("%02x", fgetc(file));
	}
	putchar('\n');
	//自定义指标的数量
	ReadBE_uint16(file, &val16);
	printf("自定义指标的数量：%u\n", val16);
	for(int i=0; i<val16; i++)
	{
		for(int j=0; (s[j]=fgetc(file))!=0; j++);
		puts(s);
	}
	//事件列表
	uint8_t type;
	while(1)
	{
		ReadBE_uint8(file, &type);
		if(type == 0) break;//事件结束标记
		else if(type >= 1 && type <= 12)//鼠标事件
		{
			//时间戳的增加量
			ReadBE_uint8(file, &val8);
			printf("鼠标事件%u时+%u ", type, val8);
			ReadBE_int16(file, &sval16);
			printf("左距+%d ", sval16);
			ReadBE_int16(file, &sval16);
			printf("上距+%d；\n", sval16);
		}
		else if(type >= 81 && type <= 99)//游戏状态事件
		{
			//时间戳的增加量
			ReadBE_uint8(file, &val8);
			printf("游戏状态%u事件时间戳的增加量：%u\n", type, val8);
		}
		else if(type >= 100 && type <= 121)//局面事件
		{
			//时间戳的增加量
			ReadBE_uint8(file, &val8);
			printf("局面事件%u时间戳的增加量：%u ", type, val8);
			ReadBE_uint8(file, &val8);
			printf("行数：%u ", val8);
			ReadBE_uint8(file, &val8);
			printf("列数：%u\n", val8);
		}
		else if(type == 200 || type == 201)//指标事件
		{
			//时间戳的增加量
			ReadBE_uint8(file, &val8);
			printf("指标事件%u时间戳的增加量：%u ", type, val8);
			ReadBE_uint16(file, &val16);
			printf("指标的索引：%u ", val16);
			if(type == 200)
			{
				ReadBE_uint64(file, &val64);
				printf("数值指标%d\n", val64);
			}
			else
			{
				for(int i=0; (s[i]=fgetc(file))!=0; i++);
				putchar('\n');
			}
		}
		else if(type == 255)//停顿事件
		{
			//时间戳的增加量
			ReadBE_uint16(file, &val16);
			printf("停顿事件时间戳的增加量：%u\n", val16);
		}
	}
	//校验码长度
	ReadBE_uint16(file, &val16);
	printf("校验码长度：%u\n", val16);
	//校验码
	for(int i=0; i<val16; i++) putchar(fgetc(file));
	putchar('\n');
	getchar();
	return 0;
}

/*--------------------------------
更新日志：
EVF Reader 1.1
——优化 使用新版编译器
——修复 上游evf文档错误导致问号模式识别错误
EVF Reader 1.2
——优化 调整大端读取法代码，风格统一
--------------------------------*/
