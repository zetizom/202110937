#include<stdio.h>
int main(void)
{
	int arr1[6] = { 1, 2, 3, 4, 5, 6 };
	int arr2[6] = { 7, 8, 9, 10, 11, 12 };
	int* ptr1 = &arr1[0];
	int* ptr2 = &arr2[0];
	int i, l;
	for (i = 0; i < 6; i++)
	{
		l = *ptr1;
		*ptr1 = *ptr2;
		*ptr2 = *ptr1;
		*ptr2 = l;
		ptr1++;
		ptr2++;
	}
	printf("after swap\n");
	for (i = 0; i < 6; i++)
		printf("%d ", arr1[i]);
	printf("\n");
	for (i= 0; i < 6; i++)
		printf("%d ", arr2[i]);
	return 0;
}
