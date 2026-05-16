#include <assert.h>
#include <stdio.h>

template<typename T>
class CArr
{
private://적지 않아도 기본적으로 private로 설정됨
    T*      m_pInt;
    int     m_iCount;
    int     m_iMaxCount;

public:
    void PushBack(const T& _iData);
    void resize(int _iResizeCount);

    T& operator[] (int idx);

public:
    CArr();//선언만 해두고 나중에 정의
    ~CArr();
};

template<typename T>
CArr<T>::CArr()
    : m_pInt(nullptr)
    , m_iCount(0)
    , m_iMaxCount(2)
{
    m_pInt = new T[2]; 
}

template<typename T>
CArr<T>::~CArr()
{
    delete[] m_pInt;
}

template<typename T>
void CArr<T>::PushBack(const T& _iData){
    if(m_iMaxCount <= m_iCount){
        resize(m_iMaxCount * 2);
    }

    m_pInt[m_iCount++] = _iData;
}

template<typename T>
void CArr<T>::resize(int _iResizeCount){
    if(m_iMaxCount >= _iResizeCount){
        assert(0);
    }

    T* pNew = new T[_iResizeCount];
    
    for(int i=0;i<m_iCount;++i){
        pNew[i] = m_pInt[i];
    }

    delete[] m_pInt;

    m_pInt = pNew;

    m_iMaxCount = _iResizeCount;
}

template<typename T>
T& CArr<T>::operator[](int idx){
    return m_pInt[idx];
}

int main(){
    CArr<int> carr;

    carr.PushBack(10);
    carr.PushBack(20);
    carr.PushBack(30);

    int iData = carr[1];

    printf("%d",iData);

    return 0;
}