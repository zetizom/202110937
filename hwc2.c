#include <stdio.h>
int main(void)
{
	double km;
	double mile;
	printf("Please enter kilometers: ");
	scanf("%lf", &km);
	mile = km / 1.609;
	printf("%.1lf is equal to %.1lf miles", km, mile);
}