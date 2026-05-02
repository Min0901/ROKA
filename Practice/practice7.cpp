#include <stdio.h>


template<typename T>
struct tListNode
{   
    T              data;
    tListNode<T>*  pNext;    
    tListNode<T>*  pPrev;

    tListNode()
        : data()
        , pPrev(nullptr)
        , pNext(nullptr)
    {
    }//인자 없다면 이 생성자가 호출

    tListNode(const T& _data, tListNode<T>* _pPrev, tListNode<T>* _pNext)
        : data(_data)
        , pPrev(_pPrev)
        , pNext(_pNext)
    {
    }//인자 전달 시 이 생성자가 호출됨
};

template<typename T>
class CList
{
private:
    tListNode<T>*   m_pHead;
    tListNode<T>*   m_pTail;
    int             m_iCount;

public:
    void push_back(const T& _data);
    void push_front(const T& _data);

public:
    CList();
    ~CList();

};

template<typename T>
CList<T>::CList()
    : m_pHead(nullptr)
    , m_pTail(nullptr)
    , m_iCount(0)
{
}

template<typename T>
CList<T>::~CList()
{
    tListNode<T>* pDeleteNode = m_pHead;

    for(int i=0;i<m_iCount;++i){
        printf("%d\n",pDeleteNode->data);
        pDeleteNode = pDeleteNode->pNext;
    }
    
    while(pDeleteNode){
        tListNode<T>* pNext = pDeleteNode->pNext;
        delete(pDeleteNode);
        pDeleteNode = pNext;
    }
    
}

template<typename T>
void CList<T>::push_back(const T& _data){

    //입력된 데이터를 저장할 노드를 동적할당 함
    tListNode<T>* pNewNode = new tListNode<T>(_data, nullptr, nullptr); //구조체 초기화까지

    //처음 입력된 데이터라면
    if(nullptr == m_pHead){
        m_pHead = pNewNode;
        m_pTail = pNewNode;
    }else{
        //현재 가장 마지막 노드가 newnode 가리키게 하기
        m_pTail->pNext = pNewNode;
        pNewNode->pPrev = m_pTail;

        m_pTail = pNewNode;
    }

    ++m_iCount;
}

template<typename T>
void CList<T>::push_front(const T& _data){

    tListNode<T>* pNewNode = new tListNode<T>(_data, nullptr, m_pHead);

    m_pHead->pPrev = pNewNode;

    m_pHead = pNewNode;

    ++m_iCount;
}




int main(){
    CList<int> list;

    for(int i=0;i<4;++i){
        list.push_back(i);
    }


    return 0;
}