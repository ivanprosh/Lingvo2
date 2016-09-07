#ifndef SYNTAXANALIZATOR_H
#define SYNTAXANALIZATOR_H

#include <QDebug>
#include <QStringList>
#include <fstream>
#include <sstream>
#include <QFile>
#include <QStack>
#include <iostream>

//структура ячейки таблицы разбора
struct Action{
    int action;
    int shift;
    int reduce;
    bool halt;
    bool error;
    Action():error(1),action(-1),shift(-1),reduce(-1),halt(0){}
};

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
    int index;
    Tsituation* situation;
    Tstate* next;
    Tstate():next(nullptr){}
    Tstate(int& count):next(nullptr){index=count++;}
};
struct TEdge{
    QString term;
    Tstate* from;
    Tstate* to;
    TEdge* next;
    TEdge():next(nullptr),from(nullptr),to(nullptr){}
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
    TEdge* edge;
    Action** table;
    QStringList keywords,
                grammar,
                nonterm,
                alphavit,
                definitions;
    int statescount;
    QStack<QString> St_magazine;
    QStack<int> St_states;
    QString inputString;

public:
    QFile* outputFile;
    QTextStream* out;
    SyntaxAnalizator(const QString& grammar,const QString& input,const QString& output);
    ~SyntaxAnalizator() {  outputFile->close(); }
    void ProvideStates();
    void addSituation(Tstate* input,QString rule,int index);
    void addedge(Tstate* input,QString symb,Tstate* output);
    void graph_update();
    void cross(Tstate* input, QString symb);
    QPair<QString,int> countSymbInRule(int indexrule);
    Tstate* SituationState(Tstate* inputstate,Tsituation* inputsit);
    QVector<int> SituationPosExist(Tsituation* inputSit);
    Tstate* edgeExist(QString term,Tstate* input);

    bool loopStateSingleRound(Tstate* input); //возвращает 1, если было добавлено новое состояние на текущем круге
    Tstate* loopState(Tstate* input);
    QString findnonterm(Tsituation* cursit);
    Tstate* findAlternativeState(const QVector<int> Sitstates,QString symb);
    QPair<QString,QString> getSymb(QString stream,int pos);
    bool isSitExist(Tstate* input,int index);
    void unique(QString word);
    bool ProvideStatesSingleRound();

    void CreateTable();
    void InitTable();
    void AnalyzeTable();
    void ShowTable();
};

#endif // SYNTAXANALIZATOR_H
