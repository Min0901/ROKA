# Project) 오락실 Pang 게임 만들기

# 조건
# 1. 캐릭터는 화면 아래에 위치, 좌우로만 이동 가능
# 2. 스페이스를 누르면 무기를 쏘아 올림
# 3. 큰 공 1개가 나타나서 바운스
# 4. 무기에 닿으면 공은 작은 크기 2개로 분할, 가장 작은 크기의 공은 사라짐
# 5. 모든 공을 없애면 게임 종료 (성공)
# 6. 캐릭터는 공에 닿으면 게임 종료 (실패)
# 7. 시간 제한 99초 초과 시 게임 종료 (실패)
# 8. FPS는 30으로 고정 (필요시 speed 값을 조정)

# 게임 이미지
# 1. 배경 : 640 * 480
# 2. 무대 : 640 * 50 (무대 위에서 움직임)
# 3. 캐릭터 : 60 * 33
# 4. 무기 : 20 * 430
# 5. 공 : 160 * 160, 80 * 80, 40 * 40, 20 * 20

from random import *
import os
import pygame
####################################################################
# 기본 초기화 (반드시 해야 하는 것들)

pygame.init() # 초기화 (반드시 필요)

# 화면 크기 설정
screen_width = 640 # 가로 크기
screen_height = 480 # 세로 크기
screen = pygame.display.set_mode((screen_width,screen_height))

# 화면 타이틀 설정
pygame.display.set_caption("Pang") # 게임 이름

# FPS
clock = pygame.time.Clock()
####################################################################

# 1. 사용자 게임 초기화 (배경 화면, 게임 이미지, 좌표, 속도, 폰트 등)
current_path = os.path.dirname(__file__) # 현재 파일 위치 반환
image_path = os.path.join(current_path,"images") # images 폴더 위치 반환

# 배경
background = pygame.image.load(os.path.join(image_path,"project_background.jpeg"))

# 스테이지
stage = pygame.Surface((640,50))
stage.fill((150,75,0))
stage_size = stage.get_rect().size
stage_height = stage_size[1] # 스테이지의 높이 위에 캐릭터를 두기 위해서

# 캐릭터
player = pygame.Surface((33,60))
player.fill((255,69,0))
player_size = player.get_rect().size
player_width = player_size[0]
player_height = player_size[1]
player_x_pos = screen_width / 2 - player_width / 2
player_y_pos = screen_height - stage_height - player_height

# 캐릭터 이동 방향
player_to_x = 0

# 캐릭터 이동 속도
player_speed = 5

# 무기
weapon = pygame.Surface((20,430))
weapon.fill((51,51,51))
weapon_size = weapon.get_rect().size
weapon_width = weapon_size[0]

# 한 번에 여러 발 발사 가능
weapons = []

# 무기 이동 속도
weapon_speed = 10

# 공 만들기 (4개 따로 처리)
ball_images = [
    pygame.Surface((160,160)),
    pygame.Surface((80,80)),
    pygame.Surface((40,40)),
    pygame.Surface((20,20))
]
for i in range(4):
    ball_images[i].fill((i*30,i*40,i*50))

# 공 최초 스피드
ball_speed_y = [-18,-15,-12,-9]

# 공들
balls = []

# 최초의 공
balls.append({
    "pos_x" : 50, # 공 x좌표
    "pos_y" : 50, # 공 y좌표
    "image_idx" : 0, # 공 이미지 인덱스
    "to_x" : 3, # x축 이동방향
    "to_y" : -6, # y축 이동방향
    "init_speed_y" : ball_speed_y[0] # y 최초 속도
})

# 사라질 무기, 공 정보 저장 변수
weapon_to_remove = -1
ball_to_remove = -1

# 폰트
game_font = pygame.font.Font(None,40)
total_time = 100

# 게임 종료 메시지 / Time Out, Mission Complete, Game Over
game_result = "Game Over"

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
                player_to_x -= player_speed
            elif event.key == pygame.K_RIGHT:
                player_to_x += player_speed
            elif event.key == pygame.K_SPACE: # 무기 발사
                weapon_x_pos = player_x_pos + player_width / 2 - weapon_width / 2
                weapon_y_pos = player_y_pos
                weapons.append([weapon_x_pos,weapon_y_pos]) # weapons에 하나의 x,y좌표값 추가


        if event.type == pygame.KEYUP:
            if event.key == pygame.K_LEFT or event.key == pygame.K_RIGHT:
                player_to_x = 0

    player_x_pos += player_to_x

    if player_x_pos < 0:
        player_x_pos = 0
    elif player_x_pos > screen_width - player_width:
        player_x_pos = screen_width - player_width

    # 무기 위치 조정
    weapons = [[w[0],w[1] - weapon_speed] for w in weapons] # 무기 위치 위로 올리기
    
    weapons = [[w[0],w[1]] for w in weapons if w[1] > 0] # 천장에 닿은 무기는 제거됨

    # 공 위치 정의
    for ball_idx, ball_val in enumerate(balls): # enumerate => index, value 반환
        ball_pos_x = ball_val["pos_x"]
        ball_pos_y = ball_val["pos_y"]
        ball_img_idx = ball_val["image_idx"]

        ball_size = ball_images[ball_img_idx].get_rect().size
        ball_width = ball_size[0]
        ball_height = ball_size[1]

        # 가로 벽에 닿았을 때 공 이동 경로 변경
        if ball_pos_x < 0 or ball_pos_x > screen_width - ball_width:
            ball_val["to_x"] *= -1
    
        # 세로 위치
        # 스테이지에 튕겨 올라감
        if ball_pos_y >= screen_height - stage_height - ball_height:
            ball_val["to_y"] = ball_val["init_speed_y"]
        else: # 그 외의 경우 가속도 설정
            ball_val["to_y"] += 0.5

        ball_val["pos_x"] += ball_val["to_x"]
        ball_val["pos_y"] += ball_val["to_y"]

    # 충돌 처리
    player_rect = player.get_rect()
    player_rect.left = player_x_pos
    player_rect.top = player_y_pos

    for ball_idx, ball_val in enumerate(balls): # enumerate => index, value 반환
        ball_pos_x = ball_val["pos_x"]
        ball_pos_y = ball_val["pos_y"]
        ball_img_idx = ball_val["image_idx"]

        # 공 rect 업데이트
        ball_rect = ball_images[ball_img_idx].get_rect()
        ball_rect.left = ball_pos_x
        ball_rect.top = ball_pos_y

        # 공과 캐릭터
        if player_rect.colliderect(ball_rect):
            running = False
            break

        # 공과 무기들
        for weapon_idx,weapon_val in enumerate(weapons):
            weapon_pos_x = weapon_val[0]
            weapon_pos_y = weapon_val[1]

            weapon_rect = weapon.get_rect()
            weapon_rect.left = weapon_pos_x
            weapon_rect.top = weapon_pos_y

            if weapon_rect.colliderect(ball_rect):
                weapon_to_remove = weapon_idx
                ball_to_remove = ball_idx

                # 가장 작은 공이 아니라면 둘로 나눠짐
                if ball_img_idx < 3:
                    # 현재 공 크기 정보
                    ball_width = ball_rect.size[0]
                    ball_height = ball_rect.size[1]

                    # 나눠진 공 정보
                    small_ball_rect = ball_images[ball_img_idx + 1].get_rect()
                    small_ball_width = small_ball_rect.size[0]
                    small_ball_height = small_ball_rect.size[1]

                    # 왼쪽
                    balls.append({
                        "pos_x" : ball_pos_x + ball_width / 2 - small_ball_width / 2, # 공 x좌표
                        "pos_y" : ball_pos_y + ball_height / 2 - small_ball_height / 2, # 공 y좌표
                        "image_idx" : ball_img_idx + 1, # 공 이미지 인덱스
                        "to_x" : -3, # x축 이동방향
                        "to_y" : -6, # y축 이동방향
                        "init_speed_y" : ball_speed_y[ball_img_idx + 1] # y 최초 속도
                    })

                    # 오른쪽
                    balls.append({
                        "pos_x" : ball_pos_x + ball_width / 2 - small_ball_width / 2, # 공 x좌표
                        "pos_y" : ball_pos_y + ball_height / 2 - small_ball_height / 2, # 공 y좌표
                        "image_idx" : ball_img_idx + 1, # 공 이미지 인덱스
                        "to_x" : 3, # x축 이동방향
                        "to_y" : -6, # y축 이동방향
                        "init_speed_y" : ball_speed_y[ball_img_idx + 1] # y 최초 속도
                    })

                break
        else:
            continue
        break


    if ball_to_remove > -1:
        del balls[ball_to_remove]
        ball_to_remove = -1
    
    if weapon_to_remove > -1:
        del weapons[weapon_to_remove]
        weapon_to_remove = -1

    # 모든 공을 없앤 경우
    if len(balls) == 0:
        game_result = "Mission Complete"
        running : False

    screen.blit(background, (0,0))

    for weapon_x_pos, weapon_y_pos in weapons:
        screen.blit(weapon, (weapon_x_pos,weapon_y_pos))

    for idx, val in enumerate(balls):
        ball_pos_x = val["pos_x"]
        ball_pos_y = val["pos_y"]
        ball_img_idx = val["image_idx"]
        screen.blit(ball_images[ball_img_idx],(ball_pos_x,ball_pos_y))

    screen.blit(stage, (0,screen_height - stage_height))
    screen.blit(player, (player_x_pos,player_y_pos))
    
    elapsed_time = (pygame.time.get_ticks() - start_ticks) / 1000
    timer = game_font.render("Time : {}".format(int(total_time - elapsed_time)), True, (255,255,255))
    screen.blit(timer, (10,10 ))

    # 시간 초과시
    if total_time <= elapsed_time:
        game_result = "Time Over"
        running = False

    pygame.display.update() # 게임 화면을 다시 그리기

msg = game_font.render(game_result,True,(255,255,0))
msg_rect = msg.get_rect(center=(int(screen_width/2),int(screen_height/2)))
screen.blit(msg,msg_rect)
pygame.display.update()

pygame.time.delay(2000)

pygame.quit()