#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

typedef int DataType;

typedef struct node{
    DataType Data;
}Node;

typedef struct Heap{
    Node* Nodes;
    int Capacity;
    int UsedSize;
}Heap;

Heap* HeapCreate(int InitialSize){
    Heap* NewHeap = (Heap*)malloc(sizeof(Heap));
    NewHeap->Capacity = InitialSize;
    NewHeap->UsedSize = 0;
    NewHeap->Nodes = (Node*)malloc(sizeof(Node)*NewHeap->Capacity);

    printf("size : %ld\n", sizeof(Node));

    return NewHeap;
}

void HeapDestroy(Heap* H){
    free(H->Nodes);
    free(H);
}

int GetParent(int Index){
    return (int)((Index-1)/2);
}

int GetLeftChild(int Index){
    return (2*Index)+1;
}

void SwapNodes(Heap* H, int Index1, int Index2){
    int CopySize = sizeof(Node);
    Node* Temp = (Node*)malloc(CopySize);

    memcpy(Temp, &H->Nodes[Index1], CopySize);
    memcpy(&H->Nodes[Index1], &H->Nodes[Index2], CopySize);
    memcpy(&H->Nodes[Index2], Temp, CopySize);

    free(Temp);
}

void Insert(Heap* H, DataType NewData){
    int CurrentPosition = H->UsedSize; //usedsize는 배열에 있는 요소의 수이기 때문에 Node[currentposition]은 말단에 있는 노드의 다음 노드를 가리킨다.
    int ParentPosition = GetParent(CurrentPosition);

    if(H->Capacity == H->UsedSize){
        H->Capacity *= 2;
        H->Nodes = (Node*)realloc(H->Nodes, sizeof(Node) * H->Capacity);
    }//heap이 꽉 차면 용량 2배로 늘리기.

    H->Nodes[CurrentPosition].Data = NewData;

    while(CurrentPosition > 0 && H->Nodes[CurrentPosition].Data < H->Nodes[ParentPosition].Data){
        SwapNodes(H, CurrentPosition, ParentPosition);

        CurrentPosition = ParentPosition;
        ParentPosition = GetParent(CurrentPosition);
    }

    H->UsedSize++;
}

void DeleteMin(Heap* H, Node* Root){
    int ParentPosition = 0;
    int LeftPosition = 0;
    int RightPosition = 0;

    memcpy(Root, &H->Nodes[0], sizeof(Node));//root에 최솟값 저장.
    memset(&H->Nodes[0], 0, sizeof(Node));//node[0]에 0저장?

    H->UsedSize--;//node[usedsize]는 말단 노드.
    SwapNodes(H, 0, H->UsedSize);//뿌리노드와 말단노드 교환 -> 말단 노드에 0저장.(사용 X)

    LeftPosition = GetLeftChild(0);
    RightPosition = LeftPosition + 1;

    while(1){
        int SelectedChild = 0;

        if(LeftPosition >= H->UsedSize)
            break;//왼쪽자식이 없다면 종료.
        
        if(RightPosition >= H->UsedSize){
            SelectedChild = LeftPosition;//왼쪽자식이 끝이라면 왼쪽자식 선택.
        }else{
            if(H->Nodes[LeftPosition].Data > H->Nodes[RightPosition].Data)
                SelectedChild = RightPosition;
            else
                SelectedChild = LeftPosition;
        }//왼쪽, 오른쪽 둘 다 있을 때 더 작은 쪽 선택.

        if(H->Nodes[SelectedChild].Data < H->Nodes[ParentPosition].Data){
            SwapNodes(H, ParentPosition, SelectedChild);
            ParentPosition = SelectedChild;
        }//선택된 자식노드의 데이터가 부모노드의 데이터보다 작다면 교환.
        else
            break;
        
        LeftPosition = GetLeftChild(ParentPosition);
        RightPosition = LeftPosition + 1;//교환했다면 왼쪽, 오른쪽 노드 갱신.
    }
    if(H->UsedSize < (H->Capacity / 2)){
        H->Capacity /= 2;
        H->Nodes = (Node*)realloc(H->Nodes, sizeof(Node) * H->Capacity);
    }
    
}

void PrintNodes(Heap* H){
    int i = 0;
    for(i=0; i < H->UsedSize; i++){
        printf("%d ",H->Nodes[i].Data);
    }
    printf("\n");
}

int main(void){
    Heap* H = HeapCreate(3);
    Node MinNode;

    Insert(H, 12);
    Insert(H, 87);
    Insert(H, 111);
    Insert(H, 34);
    Insert(H, 16);
    Insert(H, 75);
    Insert(H, 55);
    Insert(H, 94);
    PrintNodes(H);

    DeleteMin(H, &MinNode);
    PrintNodes(H);

    DeleteMin(H, &MinNode);
    PrintNodes(H);

    DeleteMin(H, &MinNode);
    PrintNodes(H);

    DeleteMin(H, &MinNode);
    PrintNodes(H);

    DeleteMin(H, &MinNode);
    PrintNodes(H);

    DeleteMin(H, &MinNode);
    PrintNodes(H);

    DeleteMin(H, &MinNode);
    PrintNodes(H);
}