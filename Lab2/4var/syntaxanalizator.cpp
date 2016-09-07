#include "syntaxanalizator.h"

SyntaxAnalizator::SyntaxAnalizator(const QString& grammarFileName,const QString& inputFileName,const QString& outputFileName):statescount(0),edge(nullptr)
{
    outputFile = new QFile(outputFileName);
    out = new QTextStream(outputFile);

    keywords
    << "typedef" << "struct" << "int" << "double" << "float"
    << "bool"    << "char"
    << "," << "." << " " << ";" << "{" << "}" << "$";

    //считываем данные входной строки
    //outputfile(outputFileName);
    if (!outputFile->open(QIODevice::WriteOnly | QIODevice::Text))
        qWarning() << "Cann't open output file!";
    //out(&outputfile);
    //считываем данные грамматики
    QFile grammarFile(grammarFileName);
    if (!grammarFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        *out << "Cann't open grammar file!";
        return;
    }

    while (!grammarFile.atEnd()) {
        QByteArray line = grammarFile.readLine();
        grammar << line;
    }
    for(int i=0;i < grammar.size();i++) {
        grammar[i].remove(QRegExp("[\n]"));
        nonterm << grammar.at(i).left(grammar.at(i).indexOf(QRegExp("[\\s*\:]")));
    }
    nonterm.removeDuplicates();

    alphavit << nonterm << keywords << "\\w";

    //считываем данные входной строки
    QByteArray in;
    QFile inputFile(inputFileName);
    if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        *out << "Cann't open input file!";
        return;
    }

    while (!inputFile.atEnd()) {
       in = inputFile.readLine();
    }
    inputString=in;

    inputFile.close();
    grammarFile.close();

    ProvideStates();
}
//функция формирует вектор состояний, в которые включена ситуация со сдвинутой вправо позицией
QVector<int> SyntaxAnalizator::SituationPosExist(Tsituation* inputSit)
{
    QVector<int> situations;
    int pos = inputSit->pos+inputSit->rule.right.term.size();
    if(inputSit->rule.actions.term.size()) pos+=inputSit->rule.actions.term.size()+2;

    Tstate* curstate = states;
    Tsituation* cursit;

    //цикл по всем состояниям и по всем ситуациям
    while(curstate){
        cursit = curstate->situation;
        while(cursit){
            //если совпали координаты ситуации
            if((cursit->pos==pos)) {
                situations.push_back(curstate->index);
            }
            cursit = cursit->next;
        }
        curstate = curstate->next;
    }
    return situations;

}
//проверка отсутствия состояния с данной ситуацией
Tstate* SyntaxAnalizator::SituationState(Tstate* inputstate,Tsituation* inputSit)
{
    TEdge* curEdge= edge;

    //если один и тот же символ встретился для одного и того же состояния
    while(curEdge)
    {
        //отдельно проверяем для входного состояния
        if((curEdge->from==inputstate) && curEdge->term==inputSit->rule.right.term ) return curEdge->to;
        curEdge = curEdge->next;
    }
    return nullptr;
}

Tstate* SyntaxAnalizator::findAlternativeState(const QVector<int> Sitstates,QString symb)
{
    TEdge* curEdge= edge;
    Tstate* curstate;
    //если один и тот же символ встретился для одного и того же состояния
    while(curEdge)
    {
        foreach (int index, Sitstates) {
            curstate = states;
            while(curstate){
                if(curstate->index==index) break;
                curstate = curstate->next;
            }
            if((curEdge->to==curstate ) && curEdge->term==symb ) return curEdge->to;
        }
        curEdge = curEdge->next;
    }

    return nullptr;
}

void SyntaxAnalizator::addedge(Tstate* input,QString symb,Tstate* output)
{
    if(!edge){
        edge = new TEdge;
        edge->from = input;
        edge->to = output;
        edge->term = symb;
    } else {
        TEdge* curedge = edge;
        while(curedge) {
            if((curedge->from == input) && (curedge->to == output) && (curedge->term==symb)) return;
            curedge = curedge->next;
        }
        curedge = edge;
        while(curedge->next) curedge=curedge->next;

        curedge->next = new TEdge;
        curedge->next->from = input;
        curedge->next->to = output;
        curedge->next->term = symb;
    }
}

void SyntaxAnalizator::cross(Tstate* input, QString symb)
{
    Tstate* startstate = input;
    Tstate* check;
    Tsituation* cursit = input->situation;
    while(cursit){
        //если найден символ среди ситуаций
        if((symb == cursit->rule.right.term)){
            if(input->index==26) {
                QString str;
            }
            QVector<int> DublicateStatesSit;
            DublicateStatesSit = SituationPosExist(cursit);
            //если в текущем состоянии нет рассматриваемой ситуации
            if(DublicateStatesSit.indexOf(input->index)==-1){
                //состояния с текущими координатами не существует, создаем
                Tsituation* newsit = new Tsituation;
                newsit->rule.left = cursit->rule.left;
                newsit->pos = cursit->pos + cursit->rule.right.term.size();

                if(cursit->rule.actions.term.size()) newsit->pos += cursit->rule.actions.term.size()+2;

                QString str = grammar.at(cursit->pos/1000);
                str = str.remove(QRegExp("^.*[\:][\\s*]"));
                //qDebug() << str;
                QPair<QString,QString> pair = getSymb(str,newsit->pos%1000);
                if(pair.second != ""){
                    QString srt;
                }
                newsit->rule.right.term = pair.first;
                newsit->rule.actions.term = pair.second;

                //проверка, есть ли в графе переход по данному символу из текущего состояния
                check = SituationState(input,cursit);
                if(!check){
                    Tstate* newstate = findAlternativeState(DublicateStatesSit,symb);
                    //если нет уже состояния, в который осуществляется подобный переход
                    if(!newstate){
                        newstate = new Tstate(statescount);
                        qDebug() << "Add new state. Current rule:"<< newsit->rule.left<< ":" << newsit->rule.right.term;
                        newstate->situation = newsit;
                        //замыкание для текущего состояния

                        newstate = loopState(newstate);
                        addedge(input,symb,newstate);

                        while(startstate->next)
                        {
                            startstate = startstate->next;
                        }
                        startstate->next = newstate;
                    } else {
                        input = loopState(input);
                        addedge(input,symb,newstate);
                    }

                } else {
                    //если есть состояние в состав которого необходимо включить данную ситуацию
                    Tsituation* sit=check->situation;
                    while(sit->next)
                        sit=sit->next;
                    sit->next = newsit;
                    check = loopState(check);
                }
            }
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
//проверка уникальности имён
void SyntaxAnalizator::unique(QString word)
{
    definitions<<word;
    if(definitions.removeDuplicates()) throw(SyntaxError("ERR: duplicate definitions "));
}
//количество символов в правиле
QPair<QString,int> SyntaxAnalizator::countSymbInRule(int indexrule)
{
    QPair<QString,int> result;
    result.second = 0;
    QVector<int> positions;
    Tstate* curstate=states;
    Tsituation* cursit;
    //цикл по всем состояниям и по всем ситуациям
    while(curstate){
        cursit = curstate->situation;
        while(cursit){
            if((cursit->pos)/1000==indexrule) {
                //если уже было состояние с этими координатами - не учитываем
                if(!positions.contains(cursit->pos)) {
                    positions.push_back(cursit->pos);
                    result.second++;
                    result.first = cursit->rule.left;
                }
            }
            cursit = cursit->next;
        }
        curstate = curstate->next;
    }
    //символ конца строки не учитываем
    result.second--;
    return result;
}

void SyntaxAnalizator::AnalyzeTable()
{
    QString word;
    //добавляем начальное состояние в стек
    St_states.push(states->index);
    qDebug() << "Init action ";
    //пока есть символы во входной цепочке
    while(!table[St_states.top()][alphavit.size()].halt)
    {
        int removesize = 0;
        QString curSymb;
        int pos=0;
        //анализируем остаток цепочки на наличие ключевых слов в начале
        foreach (QString str, keywords) {
            if(inputString.indexOf(str)==0) {
                pos = alphavit.indexOf(str);
                removesize = str.size();
                //input = input.remove(0,str.size());
                curSymb = str;
                qDebug() << "Start analyze - Keyword: "<< str << " State index" << St_states.top();
            }
        }
        //если не нашли среди ключевых слов, проверяем на наличие цифры/буквы
        if(pos==0){
            QRegExp Definition("([A-z_]+\\d*)(.*)");
            if(Definition.exactMatch(inputString)) {
                pos=alphavit.indexOf("\\w");
                removesize = Definition.cap(1).size();
                //input = input.remove(0,Definition.cap(1).size());
                word = Definition.cap(1);
                qDebug() << "cursymb word: " << word;
                curSymb = "\\w";
            }
            else {
                if(!inputString.isEmpty()) throw(SyntaxError("Error for analize: " + inputString));
            }
        }

        if(table[St_states.top()][pos].action==1 || table[St_states.top()][alphavit.size()].action==1) unique(word);
        if(table[St_states.top()][pos].shift>0){
            St_states.push(table[St_states.top()][pos].shift);
            St_magazine.push(curSymb);
            inputString = inputString.remove(0,removesize);
        }
        else if(table[St_states.top()][alphavit.size()].reduce>0){
            QPair<QString,int> ReduceRule = countSymbInRule(table[St_states.top()][alphavit.size()].reduce);
            qDebug() << "Reduce count " << ReduceRule.second;
            for(int i=0;i<ReduceRule.second;i++){
                St_magazine.pop();
                St_states.pop();
            }
            St_magazine.push(ReduceRule.first);
            St_states.push(table[St_states.top()][alphavit.indexOf(ReduceRule.first)].shift);
        }
        else if(table[St_states.top()][pos].error) throw(SyntaxError("Error for analize: Not correct symbol in " + inputString));

        qDebug() << "After analyze - Magazine: " << St_magazine.top() << " States: " << St_states.top();
        qDebug() << inputString;
    }
    St_states.pop();
    St_magazine.pop();
    if(St_states.top()==0 && St_magazine.empty()) *out << "Success!";
    else *out << "Error: Stack isn't empty ";

}
void SyntaxAnalizator::ShowTable()
{
    QFile file;
    file.setFileName("output.txt");
    file.open(QIODevice::ReadWrite | QIODevice::Text);
    QTextStream out(&file);

    out<<"\t";
    for(int j=0;j<alphavit.size();j++)
    {
       out<< alphavit.at(j);
       int countTab = alphavit.at(j).size()/4;
       if(countTab>2) out << "\t";
       else if(countTab>1) out << "\t" << "\t";
       else out << "\t" << "\t" << "\t";
    }
    out<<endl;
    //out.setFieldWidth(0);
    for(int i=0;i<statescount;i++)
    {
        out << i << "\t" << ":";
        for(int j=0;j<=alphavit.size();j++)
        {
            out << "\t" << "\t" << "\t" <<j;

            if(table[i][j].shift>0) out<<"S"<<table[i][j].shift;
            if(table[i][j].reduce>0) out<<"R"<<table[i][j].reduce;
            if(table[i][j].action>0) out<<"A"<<table[i][j].action;
            if(table[i][j].halt>0) out<<"H";
            if(table[i][j].error) out <<"-";

        }
        out << endl;
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
                    //Action* curAction = table[curstate->index][alphavit.indexOf(cursit->rule.right.term)];
                    table[curstate->index][alphavit.indexOf(cursit->rule.right.term)].error = 0;
                    //qDebug() << "Output ind. " << output->index << "alphavit.indexOf " << alphavit.indexOf(cursit->rule.right.term);
                    table[curstate->index][alphavit.indexOf(cursit->rule.right.term)].shift = output->index;
                    if(cursit->rule.actions.term!="") {
                        //qDebug() << "!Act is " << cursit->rule.actions.term;
                        table[curstate->index][alphavit.indexOf(cursit->rule.right.term)].action = cursit->rule.actions.term.toInt();
                    }
                    //qDebug() << "Row " << curstate->index << " Column" << alphavit.indexOf(cursit->rule.right.term);
                }
            }
            else {
                //символ конца строки
                //Action* curAction = table[curstate->index][alphavit.size()];
                table[curstate->index][alphavit.size()].error = false;
                if(cursit->rule.left == "RESULT"){
                    table[curstate->index][alphavit.size()].halt = true;
                } else {
                    table[curstate->index][alphavit.size()].reduce = (cursit->pos)/1000;
                }
                if(cursit->rule.actions.term!="") {
                    //qDebug() << "!Act is " << cursit->rule.actions.term;
                    table[curstate->index][alphavit.size()].action = cursit->rule.actions.term.toInt();
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
    qDebug() << "statescount " << statescount;

    //начальная ситуация
    states->situation = new Tsituation;
    states->situation->pos = curStrNum*1000+0;
    int pos = grammar.at(0).indexOf(QRegExp("[\\s*\:]"));
    states->situation->rule.left = grammar.at(0).left(pos);
    states->situation->rule.right.term = grammar.at(0).right(grammar.at(0).size()-pos).section("|",0,0).remove(QRegExp("[\:]|[\\s*]"));
    states->situation->rule.right.next = 0;

    try{
        //выполняем начальное замыкание
        states = loopState(states);

        //для каждого состояния, выполнять пока не будет найдено новых переходов или состояний
        while(ProvideStatesSingleRound());

        //
//вывод на экран состояний
        int count=0;
        Tstate* curstate = states;
        Tsituation* cursit;
        TEdge* curedge=edge;
        QStringList st;
        while(curstate){
            qDebug() << count++ << " State:";
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
        //ShowTable();
        AnalyzeTable();
    }
    catch(SyntaxError err)
    {
        *out << err.descr;
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

    if(input->index==7){
        QString str;
    }
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
