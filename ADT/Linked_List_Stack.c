#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
1. 노드 구조체
2. 스택 구조체
3. 스택 생성/소멸
4. 노드 생성/소멸
5. 노드 삽입(Push)
6. 노드 제거(Pop)
*/


typedef struct node{
    char* Data;
    struct node* NextNode;
}Node;

typedef struct stack{
    Node* List;
    Node* Top;
}LLStack;

void CreateStack(LLStack** Stack){
    (*Stack)=(LLStack*)malloc(sizeof(LLStack));
    (*Stack)->List=NULL;
    (*Stack)->Top=NULL;
}

Node* CreateNode(char* NewData){
    Node* NewNode=(Node*)malloc(sizeof(Node));
    NewNode->Data=(char*)malloc(sizeof(strlen(NewData)+1));

    strcpy(NewNode->Data,NewData);

    NewNode->NextNode=NULL;

    return NewNode;
}

void Push(LLStack* Stack,Node* NewNode){
    if(Stack->List==NULL){
        Stack->List=NewNode;
    }
    else{
        Stack->Top->NextNode=NewNode;
    }
    Stack->Top=NewNode;
}

Node* Pop(LLStack* Stack){
    Node* TopNode=Stack->Top;

    if(Stack->List==Stack->Top){
        Stack->List=NULL;
        Stack->Top=NULL;
    }
    else{
        Node* CurrentNode=Stack->List;
        while(CurrentNode!=NULL && CurrentNode->NextNode!=Stack->Top){
            CurrentNode=CurrentNode->NextNode;
        }
        Stack->Top=CurrentNode;
        Stack->Top->NextNode=NULL;
    }
    return TopNode;
}

int isEmpty(LLStack* Stack){
    return (Stack->List==NULL);
}

Node* Top(LLStack* Stack){
    return Stack->Top;
}

int main(void){
    int i=0;

    Node* Popped;

    LLStack* Stack;

    CreateStack(&Stack);
    Push(Stack,CreateNode("Hello guys"));
    Push(Stack,CreateNode("I'm in NewYorkCity"));
    Push(Stack,CreateNode("Lemonade"));

    for(i=0;i<3;i++){
        if(isEmpty(Stack))
            break;
        
        Popped=Pop(Stack);

        printf("Popped: %s, ",Popped->Data);
        free(Popped->Data);
        free(Popped);

        if(!isEmpty(Stack)){
            printf("Current Top: %s\n",Stack->Top->Data);
        }
        else{
            printf("Stack is Empty\n");
        }
    }
    while((Stack->List)!=NULL){
        Node* Popp=Pop(Stack);
        free(Popp->Data);
        free(Popp);
    }
    free(Stack);
    return 0;
}