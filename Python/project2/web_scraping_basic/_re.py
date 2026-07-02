# 정규식이란 주민번호, 이메일 주소, 번호판 등 형식이 있는 형태
import re # 정규식 라이브러리

p = re.compile("ca.e") 
# . (ca.e) : 하나의 문자를 의미 ex) care, cafe, case
# ^ (^de) : 문자열의 시작 ex) desk, destination
# $ (se$) : 문자열의 끝 ex) case, base, collapse

def print_match(m):
    if m:
        print(m.group()) # group : 일치하는 문자열 반환
        print(m.string) # string : 입력받은 문자열 반환
        print(m.start()) # start : 일치하는 문자열의 시작 index
        print(m.end()) # end : 일치하는 문자열의 끝 index
        print(m.span()) # span : 일치하는 문자열의 시작과 끝 index
    else:
        print("매칭되지 않았습니다.")


m = p.match("careless") 
# 매치되지 않으면 에러 발생
# match : 주어진 문자열의 처음부터 일치하는지 확인(비교대상이 더 길어도 상관 x)
# print_match(m)

m = p.search("good care")
# search : 주어진 문자열 중에 일치하는 게 있는지 확인
# print_match(m)

lst = p.findall("good care cafe")
# findall : 일치하는 모든 것을 리스트 형태로 반환

print(lst)

# 1. p = re.compile("원하는 형태")
# 2. m = p.match("비교할 문자열") : 주어진 문자열의 처음부터 일치하는지 확인
# 3. m = p.search("비교할 문자열") : 주어진 문자열 중에 일치하는 게 있는지 확인
# 4. lst = p.findall("비교할 문자열") : 일치하는 모든 것 리스트 형태로 반환

# 원하는 형태 : 정규식 (. ^ $)