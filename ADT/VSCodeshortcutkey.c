#include <stdio.h>

int Add(int a, int b){
    return a + b;
}

int Factorial_Recursion(int num)
{
    if(num == 1)
        return 1;

    return Factorial_Recursion(num-1)*num;
}

int fibonacci(int n){
    if(n == 1 || n == 2)
        return 1;
    
    return fibonacci(n-1) + fibonacci(n-2);
}
// n번째 피보나치 수
// 1 1 2 3 5 8 13 21 . . .
// 1 2 3 4 5 6 7  8  . . .

int main(){
    // for(int i=0;i<4;++i)
    // {

    // }
    
    int i   =     0;
    int i1  =     1;

    int iData = Add(100, 200);

    for(int i=0;i<9;++i)
    {
        printf("%d\n",i);
    }
    
    int iInput = 0;

    scanf("%d",&iInput);

    printf("%d\n",iInput);

    printf("abcedf %d !\n",10);

    printf("%d\n",fibonacci(15));

    return 0;
}

// 단축키
// 지정한 구문 주석         :Ctrl + k, c
// 지정한 구문 주석 해제    :Ctrl + k, u
// Alt Drag

// 디버깅
// 디버깅 시작              :F5
// 중단점 생성 및 해제      :F9
// 디버깅 중, 구문 수행     :F10
// 구문 수행(함수진입)      :F11