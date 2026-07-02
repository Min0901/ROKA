import requests
res = requests.get("http://google.com")
print("응답코드 :",res.status_code) # 200이면 정상
res.raise_for_status() # 올바르지 않은 웹을 스크래핑 시 오류

# if res.status_code == requests.codes.ok:
#     print("정상")
# else:
#     print("오류 [에러코드 ",res.status_code,"]")

print(len(res.text)) # res로 가져온 웹의 html 문서 글자 수

with open("example.html","w",encoding="utf8") as f:
    f.write(res.text)
