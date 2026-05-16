/*
해시: 데이터를 입력받아 완전히 다른 모습의 데이터로 바꾸는 작업(데이터의 해시값을 테이블 내 주소로 이용)
*/

#include <stdio.h>
#include <stdlib.h>

typedef int KeyType;

typedef int ValueType;

typedef struct Node{
    KeyType Key; //주소로 사용할 데이터(Hash 함수에 넣어짐)
    ValueType Value; //저장할 데이터
}Node;

typedef struct HashTable{
    int TableSize; //테이블 크기
    Node* Table;
}HashTable;

int Hash(KeyType Key, int TableSize){
    return Key % TableSize;
}

HashTable* CreateHashTable(int TableSize){
    HashTable* HT = (HashTable*)malloc(sizeof(HashTable));
    HT->Table = (Node*)malloc(sizeof(Node)*TableSize);
    HT->TableSize = TableSize;

    return HT;
}

void Set(HashTable* HT, KeyType Key, ValueType Value){
    int Address = Hash(Key, HT->TableSize);

    HT->Table[Address].Key = Key; //Key를 Hash에 넣어 얻은 Address를 인덱스로
    HT->Table[Address].Value = Value;
}

ValueType Get(HashTable* HT, KeyType Key){
    int Address = Hash(Key, HT->TableSize);

    return HT->Table[Address].Value;
}//Key Address에 접근 by Hash

void DestroyHashTable(HashTable* HT){
    free(HT->Table);
    free(HT);
}

int main(void){
    HashTable* HT = CreateHashTable(193);

    Set(HT, 418, 32114);

    return 0;
}