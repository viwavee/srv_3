#include <iostream>
#include <time.h>
long long factorial(int n) {
    long long result = 1;
    for (int i = 2; i <= n; i++)
        result *= i;
    return result;
}
int main() {
    setlocale(LC_ALL, "ru");
    clock_t start = clock();
    for (int i = 0; i < 10000000; i++) {
        factorial(10);
    }

    clock_t end = clock();
    double seconds = (double)(end - start) / CLOCKS_PER_SEC;
    std::cout << "Время: " << seconds << " сек" << std::endl;
    return 0;
}
