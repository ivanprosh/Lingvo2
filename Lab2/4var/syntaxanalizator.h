#ifndef SYNTAXANALIZATOR_H
#define SYNTAXANALIZATOR_H

#include <QDebug>
#include <QStringList>
#include <fstream>
#include <sstream>
#include <QFile>

struct TList{
    QString term;
    TList* next;
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
};
struct Tstate{
    Tsituation* situation;
    Tstate* next;
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
    bool isnonterm(Tsituation* cursit);
};

#endif // SYNTAXANALIZATOR_H
