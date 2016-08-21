#include <iostream>
#include <string>
//#include <map>
#include <cctype>
#include <fstream>
#include <sstream>
#include <conio.h>
#include <QRegExp>
#include <QStringList>
#include <QDebug>
#include <QtAlgorithms>
#include <QLinkedList>
#include "tnode.h"

using namespace std;
int no_of_errors;

Tree SyntaxTree;

std::filebuf FILE_input,FILE_output;
const string output_name = "output.txt";
const string input_name = "input.txt";

QString Rfloat ="(?:\\d+\\.\\d+|\\d+)(?:[\E\e][+-]?\\d+)?";
QString Rint ="\\d+";
QString Roper ="[+-]|[\*/=\(\)]";
QString RVar = "[\\w]+";
QRegExp RegFloat(Rfloat),RegInt(Rint),RegOper(Roper),RegVar(RVar);


int error(const string &s)
{
    no_of_errors++;
    cerr<<" :-: FAIL:"<<s<<'\n';
    return 1;
}
bool beetweenArrows(QVector<QString>::iterator goal,QVector<QString>::iterator first,QVector<QString>::iterator last)
{
    QVector<QString>::iterator index1=qFind(first, last, "(");
    QVector<QString>::iterator index21=qFind(first, last, ")");
    QVector<QString>::iterator index22=index21;
    while(index21!=last)
    {
        index22=index21;
        index21=qFind(index21+1, last, ")");
    }
    return (goal>index1 && goal<index22);
}

void fillingTree(Node* curNode,QVector<QString>::iterator first,QVector<QString>::iterator last,int level)
{

    QVector<QString>::iterator index=qFind(first, last, "=");
    if(index==last)
    {
        QVector<QString>::iterator it = first;
        do{
        index=qFind(it++, last, "+");
        } while(beetweenArrows(index,first,last));
    }
    if(index==last)
    {
        QVector<QString>::iterator it = first;
        do{
        index=qFind(it++, last, "-");
        } while(beetweenArrows(index,first,last));
    }
    if(index==last)
    {
        QVector<QString>::iterator it = first;
        do{
        index=qFind(it++, last, "*");
        } while(beetweenArrows(index,first,last));
    }
    if(index!=last){
        curNode->oper = *index;
        curNode->left = SyntaxTree.MakeNode();
        curNode->left->level = level+1;
        fillingTree(curNode->left,first,index-1,level+1);
        curNode->right = SyntaxTree.MakeNode();
        curNode->right->level = level+1;
        fillingTree(curNode->right,index+1,last,level+1);
    }
    else
    {
        if((*first=="(") && (*last==")")) {
           *first=*last="";
           fillingTree(curNode,first+1,last-1,level);
           return;
        }
        curNode->oper = *first;
    }

}
void accumNodes(Node* root,QVector<Node*>& vec)
{
    if(root->left) accumNodes(root->left,vec);
    if(root->right) accumNodes(root->right,vec);
    vec.push_back(root);
}
bool levelLessThen(const Node* n1,const Node* n2)
{
    return (n1->level) > (n2->level);
}

void generateCode(const Tree& SynTree)
{
    QVector<Node*> Nodes;
    QLinkedList<QString> Result;

    Nodes.reserve(200);
    //собираем узлы
    accumNodes(SynTree.root,Nodes);
    //сортируем по уровню
    qSort(Nodes.begin(),Nodes.end(),levelLessThen);

    int n=0;
    for(int it=0;it<(Nodes.size()-1);it++)
    {
        //имеем дело с константами
        if(RegInt.exactMatch(Nodes.at(it)->oper) || RegFloat.exactMatch(Nodes.at(it)->oper))
        {
            Nodes.at(it)->code = "="+Nodes.at(it)->oper;
        }
        //знаками
        else if (RegOper.exactMatch(Nodes.at(it)->oper)) {
            Nodes.at(it)->code = Nodes.at(it)->right->code  + "\n"+//C(n3)
                                     "STORE $" + QString::number(++n) +"\n"+//STORE $l(n)
                                     "LOAD " + Nodes.at(it)->left->code +"\n";//LOAD C(n1)
            if(Nodes.at(it)->oper == "+")
                  Nodes.at(it)->code = Nodes.at(it)->code + "ADD $" + QString::number(n)+ "\n";
            if(Nodes.at(it)->oper == "*")
                  Nodes.at(it)->code = Nodes.at(it)->code + "MPY $" + QString::number(n)+ "\n";

        }
        //переменными
        else if (RegVar.exactMatch(Nodes.at(it)->oper)) {
           Nodes.at(it)->code = Nodes.at(it)->oper;
        }
        //

    }
    //отдельно обработка для корня
    Nodes.last()->code = "LOAD " + (Nodes.last()->right->code) + "STORE " + Nodes.last()->left->oper + "\n";
    Nodes.last()->code.replace(QRegExp("\n\n"),"\n");

}

bool readfile(string& buff)
{
    //поток ввода из файла input.txt
    FILE_input.open("input.txt",std::ios::in);
    std::istream is(&FILE_input);

    while(is)
        is>>buff;

    FILE_input.close();

    return !buff.empty();
}
QString optimize(QString code)
{
    qDebug() << code;

    QRegExp comADD("(.*)\\bSTORE\\s.(\\d)\\n+LOAD ([=]?"+Rfloat+"|"+Rint+"|"+RVar+")\\n+ADD .\\2");
    QRegExp comMPY("(.*)\\bSTORE\\s.(\\d)\\n+LOAD ([=]?"+Rfloat+"|"+Rint+"|"+RVar+")\\n+MPY .\\2");
    QRegExp com("(.*)LOAD ([=]?(?:\\d+\\.\\d+|\\d+)(?:[\E\e][+-]?\\d+)?)\\n+STORE\\s.(\\d+)\\n+LOAD (.*)");

    //QString test = "LOAD =0.98\nMPY $2\nSTORE $2\nLOAD TAX\n";

    comADD.exactMatch(code);
    code.replace(comADD,comADD.cap(1)+"ADD "+comADD.cap(3));

    qDebug() << "after first iter "<< code;

    comMPY.exactMatch(code);
    code.replace(comMPY,comMPY.cap(1)+"MPY "+comMPY.cap(3));

    qDebug() << "after second iter "<< code;

    qDebug() << com.exactMatch(code);
    qDebug() << com.cap(1) << com.cap(2) << com.cap(3) << com.cap(4);
    code.replace(com,com.cap(1)+"LOAD "+com.cap(4));

    qDebug() << "after third iter "<< code;
    //поиск позиции в подстроке до ключевого слова STORE
    QRegExp rfind("(STORE ."+com.cap(3)+")(?:\\s*)");
    int pos=rfind.indexIn(code,pos);

    //если найдена позиция STORE $num
    if(pos!=-1){
        QString right = code.right(code.size()-pos);
        QString left = code.left(pos);
        qDebug() << left << right;
        //заменяем везде до STORE $num
        left.replace(QRegExp("(MPY|ADD) (."+com.cap(3)+")"),"\\1 "+ com.cap(2));
        code = left+right;
    }//если не найдена, то меняем везде
    else
    {
        code.replace(QRegExp("(MPY|ADD) (."+com.cap(3)+")"),"\\1 "+ com.cap(2));
    }

    return code;

}

int main(int argc, char* argv[])
{   
    QVector<QString> Symbols;

    //поток вывода в файл output.txt
    FILE_output.open ("output.txt",std::ios::out);
    std::ostream os(&FILE_output);

    string readingfile;
    if(!readfile(readingfile)) {
        error("file is empty!");
        return no_of_errors;
    }
    QString str = QString::fromStdString(readingfile);

    qDebug()<<str;
    //QRegExp rxlen("(\\d+|(?:\\d+\\.\\d+|\\d+)(?:[\E\e][+-]?\\d+)?|[+-]|[\*/=\(\)]|[\\w]+)(?:\\s*)");

    QRegExp rxlen("("+Rint+"|"+Rfloat+"|"+Roper+"|"+RVar+")"+"(?:\\s*)");

    os << "Таблица символов: \n";
    int pos(0),it(0);
    while ((pos=rxlen.indexIn(str,pos))!=-1) {
        it=1;
        while(rxlen.cap(it)!="")
        {
             Symbols.push_back(rxlen.cap(it++));
             os.width(10);
             //os<<Symbols.last().toStdString()<<"    :";
             if(RegInt.exactMatch(Symbols.last()))
                os << Symbols.last().toStdString()<<"    : const integer";
             else if (RegFloat.exactMatch(Symbols.last())) {
                os << Symbols.last().toStdString()<<"    : const float";
             }
             else if (RegVar.exactMatch(Symbols.last())) {
                os << Symbols.last().toStdString()<<"    : variable";
             }
             else if (RegOper.exactMatch(Symbols.last())) {
                continue;
             }
             else
               os << Symbols.last().toStdString()<<"    : unknown type";
             os << endl;
        }
        pos+=rxlen.matchedLength();
    }

    fillingTree(SyntaxTree.root,Symbols.begin(),Symbols.end(),0);
    generateCode(SyntaxTree);

    os << "Неоптимизированный код: \n";
    os << SyntaxTree.root->code.toStdString();
    os << "\n";

    os << "Оптимизированный код: \n";
    os << optimize(SyntaxTree.root->code).toStdString();

    FILE_output.close();
    return no_of_errors;
}

