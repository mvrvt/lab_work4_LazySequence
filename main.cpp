#include <iostream>
#include <cassert>
#include <string>

// Подключаем наши классы
#include "src/lab2_files/ArraySequence.hpp"
#include "src/LazySequence.hpp"
#include "src/Stream.hpp"
#include "src/StateMachine.hpp"

using namespace std;

// ==========================================
// ПРАВИЛО ДЛЯ ЛЕНИВОГО СПИСКА (Фибоначчи)
// ==========================================
int FibonacciRule(const Sequence<int>* cache) {
    int len = cache->GetLength();
    if (len == 0) return 0;
    if (len == 1) return 1;
    // Складываем два предыдущих элемента
    return cache->Get(len - 1) + cache->Get(len - 2);
}

// ==========================================
// ТЕСТ 1: Ленивая последовательность
// ==========================================
void TestLazySequence() {
    cout << "[TEST] Running LazySequence Tests..." << endl;
    
    LazySequence<int> fib(FibonacciRule, nullptr);

    // До обращения к элементам кэш должен быть пуст
    assert(fib.GetMaterializedCount() == 0);

    // Запрашиваем 11-й элемент (индекс 10). Должно быть число 55.
    assert(fib.Get(10) == 55);

    // Теперь в кэше должно быть ровно 11 вычисленных элементов (индексы от 0 до 10)
    assert(fib.GetMaterializedCount() == 11);

    // Если мы запросим 5-й элемент (индекс 4), он достанется из кэша.
    // Количество вычисленных элементов не должно измениться.
    assert(fib.Get(4) == 3);
    assert(fib.GetMaterializedCount() == 11);

    cout << "  -> LazySequence Tests Passed!" << endl;
}

// ==========================================
// ТЕСТ 2: Потоки (Streams)
// ==========================================
void TestStream() {
    cout << "[TEST] Running Stream Tests..." << endl;

    // Создаем обычный массив для имитации данных
    int data[] = {10, 20, 30, 40};
    Sequence<int>* seq = new MutableArraySequence<int>(data, 4);

    ReadOnlyStream<int> stream(seq);

    // До открытия должно выбрасываться исключение
    bool exception_thrown = false;
    try { stream.Read(); } catch (const logic_error&) { exception_thrown = true; }
    assert(exception_thrown);

    stream.Open();
    assert(stream.GetPosition() == 0);
    
    // Читаем первые два элемента
    assert(stream.Read() == 10);
    assert(stream.Read() == 20);
    assert(stream.GetPosition() == 2);

    // Проверяем возможность Seek
    assert(stream.CanSeek() == true);
    stream.Seek(0);
    assert(stream.Read() == 10); // Снова 10, так как мы вернулись в начало

    stream.Close();
    delete seq;

    cout << "  -> Stream Tests Passed!" << endl;
}

// ==========================================
// ТЕСТ 3: Машина состояний (FSM)
// ==========================================
void TestStateMachine() {
    cout << "[TEST] Running State Machine Tests..." << endl;

    fsm::StateMachine<char> machine;

    // Автомат, проверяющий, что строка состоит только из '0' и '1'
    machine.AddState("START", false);
    machine.AddState("BIN_READING", true); // Конечное состояние

    machine.SetInitialState("START");

    // Если в START пришел 0 или 1, переходим в BIN_READING
    machine.AddTransition("START", "BIN_READING", [](const char& c) { return c == '0' || c == '1'; });
    
    // Если в BIN_READING пришел 0 или 1, остаемся в BIN_READING
    machine.AddTransition("BIN_READING", "BIN_READING", [](const char& c) { return c == '0' || c == '1'; });

    // Тест 1: Валидная строка "101"
    char valid_data[] = {'1', '0', '1'};
    Sequence<char>* valid_seq = new MutableArraySequence<char>(valid_data, 3);
    assert(machine.Process(valid_seq) == true);
    delete valid_seq;

    // Тест 2: Невалидная строка "1021"
    char invalid_data[] = {'1', '0', '2', '1'};
    Sequence<char>* invalid_seq = new MutableArraySequence<char>(invalid_data, 4);
    assert(machine.Process(invalid_seq) == false); // '2' вызовет остановку автомата
    delete invalid_seq;

    cout << "  -> State Machine Tests Passed!" << endl;
}

// ==========================================
// ГЛАВНАЯ ФУНКЦИЯ
// ==========================================
int main() {
    cout << "========================================" << endl;
    cout << " Starting Automated Tests..." << endl;
    cout << "========================================" << endl;

    TestLazySequence();
    TestStream();
    TestStateMachine();

    cout << "========================================" << endl;
    cout << " ALL TESTS PASSED SUCCESSFULLY! " << endl;
    cout << "========================================" << endl;

    return 0;
}
