#include <iostream>
#include <string>
#include <map>
#include <cctype>
#include <sstream>
#include <conio.h>
#include <complex>
#include <QRegExp>
#include <QStringList>
#include <QDebug>

using namespace std;
int no_of_errors;

int error(const string &s)
{
    no_of_errors++;
    cerr<<" :-: FAIL:"<<s<<'\n';
    return 1;
}

int main(int argc, char* argv[])
{   
    QStringList value;
    int it = 0;
    QRegExp rxlen("(\\d+[^Ee]|(?:\\d+\\.\\d+|\\d+)(?:[\E\e][+-]?\\d+)?|[+-*/])(?:\\s*)");
    QString str = "5.55 23 43 23 444 2.1E+10 44 523 3.2 5E15 12e-210 7 -13.2 33e 12E10.2";
    int pos=0;

    while ((pos=rxlen.indexIn(str,pos))!=-1) {
        it=1;
        while(rxlen.cap(it)!="")
        qDebug() << rxlen.cap(it++) << endl;

        pos+=rxlen.matchedLength();
    }

    return no_of_errors;
}

