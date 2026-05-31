# Quiz 하늘에서 떨어지는 똥 피하기 게임

# 조건
# 1. 캐릭터는 화면 가장 아래에 위치, 좌우로만 이동 가능
# 2. 똥은 화면 가장 위에서 떨어짐. x 좌표는 매번 랜덤으로 설정
# 3. 캐릭터가 똥을 피하면 다음 똥이 다시 떨어짐.
# 4. 캐릭터가 똥과 충돌하면 게임 종료
# 5. FPS는 30으로 고정

# 게임 이미지
# 1. 배경 : 480 * 640
# 2. 캐릭터 : 70 * 70
# 3. 똥 : 70 * 70

from random import *

import pygame
####################################################################
# 기본 초기화 (반드시 해야 하는 것들)

pygame.init() # 초기화 (반드시 필요)

# 화면 크기 설정
screen_width = 480 # 가로 크기
screen_height = 640 # 세로 크기
screen = pygame.display.set_mode((screen_width,screen_height))

# 화면 타이틀 설정
pygame.display.set_caption("Avoid Shit") # 게임 이름

background = pygame.image.load("/home/user/project/Python/project1/background.png")

# FPS
clock = pygame.time.Clock()
####################################################################

# 1. 사용자 게임 초기화 (배경 화면, 게임 이미지, 좌표, 속도, 폰트 등)

player = pygame.Surface((70,70))
player.fill((128,128,128))
player_size = player.get_rect().size
player_width = player_size[0]
player_height = player_size[1]
player_x_pos = screen_width / 2 - player_width / 2
player_y_pos = screen_height - player_height

# 플레이어 이동할 좌표
to_x = 0

# 이동 속도
player_speed = 0.3

# 똥 -> 떨어지는 모션 구현하기, 지날수록 빨라지기
shit = pygame.Surface((70,70))
shit.fill((150,75,0))
shit_size = shit.get_rect().size
shit_width = shit_size[0]
shit_height = shit_size[1]
shit_x_pos = randint(0,screen_width - shit_width)
shit_y_pos = 0
shit_speed = 0.1


# 폰트
game_font = pygame.font.Font(None,40)

# 시작 시간
start_ticks = pygame.time.get_ticks()

running = True
while running:
    dt = clock.tick(30) # 게임화면의 초당 프레임 수를 설정(dt는 프레임에 반비례)

    # 2. 이벤트 처리 (키보드, 마우스 등)
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

        if event.type == pygame.KEYDOWN:
            if event.key == pygame.K_LEFT:
                to_x -= player_speed
            elif event.key == pygame.K_RIGHT:
                to_x += player_speed
        
        if event.type == pygame.KEYUP:
            to_x = 0

    player_x_pos += to_x * dt
    shit_y_pos += shit_speed * dt

    # 가로 경계값 처리
    if player_x_pos < 0:
        player_x_pos = 0
    elif player_x_pos > screen_width - player_width:
        player_x_pos = screen_width - player_width

    # 똥 피했을 때
    if shit_y_pos > screen_height - shit_height:
        shit_y_pos = 0
        shit_speed += 0.1
        shit_x_pos = randint(0,screen_width - shit_width)
    
    player_rect = player.get_rect()
    player_rect.left = player_x_pos
    player_rect.top = player_y_pos

    shit_rect = shit.get_rect()
    shit_rect.left = shit_x_pos
    shit_rect.top = shit_y_pos

    # 충돌 체크
    if player_rect.colliderect(shit_rect):
        print("충돌했어요")
        running = False

    screen.blit(background,(0,0))
    screen.blit(player,(player_x_pos,player_y_pos))
    screen.blit(shit,(shit_x_pos,shit_y_pos))

    elapsed_time = (pygame.time.get_ticks() - start_ticks) / 1000

    timer = game_font.render(str(int(elapsed_time)),True,(255,255,255))

    screen.blit(timer,(10,10))

    pygame.display.update() # 게임 화면을 다시 그리기

pygame.quit()