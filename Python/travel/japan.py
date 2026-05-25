class JapanPackage:
    def detail(self):
        print("[일본 패키지 3박 4일] 도쿄 100만 원")

if __name__ == "__main__":
    print("japan 모듈 직접 실행")
    print("이 문장은 모듈 직접 실행시에만 실행됨")
    trip_to = JapanPackage()
    trip_to.detail()
else:
    print("japan 외부에서 모듈 호출")