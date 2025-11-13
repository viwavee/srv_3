#include <thread>
#include <mutex>
#include <string>
#include <iostream>
#include <chrono>
std::mutex m;
void Func(std::string name)
{
    long double i = 0;
    auto start = std::chrono::high_resolution_clock::now();
    while (true)
    {
        auto now = std::chrono::high_resolution_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() >= 1000)
            break;
        i += 1e-9;
    }
    
    std::cout << name << ": " << i << std::endl;
    
}
int main()
{  std::thread thread1(Func, "t1");
    std::thread thread2(Func, "t2");
    std::thread thread3(Func, "t3");
    thread1.join();
    thread2.join();
    thread3.join();
    system("pause");}
