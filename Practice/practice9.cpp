// 객체 지향 언어 특징
// 1. 캡슐화(은닉성)
// 2. 상속
// 3. 다형성
// 4. 추상화

#include <iostream>

class CParent
{
protected:
    int     m_i;

public:
    CParent()
        :m_i(0)
    {}
    ~CParent()
    {}
};

class CChild : public CParent
{
private:
    float   m_f;

public:
    CChild()
        :m_f(0.f)
    {
        m_i = 0;
    }
};
//CParent에서 상속받은 CChild
//CChild의 memory는 CParent + 자기 자신 메모리 (8 Byte) => parent부터 child 순으로 순차적으로 메모리 형성
//상속받았다고 해도 CParent의 private에는 접근 불가 => protected(상속받은 자식은 접근 가능)
//Child 생성자 실행 시 Parent 생성자도 실행되어야 함
//Child의 생성자에서 Parent의 값을 초기화(이니셜라이저)는 불가능(접근은 가능)
//생성자 호출 순서: Child(주체 클래스) 생성자 -> Parent 생성자
//Child 생성자에는 CParent()가 자동으로 생략되어 있음.(따라서 호출 순서는 child->parent, 실행 순서는 parent->child) => Parent먼저 초기화됨.
//실행 순서와 호출 순서를 혼동하지 말자. 실행 순서는 종료되는 시점 기준, 호출 순서는 호출되는 시점 기준임.

//오버라이딩
//오버로딩: 같은 이름의 함수지만 인자가 달라서 컴파일러가 구별할 수 있는 여지가 있는 경우
//오버라이딩: 상속 관계에서만 발생(부모와 자식 둘다 같은 함수(이름, 인자 같음)가 있는 경우)
//Parent에 Output() 함수가 있고 Child에도 Output() 함수가 있는 경우 => 자식 쪽에 구현된 함수가 호출됨.
//이 경우 Child의 Output()은 Parent의 함수를 재정의(오버라이딩)한 것임

//Child 소멸자
//Child 소멸자에서는 Child의 소멸자 먼저 실행된 후 Parent의 소멸자가 실행하도록 설계됨
//Child->Parent 순으로 실행, 호출됨

//다형성
//상속 and 포인터
//CParent* pParent = &child;    --(1)
//CChild* pChild = &parent;     --(2)
// (1)의 경우 CParent* 타입은 메모리의 크기, 형태(어떤 자료형인지) 확인함.
// &child의 메모리 구조는 parent -> child 이기 때문에 문제 없음.
// (2)의 경우 CChild* 타입이 &parent를 읽을 때 CParent* 타입이므로 크기가 다름 -> 컴파일 오류
//따라서 parent의 모든 child들은 CParent* 타입으로 자식 객체를 가리킬 수 있음
//=>pParent의 변수 타입은 하나여도 모든 child 중 하나를 가리킬 수 있음(주소만 바꿔주면 어떤 child든 될 수 있음)

//pParent로 가리키고 오버라이딩 된 경우
//pParent만 가지고는 어떤 child를 가리키고 있는지 알 수 없음(실제 객체 뭔지 모름).
//=>함수에 virtual 키워드(가상함수) 추가.
//virtual 함수 테이블에는 타입에 대한 정보 + 호출시 적절한 함수 찾아줌(Parent의 Output()인지 Child의 Output()인지)
//따라서 pParent에서 virtual함수 호출 시 pParent의 virtual함수 테이블로 이동 => 등록되어 있는 virtual함수 호출
//=> 실제 객체가 어떤 것인지 몰라도 virtual 함수 테이블에 등록되어 있는(class 생성시 테이블 저장됨) 함수 호출하는 것임.

//다운캐스팅
//pParent로 가리키고 오버라이딩 되지 않고 새로 생성한 함수를 호출해야 하는 경우
//자식 포인터 타입으로 일시적 캐스팅해서 호출한다.
//dynamic_cast 이용(캐스팅 실패한 경우 nullptr 반환)
//
// ((CChild*)pParent)->Func();
//=>객체와 타입이 다를 경우 위험함
//
// CChild* pChild = dynamic_cast<CChild*>(pParent);
// if(nullptr != pChild){
//     pChild->Func();
// }
//=>타입이 다른 경우를 걸러줌