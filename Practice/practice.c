#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef struct _tagArr{
    int* pInt;
    int iCount;
    int iMaxCount;
}tArr;//가변배열 자료형(메모리 연속)

// typedef struct _tagNode{
//     int idata;
//     tNode* pNextNode;
// }tNode;

// typedef struct _tagList{
//     tNode* pHeadnode;
//     int iCount;
// }tLinkedList;

void InitArr(tArr* _pArr){
    _pArr->pInt = (int*)malloc(sizeof(int)*2);
    _pArr->iCount = 0;
    _pArr->iMaxCount = 2;
}

void ReleaseArr(tArr* _pArr){
    free(_pArr->pInt);
    _pArr->iCount = 0;
    _pArr->iMaxCount = 0;
}

void Reallocate(tArr* _pArr){
    int* temp = _pArr->pInt;
    _pArr->pInt = (int*)malloc(_pArr->iMaxCount * 2 * sizeof(int));

    for(int i=0;i<_pArr->iCount;++i){
        _pArr->pInt[i] = temp[i];
    }
    
    _pArr->iMaxCount *= 2;
    free(temp);
}

void PushBack(tArr* _pArr, int _iData){
    if(_pArr->iCount >= _pArr->iMaxCount){//힙영역 다 참
        Reallocate(_pArr);
    }
    
    _pArr->pInt[_pArr->iCount++] = _iData;
}

int getLength(const char* String){
    int i=0;
    while(String[i] != 0){
        ++i;
    }
    return i;
}

void StrCat(char* String1, unsigned int bufferSize, const char* String2){
    int str1Len = getLength(String1);
    int str2Len = getLength(String2);

    if(bufferSize < str1Len+str2Len+1)
        assert(NULL);

    for(int i=0;i<str2Len+1;++i){
        String1[str1Len + i] = String2[i];
    }
}

//int wcscmp(const char* String1, const char* String2) 두 문자열이 일치할 경우 0 반환, 왼쪽 문자열이 우열이 높으면(a>b) -1 반대는 1
int StrCmp(const char* String1, const char* String2){
    int leftLen = getLength(String1);
    int rightLen = getLength(String2);

    int iLoop = leftLen;
    int iReturn = 0;
    
    if(leftLen < rightLen){
        iLoop = leftLen;
        iReturn = -1;
    }else if(leftLen > rightLen){
        iLoop = rightLen;
        iReturn = 1;
    }
    
    for(int i=0;i<iLoop;++i){
        if(String1[i] > String2[i])
            return 1;
        else if(String1[i] < String2[i])
            return -1;
    }
    return iReturn;
}



int main() {
    tArr s = {};

    InitArr(&s);

    for(int i=0;i<10;++i){
        PushBack(&s,i);
    }

    for(int i=0;i<s.iCount;++i){
        printf("%d\n",s.pInt[i]);
    }

    printf("%d",s.iMaxCount);
    ReleaseArr(&s);
    
    return 0;
}
