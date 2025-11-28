#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <atomic>
#include <chrono>
#include <string>

using namespace std;


// Структура для хранения результатов сценария
struct ScenarioResult {
    string outcome;              // Итоговый результат сценария
    int efficiency_factor;       // Коэффициент эффективности повара
    int gluttony;                // Прожорливость толстяков
    vector<int> final_plates;    // Финальное состояние тарелок
    vector<long long> eaten;     // Количество съеденного каждым толстяком
    double elapsed_seconds;      // Затраченное время в секундах
};

// Функция запуска одного сценария 
ScenarioResult run_scenario(int efficiency_factor, int gluttony) {
    const int initial = 3000;  // Начальное количество еды на каждой тарелке
    vector<int> plates(3, initial);  // Тарелки с едой для каждого толстяка
    vector<long long> eaten(3, 0);   // Счетчики съеденной еды
    vector<bool> alive(3, true);     // Статус жизни толстяков

    // Синхронизационные примитивы
    mutex mtx;
    condition_variable cv;
    int stage = 0;              // 0 - готовка, 1 - прием пищи
    int finished_eaters = 0;    // Количество завершивших прием пищи
    bool finished = false;      // Флаг завершения 

    // флаги для различных условий завершения
    atomic<bool> cook_fired(false);      // Повар уволен
    atomic<bool> cook_resigned(false);   // Повар уволился
    atomic<bool> all_fatties_dead(false); // Все толстяки умерли
    atomic<int> dead_count(0);           // Счетчик умерших толстяков

    auto start_time = chrono::steady_clock::now();  // Время начала 

    // Поток повара
    thread cook([&] {
        while (true) {
            unique_lock<mutex> lk(mtx);
            // Ожидаем стадию готовки или завершения
            cv.wait(lk, [&] { return stage == 0 || finished; });
            if (finished) break;

            // Распределяем еду по тарелкам
            int base = efficiency_factor / 3;
            int rem = efficiency_factor % 3;
            for (int i = 0; i < 3; ++i) {
                plates[i] += base + (i < rem ? 1 : 0);
            }

            // Переходим к стадии приема пищи
            stage = 1;
            finished_eaters = 0;
            lk.unlock();
            cv.notify_all();  // Будим толстяков

            // Имитация времени готовки
            this_thread::sleep_for(5ms);

            // Проверка времени работы (5 секунд = 5 дней)
            auto now = chrono::steady_clock::now();
            double elapsed = chrono::duration_cast<chrono::duration<double>>(now - start_time).count();
            if (elapsed >= 5.0) {
                cook_resigned = true;
                finished = true;
                lk.lock();
                stage = 2;  // Финальная стадия
                lk.unlock();
                cv.notify_all();
                break;
            }

            // Проверка условий увольнения или смерти всех толстяков
            if (cook_fired || dead_count.load() == 3) {
                finished = true;
                lk.lock();
                stage = 2;
                lk.unlock();
                cv.notify_all();
                break;
            }
        }
        });

    // Потоки толстяков
    vector<thread> fats;
    for (int id = 0; id < 3; ++id) {
        fats.emplace_back([&, id] {
            while (true) {
                unique_lock<mutex> lk(mtx);
                // Ожидаем стадию приема пищи или завершения
                cv.wait(lk, [&] { return stage == 1 || finished; });
                if (finished) break;

                // Если толстяк мертв, пропускаем прием пищи
                if (!alive[id]) {
                    finished_eaters++;
                    if (finished_eaters == 3) {
                        stage = 0;  // Возвращаемся к готовке
                        lk.unlock();
                        cv.notify_all();
                    }
                    else {
                        lk.unlock();
                    }
                    continue;
                }

                // Проверка наличия еды
                int available = plates[id];
                if (available <= 0) {
                    cook_fired = true;
                    finished = true;
                    lk.unlock();
                    cv.notify_all();
                    break;
                }

                // Определяем сколько съесть
                int will_eat = gluttony;
                if (available < will_eat) will_eat = available;

                // Уменьшаем еду на тарелке и увеличиваем счетчик съеденного
                plates[id] -= will_eat;
                eaten[id] += will_eat;

                // Проверка опустения тарелки после еды
                if (plates[id] <= 0) {
                    cook_fired = true;
                    finished = true;
                    lk.unlock();
                    cv.notify_all();
                    break;
                }

                // Проверка смерти от переедания
                if (eaten[id] > 10000) {
                    alive[id] = false;
                    if (dead_count.fetch_add(1) + 1 == 3) {
                        all_fatties_dead = true;
                        finished = true;
                        lk.unlock();
                        cv.notify_all();
                        break;
                    }
                }

                // Увеличиваем счетчик завершивших прием пищи
                finished_eaters++;
                if (finished_eaters == 3) {
                    stage = 0;  // Все поели, возвращаемся к готовке
                    lk.unlock();
                    cv.notify_all();
                }
                else {
                    lk.unlock();
                }

                // Имитация времени приема пищи
                this_thread::sleep_for(1ms);

                // Проверка истечения времени 
                auto now = chrono::steady_clock::now();
                double elapsed = chrono::duration_cast<chrono::duration<double>>(now - start_time).count();
                if (elapsed >= 5.0) {
                    cook_resigned = true;
                    finished = true;
                    cv.notify_all();
                    break;
                }

                // Проверка других условий завершения
                if (cook_fired || dead_count.load() == 3) {
                    finished = true;
                    cv.notify_all();
                    break;
                }
            }
            });
    }

    // Запускаем симуляцию
    {
        lock_guard<mutex> lk(mtx);
    }
    cv.notify_all();

    // Ожидаем завершения симуляции
    while (!finished) {
        this_thread::sleep_for(1ms);
    }

    // Завершаем все потоки
    if (cook.joinable()) cook.join();
    for (auto& t : fats) if (t.joinable()) t.join();

    // Вычисляем общее время выполнения
    auto end_time = chrono::steady_clock::now();
    double elapsed = chrono::duration_cast<chrono::duration<double>>(end_time - start_time).count();

    // Формируем результат
    ScenarioResult res;
    res.efficiency_factor = efficiency_factor;
    res.gluttony = gluttony;
    res.final_plates = plates;
    res.eaten = eaten;
    res.elapsed_seconds = elapsed;

    // Определяем итоговый исход симуляции
    if (cook_fired) {
        res.outcome = "Повар уволен (хотя бы одна тарелка опустела)";
    }
    else if (all_fatties_dead) {
        res.outcome = "Все толстяки умерли => Повар НЕ получил зарплату";
    }
    else if (cook_resigned) {
        res.outcome = "Повар уволился после 5 дней";
    }
    else {
        res.outcome = "Симуляция завершена по неизвестной причине";
    }

    return res;
}

int main() {
    setlocale(LC_ALL, "RU");  
    cout << "Запуск 3 сценариев.\n";

    // Параметры для трех сценариев:
    int ef1 = 3;   // Низкая эффективность
    int gl1 = 2000; // Высокая прожорливость

    int ef2 = 150; // Высокая эффективность  
    int gl2 = 50;  // Низкая прожорливость

    int ef3 = 3;   // Низкая эффективность
    int gl3 = 1;   // Очень низкая прожорливость

    vector<pair<int, int>> scenarios = { {ef1, gl1}, {ef2, gl2}, {ef3, gl3} };

    
    for (size_t i = 0; i < scenarios.size(); ++i) {
        int ef = scenarios[i].first;
        int gl = scenarios[i].second;
        cout << "\n=== Сценарий " << (i + 1) << " ===\n";
        cout << "Коэффициент эффективности = " << ef << ", прожорливость = " << gl << "\n";
        ScenarioResult r = run_scenario(ef, gl);
        cout << "Результат: " << r.outcome << "\n";
        cout << "Затраченное время: " << r.elapsed_seconds << " с" << "\n";
        cout << "Финальное состояние тарелок: [" << r.final_plates[0] << ", " << r.final_plates[1] << ", " << r.final_plates[2] << "]\n";
        cout << "Всего съедено каждым толстяком: [" << r.eaten[0] << ", " << r.eaten[1] << ", " << r.eaten[2] << "]\n";
    }

    cout << "\nЗавершено.\n";
    return 0;
}