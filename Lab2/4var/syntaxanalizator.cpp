#include "syntaxanalizator.h"

SyntaxAnalizator::SyntaxAnalizator(const QString& input):statescount(0)
{
    keywords
    << "typedef" << "struct" << "int" << "double" << "float"
    << "bool"    << "char"   << "signed int" << "unsigned int"
    << "," << "." << " " << ";" << "{" << "}" << "$";

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

    alphavit << nonterm << keywords << "\\d" << "\\w";
    //qDebug() << nonterm;

    edge = new TEdge;

    ProvideStates();
}
//проверка отсутствия состояния с данной ситуацией
bool SyntaxAnalizator::notExistState(Tsituation* input)
{
    int pos = input->pos+input->rule.right.term.size();
    if(input->rule.actions.term.size()) pos+=input->rule.actions.term.size()+2;
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
void SyntaxAnalizator::addedge(Tstate* input,QString symb,Tstate* output)
{
    TEdge* curedge = edge;
    if(curedge->from){
        while(curedge->next) {
            if((curedge->from == input) && (curedge->to == output) && (curedge->term==symb)) return;
            curedge = curedge->next;
        }
        curedge->next = new TEdge;
        curedge = curedge->next;
    }
    curedge->from = input;
    curedge->to = output;
    curedge->term = symb;
}

void SyntaxAnalizator::cross(Tstate* input, QString symb)
{
    Tstate* startstate = input;
    Tsituation* cursit = input->situation;
    while(cursit){
        //если найден символ среди ситуаций
        if((symb == cursit->rule.right.term) && notExistState(cursit)){
            Tsituation* newsit = new Tsituation;
            newsit->rule.left = cursit->rule.left;
            newsit->pos = cursit->pos + cursit->rule.right.term.size();
            if(cursit->rule.actions.term.size()) newsit->pos += cursit->rule.actions.term.size()+2;
            QString str = grammar.at(cursit->pos/1000);
            str = str.remove(QRegExp("^.*[\:][\\s*]"));
            //qDebug() << str;
            QPair<QString,QString> pair = getSymb(str,newsit->pos%1000);
            newsit->rule.right.term = pair.first;
            newsit->rule.actions.term = pair.second;
            Tstate* newstate = new Tstate(statescount);
            newstate->situation = newsit;
            //замыкание для текущего состояния

            qDebug() << "Add new state. Current rule:"<< newsit->rule.left<< ":" << newsit->rule.right.term;

            addedge(input,symb,newstate);
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
Tstate* SyntaxAnalizator::edgeExist(QString term,Tstate* input)
{
    TEdge* cureedge=edge;
    while(cureedge){
        if(cureedge->from==input && cureedge->term==term) return cureedge->to;
        cureedge=cureedge->next;
    }
    return nullptr;
}
//проерка уникальности имён
void SyntaxAnalizator::unique()
{
    definitions<<St_magazine.top();
    if(definitions.removeDuplicates()) throw(SyntaxError("ERR: duplicate definitions "));
}

void SyntaxAnalizator::AnalyzeTable()
{
    //добавляем начальное состояние в стек
    St_states.push(states->index);
    QString test = "struct a {int v;float asd,a,b,c}";
    //пока есть символы во входной цепочке
    while(test.size())
    {
        QString curSymb;
        int pos=0;
        //анализируем остаток цепочки на наличие ключевых слов в начале
        foreach (QString str, keywords) {
            if(test.indexOf(str)==0) {
                pos = alphavit.indexOf(str);
                test = test.remove(0,str.size());
                curSymb = str;
                qDebug() << test;
            }
        }
        //если не нашли среди ключевых слов, проверяем на наличие цифры/буквы
        if(pos==0){
            QRegExp Definition("([A-z_]+\\d*)(.*)");
            if(Definition.exactMatch(test)) {
                pos=alphavit.indexOf("\\w");
                test = test.remove(0,Definition.cap(1).size());
                qDebug() << "Reg: " << Definition.cap(1);
                curSymb = Definition.cap(1);
            }
            else {
                throw(SyntaxError("Error for analize: " + test));
            }
        }
        if(table[St_states.top(),pos]->action==1) unique();
        if(table[St_states.top(),pos]->shift){
            St_states.push(table[St_states.top(),pos]->shift);
            St_magazine.push(curSymb);
        }
        else if(table[St_states.top(),pos]->reduce){

        }
    }
}

void SyntaxAnalizator::InitTable()
{
    Tstate* curstate=states;
    Tsituation* cursit;
    while(curstate){
        cursit=curstate->situation;
        while(cursit){
            if(cursit->rule.right.term!=""){
                Tstate* output = edgeExist(cursit->rule.right.term,curstate);
                //если есть переход в графе и правило не в конце цепочки
                if(output){
                    Action* curAction = &table[curstate->index][alphavit.indexOf(cursit->rule.right.term)];
                    curAction->error = 0;
                    curAction->shift = output->index;
                    curAction->action = cursit->rule.actions.term.toInt();
                }
            }
            else {
                //символ конца строки
                Action* curAction = &table[curstate->index][alphavit.size()];
                curAction->error = false;
                if(cursit->rule.left == "RESULT"){
                    curAction->halt = true;
                } else {
                    curAction->reduce = (cursit->pos)/1000;
                }
            }
            cursit=cursit->next;
        }
        curstate=curstate->next;
    }

}

void SyntaxAnalizator::CreateTable()
{
    table = new Action*[statescount];
    for(int i=0;i<statescount;i++)
        table[i] = new Action[alphavit.size()+1];
}

void SyntaxAnalizator::ProvideStates()
{
    int curStrNum = 0;
    states = new Tstate(statescount);

    //начальная ситуация
    states->situation = new Tsituation;
    states->situation->pos = curStrNum*1000+0;
    //states->situation->term = "";
    int pos = grammar.at(0).indexOf(QRegExp("[\\s*\:]"));
    states->situation->rule.left = grammar.at(0).left(pos);
    states->situation->rule.right.term = grammar.at(0).right(grammar.at(0).size()-pos).section("|",0,0).remove(QRegExp("[\:]|[\\s*]"));
    states->situation->rule.right.next = 0;
    /*
    curStrNum++;
    //конечный символ $
    states->situation->next = new Tsituation;
    states->situation->next->pos = curStrNum*1000+0;
    states->situation->next->term = "$";
    pos = grammar.at(1).indexOf(QRegExp("[\\s*\:]"));
    states->situation->next->rule.left = grammar.at(1).left(pos);
    states->situation->next->rule.right.term = grammar.at(1).right(grammar.at(1).size()-pos).section("|",0,0).remove(QRegExp("[\:]|[\\s*]"));
    states->situation->next->rule.right.next = 0;
    */
    try{
        //выполняем начальное замыкание
        states = loopState(states);

        //для каждого состояния, выполнять пока не будет найдено новых переходов или состояний
        while(ProvideStatesSingleRound());
//вывод на экран состояний
        int count=0;
        Tstate* curstate = states;
        Tsituation* cursit;
        TEdge* curedge=edge;
        QStringList st;
        while(curstate){
            qDebug() << ++count << " State:";
            cursit=curstate->situation;
            while(cursit){
                qDebug() << "pos: "<< cursit->pos << ", rule: " << cursit->rule.left << ":" << cursit->rule.right.term;
                cursit = cursit->next;
            }
            st<<QString(QString::number(int(curstate)));
            curstate = curstate->next;
        }
        qDebug() << "Graph:";
        while(curedge){
            qDebug() << "from " << curedge->from->index << " to " << curedge->to->index << " term" << curedge->term;
            curedge = curedge->next;
        }
        qDebug() << "States count: " << statescount;
//конец промежуточного вывода
        CreateTable();
        InitTable();
        AnalyzeTable();
    }
    catch(SyntaxError err)
    {
        qDebug() << err.descr;
    }
}
//получить пару символ-действие
QPair<QString,QString> SyntaxAnalizator::getSymb(QString stream,int pos=0)
{
    QPair<QString,QString> result;
    QRegExp Raction("<(\\d+)>(.*)");
    QString input_str = stream.right(stream.size()-pos);

    qDebug() << "Input string" << input_str;
    if(Raction.exactMatch(input_str)){
        //qDebug() << "Regular action is" << Raction.cap(1);
        result.second = Raction.cap(1);
        input_str.remove(0,result.second.size()+2);
    }
    qDebug() << "Input string" << input_str;

    if (input_str.size()==0) {
        result.first = "";
        return result;
    }
    foreach (QString str, alphavit) {
        if(input_str.indexOf(str)==0) {
            result.first = str;
            break;
        }
    }
    if(!result.first.isEmpty()){
        return result;
    }
    else {
        throw SyntaxError("ERR: invalid symbol in rule: " + stream + " pos:" + QString::number(pos));
    }
}

void SyntaxAnalizator::addSituation(Tstate* input,QString rule,int index)
{
    int pos = rule.indexOf(QRegExp("[\\s*\:]"));
    Tsituation* last=input->situation;

    Tsituation* newsit = new Tsituation;
    newsit->pos = index*1000+0;
    newsit->rule.left = rule.left(pos);
    QPair<QString,QString> pair = getSymb(rule.right(rule.size()-pos).remove(QRegExp("^[\\s*][\:][\\s*]")));
    newsit->rule.right.term = pair.first;
    newsit->rule.actions.term = pair.second;

    //ищем конец списка ситуаций
    while(last->next){
        last=last->next;
    }
    last->next = newsit;

    qDebug() << "Index is " << index << ", new situation add: " << newsit->rule.left << ":" << newsit->rule.right.term;

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
        if(str!="0")
        {
            //если нашли точку перед нетерминалом, найдем правило
            for(int i=0;i<grammar.size();++i)
            {
                if(grammar.at(i).contains(QRegExp(str+"\\s*:"))) {
                    //qDebug() << grammar.at(i);
                    //если нет ситуации с этим правилом, то добавим ее
                    if(!isSitExist(input,i)) {
                        addSituation(input,grammar.at(i),i);
                        result = true;
                    }
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
