#include <stdio.h>
#include <stdlib.h>


typedef struct _tagNode{
    int iData;
    struct _tagNode* pNextNode;
}tNode;

typedef struct _tagList{
    tNode* pHeadNode;
    int iCount;
}tLinkedList;

void InitList(tLinkedList* List){
    List->iCount = 0;
    List->pHeadNode = NULL;
}

void PushBack(tLinkedList* List, int _iData){
    tNode* pNode = (tNode*)malloc(sizeof(tNode));

    if(0 == List->iCount){
        List->pHeadNode = pNode;
    }

    tNode* temp = List->pHeadNode;
    for(int i=0;i<List->iCount;++i){
        temp->pNextNode = pNode;
    }
    //혹은 while(List->pHeadNode) => ()안이 그자체로 null인지 확인하는 역할 수행


    ++List->iCount;
    pNode->iData = _iData;
    pNode->pNextNode = NULL;
}

void ReleaseList(tLinkedList* List){
    tNode* temp = List->pHeadNode;
    tNode* destroy = NULL;

    for(int i=0;i<List->iCount;++i){
        destroy = temp;
        temp = temp->pNextNode;
        free(destroy);
    }

}

void PushFront(tLinkedList* List, int _iData){
    tNode* pNode = (tNode*)malloc(sizeof(tNode));
    List->pHeadNode = pNode;
    pNode->pNextNode = List->pHeadNode->pNextNode;
    pNode->iData = _iData;
    ++List->iCount;
}

int main(){
    tLinkedList List = {};
    InitList(&List);

    PushBack(&List,100);
    PushBack(&List,200);
    PushBack(&List,300);

    tNode* pNode = List.pHeadNode;

    for(int i=0;i<List.iCount;++i){
        printf("%d\n",pNode->iData);
        pNode = pNode->pNextNode;
    }
    ReleaseList(&List);

    return 0;
}