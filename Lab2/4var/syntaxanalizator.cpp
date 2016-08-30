#include "syntaxanalizator.h"

SyntaxAnalizator::SyntaxAnalizator(const QString& input)
{
    keywords
    << "typedef" << "struct" << "int" << "double" << "float"
    << "bool"    << "char"   << "signed int" << "unsigned int";

    QFile file(input);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        grammar << line;
    }
    for(int i=0;i < grammar.size();i++) {
        grammar[i].remove(QRegExp("[\n]"));
        nonterm << grammar.at(i).left(grammar.at(i).indexOf(QRegExp("[\\s*\:]")));
    }
    nonterm.removeDuplicates();

    ProvideStates();
}

void SyntaxAnalizator::ProvideStates()
{
    int curStrNum = 1;
    states = new Tstate;
    //новая ситуация
    states->situation = new Tsituation;
    states->situation->pos = curStrNum*10+0;
    states->situation->term = "$";
    states->situation->next = nullptr;

    int pos = grammar.at(0).indexOf(QRegExp("[\\s*\:]"));
    states->situation->rule.left = grammar.at(0).left(pos);
    states->situation->rule.right.term = grammar.at(0).right(grammar.at(0).size()-pos).section("|",0,0).remove(QRegExp("[\:]|[\\s*]"));
    states->situation->rule.right.next = 0;

    loopState(states);
}

Tstate SyntaxAnalizator::loopState(Tstate* input)
{
    Tstate* curState = input;
    while(curState)
    {
        qDebug() << isnonterm(input->situation);
        curState = curState->next;
    }
    return *input;
}

bool SyntaxAnalizator::isnonterm(Tsituation* sit)
{
    Tsituation* cursit = sit;
    TList* rightrule = &cursit->rule.right;

    while(cursit){
        foreach (QString str, nonterm) {
            while(rightrule){
                if(rightrule->term.indexOf(str)==0) return 1;
                rightrule = rightrule->next;
            }
        }
        cursit = cursit->next;
    }
    return 0;
}
