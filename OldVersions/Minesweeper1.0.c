#include<stdio.h>
#include<stdlib.h>
#include<time.h>//ʱ������������������

/*
* ɨ�� by-Jws
* 
* ��Ϸ˼·��
* ��ʼ���桪����Ϸ���桪��������Ϸ��̨�����׼�����Χ�����֡�����Ϸ���桪����Ϸ���ƣ��ж�ʤ����������ʤ��/ʧ�ܣ�/������Ϸ
*/

void ForeGround();//��Ϸ����
void BackGround(int x, int y);//��Ϸ��̨
void Mechanism(int mode, int x, int y);//��Ϸ����
void Set();//��Ϸ����
void End();//��Ϸ����

char Board[64][64];//��Ϸ��̨
char ForeBoard[64][64];//��Ϸ����

int Process;//ִֻ��һ����Ϸ��ʼ��
int iWhile = 0;//����������������ӵĸ���
int Mine[128][2];//�����ף����±�Ϊ�������������
int Number[65][65];//�����洢���ɵ��׵����꣨�Ƚ��������һ��Ԫ�ط�ֹ���������

int mineNumer = 10;//�������ĳ�ʼ��
int boardHeighthSize = 10;//����߶ȳ�ʼ��
int boardWideSize = 10;//�����ȳ�ʼ��

int main(void) {

	Process = 0;//��ʼ��Process��ֵ
	int choiceMode;//��Ϸ���ܵ�ѡ��

	while (1) {

		system("cls");//����

		printf("*******************************\n");
		printf("**����Ϸ(1)**����(2)**�˳�(3)**\n");
		printf("*******************************\n");
		scanf("%d", &choiceMode);
		switch (choiceMode)
		{
		case 1:
			ForeGround();//������Ϸǰ̨����ʼ��Ϸ
			break;
		case 2:
			Set();//������Ϸ���ã�������Ϸ����
			break;
		case 3:
			exit(0);//�˳���Ϸ
			break;
		default:
			break;
		}
	}//��ʼ�˵�

	return 0;

}//��Ϸ��ʼ

void ForeGround() {

	system("cls");//����

	if (Process == 0) {

		int a = 48;
		for (int Row = 0; Row < boardHeighthSize; Row++) {
			if (a == 58) a = 97;
			if (a == 122) a = 65;
			ForeBoard[Row][0] = (char)a;
			a++;
		}
		a = 48;
		for (int Column = 0; Column < boardWideSize; Column++) {
			if (a == 58) a = 97;
			if (a == 122) a = 65;
			ForeBoard[0][Column] = (char)a;
			a++;
		}//ǰ̨����������������

		for (int Row = 1; Row < boardHeighthSize; Row++) {
			for (int Column = 1; Column < boardWideSize; Column++) {
				ForeBoard[Row][Column] = '%';
			}
		}//ǰ̨���������

		for (int Row = 0; Row < boardHeighthSize; Row++) {
			for (int Column = 0; Column < boardWideSize; Column++) {
				printf("%c ", ForeBoard[Row][Column]);
			}
			printf("\n");
		}//��һ�δ�ӡ����

		char x1, y1;//ѡ�������
		int x, y;//ת���������
		printf("ѡ������[x][y]\n");
		//ͨ�����̻�ȡ��һ��ѡ�������
		getchar();
		x1 = getchar();
		getchar();
		y1 = getchar();
		//��char����תΪint����
		if ((int)x1 > 96) x = (int)x1 - 87;
		else if ((int)x1 > 64) x = (int)x1 - 29;
		else if ((int)x1 > 48) x = (int)x1 - 48;
		if ((int)y1 > 96) y = (int)y1 - 87;
		else if ((int)y1 > 64) y = (int)y1 - 29;
		else if ((int)y1 > 48) y = (int)y1 - 48;

		Process++;

		BackGround(x, y);//������Ϸ��̨������

	}//��Ϸ�ĳ�ʼ��

	for (int Row = 0; Row < boardHeighthSize; Row++) {
		for (int Column = 0; Column < boardWideSize; Column++) {
			printf("%c ", ForeBoard[Row][Column]);
		}
		printf("\n");
	}//��ӡ��Ϸ����
	printf("ʣ������: %d\n", mineNumer);//��ӡʣ������

	int determine = 0;
	for (int Row = 0; Row < boardHeighthSize; Row++) {
		for (int Column = 0; Column < boardWideSize; Column++) {
			if (ForeBoard[Row][Column] == '%') determine++;
		}
	}//�ж��Ƿ�ʤ��
	if (determine == mineNumer) Mechanism(1, 0, 0);//���ʣ���������һ�£���ʤ��

	char choosemode;//ѡ�������
	char x1, y1;//ѡ�������
	int x, y;//ת���������
	printf("ѡ��ģʽ������(@:����/#:���)\n[@/#] [x] [y]\n");
	//ͨ�����̻�ȡѡ�������
	getchar();
	choosemode = getchar();
	getchar();
	x1 = getchar();
	getchar();
	y1 = getchar();
	//��char����תΪint����
	if ((int)x1 > 96) x = ((int)x1) - 87;
	else if ((int)x1 > 64) x = ((int)x1) - 29;
	else if ((int)x1 > 48) x = ((int)x1) - 48;
	if ((int)y1 > 96) y = ((int)y1) - 87;
	else if ((int)y1 > 64) y = ((int)y1) - 29;
	else if ((int)y1 > 48) y = ((int)y1) - 48;

	if (choosemode == '@') {
		if (Board[x][y] == '*') Mechanism(2, 0, 0); //����ȵ��ף���ʧ��
		if (Board[x][y] == '1'
			|| Board[x][y] == '2'
			|| Board[x][y] == '3'
			|| Board[x][y] == '4'
			|| Board[x][y] == '5'
			|| Board[x][y] == '6'
			|| Board[x][y] == '7'
			|| Board[x][y] == '8') Mechanism(3, x, y);//����ȵ�����
		if (Board[x][y] == ' ') Mechanism(4, x, y);//����ȵ��հ�
	}
	if (choosemode == '#') {
		if (ForeBoard[x][y] == '%') {
			ForeBoard[x][y] = '#';
			mineNumer--;//ʣ������-1
			ForeGround();
		}
		if (ForeBoard[x][y] == '#') {
			ForeBoard[x][y] = '%';
			mineNumer++;//ʣ������+1
			ForeGround();
		}
		ForeGround();
	}

}//��Ϸ����

void BackGround(int x, int y) {

	for (int Row = 1; Row < boardHeighthSize; Row++) {
		for (int Column = 1; Column < boardWideSize; Column++) {
			Board[Row][Column] = ' ';
		}
	}//��̨��ʼ���������

	srand((unsigned)time(NULL) + iWhile);//��ǰʱ������������������
	for (int Site = 0; Site < mineNumer; Site++) {
		Mine[Site][0] = rand() % (boardHeighthSize - 1) + 1;//�׵����±�
		Mine[Site][1] = rand() % (boardWideSize - 1) + 1;//�׵����±�
		if (Board[Mine[Site][0]][Mine[Site][1]] != '*') {
			Board[Mine[Site][0]][Mine[Site][1]] = '*';
		}//�������λ���Ƿ�������ظ���ΪBoard���鸳ֵ
		else Site--;
	}//�׵����ɣ����塰Mine�����飬���±��ʾ�׵ĸ��������±������ֵ��ʾ�׵����±������±ꡣ
	iWhile++;//��ֹ��һ�ε��ó���ʱ��������������ӣ�ʱ������ظ�

	for (int Row = 1; Row < boardHeighthSize; Row++) {
		for (int Column = 1; Column < boardWideSize; Column++) {
			Number[Row][Column] = 48;//��ʼ����Ϊ48��0��Ascii�룩����������ת��
		}
	}//����Χ���ֵ�����
	for (int Row = 0; Row < boardHeighthSize; Row++) {
		for (int Column = 0; Column < boardWideSize; Column++) {
			if (Board[Row][Column] == '*') {
				Number[Row - 1][Column - 1]++;
				Number[Row - 1][Column]++;
				Number[Row - 1][Column + 1]++;
				Number[Row][Column - 1]++;
				Number[Row][Column + 1]++;
				Number[Row + 1][Column - 1]++;
				Number[Row + 1][Column]++;
				Number[Row + 1][Column + 1]++;//ѭ�������ף�������Χ��������
				if (Board[Row - 1][Column - 1] != '*')
					Board[Row - 1][Column - 1] = (char)Number[Row - 1][Column - 1];
				if (Board[Row - 1][Column] != '*')
					Board[Row - 1][Column] = (char)Number[Row - 1][Column];
				if (Board[Row - 1][Column + 1] != '*')
					Board[Row - 1][Column + 1] = (char)Number[Row - 1][Column + 1];
				if (Board[Row][Column - 1] != '*')
					Board[Row][Column - 1] = (char)Number[Row][Column - 1];
				if (Board[Row][Column + 1] != '*')
					Board[Row][Column + 1] = (char)Number[Row][Column + 1];
				if (Board[Row + 1][Column - 1] != '*')
					Board[Row + 1][Column - 1] = (char)Number[Row + 1][Column - 1];
				if (Board[Row + 1][Column] != '*')
					Board[Row + 1][Column] = (char)Number[Row + 1][Column];
				if (Board[Row + 1][Column + 1] != '*')
					Board[Row + 1][Column + 1] = (char)Number[Row + 1][Column + 1];
			}//ѭ�������ף�������Χ�������֣���������ָ��ǵ��׵����
		}
	}//����Χ���ֵ�����

	if (Board[x][y] != ' ') BackGround(x, y);//���ѡ�����겻Ϊ�ո�������������

	int a = 48;
	for (int Row = 0; Row < boardHeighthSize; Row++) {
		if (a == 58) a = 97;
		if (a == 123) a = 65;
		Board[Row][0] = (char)a;
		a++;
	}
	a = 48;
	for (int Column = 0; Column < boardWideSize; Column++) {
		if (a == 58) a = 97;
		if (a == 123) a = 65;
		Board[0][Column] = (char)a;
		a++;
	}//��Ϸ��̨����������������

	Mechanism(4, x, y);//չ��

}//��Ϸ��̨

void Mechanism(int mode, int x, int y) {

	if (mode == 1) {
		printf("You Win!\n");
		system("pause");
		End();
	}//ʤ��

	if (mode == 2) {
		printf("Game Over!\n");
		system("pause");
		End();
	}//�ȵ��׵����

	if (mode == 3) {
		ForeBoard[x][y] = Board[x][y];
		int markMine = 0;
		if (ForeBoard[x - 1][y - 1] == '#') markMine++;
		if (ForeBoard[x - 1][y] == '#')markMine++;
		if (ForeBoard[x - 1][y + 1] == '#') markMine++;
		if (ForeBoard[x][y - 1] == '#')  markMine++;
		if (ForeBoard[x][y + 1] == '#') markMine++;
		if (ForeBoard[x + 1][y - 1] == '#') markMine++;
		if (ForeBoard[x + 1][y] == '#') markMine++;
		if (ForeBoard[x + 1][y + 1] == '#') markMine++;

		if (markMine == int(ForeBoard[x][y] - 48)) {
			if ((ForeBoard[x - 1][y - 1] != '#' && Board[x - 1][y - 1] == '*')
				|| (ForeBoard[x - 1][y] != '#' && Board[x - 1][y] == '*')
				|| (ForeBoard[x - 1][y + 1] != '#' && Board[x - 1][y + 1] == '*')
				|| (ForeBoard[x][y - 1] != '#' && Board[x][y - 1] == '*')
				|| (ForeBoard[x][y + 1] != '#' && Board[x][y + 1] == '*')
				|| (ForeBoard[x + 1][y - 1] != '#' && Board[x + 1][y - 1] == '*')
				|| (ForeBoard[x + 1][y] != '#' && Board[x + 1][y] == '*')
				|| (ForeBoard[x + 1][y + 1] != '#' && Board[x + 1][y + 1] == '*'))Mechanism(2, 0, 0); 
			//�����Χ��δ�򿪵�����ʧ��
			
			if (ForeBoard[x - 1][y - 1] != '#')
				ForeBoard[x - 1][y - 1] = Board[x - 1][y - 1];
			if (ForeBoard[x - 1][y] != '#')
				ForeBoard[x - 1][y] = Board[x - 1][y];
			if (ForeBoard[x - 1][y + 1] != '#')
				ForeBoard[x - 1][y + 1] = Board[x - 1][y + 1];
			if (ForeBoard[x][y - 1] != '#')
				ForeBoard[x][y - 1] = Board[x][y - 1];
			if (ForeBoard[x][y + 1] != '#')
				ForeBoard[x][y + 1] = Board[x][y + 1];
			if (ForeBoard[x + 1][y - 1] != '#')
				ForeBoard[x + 1][y - 1] = Board[x + 1][y - 1];
			if (ForeBoard[x + 1][y] != '#')
				ForeBoard[x + 1][y] = Board[x + 1][y];
			if (ForeBoard[x + 1][y + 1] != '#')
				ForeBoard[x + 1][y + 1] = Board[x + 1][y + 1];
			//��������Χδ�򿪵�����

			for (int round = 0; round < 100; round++) {
				for (int Row = 0; Row < boardHeighthSize; Row++) {
					for (int Column = 0; Column < boardWideSize; Column++) {
						if (ForeBoard[Row][Column] == ' ') {
							ForeBoard[Row][Column] = Board[Row][Column];
							if (Board[Row - 1][Column + 1] != '*' && Board[Row - 1][Column + 1] != ' ')
								ForeBoard[Row - 1][Column + 1] = Board[Row - 1][Column + 1];
							if (Board[Row - 1][Column - 1] != '*' && Board[Row - 1][Column - 1] != ' ')
								ForeBoard[Row - 1][Column - 1] = Board[Row - 1][Column - 1];
							if (Board[Row - 1][Column] != '*') ForeBoard[Row - 1][Column] = Board[Row - 1][Column];
							if (Board[Row + 1][Column + 1] != '*' && Board[Row + 1][Column + 1] != ' ')
								ForeBoard[Row + 1][Column + 1] = Board[Row + 1][Column + 1];
							if (Board[Row + 1][Column - 1] != '*' && Board[Row + 1][Column - 1] != ' ')
								ForeBoard[Row + 1][Column - 1] = Board[Row + 1][Column - 1];
							if (Board[Row + 1][Column] != '*') ForeBoard[Row + 1][Column] = Board[Row + 1][Column];
							if (Board[Row][Column + 1] != '*') ForeBoard[Row][Column + 1] = Board[Row][Column + 1];
							if (Board[Row][Column - 1] != '*') ForeBoard[Row][Column - 1] = Board[Row][Column - 1];
						}
					}
				}
			}//ͨ��ѭ�������㹻������ForeBoard���飨��Ϸǰ̨��ʵ��չ��������ʼǣ�
		}

		ForeGround();
	}//�ȵ����ֵ����

	if (mode == 4) {

		if (Board[x][y] != '*') ForeBoard[x][y] = Board[x][y];

		for (int round = 0; round < 100; round++) {
			for (int Row = 0; Row < boardHeighthSize; Row++) {
				for (int Column = 0; Column < boardWideSize; Column++) {
					if (ForeBoard[Row][Column] == ' ') {
						ForeBoard[Row][Column] = Board[Row][Column];
						if (Board[Row - 1][Column + 1] != '*' && Board[Row - 1][Column + 1] != ' ') 
							ForeBoard[Row - 1][Column + 1] = Board[Row - 1][Column + 1];
						if (Board[Row - 1][Column - 1] != '*' && Board[Row - 1][Column - 1] != ' ') 
							ForeBoard[Row - 1][Column - 1] = Board[Row - 1][Column - 1];
						if (Board[Row - 1][Column] != '*') ForeBoard[Row - 1][Column] = Board[Row - 1][Column];
						if (Board[Row + 1][Column + 1] != '*' && Board[Row + 1][Column + 1] != ' ') 
							ForeBoard[Row + 1][Column + 1] = Board[Row + 1][Column + 1];
						if (Board[Row + 1][Column - 1] != '*' && Board[Row + 1][Column - 1] != ' ') 
							ForeBoard[Row + 1][Column - 1] = Board[Row + 1][Column - 1];
						if (Board[Row + 1][Column] != '*') ForeBoard[Row + 1][Column] = Board[Row + 1][Column];
						if (Board[Row][Column + 1] != '*') ForeBoard[Row][Column + 1] = Board[Row][Column + 1];
						if (Board[Row][Column - 1] != '*') ForeBoard[Row][Column - 1] = Board[Row][Column - 1];
					}
				}
			}
		}//ͨ��ѭ�������㹻������ForeBoard���飨��Ϸǰ̨��ʵ��չ��������ʼǣ�

		ForeGround();

	}//�ȵ��հ׵����
}//��Ϸ����

void Set() {

	int set;

	while (1) {

		system("cls");

		printf("*******************************\n");
		printf("********������Ϸ�Ѷ�(1)********\n");
		printf("*******�Զ�����������(2)*******\n");
		printf("*****�Զ������ý����С(3)*****\n");
		printf("**********���ز˵�(4)**********\n");
		printf("*******************************\n");
		printf("��ǰ����:%d|��ǰ�����С:%d*%d\n", mineNumer, (boardWideSize - 1), (boardHeighthSize - 1));
		scanf("%d", &set);

		if (set == 1) {
			int difficuly;
			system("cls");
			printf("*******************************\n");
			printf("************����(1)************\n");
			printf("************�м�(2)************\n");
			printf("************�߼�(3)************\n");
			printf("*******************************\n");
			scanf("%d", &difficuly);
			if (difficuly == 1) {
				mineNumer = 10;
				boardWideSize = 9 + 1;
				boardHeighthSize = 9 + 1;
			}
			if (difficuly == 2) {
				mineNumer = 40;
				boardWideSize = 16 + 1;
				boardHeighthSize = 16 + 1;
			}
			if (difficuly == 3) {
				mineNumer = 99;
				boardWideSize = 30 + 1;
				boardHeighthSize = 24 + 1;
			}
		}

		if (set == 2) {
			printf("����������:\n");
			scanf("%d", &mineNumer);
		}

		if (set == 3) {
			int Wide, Heighth;
			printf("�����������:\n");
			scanf("%d", &Wide);
			boardWideSize = ++Wide;
			printf("���������߶�:\n");
			scanf("%d", &Heighth);
			boardHeighthSize = ++Heighth;
		}

		if (set == 4) main();//����������

	}

}//��Ϸ����

void End() {

	int next;

	system("cls");

	printf("********************************\n");
	printf("*********����(1)������Ϸ********\n");
	printf("******�������������˳���Ϸ******\n");
	printf("********************************\n");
	scanf("%d", &next);

	switch (next)
	{
	case 1: {
		mineNumer = 10;
		boardWideSize = 9 + 1;
		boardHeighthSize = 9 + 1;
		main();
	}
	default:
		exit(0);
	}

}//��Ϸ����
