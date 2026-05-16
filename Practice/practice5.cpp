#include <assert.h>
#include <stdio.h>

class CArr
{
private://적지 않아도 기본적으로 private로 설정됨
    int*    m_pInt;
    int     m_iCount;
    int     m_iMaxCount;

public:
    void PushBack(int _iData);
    void resize(int _iResizeCount);

    int operator[] (int idx);

public:
    CArr();//선언만 해두고 나중에 정의
    ~CArr();
};

CArr::CArr()
    : m_pInt(nullptr)
    , m_iCount(0)
    , m_iMaxCount(2)
{
    m_pInt = new int[2]; 
}

CArr::~CArr()
{
}

void CArr::PushBack(int _iData){
    int i = 0;

    if(m_iMaxCount <= m_iCount){
        resize(m_iMaxCount * 2);
    }

    m_pInt[m_iCount++] = _iData;
}

void CArr::resize(int _iResizeCount){
    if(m_iMaxCount >= _iResizeCount){
        assert(nullptr);
    }

    int* pNew = new int[_iResizeCount];
    
    for(int i=0;i<m_iCount;++i){
        pNew[i] = m_pInt[i];
    }

    delete[] m_pInt;

    m_pInt = pNew;

    m_iMaxCount = _iResizeCount;
}

int CArr::operator[](int idx){
    return m_pInt[idx];
}

int main(){
    CArr carr;

    carr.PushBack(10);
    carr.PushBack(20);
    carr.PushBack(30);

    int iData = carr[1];
    //carr[1] = 200; -> 이건 안됨(
    //operator의 반환형이 int이기 때문에
    //carr[1]이 반환하는 값은 operator[]함수가 종료될 때 
    //임시로 할당한 공간에 넣어둔 데이터 값을 반환하고
    //수정하면 임시공간에 삽입하게 되는 것)


    // int* CArr::operator[](int idx){
    //     return &m_pInt[idx] 또는 (m_pInt + idx);
    // }
    //->이렇게 하면 주소로 접근해서 원래 배열값 수정가능(*연산자를 이용해야됨)=>배열과 완전히 동일하진 않음

    //int& CArr::operator[](int idx){
    //    return m_pInt[idx];
    //}
    //->carr[1] = 200; 으로 바로 수정가능


    return 0;
}