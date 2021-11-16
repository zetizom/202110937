#include<stdio.h>
int binary(int num)
{
	{
		if (num == 1)
			printf("1");
		else
		{
			int n = num % 2;
			binary(num / 2);
			printf("%d", n);
		}
	}
}
int main(void)
{
	int num;
	printf("Á¤¼ö´Â");
	scanf("%d", &num);
	binary(num);
}