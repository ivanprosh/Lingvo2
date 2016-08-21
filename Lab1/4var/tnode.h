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
    //char num,maxnum;		//������� ����� � ������������ ���
    int maxrow, offset;		//������������ �������, �������� �����
    int currdepth; //������� �����������
    char **SCREEN;	// ������ ��� ������ �� �����
    void clrscr();	// ������� ������� ������

    //������������
    Tree(const Tree&);
    Tree(Tree&&);
    Tree operator = (const Tree&) const;
    Tree operator = (Tree&&) const;
    //������

    void OutNodes(Node * v, int r, int c); // ������ ���������
    void innerAlg(Node * v,int depth, int* count);
public:
    Node* root;
    Tree();
    ~Tree();
    Node* MakeNode(); // �������� ���������
    bool exist() { return root != 0; } // �������� ������� �� �����
    //int CountLastLeafs();	// ���������� ����� ������ (������� ������� �� ������ ������)
    void OutTree();	// ������ �� �����

};

#endif // TNODE_H
