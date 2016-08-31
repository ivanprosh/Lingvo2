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

    //qDebug() << nonterm;

    ProvideStates();
}

void SyntaxAnalizator::ProvideStates()
{
    int curStrNum = 1;
    states = new Tstate;
    states->next = 0;
    //новая ситуация
    states->situation = new Tsituation;
    states->situation->pos = curStrNum*10+0;
    states->situation->term = "$";
    states->situation->next = nullptr;

    int pos = grammar.at(0).indexOf(QRegExp("[\\s*\:]"));
    states->situation->rule.left = grammar.at(0).left(pos);
    states->situation->rule.right.term = grammar.at(0).right(grammar.at(0).size()-pos).section("|",0,0).remove(QRegExp("[\:]|[\\s*]"));
    states->situation->rule.right.next = 0;

    //qDebug() << states->situation->rule.right.term;

    loopState(states);
}

void addSituation(Tstate* input,QString rule,int index)
{
    int pos = rule.indexOf(QRegExp("[\\s*\:]"));
    states->situation->pos = curStrNum*10+0;
    states->situation->rule.left = grammar.at(0).left(pos);
    states->situation->rule.right.term = grammar.at(0).right(grammar.at(0).size()-pos).section("|",0,0).remove(QRegExp("[\:]|[\\s*]"));
    states->situation->rule.right.next = 0;
}

Tstate SyntaxAnalizator::loopState(Tstate* input)
{
    Tsituation* cursit = input->situation;
    //Tstate* curState = input;
    QString str;
    while(cursit)
    {
        str=findnonterm(cursit);
        qDebug() << str;
        if(str!="0")
        {
            //если нашли точку перед нетерминалом, найдем правило
            for(int i=0;i<grammar.size();++i)
            {
                if(!grammar.at(i).contains(QRegExp(str+"\\s*:"))) continue;
                qDebug() << grammar.at(i);
                addSituation(input,grammar.at(i),i);
            }
        }
        cursit = cursit->next;
    }
    return *input;
}

QString SyntaxAnalizator::findnonterm(Tsituation* sit)
{
    Tsituation* cursit = sit;
    TList* rightrule = &cursit->rule.right;

    while(cursit){
        foreach (QString str, nonterm) {
            while(rightrule){
                if(rightrule->term.indexOf(str)==0 && !rightrule->marked) {
                    rightrule->marked = 1;
                    return str;
                }
                rightrule = rightrule->next;
            }
            rightrule = &cursit->rule.right;
        }
        cursit = cursit->next;
    }
    return "0";
}
