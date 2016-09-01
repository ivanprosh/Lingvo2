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

    alphavit << nonterm << keywords << "\\d" << "\\w" << "," << "." << " " << ";" << "{" << "}" << "$";
    //qDebug() << nonterm;

    ProvideStates();
}
bool SyntaxAnalizator::notExistState(Tsituation* input)
{
    int pos = input->pos+input->rule.right.term.size();
    Tstate* curstate = states;
    Tsituation* cursit;

    //цикл по всем состояниям и по всем ситуациям
    while(curstate){
        cursit = curstate->situation;
        while(cursit){
            if(cursit->pos==pos) {
                //qDebug() << "State with rule "<< input->rule.left + ":" + input->rule.right.term <<"already exist!";
                return 0;
            }
            cursit = cursit->next;
        }
        curstate = curstate->next;
    }
    return 1;
}

void SyntaxAnalizator::cross(Tstate* input, QString symb)
{
    Tstate* startstate = input;
    Tsituation* cursit = input->situation;
    while(cursit){
        //если найден символ среди ситуаций
        if((symb == cursit->rule.right.term) && notExistState(cursit)){
            qDebug() << "Add new state. Current rule:"<< cursit->rule.left<< ":" << cursit->rule.right.term;

            Tsituation* newsit = new Tsituation;
            newsit->rule.left = cursit->rule.left;
            newsit->pos = cursit->pos + cursit->rule.right.term.size();
            QString str = grammar.at(cursit->pos/1000);
            str = str.remove(QRegExp("^.*[\:][\\s*]"));
            //qDebug() << str;
            newsit->rule.right.term = getSymb(str,newsit->pos%1000);
            Tstate* newstate = new Tstate;
            newstate->situation = newsit;
            //замыкание для текущего состояния
            newstate = loopState(newstate);

            while(startstate->next)
            {
               startstate = startstate->next;
            }
            startstate->next = newstate;


        }
        cursit=cursit->next;
    }

}

bool SyntaxAnalizator::ProvideStatesSingleRound()
{
    bool result=0;
    Tstate* curstate = states;

    //цикл по всем состояниям
    while(curstate){
            foreach (QString str, alphavit) {
                cross(curstate,str);
            }
        curstate = curstate->next;
    }
    return result;
}

void SyntaxAnalizator::ProvideStates()
{
    Tsituation* sit;
    int statescount = 0;
    int curStrNum = 0;
    states = new Tstate;
    //states->next = 0;
    //начальная ситуация
    states->situation = new Tsituation;
    states->situation->pos = curStrNum*1000+0;
    //states->situation->term = "";
    int pos = grammar.at(0).indexOf(QRegExp("[\\s*\:]"));
    states->situation->rule.left = grammar.at(0).left(pos);
    states->situation->rule.right.term = grammar.at(0).right(grammar.at(0).size()-pos).section("|",0,0).remove(QRegExp("[\:]|[\\s*]"));
    states->situation->rule.right.next = 0;

    curStrNum++;
    //конечный символ $
    states->situation->next = new Tsituation;
    states->situation->next->pos = curStrNum*1000+0;
    states->situation->next->term = "$";
    pos = grammar.at(1).indexOf(QRegExp("[\\s*\:]"));
    states->situation->next->rule.left = grammar.at(1).left(pos);
    states->situation->next->rule.right.term = grammar.at(1).right(grammar.at(1).size()-pos).section("|",0,0).remove(QRegExp("[\:]|[\\s*]"));
    states->situation->next->rule.right.next = 0;

    try{
        //выполняем начальное замыкание
        states = loopState(states);

        //debug
        sit=states->situation;
        while(sit) {
            statescount++;
            sit = sit->next;
        }
        qDebug() << "sit count" << statescount;
        //для каждого состояния, выполнять пока не будет найдено новых переходов или состояний
        while(ProvideStatesSingleRound());
    }
    catch(SyntaxError err)
    {
        qDebug() << err.descr;
    }
}
QString SyntaxAnalizator::getSymb(QString stream,int pos=0)
{
    QString input_str = stream.right(stream.size()-pos);
    if (input_str.size()==0) return "";
    foreach (QString str, alphavit) {
        if(input_str.indexOf(str)==0) {
            return str;
        }
    }
    throw SyntaxError("ERR: invalid symbol in rule: " + stream + " pos:" + QString::number(pos));
}

void SyntaxAnalizator::addSituation(Tstate* input,QString rule,int index)
{
    int pos = rule.indexOf(QRegExp("[\\s*\:]"));
    Tsituation* last=input->situation;

    Tsituation* newsit = new Tsituation;
    newsit->pos = index*1000+0;
    newsit->rule.left = rule.left(pos);

    newsit->rule.right.term = getSymb(rule.right(rule.size()-pos).remove(QRegExp("^[\\s*][\:][\\s*]")));

    //ищем конец списка ситуаций
    while(last->next){
        last=last->next;
    }
    last->next = newsit;

    qDebug() << "New situation add: " << newsit->rule.right.term;
}

bool SyntaxAnalizator::isSitExist(Tstate* input,int index)
{
    Tsituation* cursit = input->situation;
    while(cursit)
    {
        if((cursit->pos)/1000 == index) return 1;
        cursit = cursit->next;
    }
    return 0;
}

bool SyntaxAnalizator::loopStateSingleRound(Tstate* input)
{
    bool result=0;
    Tsituation* cursit = input->situation;
    //Tstate* curState = input;
    QString str;
    while(cursit)
    {
        str=findnonterm(cursit);
        //qDebug() << str;
        if(str!="0")
        {
            //если нашли точку перед нетерминалом, найдем правило
            for(int i=0;i<grammar.size();++i)
            {
                if(!grammar.at(i).contains(QRegExp(str+"\\s*:"))) continue;
                //qDebug() << grammar.at(i);
                //если нет ситуации с этим правилом, то добавим ее
                if(!isSitExist(input,i)) {
                    addSituation(input,grammar.at(i),i);
                    result = true;
                }
            }
        }
        cursit = cursit->next;
    }
    return result;
}

Tstate* SyntaxAnalizator::loopState(Tstate* input)
{
    while(loopStateSingleRound(input)){};
    return input;
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
