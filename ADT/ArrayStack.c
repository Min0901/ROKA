#include <stdio.h>
#include <stdlib.h>

typedef int DataType;

typedef struct node{
    DataType Data;
}Node;

typedef struct arrstack{
    int Capacity;
    int Top;
    Node* Nodes;
}ArrayStack;

void CreateStack(ArrayStack** Stack, int Capacity){
    (*Stack)=(ArrayStack*)malloc(sizeof(ArrayStack));
    (*Stack)->Nodes=(Node*)malloc(sizeof(Node)*Capacity);

    (*Stack)->Capacity=Capacity;
    (*Stack)->Top=-1;
}

void DestroyStack(ArrayStack* Stack){
    free(Stack->Nodes);
    free(Stack);
}

void Push(ArrayStack* Stack, DataType Data){
    Stack->Top++;
    Stack->Nodes[Stack->Top].Data=Data;
}

DataType Pop(ArrayStack* Stack){
    int Position=Stack->Top--;
    return Stack->Nodes[Position].Data;
}

DataType Top(ArrayStack* Stack){
    return Stack->Nodes[Stack->Top].Data;
}

int GetSize(ArrayStack* Stack){
    return Stack->Top+1;
}

int IsEmpty(ArrayStack* Stack){
    return (Stack->Top==-1);
}

int IsFull(ArrayStack* Stack){
    return ((Stack->Top)+1==(Stack->Capacity));
}

int main(void){
    ArrayStack* Stack=NULL;

    CreateStack(&Stack,4);

    Push(Stack,30);
    Push(Stack,36);
    Push(Stack,2003);
    Push(Stack,2005);

    printf("is full? %c\n",IsFull(Stack)?'Y':'N');

    for(int i=0;i<4;i++){
        if(IsEmpty(Stack))
            break;
        
        printf("Popped: %d, ",Pop(Stack));

        if(!IsEmpty(Stack))
            printf("Current Top: %d\n",Top(Stack));
        else
            printf("Stack Is Empty.\n");
    }

        printf("is full? %c\n",IsFull(Stack)?'Y':'N');
}