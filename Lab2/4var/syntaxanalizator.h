#ifndef SYNTAXANALIZATOR_H
#define SYNTAXANALIZATOR_H

#include <QDebug>
#include <QStringList>
#include <fstream>
#include <sstream>
#include <QFile>

struct TList{
    QString term;
    bool marked;
    TList* next;
    TList():marked(0),next(nullptr),term(""){}
};
struct TRule{
    QString left;
    TList right;
    TList actions;
    TList start;
};
struct Tsituation{
    TRule rule;
    int pos;
    QString term;
    Tsituation *next;
    Tsituation():next(nullptr),pos(0),term(""){}
};
struct Tstate{
    Tsituation* situation;
    Tstate* next;
    Tstate():next(nullptr){}
};

class SyntaxAnalizator
{
    Tstate* states;
    QStringList keywords,
                grammar,
                nonterm;
public:  
    SyntaxAnalizator(const QString& input);
    void ProvideStates();
    Tstate loopState(Tstate* input);
    QString findnonterm(Tsituation* cursit);
};

#endif // SYNTAXANALIZATOR_H
