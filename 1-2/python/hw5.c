#include <stdio.h>

void num(int param[], int len)
{
    for (int i = 0; i < len; i++)
    {
        if (param[i] % 2 == 0)
            printf("%d ", param[i]);
    }
    printf("\n");
}

void num3(int param[], int len)
{
    for (int i = 0; i < len; i++)
    {
        if (param[i] % 2 == 1)
            printf("%d ", param[i]);
    }
    printf("\n");
}

int main(void)
{
    int arr[5];
    int len = (sizeof(arr) / sizeof(int));

    printf("총 5개의 정수를 입력 \n");

    for (int i = 0; i < len; i++)
    {
        printf("입력: ");
        scanf("%d", &arr[i]);
    }

    printf("홀수 출력 \n");
    showNumberOdd(arr, len);
    printf("짝수 출력 \n");
    showNumberEven(arr, len);

    return 0;
}