#include <stdio.h>
#include <stdlib.h>

typedef char DataType;

typedef struct node{
    struct node* Parent;
    struct node* Left;
    struct node* Right;

    enum{RED, BLACK} Color;

    DataType Data;
}Node;

void RotateRight(Node** Root, Node* Parent){
    Node* LeftChild = Parent->Left;

    Parent->Left = LeftChild->Right;

    //if(LeftChild->Right != Nil)
        LeftChild->Right->Parent = Parent;
    
    LeftChild->Parent = Parent->Parent;

    if(Parent->Parent == NULL)
        (*Root) = LeftChild;
    else
    {
        if(Parent == Parent->Parent->Left)
            Parent->Parent->Left = LeftChild;
        else
            Parent->Parent->Right = LeftChild;
    }

    LeftChild->Right = Parent;
    Parent->Parent = LeftChild;
}