class Unit:
    def __init__(self,name,hp): # like 생성자 self는 인자로 넣어줄 필요 없음
        self.name = name # 멤버 변수
        self.hp = hp

class AttackUnit(Unit): # Class Unit을 상속받음
    def __init__(self,name,hp,damage):
        Unit.__init__(self,name,hp)
        self.damage = damage
        print("{} 유닛이 생성되었습니다.".format(self.name))
        print("체력 {}, 공격력 {}".format(self.hp,self.damage))

    def attack(self,location):
        print("{} : {} 방향으로 적군을 공격합니다. 공격력 : {}입니다."\
        .format(self.name,location,self.damage))
        
    def damaged(self,damage):
        print("{} : {} 데미지를 입었습니다.".format(self.name,damage))
        self.hp -= damage
        print("{} : 현재 체력은 {}입니다.".format(self.name,self.hp))
        if self.hp <= 0:
            print("{} : 파괴되었습니다.".format(self.name))

firebat1 = AttackUnit("파이어뱃",50,16)
firebat1.attack("5시")
firebat1.damaged(25)
firebat1.damaged(25)