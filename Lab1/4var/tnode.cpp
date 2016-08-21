#include <iostream>
#include "tnode.h"

using namespace std;

Tree::Tree(): maxrow(20),offset(40), currdepth(0), root(nullptr), SCREEN(new char * [maxrow])
{
    for(int i = 0; i < maxrow; i++) SCREEN[ i ] = new char[80];
    root = MakeNode();
}
Tree :: ~Tree()
{
    for(int i = 0; i < maxrow; i++) delete [ ]SCREEN[i];
    delete [ ]SCREEN;
    delete root;
}

Node * Tree :: MakeNode()
{
    return new Node;
}
void Tree :: clrscr()
{
    for(int i = 0; i < maxrow; i++)
    memset(SCREEN[i], '.', 80);
}

void Tree :: OutNodes(Node * v, int r, int c)
{
    if (r && c && (c<80)) SCREEN[ r - 1 ][ c - 1 ] = v->oper[0].toLatin1(); // вывод метки
    if (r < maxrow) {
    if (v->left) OutNodes(v->left, r + 1, c - (offset >> r)); //левый сын
    // if (v->mdl) OutNode(v->mdl, r + 1, c);	— средний сын (если нужно)
    if (v->right) OutNodes(v->right, r + 1, c + (offset >> r)); //правый сын
    }
}


void Tree :: OutTree()
{
    clrscr();
    OutNodes(root, 1, offset);
    for (int i = 0; i < maxrow; i++)
    {
        SCREEN[ i ][ 79 ] = 0;
        cout << "\n" << SCREEN[ i ];
    }
    cout << "\n";
}



