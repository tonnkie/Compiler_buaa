#include "glob.h"

int codeindex = 0;
int block[200];
int blockindex = 0;

//扫描这个四元式数组，处理所有的基本块
void scan()
{
	//为块的划分作个标记
	int temp = 0;
	while (temp < midcodeiter) {
		if (strcmp(midcode[temp].op, "func") == 0 ||
			strcmp(midcode[temp].op, "begin") == 0 ||
			strcmp(midcode[temp].op, "jne") == 0 ||
			strcmp(midcode[temp].op, "jmp") == 0 ||
			strcmp(midcode[temp].op, "call") == 0) {
			block[blockindex++] = temp;
		}
		temp++;
	}
	//for (int i = 0; i < blockindex; i++) {
	//	cout << block[i] << endl;
	//}

	//遍历各个块
	int i = 0;
	while (i < blockindex) {
		while (codeindex < block[i]) {
			combine();
			delpublic();
			codeindex++;
		}
		i++;
	}
	//删除冗余的lab标签
	delsetlab();
	printOptimize();
	return;
}

//判断是不是数字
int isconst(char name[])
{
	int i = 0;
	if (name[i] == '-') i++;
	while (name[i] != '\0') {
		if (name[i]>'9' || name[i] < '0') return 0;
		i++;
	}
	return 1;
}

//合并常数
void combine() {
	int i = codeindex, num1, num2, temp = 0;
	char sum[30];
	while (strcmp(midcode[i].op, "+") == 0 || strcmp(midcode[i].op, "-") == 0 ||
		strcmp(midcode[i].op, "*") == 0 || strcmp(midcode[i].op, "/") == 0) {
		if (isconst(midcode[i].var1) && isconst(midcode[i].var2)) {
			num1 = atoi(midcode[i].var1);
			num2 = atoi(midcode[i].var2);
			if (strcmp(midcode[i].op, "+") == 0) temp = num1 + num2;
			if (strcmp(midcode[i].op, "-") == 0) temp = num1 - num2;
			if (strcmp(midcode[i].op, "*") == 0) temp = num1*num2;
			if (strcmp(midcode[i].op, "/") == 0) temp = num1 / num2;
			sprintf(sum, "%d", temp);
			strcpy(midcode[i].op, "=");
			strcpy(midcode[i].var1, sum);
			strcpy(midcode[i].var2, "  ");
		}
		i++;
	}
}


void delpublic()
{
	//which is not quite correct!!!
	int i, j, h, k;
	//划分基本块
	for (i = codeindex; strcmp(midcode[i].op, "+") == 0 || strcmp(midcode[i].op, "-") == 0 ||
		strcmp(midcode[i].op, "*") == 0 || strcmp(midcode[i].op, "/") == 0 || 
		strcmp(midcode[i].op, "=") == 0; i++) {
		if (i >= midcodeiter) {
			return;
		}
		if (midcode[i].var3[0] == '$') {
			for (j = i + 1; strcmp(midcode[j].op, "+") == 0 || strcmp(midcode[j].op, "-") == 0 ||
				strcmp(midcode[j].op, "*") == 0 || strcmp(midcode[j].op, "/") == 0 || 
				strcmp(midcode[j].op, "=") == 0; j++) {
				if (j >= midcodeiter) {
					return;
				}
				//寻找公共子表达式
				if (strcmp(midcode[j].op, midcode[i].op) == 0 && strcmp(midcode[j].var1, midcode[i].var1) == 0 &&
					strcmp(midcode[j].var2, midcode[i].var2) == 0 && midcode[j].var3[0] == '$') {
					//修改变量的名字
					for (h = j + 1; strcmp(midcode[h].op, "+") == 0 || strcmp(midcode[h].op, "-") == 0 || 
						strcmp(midcode[h].op, "*") == 0 || strcmp(midcode[h].op, "/") == 0 ||
						strcmp(midcode[h].op, "=") == 0; h++) {
						if (h >= midcodeiter) {
							return;
						}
						if (strcmp(midcode[h].var1, midcode[j].var3) == 0)
							strcpy(midcode[h].var1, midcode[i].var3);
						if (strcmp(midcode[h].var2, midcode[j].var3) == 0)
							strcpy(midcode[h].var2, midcode[i].var3);
					}
					for (k = j; k < midcodeiter; k++) {
						strcpy(midcode[k].op, midcode[k + 1].op);     //删除
						strcpy(midcode[k].var1, midcode[k + 1].var1);
						strcpy(midcode[k].var2, midcode[k + 1].var2);
						strcpy(midcode[k].var3, midcode[k + 1].var3);
					}
					midcodeiter--;
					j--;
				}
			}
		}
	}
}

//删除冗余跳转代码
void delsetlab()
{
	int i, k, j, t, flag;
	char temp[30][10];
	for (i = 0; i < midcodeiter; i++) {
		if (i >= midcodeiter) {
			return;
		}
		if (strcmp(midcode[i].op, "lab:") == 0) {
			j = 0;
			flag = i;
			i = i + 1;
			while (strcmp(midcode[i].op, "lab:") == 0) {
				strcpy(temp[j++], midcode[i].var3);
				for (k = i; k < midcodeiter; k++) {
					strcpy(midcode[k].op, midcode[k + 1].op);     //删除
					strcpy(midcode[k].var1, midcode[k + 1].var1);
					strcpy(midcode[k].var2, midcode[k + 1].var2);
					strcpy(midcode[k].var3, midcode[k + 1].var3);
				}
				midcodeiter--;
			}
			if (j == 0) continue;
			for (k = 0; k <= midcodeiter; k++) {
				if (k >= midcodeiter) {
					return;
				}
				if (strcmp(midcode[k].op, "jmp") == 0 || strcmp(midcode[k].op, "jne") == 0) {
					for (t = 0; t < j; t++) {
						if (strcmp(midcode[k].var3, temp[t]) == 0)
							strcpy(midcode[k].var3, midcode[flag].var3);
					}
				}
			}
		}
	}
}



//打印优化后的四元式
void printOptimize()
{
	optcodefile = fopen("optMidCode.txt", "w");
	int i = 0;
	while (i < midcodeiter) {
		fprintf(optcodefile, "\t\t");
		fprintf(optcodefile, "%s,\t", midcode[i].op);
		fprintf(optcodefile, "%s,\t", midcode[i].var1);
		fprintf(optcodefile, "%s,\t", midcode[i].var2);
		fprintf(optcodefile, "%s;\n", midcode[i].var3);
		if (strcmp(midcode[i].op, "end") == 0)
			fprintf(optcodefile, "\n\n");
		i++;
	}
	fclose(optcodefile);
	return;
}
