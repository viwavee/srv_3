#include <iostream>
#include <atomic>
#include <thread>

using namespace std;

class CustomMutex {
private:
    atomic<bool> locked;

public:
    CustomMutex() : locked(false) {}

    
    void lock() {
        bool expected = false;
        // Пытаемся установить locked в true, если оно было false
        while (!locked.compare_exchange_weak(expected, true,
            memory_order_acquire)) {
            expected = false; // Сбрасываем expected для следующей попытки
            // Даем возможность другим потокам поработать
            this_thread::yield();
        }
        // Мьютекс успешно захвачен
    }

    // Освобождение мьютекса
    void unlock() {
        locked.store(false, memory_order_release);
    }

    // Нельзя копировать мьютекс
    CustomMutex(const CustomMutex&) = delete;
    CustomMutex& operator=(const CustomMutex&) = delete;
};

// Демонстрация работы CustomMutex
int main() {
    setlocale(LC_ALL, "RU");
    CustomMutex mtx;
    int shared_counter = 0;
    const int iterations = 10000;

    auto worker = [&](int id) {
        for (int i = 0; i < iterations; ++i) {
            mtx.lock();
            // Критическая секция
            ++shared_counter;
            mtx.unlock();
        }
        };

    thread t1(worker, 1);
    thread t2(worker, 2);

    t1.join();
    t2.join();

    cout << "Ожидаемое значение: " << 2 * iterations << endl;
    cout << "Фактическое значение: " << shared_counter << endl;
    cout << "Тест " << (shared_counter == 2 * iterations ? "ПРОЙДЕН" : "ПРОВАЛЕН") << endl;

    return 0;
}