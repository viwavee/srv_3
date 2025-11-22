#include <iostream>
#include <thread>
#include <mutex>
#include <locale>   

using namespace std;

int coins = 100;      // общее количество монет 
int Bob_coins = 0;    // монеты Боба
int Tom_coins = 0;    // монеты Тома

int desired_remaining = 0; // сколько монет должно остаться в мешке (0 или 1)

mutex m; // мьютекс для синхронизации потоков

// Функция делёжки монет между ворами
void coin_sharing(string name, int& thief_coins, int& companion_coins) {

    while (true) {
        lock_guard<mutex> lock(m); // захватываем мьютекс

        // Если монет в мешке меньше или равно нужному оставшемуся количеству — прекращаем
        if (coins <= desired_remaining)
            break;

        // Вор берёт монету, если у него монет меньше или столько же, как у напарника
        if (thief_coins <= companion_coins) {
            coins--;        // берем одну монету из мешка
            thief_coins++;  // кладём в карман вора

            // Вывод текущего состояния
            cout << name << ": " << thief_coins << " | "
                << "Подельник: " << companion_coins
                << " | Осталось: " << coins << endl;
        }
        // Если условие взятия не выполнено, просто отпускаем мьютекс и цикл повторится,
        
    }
}

int main() {
    setlocale(LC_ALL, ""); 

    // вычисляем, сколько должно остаться монет в конце:
    // если начальное кол-во нечётное то оставляем 1, иначе 0
    desired_remaining = coins % 2; // 1 для нечётного, 0 для чётного

    // Создание потоков для Боба и Тома
    thread Bob(coin_sharing, "Bob", ref(Bob_coins), ref(Tom_coins));
    thread Tom(coin_sharing, "Tom", ref(Tom_coins), ref(Bob_coins));

    // Ожидание завершения потоков
    Bob.join();
    Tom.join();

    // Итоговый вывод
    cout << "\nИтоги:" << endl;
    cout << "Bob: " << Bob_coins << endl;
    cout << "Tom: " << Tom_coins << endl;
    cout << "Покойник: " << coins << endl;

    return 0;
}
