#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void BuildExpressionTree(char* PostfixExpression, Node** Node){
    int len = strlen(PostfixExpression);
    char Token = PostfixExpression[len-1];//store last letter
    PostfixExpression[len-1] = '\0';//remove last letter in string

    switch(Token){
        case '+': case '-': case '*': case '/'://if operator, read -> operator or number
            (*Node) = CreateNode(Token);//build root node
            BuildExpressionTree(PostfixExpression,&(*Node)->Right);//read1
            BuildExpressionTree(PostfixExpression,&(*Node)->Left);//read2
            break;
        
        default:
            (*Node) = CreateNode(Token);
            break;
    }
}//build tree

double Evaluate(Node* Tree){
    char Temp[2];

    double Left = 0;
    double Right = 0;
    double Result = 0;

    if(Tree == NULL)
        return 0;
    
    switch(Tree->Data){
        case '+': case '-': case '*': case '/':
            Left = Evaluate(Tree->Left);
            Right = Evaluate(Tree->Right);

                 if(Tree->Data == '+') Result = Left + Right;
            else if(Tree->Data == '-') Result = Left - Right;
            else if(Tree->Data == '*') Result = Left * Right;
            else if(Tree->Data == '/') Result = Left / Right;

            break;
        
        default:
            memset(Temp,0,sizeof(Temp));
            Temp[0] = Tree->Data;
            Result = atof(Temp);
            break;
    }

    return Result;
}