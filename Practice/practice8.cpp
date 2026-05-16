#include <iostream>
#include <vector>
#include <list>

using namespace std;

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
    T* data(){return m_pInt};
    int size(){return m_iCount};
    int capacity(){return m_iMaxCount};

    T& operator[] (int idx);

    class iterator;
    iterator begin();
    iterator end();
    iterator erase(iterator& _iter);

    void clear(){
        m_iCount = 0;
    }

public:
    CArr();//선언만 해두고 나중에 정의
    ~CArr();

    class iterator
    {
    private:
        CArr*   m_pArr;
        T*      m_pData;
        int     m_iIdx;
        bool    m_bValid;

    public:
        T& operator * ()
        {
            //iterator가 알고 있는 주소와 가변배열이 알고 있는 주소가 달라진 경우(realloc으로 주소가 달라진 경우)
            //iterator가 end iterator일 경우
            if(m_pArr->m_pInt != m_pData || -1 == m_iIdx || !m_bValid)
                assert(nullptr);

            
            return m_pData[m_iIdx];
        }

        //전위
        iterator& operator ++()
        {
            //1. iterator가 마지막 데이터를 가리키고 있을 때 => 증가 시 end iterator 반환
            if(m_pArr->size() -1 == m_iIdx){
                m_iIdx = -1;
            }else{
                ++m_iIdx;
            }

            //2. end iterator인 경우 => 증가 불가
            //3. 주소가 변경된 경우
            if(m_pArr->m_pInt != m_pData || -1 == m_iIdx)
                assert(nullptr);
            
            return *this;
        }

        //후위
        iterator operator ++(int){//인자에 int를 넣은 것은 컴파일러가 ++을 후위로 인식하게 하기 위함이다.
            //fake => 실행시점은 전위와 같지만 후위인 것처럼 동작하기 위한 조건.
            iterator copy_iter = *this;//지역변수로 iterator 생성 => 현재 iterator 객체와 값이 같음

            ++(*this);//원본 객체는 ++하되, 반환은 원래 this(연산 전)를 반환함

            return copy_iter;//또다른 복사본 반환
        }

        iterator& operator --()
        {
            return *this;
        }

        bool operator ==(const iterator& _otheriter){
            if(m_pArr==_otheriter.m_pArr && m_iIdx==_otheriter.m_iIdx)
                return true;
            return false;
        }

        bool operator !=(const iterator& _otheriter){
            if(m_pArr==_otheriter.m_pArr && m_iIdx==_otheriter.m_iIdx)
                return false;
            return true;
            //return !(*this == _otheriter);과 같음 => ==를 위에 구현했기 때문에 다시 활용 가능.
            //*this: 함수를 호출시킨 객체
        }

    public:
        iterator()
            :m_pArr(nullptr)
            ,m_pData(nullptr)
            ,m_iIdx(-1)
            ,m_bValid(false);
        {

        }

        iterator(CArr* m_pArr, T* _pData, int iIdx)
            :m_pArr(_pData)
            ,m_pData(_pData)
            ,m_iIdx(_iIdx)
            ,m_bValid(false)
        {
            if(nullptr != _pArr && 0<=-iIdx){
                m_bValid = true;
            }
        }

        ~iterator();

        friend class CArr;
        //가변배열에서 iterator의 private 멤버에 접근 가능
    }
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

template<typename T>
typename CArr<T>::iterator CArr<T>::begin(){
    //시작을 가리키는 iterator를 만들어서 반환해줌
    if(0 == m_iCount)
        return iterator(this,m_pInt,-1); //데이터가 없는 경우, begin() == end()
    else
        return iterator(this,m_pInt,0);
}

template<typename T>
typename CArr<T>::iterator CArr<T>::end(){
    //끝의 다음을 가리키는 iterator를 만들어서 반환해줌
    return iterator(this,m_pInt,-1);
}

template<typename T>
typename CArr<T>::iterator CArr<T>::erase(iterator& _iter){
    if(this != _iter.m_pArr || end() == _iter || m_iCount <= _iter.m_iIdx){
        assert(nullptr);
    }

    int iLoopCount = m_iCount - (_iter.m_iIdx + 1);

    for(int i=0;i<iLoopCount;++i){
        m_pData[i + _iter.m_iIdx] = m_pData[i + _iter.m_iIdx + 1];
    }
    //iterator가 가리키는 데이터를 배열 내에서 제거한다.

    _iter.m_bValid = false;

    --m_iCount;
    //카운트 감소

    return iterator(this,m_pData,_iter.m_iIdx);
}

int main(){
    vector<int> vecInt;

    vector<int>::iterator veciter = vecInt.begin();
    *veciter = 100;
    ++veciter;
    //iterator: 내부 원리는 실제 vector, list와 다르더라도 인터페이스는 동일하게 설정함
    vecInt[0] = 100;

    list<int> listInt;
    listInt.push_back(10);
    listInt.push_back(100);

    list<int>::iterator iter = listInt.begin();
    int iData = *iter;
    ++iter;
    iData = *iter;

    
    for(iter = listInt.begin();iter != listInt.end();++iter){
        cout << *iter << endl;
    }
    //end()는 마지막+1의 값

    CArr<int> myvector;

    myvector.PushBack(1);
    myvector.PushBack(2);
    myvector.PushBack(3);

    CArr<int>::iterator myiter = myvector.begin();
    //++(전위, 후위), --, ==, !=
    //*myiter;
    //->연산자 오버로딩 구현해야함


    return 0;
}