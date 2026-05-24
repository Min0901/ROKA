class BigNumberError(Exception): # 사용자 정의 예외처리
    def __init__(self,msg):
        self.msg = msg

    def __str__(self): # print함수 사용시 자동 호출
        return self.msg

try:
    print("나누기 전용 계산기입니다")
    num1 = int(input("첫 번째 숫자를 입력하세요 : "))
    num2 = int(input("두 번째 숫자를 입력하세요 : "))
    print("{} / {} = {}".format(num1,num2,int(num1/num2)))
except ValueError as e: # except 에러종류: => 에러 발생시 실행할 코드
    print(f"에러! 잘못된 값을 입력하였습니다. {e}") # print(f"{x}")시 x를 변수로 인식
except ZeroDivisionError as err:
    print(err)
except:
    print("알 수 없는 에러가 발생하였습니다.") 

try:
    print("한 자리 전용 숫자 계산기입니다.")
    num1 = int(input("첫 번째 숫자를 입력하세요 : "))
    num2 = int(input("두 번째 숫자를 입력하세요 : "))
    if num1 >= 10 or num2 >= 10:
        raise ValueError # 강제 예외(에러)처리 => except의 ValueError처리 부분으로 바로 넘어감
    print("{} / {} = {}".format(num1,num2,int(num1/num2)))
except ValueError:
    print("한 자리 숫자만 입력하세요.")
except BigNumberError as err:
    print("에러가 발생하였습니다.")
    print(err)
finally: # 무조건 실행되는 구문
    print("이용해 주셔서 감사합니다.")


class SoldOutError(Exception):
    def __init__(self,msg):
        self.msg = msg
    
    def __str__(self):
        return self.msg

total = 0
while True:
    try:
        cus = int(input("몇 마리 살거야?"))
        if cus < 1:
            raise ValueError
        total += cus
        if total > 10:
            raise SoldOutError("재고가 소진되어 더 이상 주문X")
    except ValueError:
        print("잘못된 값을 입력하였습니다.")
    except SoldOutError as err:
        print(err)
        break