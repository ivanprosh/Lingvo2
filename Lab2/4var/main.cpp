#include <iostream>
#include <string>
//#include <map>
#include <cctype>
#include <conio.h>
#include <QRegExp>
#include <QDebug>
#include "syntaxanalizator.h"

using namespace std;
int no_of_errors;

const QString output_name = "output.txt";
const QString grammar_name = "grammar.txt";

int main()
{
    SyntaxAnalizator Analizator(grammar_name);

    return 0;
}
