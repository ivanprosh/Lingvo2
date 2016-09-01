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

class SyntaxError
{
public:
    QString descr;
    SyntaxError(QString str):descr(str){}
};

class SyntaxAnalizator
{
    Tstate* states;
    QStringList keywords,
                grammar,
                nonterm,
                alphavit;
public:  
    SyntaxAnalizator(const QString& input);
    void ProvideStates();
    void addSituation(Tstate* input,QString rule,int index);
    void cross(Tstate* input, QString symb);
    bool notExistState(Tsituation* input);

    bool loopStateSingleRound(Tstate* input); //возвращает 1, если было добавлено новое состояние на текущем круге
    Tstate* loopState(Tstate* input);
    QString findnonterm(Tsituation* cursit);
    QString getSymb(QString stream,int pos);
    bool isSitExist(Tstate* input,int index);
    bool SyntaxAnalizator::ProvideStatesSingleRound();
};

#endif // SYNTAXANALIZATOR_H
