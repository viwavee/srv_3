#include <iostream>
#include <thread>
#include <string>
#include <chrono>
using namespace std;
void Func(string name)
{
    const int N = 20'000'000;  // большое число для нагрузки
    unsigned long long s = 1;
    for (int i = 1; i < N; ++i)
    {
        s = (s * i) % 1000000007;  // защищаемся от переполнения
    }
    cout << "Поток " << name << " завершил работу\n";
}
int main()
{
    setlocale(LC_ALL, "ru");
    using clock = chrono::high_resolution_clock;
    //Параллельный запуск 
    auto start_parallel = clock::now();
    thread thread1(Func, "t1");
    thread thread2(Func, "t2");
    thread1.join();
    thread2.join();

    auto end_parallel = clock::now();
    auto parallel_time = chrono::duration<double>(end_parallel - start_parallel).count();
    cout << "Время выполнения в параллельных потоках: "
        << parallel_time << " сек\n\n";
    // Последовательный запуск
    auto start_seq = clock::now();
    Func("послед1");
    Func("послед2");
    auto end_seq = clock::now();
    auto seq_time = chrono::duration<double>(end_seq - start_seq).count();
    cout << "Время последовательного выполнения: "
        << seq_time << " сек\n";
    system("pause");
    return 0;
}
