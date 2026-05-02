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
        (*Head)->PrevNode= *Head;
        (*Head)->NextNode= *Head;
    }
    else{
        Node* Tail=(*Head)->PrevNode;

        Tail->NextNode->PrevNode=NewNode;
        Tail->NextNode=NewNode;

        NewNode->NextNode=(*Head);
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
        (*Head)->PrevNode=Remove->PrevNode;
        Remove->PrevNode->NextNode=(*Head);

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

int main(void){
    int   i       =0;
    Node* List    =NULL;
    Node* NewNode =NULL;
    Node* Current =NULL;
    for(i=0;i<5;i++){
        NewNode=CreateNode(i);
        AppendNode(&List,NewNode);
    }
    
    for(i=0;i<15;i++){
        if(i==0)
            Current=List;
        else
            Current=Current->NextNode;
        
        printf("%d\n",Current->Data);
    }

    for(i=0;i<5;i++){
        Current=GetNodeAt(List,0);
        if(Current!=NULL){
            RemoveNode(&List,Current);
            DestroyNode(Current);
        }
    }
    return 0;
}