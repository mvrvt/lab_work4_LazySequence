#include <iostream>
#include <cassert>
#include <string>

// Подключаем наши классы
#include "../src/lab2_files/ArraySequence.hpp"
#include "../src/LazySequence.hpp"
#include "../src/Stream.hpp"
#include "../src/StateMachine.hpp"

using namespace std;

int FibonacciRule(const Sequence<int>* cache) {
    int len = cache->GetLength();
    if (len == 0) return 0;
    if (len == 1) return 1;
    return cache->Get(len - 1) + cache->Get(len - 2);
}

void TestLazySequence() {
    cout << "[TEST] Running LazySequence Tests..." << endl;
    LazySequence<int> fib(FibonacciRule, nullptr);
    assert(fib.GetMaterializedCount() == 0);
    assert(fib.Get(10) == 55);
    assert(fib.GetMaterializedCount() == 11);
    assert(fib.Get(4) == 3);
    assert(fib.GetMaterializedCount() == 11);
    cout << "  -> LazySequence Tests Passed!" << endl;
}

void TestStream() {
    cout << "[TEST] Running Stream Tests..." << endl;
    int data[] = {10, 20, 30, 40};
    Sequence<int>* seq = new MutableArraySequence<int>(data, 4);
    ReadOnlyStream<int> stream(seq);
    bool exception_thrown = false;
    try { stream.Read(); } catch (const logic_error&) { exception_thrown = true; }
    assert(exception_thrown);
    stream.Open();
    assert(stream.GetPosition() == 0);
    assert(stream.Read() == 10);
    assert(stream.Read() == 20);
    assert(stream.GetPosition() == 2);
    assert(stream.CanSeek() == true);
    stream.Seek(0);
    assert(stream.Read() == 10);
    stream.Close();
    delete seq;
    cout << "  -> Stream Tests Passed!" << endl;
}

void TestStateMachine() {
    cout << "[TEST] Running State Machine Tests..." << endl;
    fsm::StateMachine<char> machine;
    machine.AddState("START", false);
    machine.AddState("BIN_READING", true);
    machine.SetInitialState("START");
    machine.AddTransition("START", "BIN_READING", [](const char& c) { return c == '0' || c == '1'; });
    machine.AddTransition("BIN_READING", "BIN_READING", [](const char& c) { return c == '0' || c == '1'; });
    
    char valid_data[] = {'1', '0', '1'};
    Sequence<char>* valid_seq = new MutableArraySequence<char>(valid_data, 3);
    assert(machine.Process(valid_seq) == true);
    delete valid_seq;

    char invalid_data[] = {'1', '0', '2', '1'};
    Sequence<char>* invalid_seq = new MutableArraySequence<char>(invalid_data, 4);
    assert(machine.Process(invalid_seq) == false); 
    delete invalid_seq;

    cout << "  -> State Machine Tests Passed!" << endl;
}

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
