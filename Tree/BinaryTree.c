#include <stdio.h>
#include <stdlib.h>

typedef char DataType;

typedef struct node{
    struct node* Left;
    struct node* Right;

    DataType Data;
}Node;

Node* CreateNode(DataType NewData){
    Node* NewNode = (Node*)malloc(sizeof(Node));
    NewNode->Left = NULL;
    NewNode->Right = NULL;
    NewNode->Data = NewData;

    return NewNode;
}

void DestroyNode(Node* Node){
    free(Node);
}

void DestroyTree(Node* Node){
    if(Node == NULL)
        return;
    
    DestroyTree(Node->Left);
    DestroyTree(Node->Right);
    DestroyNode(Node);
}

void PreorderPrintTree(Node* Node, int Depth){
    if(Node = NULL)
        return;

    printf("%c",Node->Data);
    PreorderPrintTree(Node->Left,Depth+1);
    PreorderPrintTree(Node->Right,Depth);
}