// <iostream> on the two peripherals: std::cin reads SAPHO/myKeyPS2.v on link
// 0, std::cout writes SAPHO/myVGAText.v on link 1 (the defaults of
// <yanc_term>), a 32-bit C++ SAPHO. A name read with getline (typed with a
// backspace that std::cin edits away), two numbers read with >>, then
// integers, hex and floats as C++ prints them. Built and run by the regress
// project pass VGATEXT (TopLevel/vgatext_tb.v, system C).
#include <iostream>
#include <cstring>
using namespace std;

int main()
{
    char nome[20];
    int a, b;
    cout << "Nome? ";
    cin.getline(nome, 20);
    cout << "Oi, " << nome << '!' << endl;
    cout << "a b? ";
    cin >> a >> b;
    cout << a << " + " << b << " = " << a + b << endl;
    cout << -7 << ' ' << hex << 255 << dec << ' ' << 1.5f * a << ' '
         << 3.14159265f << ' ' << 1e10f << ' ' << 0.00015f << endl;
    cout << nome << " tem " << (int)strlen(nome) << " letras" << endl;
    return 0;
}
