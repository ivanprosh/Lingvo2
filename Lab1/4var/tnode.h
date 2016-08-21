#ifndef TNODE_H
#define TNODE_H
#include <QString>

class Node
{
public:
    Node* left;
    Node* right;
    QString oper,code;
    int level;
    Node():left(nullptr),right(nullptr){}
    ~Node() { if(left) delete left; if(right) delete right;}
    friend class Tree;

};

class Tree
{
    //char num,maxnum;		//счётчик тегов и максимальный тег
    int maxrow, offset;		//максимальная глубина, смещение корня
    int currdepth; //глубина фактическая
    char **SCREEN;	// память для выдачи на экран
    void clrscr();	// очистка рабочей памяти

    //конструкторы
    Tree(const Tree&);
    Tree(Tree&&);
    Tree operator = (const Tree&) const;
    Tree operator = (Tree&&) const;
    //методы

    void OutNodes(Node * v, int r, int c); // выдача поддерева
    void innerAlg(Node * v,int depth, int* count);
public:
    Node* root;
    Tree();
    ~Tree();
    Node* MakeNode(); // создание поддерева
    bool exist() { return root != 0; } // проверка «дерево не пусто»
    //int CountLastLeafs();	// внутренний обход дерева (подсчет листьев на нижнем уровне)
    void OutTree();	// выдача на экран

};

#endif // TNODE_H
