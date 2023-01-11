// Решите загадку: Сколько чисел от 1 до 1000 содержат как минимум одну цифру 3?
// Напишите ответ здесь: 271
#include <iostream>
#include <string>
#include <algorithm>


int main()
{
    const int N = 1000;
    int counter = 0;
    for (int i = 1; i < N; ++i) {   //brute-force
        std::string number(std::to_string(i));
        if (std::count(number.begin(), number.end(), '3'))
            ++counter;
    }
    std::cout << counter;
    return 0;
}
// Закомитьте изменения и отправьте их в свой репозиторий.
