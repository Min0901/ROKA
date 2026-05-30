# 하나의 파이썬 파일이 모듈이 될 수 있다.
# 모듈과 쓰려는 파일이 같은 경로에 있거나
# 파이썬 라이브러리들이 모여있는 폴더에 있으면 사용 가능

# 일반 가격
def price(people):
    print("{}명 가격은 {}원 입니다.".format(people,people * 10000))

# 조조할인 가격
def price_morning(people):
    print("조조 {}명 가격은 {}원 입니다.".format(people,people * 6000))

# 군인할인 가격
def price_soldier(people):
    print("군인 {}명 가격은 {}원 입니다.".format(people,people * 4000))