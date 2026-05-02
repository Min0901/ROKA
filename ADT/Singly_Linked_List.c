#include <stdio.h>
#include <stdlib.h>

typedef int DataType;

typedef struct node
{
    DataType Data;
    struct node* NextNode;
}Node;

Node* CreateNode(DataType NewData){
    Node* NewNode=(Node*)malloc(sizeof(Node));
    NewNode->Data=NewData;
    NewNode->NextNode=NULL;

    return NewNode;
}//노드 생성

void DestroyNode(Node* Node){
    free(Node);
}//노드 소멸

void AppendNode(Node** Head,Node* NewNode){
    if((*Head)==NULL){
        (*Head)=NewNode;
    }
    else{
        Node* Tail=(*Head);
        while(Tail->NextNode!=NULL){
            Tail=Tail->NextNode;
        }
        Tail->NextNode=NewNode;
    }
}//노드 추가

void InsertNode(Node* Current,Node* NewNode){
    NewNode->NextNode=Current->NextNode;
    Current->NextNode=NewNode;
}//노드 삽입

void RemoveNode(Node** Head, Node* Remove){
    if((*Head)==Remove){
        (*Head)=Remove->NextNode;
    }
    else{
        Node* Current=*Head;
        while(Current->NextNode!=Remove){
            Current=Current->NextNode;
        }
        Current->NextNode=Remove->NextNode;
    }
}//노드 삭제

Node* GetNodeAt(Node* Head, int Location){
    Node* Current=Head;
    while(Current!=NULL&&(--Location)>=0){
        Current=Current->NextNode;
    }

    return Current;
}//노드 탐색

int GetNodeCount(Node* Head){
    int cnt=1;
    Node* Current=Head;

    while(Current->NextNode!=NULL){
        Current=Current->NextNode;
        cnt++;
    }

    return cnt;
}//노드 개수 세기

void InsertBefore(Node** Head, Node* Current, Node* NewHead){
    if(Current==(*Head)){
        NewHead->NextNode=Current;
        (*Head)=NewHead;
    }
    else{
        Node* Some=(*Head);
        while(Some->NextNode!=Current){
            Some=Some->NextNode;
        }
        Some->NextNode=NewHead;
        NewHead->NextNode=Current;
    }
}

void DestroyAllNodes(Node** List){
    int i;
    for(i=0;i<GetNodeCount(*List)-1;i++){
        free(GetNodeAt(*List,i));
    }
}

int main(void){
    Node* List=NULL;
    Node* NewNode=CreateNode(96);
    AppendNode(&List,NewNode);
    NewNode=CreateNode(564);
    AppendNode(&List,NewNode);
    NewNode=CreateNode(22222);
    InsertBefore(&List,GetNodeAt(List,1),NewNode);
    int Count=GetNodeCount(List);
    Node* Node1=GetNodeAt(List,0);
    Node* Node2=GetNodeAt(List,1);
    Node* Node3=GetNodeAt(List,2);
    printf("%d, %d, %d, %d\n",Node1->Data,Node2->Data,Node3->Data,Count);
    DestroyAllNodes(&List);
    return 0;
}