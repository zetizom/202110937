#include<stdio.h>
#define MAX_LENGTH 64

void Change(char a_str[])
{
    int i;

    for (i = 0; a_str[i] != 0; i++) {
        if (a_str[i] >= 'A' && a_str[i] <= 'Z') {
            a_str[i] = a_str[i] + 32;
        }
        else if (a_str[i] >= 'a' && a_str[i] <= 'z') {
            a_str[i] = a_str[i] - 32;
        }
    }
}

int main()
{
	char str[30];

	printf("Input> ");
	scanf_s("%[^\n]s", str, MAX_LENGTH - 1);

	Change(str);
	printf("Output> %s\n", str);
	return 0;
}
