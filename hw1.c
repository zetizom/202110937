#include <stdio.h>

int main(void)
{
	int num1;
	int num2;
	int result1;
	int result2;
	int result3;
	printf("두정수를 입력하세요 입력하세요: 10 15");
	printf("\n");
	scanf("%d", &num1);
	scanf("%d", &num2);
	result1 = num1>>1 & num2<<2;
	result2 = num1<<1 | num2;
	result3 = num1>>1 ^ num2<<1;
	printf("10 & 15 = %d \n", result1);
	printf("10 | 15 = %d \n", result2);
	printf("10 ^ 15 = %d \n", result3);

	return 0;
}

