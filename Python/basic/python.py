from random import *

class Unit:
    def __init__(self,name,hp,speed): # like 생성자 self는 인자로 넣어줄 필요 없음
        self.name = name # 멤버 변수
        self.hp = hp
        self.speed = speed
        print("{} 유닛이 생성되었습니다.".format(name))

    def move(self,location):
        print("[지상 유닛 이동]")
        print("{} : {} 방향으로 이동합니다. [속도 {}]"\
            .format(self.name,location,self.speed))

    def damaged(self,damage):
        print("{} : {} 데미지를 입었습니다.".format(self.name,damage))
        self.hp -= damage
        print("{} : [현재 체력 {}]".format(self.name,self.hp))
        if self.hp <= 0:
            print("{} : 파괴되었습니다.".format(self.name))

class AttackUnit(Unit): # Class Unit을 상속받음
    def __init__(self,name,hp,speed,damage):
        Unit.__init__(self,name,hp,speed)
        self.damage = damage
        print("{} 유닛이 생성되었습니다.".format(self.name))
        print("[체력 {}] [공격력 {}]".format(self.hp,self.damage))

    def attack(self,location):
        print("{} : {} 방향으로 적군을 공격합니다. [공격력 {}]"\
        .format(self.name,location,self.damage))
        
class Marine(AttackUnit):
    def __init__(self):
        AttackUnit.__init__(self,"마린",40,1,5)

    def stimpack(self):
            if self.hp > 10:
                self.hp -= 10
                print("{} : 스팀팩을 사용합니다. (HP 10 감소)".format(self.name))
            else:
                print("{} : 체력이 부족하여 스팀팩을 사용하지 않습니다.".format(self.name))

class Tank(AttackUnit):
    seize_developed = False # 시즈모드 개발여부

    def __init__(self):
        AttackUnit.__init__(self,"탱크",150,1,35)
        self.seize_mode = False

    def set_seize_mode(self):
        if Tank.seize_developed == False:
            return
        
        if self.seize_mode == False:
            print("{} : 탱크가 시즈모드로 전환합니다.".format(self.name))
            self.damage *= 2
            self.seize_mode = True
        else:
            print("{} : 시즈모드를 해제합니다.".format(self.name))
            self.damage /= 2
            self.seize_mode = False

class Flyable:
    def __init__(self,flying_speed):
        self.flying_speed = flying_speed

    def fly(self,name,location):
        print("{} : {} 방향으로 날아갑니다. [속도 {}]".format(name,location,self.flying_speed))
    
class FlyableAttackUnit(AttackUnit,Flyable): # 다중 상속
    def __init__(self,name,hp,damage,flying_speed):
        AttackUnit.__init__(self,name,hp,0,damage) # 지상 speed는 0
        Flyable.__init__(self,flying_speed)

    def move(self,location):
        print("공중 유닛 이동")
        self.fly(self.name,location)

class Wraith(FlyableAttackUnit):
    def __init__(self):
        FlyableAttackUnit.__init__(self,"레이스",80,20,5)
        self.clocked = False

    def clocking(self):
        if self.clocked == True:
            print("{} : 클로킹 모드 해제합니다.")
            self.clocked = False
        else:
            print("{} : 클로킹 모드 설정합니다.".format(self.name))
            self.clocked = True

def game_start():
    print("[알림] 새로운 게임을 시작합니다.")

def game_over():
    print("Player : gg")
    print("[Player] 님이 게임에서 퇴장하셨습니다.")

game_start()

m1 = Marine()
m2 = Marine()
m3 = Marine()

t1 = Tank()
t2 = Tank()

w1 = Wraith()

attack_units = []
attack_units.append(m1)
attack_units.append(m2)
attack_units.append(m3)
attack_units.append(t1)
attack_units.append(t1)
attack_units.append(w1)

# 전군 이동
for unit in attack_units:
    unit.move("1시")

# 탱크 시즈모드 개발
Tank.seize_developed = True
print("[알림] 탱크 시즈 모드 개발이 완료되었습니다.")

# 공격 모드 준비(스팀팩,시즈모드,클로킹)
for unit in attack_units:
    if isinstance(unit,Marine): # isinstance() 특정 개체가 지정한 타입의 인스턴스인지
        unit.stimpack()
    elif isinstance(unit,Tank):
        unit.set_seize_mode()
    elif isinstance(unit,Wraith):
        unit.clocking()

# 전군 공격
for unit in attack_units:
    unit.attack("1시")

# 전군 피해
for unit in attack_units:
    unit.damaged(randint(5,21))

game_over()
