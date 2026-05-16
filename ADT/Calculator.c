#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

//Calculator Start
typedef enum{
    LEFT_PARENTHESIS='(',RIGHT_PARENTHESIS=')',
    PLUS    ='+', MINUS   ='-',
    MULTIPLY='*', DIVIDE  ='/',
    SPACE   =' ', OPERAND
}SYMBOL;

int             IsNumber(char Cipher);
unsigned int    GetNextToken(char* Expression, char* Token, int* TYPE);
int             IsPrior(char Operator1, char Operator2);
void            GetPostfix(char* InfixExpression, char* PostfixExpression);
double          Calculate(char* PostfixExpression);

char NUMBER[]={'0','1','2','3','4','5','6','7','8','9','.'};

int IsNumber(char Cipher){
    int i=0;
    int ArrayLength=sizeof(NUMBER);

    for(i=0;i<ArrayLength;i++){
        if(Cipher==NUMBER[i])
            return 1;
    }
}//피연산자(숫자)인가?

unsigned int GetNextToken(char* Expression, char* Token, int* TYPE){
    unsigned int i=0;

    for(i=0;0!=Expression[i];i++){
        Token[i]=Expression[i];

        if(IsNumber(Expression[i])==1){
            *TYPE = OPERAND;

            if(IsNumber(Expression[i+1])!=1)
                break;
        }//숫자라면 type에 피연산자 넣기, 다음 문자가 숫자가 아니라면 break(for문 탈출).
        else{
            *TYPE=Expression[i];
            break;
        }//연산자라면 type에 연산자 넣기, for문 탈출.
    }//다음 문자가 숫자인 경우에만 for문 진행.

    Token[++i]='\0';
    return i;//문자열 길이 반환.
}

int GetPriority(char Operator, int InStack){
    int Priority=-1;

    switch(Operator){
    case LEFT_PARENTHESIS:
        if(InStack)
            Priority=3;
        else
            Priority=0;
        break;
    
    case MULTIPLY:
    case DIVIDE:
        Priority=1;
        break;
    
    case PLUS:
    case MINUS:
        Priority=2;
        break;
    }
    return Priority;
}

int IsPrior(char operatorInStack, char OperatorInToken){
    return (GetPriority(operatorInStack,1)>GetPriority(OperatorInToken,0));
}

void GetPostfix(char* InfixExpression, char* PostfixExpression){
    LLStack* Stack;

    char Token[32];
    int Type=-1;
    unsigned int Position=0;
    unsigned int Length=strlen(InfixExpression);

    while(Position<Length){
        Position += GetNextToken
    }
}