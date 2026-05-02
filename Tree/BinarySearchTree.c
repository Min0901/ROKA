#include <stdio.h>
#include <stdlib.h>

//이진 탐색 트리에서의 이진 탐색, 노드 삽입, 노드 삭제

typedef int DataType;

typedef struct node{
    struct node* Left;
    struct node* Right;

    DataType Data;
}Node;

Node* CreatenNode(DataType NewData){
    Node* NewNode = (Node*)malloc(sizeof(Node));
    NewNode->Left = NULL;
    NewNode->Right = NULL;
    NewNode->Data = NewData;

    return NewNode;
}

void DestroyNode(Node* Node){
    free(Node);
}

void DestroyTree(Node* Tree){
    if(Tree->Right != NULL)
        DestroyTree(Tree->Right);
    
    if(Tree->Left != NULL)
        DestroyTree(Tree->Left);

    Tree->Left = NULL;
    Tree->Right = NULL;

    DestroyNode(Tree);
}

Node* SearchNode(Node* Tree, DataType Target){
    if(Tree == NULL)
        return NULL;

    if(Tree->Data == Target)
        return Tree;
    else if(Tree->Data > Target)
        return SearchNode(Tree->Left, Target);
    else
        return SearchNode(Tree->Right, Target);
}

Node* SearchMinNode(Node* Tree){
    if(Tree == NULL)
        return NULL;
    
    if(Tree->Left != NULL)
        return SearchMinNode(Tree->Left);
    else
        return Tree;
}

void InsertNode(Node* Tree, Node* Child){
    if(Tree->Data < Child->Data){
        if(Tree->Right == NULL)
            Tree->Right = Child;
        else
            InsertNode(Tree->Right, Child);
    }else if(Tree->Data > Child->Data){
        if(Tree->Left == NULL)
            Tree->Left = Child;
        else
            InsertNode(Tree->Left, Child);
    }
}

Node* RemoveNode(Node* Tree, Node* Parent, DataType Target){
    Node* Removed = NULL;

    if(Tree == NULL)
        return NULL;

    if(Tree->Data > Target)
        Removed = RemoveNode(Tree->Left, Tree, Target);
    else if(Tree->Data < Target)
        Removed = RemoveNode(Tree->Right, Tree, Target);
    else{
        Removed = Tree;

        if(Tree->Left == NULL && Tree->Right == NULL){
            if(Parent->Left == Tree)
                Parent->Left = NULL;
            else
                Parent->Right = NULL;
        }//잎 노드인 경우
        else{
            if(Tree->Left != NULL && Tree->Left != NULL){
                Node* MinNode = SearchMinNode(Tree->Right);
                MinNode = RemoveNode(Tree, NULL, MinNode->Data);
                Tree->Data = MinNode->Data;
            }
            else{
                Node* Temp = NULL;
                if(Tree->Left != NULL)
                    Temp = Tree->Left;
                else
                    Temp = Tree->Right;

                if(Parent->Left == Tree)
                    Parent->Left = Temp;
                else
                    Parent->Right = Temp;
            }
        }//자식이 있는 경우
    }
    return Removed;
}

void InorderPrintTree(Node* Node){
    if(Node == NULL)
        return;
    
    InorderPrintTree(Node->Left);

    printf("%d\n",Node->Data);

    InorderPrintTree(Node->Right);
}

void PrintSearchResult(int SearchTarget, Node* Result){
    if(Result != NULL)
        printf("Found: %d \n",Result->Data);
    else
        printf("Not Found: %d \n",SearchTarget);
}

int main(void){
    Node* Tree = CreatenNode(123);
    Node* Node = NULL;

    InsertNode(Tree, CreatenNode(22));
    InsertNode(Tree, CreatenNode(9918));
    InsertNode(Tree, CreatenNode(5534));
    InsertNode(Tree, CreatenNode(24567));
    InsertNode(Tree, CreatenNode(24743));
    InsertNode(Tree, CreatenNode(4576423));

    int SearchTarget = 24567;
    Node = SearchNode(Tree, SearchTarget);
    PrintSearchResult(SearchTarget, Node);

    InorderPrintTree(Tree);

    DestroyTree(Tree);

    return 0;
}