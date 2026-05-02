#include <stdio.h>
#include <stdlib.h>

typedef int DataType;

typedef struct node{
    DataType Data;
    struct node* PrevNode;
    struct node* NextNode;
}Node;

Node* CreateNode(DataType NewData){
    Node* NewNode=(Node*)malloc(sizeof(Node));

    NewNode->Data=NewData;
    NewNode->PrevNode=NULL;
    NewNode->NextNode=NULL;

    return NewNode;
}//노드 생성

void DestroyNode(Node* Node){
    free(Node);
}//노드 삭제

void AppendNode(Node** Head, Node* NewNode){
    if((*Head)==NULL){
        *Head=NewNode;
    }
    else{
        Node* Tail=(*Head);
        while(Tail->NextNode!=NULL){
            Tail=Tail->NextNode;
        }
        Tail->NextNode=NewNode;
        NewNode->PrevNode=Tail;
    }
}//노드 추가

Node* GetNodeAt(Node* Head, int Location){
    Node* Current=Head;
    while(Current!=NULL && (--Location)>=0){
        Current=Current->NextNode;    
    }

    return Current;
}//노드 탐색

void RemoveNode(Node** Head, Node* Remove){
    if((*Head)==Remove){
        (*Head)=Remove->NextNode;
        if((*Head)!=NULL)
            (*Head)->PrevNode=NULL;

        Remove->PrevNode=NULL;
        Remove->NextNode=NULL;
    }
    else{
        Node* Temp=Remove;
        if(Remove->PrevNode!=NULL)
            (Remove->PrevNode)->NextNode=Temp->NextNode;
        if(Remove->NextNode!=NULL)
            (Remove->NextNode)->PrevNode=Temp->PrevNode;

        Remove->PrevNode=NULL;
        Remove->NextNode=NULL;
    }
}//노드 삭제

void InsertNode(Node* Current, Node* NewNode){
    NewNode->PrevNode=Current;
    NewNode->NextNode=Current->NextNode;
    if(Current->NextNode!=NULL){
        Current->NextNode->PrevNode=NewNode;
        Current->NextNode=NewNode;
    }
}//노드 삽입

int NodeCount(Node* Head){
    unsigned int Cnt=0;
    Node* Current=Head;
    while(Current!=NULL){
        Current=Current->NextNode;
        Cnt++;
    }
    return Cnt;
}//노드 개수 세기

void PrintReverse(Node* Head){
    Node* Current=Head;
    while(Current->NextNode!=NULL){
        Current=Current->NextNode;
    }
    while(Current!=NULL){
        printf("%d\n",Current->Data);
        Current=Current->PrevNode;
    }
}

int main(void){
    Node* List=NULL;
    Node* NewNode=NULL;

    NewNode=CreateNode(123);
    AppendNode(&List,NewNode);
    NewNode=CreateNode(234);
    AppendNode(&List,NewNode);
    NewNode=CreateNode(345);
    AppendNode(&List,NewNode);
    NewNode=CreateNode(456);
    AppendNode(&List,NewNode);
    NewNode=CreateNode(567);
    AppendNode(&List,NewNode);
    PrintReverse(List);
    return 0;
}