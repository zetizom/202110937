#include <stdio.h>
#include <math.h>


int size = 5;
static double average, deviation;

void insert_arr(double* a, int size);
void average_arr(double* a, int size);
void deviation_arr(double* a, int size);

int main() {
    double arr[5] = { 0 };

    insert_arr(arr, 5);
    deviation_arr(arr, 5);

    return 0;
}

void insert_arr(double* a, int size) {
    int i;

    for (i = 0; i < size; i++) {
        printf("Enter 5 real numbers");
        scanf("%lf", &a[i]);
    }
}
void deviation_arr(double* a, int size) {
    int i;
    double sum = 0;

    for (i = 0; i < size; i++)
        sum += (a[i] - average) * (a[i] - average);
    deviation = sqrt(sum / size);

    printf("Standard Deviation = %f \n", deviation);
}
