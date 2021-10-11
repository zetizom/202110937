#include<stdio.h>
int main(void)
{
	int num;
	int is = 2;
	printf("임의의 양의 정수를 입력하시오");
	scanf("%d", &num);
	while (1)
	{
		if (num == 1)
		{
			printf("It is not a prime number");
			break;
		}
		else if (num == 2)
		{
			printf("It is a prime number.");
			break;
		}
		else if (num == is)
		{
			printf("It is a prime number.");
			break;
		}
		else if (num % is == 0)
		{
			printf("It is not a prime number.");
			break;
		}
		else
		{
			is++;
		}
	}
	return 0;
}