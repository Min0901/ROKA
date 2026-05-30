# import practice_module
# practice_module.price(3)
# practice_module.price_morning(6)
# practice_module.price_soldier(15)

# import practice_module as mv # as => 별명을 붙이겠다(이름이 길 때)
# mv.price(3)

# from practice_module import * # 전부 사용 => 모듈명 적을 필요 없음
# price(3)
# price_morning(9)

# from practice_module import price, price_morning
# # => price_soldier은 쓸 수 없음

# package 사용
# from travel import *
# # trip_to = japan.JapanPackage()
# # trip_to.detail()

# import inspect
# import random
# print(inspect.getfile(japan)) # 파일 위치 확인

# dir : 어떤 객체를 넘겼을 때 그 객체가 어떤 변수와 함수를 가지고 있는지 표시
# print(dir())
# import random # 외장 함수
# print(dir()) # random 추가

# import glob # glob : 경로 내의 폴더,파일 목록 조회(윈도우 dir)
# print(glob.glob("*.py"))

# import time
# print(time.strftime("%Y-%m-%d %H:%M:%S"))