import requests

url = "http://google.com"
headers = {"User-Agent":"Mozilla/5.0 (X11; Linux x86_64; rv:152.0) Gecko/20100101 Firefox/152.0"}

res = requests.get(url, headers=headers)
res.raise_for_status() # 올바르지 않은 웹을 스크래핑 시 오류

with open("google.html","w",encoding="utf8") as f:
    f.write(res.text)

