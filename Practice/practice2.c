#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

//1.가변배열(스스로구현가능) 2.가변배열 안에 넣은 데이터 정렬(버블)

typedef struct _tagArr{
    int* _pArr;
    int iCount;
    int iMaxCount;
}tArr;

void InitArr(tArr* s){
    s->_pArr = (int*)malloc(sizeof(int) * 2);
    s->iCount = 0;
    s->iMaxCount = 2;
}

void ReleasedArr(tArr* s){
    free(s->_pArr);
    s->iCount = 0;
    s->iMaxCount = 0;
}

void ReallocArr(tArr* s){
    int* temp = (int*)malloc(sizeof(int) * s->iMaxCount * 2);
    
    for(int i=0;i<s->iCount;++i){
        temp[i] = s->_pArr[i];
    }

    free(s->_pArr);

    s->_pArr = temp;

    s->iMaxCount *= 2;
}

void PushBack(tArr* s, int iData){
    if(s->iCount >= s->iMaxCount)
        ReallocArr(s);
    
    s->_pArr[s->iCount++] = iData;
}

void BubbleSort(tArr* s){
    //총 s->iCount - 1회 반복
    //반복할 때마다 확인 범위 1씩 줄어듦
    bool bFinish = true;

    for(int i=0;i<s->iCount-1;++i){
        for(int k=0;k<s->iCount-1-i;++k){
            if(s->_pArr[k] > s->_pArr[k+1]){
                int temp = s->_pArr[k+1];
                s->_pArr[k+1] = s->_pArr[k];
                s->_pArr[k] = temp;

                bFinish = false;
            }
        }
        if(bFinish == true)
            break;
    }
}


int main(){
    tArr s = {};

    InitArr(&s);

    for(int i=10;i>0;--i){
        PushBack(&s, i);
    }

    BubbleSort(&s);
    
    for(int i=0;i<s.iCount;++i){
        printf("%d\n",s._pArr[i]);
    }

    ReleasedArr(&s);

    return 0;
}