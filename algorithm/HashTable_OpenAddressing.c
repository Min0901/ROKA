/*
Chaning이란 서로 다른 키에 대해 같은 주소값을 반환해서 충돌이 발생하면 해당 주소에 있는 링크드 리스트에 삽입하는 기법이다.
Chaning은 해시 함수가 만들어낸 주소값만 사용한다.(Closed Addressing)
Chaning에서 클러스터가 발생하면 해시 테이블이 담았던 링크드 리스트에 연결하고 Open Addressing에서는 테이블 내 메모리를 탐사한다.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>




typedef char* KeyType;
typedef char* ValueType;

enum ElementStatus{
    EMPTY = 0,
    OCCUPIED = 1
};//해시 테이블 요소의 상태(비어 있을 때 0, 차있을 때 1)

typedef struct ElementType{
    KeyType Key;
    ValueType Value;

    enum ElementStatus Status;
}ElementType;

typedef struct HashTable{
    int OccupiedCount;
    int TableSize;

    ElementType* Table;
}HashTable;

void Set(HashTable** HT, KeyType Key, ValueType Value);
void Rehash(HashTable** HT);

HashTable* CreateHashTable(int TableSize){
    HashTable* HT = (HashTable*)malloc(sizeof(HashTable));
    HT->Table = (ElementType*)malloc(sizeof(ElementType)*TableSize);

    memset(HT->Table,0,sizeof(ElementType)*TableSize);

    HT->TableSize = TableSize;
    HT->OccupiedCount = 0;

    return HT;
}

int Hash(KeyType Key, int KeyLength, int TableSize){
    int i = 0;
    int HashValue = 0;

    for(i=0; i<KeyLength; i++){
        HashValue = (HashValue << 3) + Key[i];
    }

    HashValue = HashValue % TableSize;

    return HashValue;
}

int Hash2(KeyType Key, int KeyLength, int TableSize){
    int i = 0;
    int HashValue = 0;

    for(i=0;i<KeyLength;i++){
        HashValue = (HashValue << 2) + Key[i];
    }

    HashValue = HashValue % (TableSize - 3);

    return HashValue + 1;
}

void ClearElement(ElementType* Element){
    if(Element->Status == EMPTY)
        return;

    free(Element->Key);
    free(Element->Value);
}


void DestroyHashTable(HashTable* HT){
    int i = 0;
    for(i=0; i<HT->TableSize; i++){
        ClearElement(&(HT->Table[i]));
    }

    free(HT->Table);
    free(HT);
}



ValueType Get(HashTable* HT, KeyType Key){
    int KeyLen = strlen(Key);

    int Address = Hash(Key, KeyLen, HT->TableSize);
    int StepSize = Hash2(Key, KeyLen, HT->TableSize);

    while(HT->Table[Address].Status != EMPTY && strcmp(HT->Table[Address].Key, Key) != 0){
        Address = (Address + StepSize) % HT->TableSize;
    }

    return HT->Table[Address].Value;
}
void Set(HashTable** HT, KeyType Key, ValueType Value){
    int KeyLen, Address, StepSize;
    double Usage;

    Usage = (double)(*HT)->OccupiedCount / (*HT)->TableSize;

    if(Usage > 0.5){
        Rehash(HT);
    }

    KeyLen = strlen(Key);
    Address = Hash(Key,KeyLen,(*HT)->TableSize);
    StepSize = Hash2(Key,KeyLen,(*HT)->TableSize);

    while((*HT)->Table[Address].Status != EMPTY && strcmp((*HT)->Table[Address].Key,Key) != 0){
        printf("Collision occured! : Key(%s), Address(%d), StepSize(%d)\n",Key,Address,StepSize);

        Address = (Address + StepSize) % (*HT)->TableSize;
    }

    (*HT)->Table[Address].Key = (char*)malloc(sizeof(char)*(KeyLen + 1));
    strcpy((*HT)->Table[Address].Key, Key);

    (*HT)->Table[Address].Value = (char*)malloc(sizeof(char)*(strlen(Value)+1));
    strcpy((*HT)->Table[Address].Value, Value);

    (*HT)->Table[Address].Status = OCCUPIED;

    (*HT)->OccupiedCount++;

    printf("Key(%s) entered at address(%d)\n",Key,Address);
}


void Rehash(HashTable** HT){
    int i = 0;
    ElementType* OldTable = (*HT)->Table;

    HashTable* NewHT = CreateHashTable((*HT)->TableSize*2);

    printf("\nRehased. New table size is : %d\n\n", NewHT->TableSize);

    for(i=0;i<(*HT)->TableSize;i++){
        if(OldTable[i].Status == OCCUPIED){
            Set(&NewHT,OldTable[i].Key,OldTable[i].Value);
        }
    }

    DestroyHashTable((*HT));

    (*HT) = NewHT;
}



int main(void){
    HashTable* HT = CreateHashTable(11);
    Set(&HT, "MSFT", "Microsoft Corporation");
    Set(&HT, "JAVA", "Sun Microsystems");
    Set(&HT, "REDH", "Red Hat Linux");
    Set(&HT, "APAC", "Apache Org");
    Set(&HT, "ZYMZZ", "Unisys Ops Check");
    Set(&HT, "IBM", "IBM Ltd.");
}