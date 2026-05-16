'use strict';

const canvas = document.getElementById('game-canvas');
const ctx = canvas.getContext('2d');

ctx.imageSmoothingEnabled = false;
ctx.imageSmoothingQuality = 'low';

const physics = new PikaPhysics(false, true);
const keyboardArray = [
    new PikaKeyboard('KeyD', 'KeyG', 'KeyR', 'KeyV', 'KeyZ', 'KeyF'),
    new PikaKeyboard('ArrowLeft', 'ArrowRight', 'ArrowUp', 'ArrowDown', 'Enter')
];

const TARGET_FPS = 25;
const INTERVAL = 1000 / TARGET_FPS;

let lastTime = performance.now();
let scores = [0, 0];
const WINNING_SCORE = 15;
let isPlayer2Serve = false;
let isRoundEndDelay = false;
let gameEnded = false;
let isPaused = false;

document.getElementById('mode-select').addEventListener('change', (e) => {
    physics.player2.isComputer = (e.target.value === 'ai');
    resetGame();
});

document.getElementById('btn-restart').addEventListener('click', () => {
    isPaused = true;
    alert("심슨 배구 사용법\n\n"+
          "1P(호머) : D,F,G,R (이동, 점프) | Z (스매시, 다이빙)\n" +
          "2P(바트) : 방향키 (이동, 점프) | Enter (스매시, 다이빙)\n" +
          "다이빙   : 방향키+Z(Enter)\n" +
          "스매시   : 몬스터볼과 닿을 때 방향키+Z(Enter)\n" +
          "일시정지 : P\n" +
          "먼저 15점을 내는 사람이 승리"
          );
    isPaused = false;
});

window.addEventListener('keydown', (e) => {
    if (e.key.toLowerCase() === 'p') {
        isPaused = !isPaused;
    }
});

function resetGame() {
    scores = [0, 0];
    gameEnded = false;
    isRoundEndDelay = false;
    isPlayer2Serve = false;
    isPaused = false;
    physics.player1.initializeForNewRound();
    physics.player2.initializeForNewRound();
    physics.ball.initializeForNewRound(isPlayer2Serve);
}


// ---------------- 배경 ----------------
function drawArcadeCloud(x, y, scale = 1) {
    ctx.save();
    ctx.translate(x, y);
    ctx.scale(scale, scale);
    ctx.fillStyle = '#B8B8B8'; ctx.strokeStyle = '#1a1a1a'; ctx.lineWidth = 1.2 / scale;
    ctx.beginPath(); ctx.moveTo(0, 8);
    ctx.quadraticCurveTo(-4, 2, 4, -2); ctx.quadraticCurveTo(8, -10, 18, -8);
    ctx.quadraticCurveTo(24, -14, 34, -10); ctx.quadraticCurveTo(40, -16, 50, -10);
    ctx.quadraticCurveTo(58, -12, 62, -4); ctx.quadraticCurveTo(68, 2, 62, 8);
    ctx.quadraticCurveTo(50, 14, 32, 12); ctx.quadraticCurveTo(16, 15, 0, 8);
    ctx.closePath(); ctx.fill(); ctx.stroke();
    ctx.fillStyle = 'rgba(255,255,255,0.18)';
    ctx.beginPath(); ctx.ellipse(31, -2, 18, 5, 0, 0, Math.PI * 2); ctx.fill();
    ctx.restore();
}

function drawBackground() {
    const skyGradient = ctx.createLinearGradient(0, 0, 0, 267);
    skyGradient.addColorStop(0, '#3A88FE'); skyGradient.addColorStop(0.6, '#7CB3FF'); skyGradient.addColorStop(1, '#C7E4FF');
    ctx.fillStyle = skyGradient; ctx.fillRect(0, 0, 432, 304);

    drawArcadeCloud(258, 28, 0.68); drawArcadeCloud(298, 55, 0.80); drawArcadeCloud(340, 82, 0.65);
    drawArcadeCloud(152, 118, 0.70); drawArcadeCloud(328, 138, 0.62);

    ctx.fillStyle = '#1E7F1E';
    ctx.beginPath(); ctx.moveTo(0, 267); ctx.lineTo(0, 248); ctx.lineTo(42, 214); ctx.lineTo(98, 192);
    ctx.lineTo(152, 185); ctx.lineTo(205, 193); ctx.lineTo(250, 182); ctx.lineTo(305, 184);
    ctx.lineTo(360, 196); ctx.lineTo(432, 186); ctx.lineTo(432, 267); ctx.closePath(); ctx.fill();

    const fogGradient = ctx.createLinearGradient(0, 190, 0, 267);
    fogGradient.addColorStop(0, 'rgba(200, 200, 200, 0)'); fogGradient.addColorStop(1, 'rgba(200, 200, 200, 0.85)');
    ctx.fillStyle = fogGradient; ctx.fillRect(0, 190, 432, 77);

    ctx.fillStyle = '#F7D76A'; ctx.fillRect(0, 267, 432, 37);
    ctx.fillStyle = '#B65A00';
    for (let x = 0; x < 432; x += 12) {
        ctx.fillRect(x, 276, 3, 2); ctx.fillRect(x + 6, 286, 3, 2); ctx.fillRect(x + 2, 295, 2, 1);
    }

    ctx.fillStyle = '#D7D7D7'; ctx.fillRect(216 - 2, 173, 4, 132);
    ctx.fillStyle = '#F80000'; ctx.fillRect(216 - 0.7, 173, 1, 132);
    ctx.fillStyle = '#FFFFFF'; ctx.strokeStyle = '#000000'; ctx.lineWidth = 1;
    ctx.beginPath(); ctx.arc(216, 173, 4, 0, Math.PI * 2); ctx.fill(); ctx.stroke();
    ctx.fillStyle = '#F80000'; ctx.beginPath(); ctx.arc(216, 173, 1.5, 0, Math.PI * 2); ctx.fill();

    ctx.strokeStyle = '#000000'; ctx.lineWidth = 1;
    ctx.beginPath(); ctx.moveTo(0, 304); ctx.lineTo(0, 270); ctx.lineTo(58, 270); ctx.stroke();
    ctx.beginPath(); ctx.moveTo(432, 304); ctx.lineTo(432, 270); ctx.lineTo(374, 270); ctx.stroke();
}

function drawShadow(x, y, radiusX) {
    ctx.fillStyle = 'rgba(0, 0, 0, 0.4)';
    ctx.beginPath(); ctx.ellipse(x, y, radiusX, 4, 0, 0, Math.PI * 2); ctx.fill();
}

function poly(points) {
    ctx.beginPath(); ctx.moveTo(points[0][0], points[0][1]);
    for (let i = 1; i < points.length; i++) ctx.lineTo(points[i][0], points[i][1]);
    ctx.closePath();
}

function drawSmashEffect(x, y) {
    ctx.fillStyle = '#FFEB3B'; ctx.strokeStyle = '#F44336'; ctx.lineWidth = 2;
    ctx.beginPath();
    for (let i = 0; i < 8; i++) {
        ctx.lineTo(x + Math.cos(i * Math.PI / 4) * 18, y + Math.sin(i * Math.PI / 4) * 18);
        ctx.lineTo(x + Math.cos(i * Math.PI / 4 + Math.PI / 8) * 7, y + Math.sin(i * Math.PI / 4 + Math.PI / 8) * 7);
    }
    ctx.closePath(); ctx.fill(); ctx.stroke();
    ctx.fillStyle = '#FF9800'; ctx.beginPath();
    for (let i = 0; i < 8; i++) {
        ctx.lineTo(x + Math.cos(i * Math.PI / 4) * 9, y + Math.sin(i * Math.PI / 4) * 9);
        ctx.lineTo(x + Math.cos(i * Math.PI / 4 + Math.PI / 8) * 3, y + Math.sin(i * Math.PI / 4 + Math.PI / 8) * 3);
    }
    ctx.closePath(); ctx.fill();
}

// ---------------- 몬스터볼 ----------------
function drawPokeball(ball) {
    drawShadow(ball.x, 278, 14);
    if (ball.isPowerHit) {
        ctx.lineWidth = 1; ctx.strokeStyle = '#000000'; ctx.fillStyle = 'rgba(0, 60, 255, 0.7)';
        ctx.beginPath(); ctx.arc(ball.previousPreviousX, ball.previousPreviousY, 20, 0, Math.PI * 2); ctx.fill(); ctx.stroke();
        ctx.fillStyle = 'rgba(255, 0, 0, 0.7)';
        ctx.beginPath(); ctx.arc(ball.previousX, ball.previousY, 20, 0, Math.PI * 2); ctx.fill(); ctx.stroke();
    }
    ctx.save(); ctx.translate(ball.x, ball.y); ctx.rotate(ball.rotation * Math.PI / 4);
    ctx.lineWidth = 1.5; ctx.strokeStyle = '#000000'; ctx.beginPath(); ctx.arc(0, 0, 20, 0, Math.PI * 2); ctx.clip();
    ctx.fillStyle = '#F80000'; ctx.beginPath(); ctx.arc(0, 0, 20, Math.PI, Math.PI * 2); ctx.fill();
    ctx.fillStyle = '#A8A8A8'; ctx.beginPath(); ctx.arc(0, 0, 20, 0, Math.PI); ctx.fill();
    ctx.fillStyle = '#000000'; ctx.fillRect(-20, -2, 40, 4);
    ctx.fillStyle = '#A8A8A8'; ctx.beginPath(); ctx.arc(0, 0, 6, 0, Math.PI * 2); ctx.fill(); ctx.stroke();
    ctx.fillStyle = '#FFFFFF'; ctx.beginPath(); ctx.arc(0, 0, 2, 0, Math.PI * 2); ctx.fill();
    ctx.restore();
    ctx.lineWidth = 1.5; ctx.strokeStyle = '#000000'; ctx.beginPath(); ctx.arc(ball.x, ball.y, 20, 0, Math.PI * 2); ctx.stroke();
}

// ---------------- 🍩 1P: 호머 심슨 ----------------
const Y = '#FFD90F'; const W = '#FFFFFF'; const BP = '#6DC0F2'; const M = '#D1B271'; const S = '#333333';

function drawHomerHead(isSmash) {
    // 1. 얼굴 윤곽 (오른쪽 끝 경계 x=12)
    ctx.fillStyle = Y; ctx.beginPath(); ctx.moveTo(-8, -4); ctx.lineTo(-10, -20);
    ctx.bezierCurveTo(-10, -36, 12, -36, 12, -20); ctx.lineTo(12, -4); ctx.fill(); ctx.stroke();
    
    // 귀 및 머리카락 (기존 유지)
    ctx.beginPath(); ctx.arc(-10, -6, 2.5, Math.PI/2, -Math.PI/2); ctx.fill(); ctx.stroke();
    ctx.beginPath(); ctx.moveTo(-10, -7); ctx.lineTo(-9, -6); ctx.lineTo(-10, -5); ctx.stroke();
    ctx.beginPath(); ctx.moveTo(-10, -6); ctx.lineTo(-14, -8); ctx.lineTo(-11, -10); ctx.lineTo(-14, -12); ctx.lineTo(-10, -14); ctx.stroke();
    ctx.beginPath(); ctx.arc(-4, -33, 4, Math.PI, 0); ctx.stroke(); ctx.beginPath(); ctx.arc(3, -32, 3.5, Math.PI, 0); ctx.stroke();

    // 👄 입 주변 영역 (Muzzle): 수염 형태 유지
    ctx.fillStyle = M; ctx.beginPath(); 
    ctx.moveTo(0, -2); ctx.lineTo(10, -4);
    
    if (isSmash) {
        // 수염 윤곽
        ctx.bezierCurveTo(20, -4, 24, 6, 12, 12); 
        ctx.lineTo(10, 16); ctx.lineTo(4, 12);
        ctx.bezierCurveTo(-2, 10, -2, 4, 0, -2);
        ctx.fill(); ctx.stroke();
        
        // 📢 열린 입: 왼쪽 시작점은 앞으로 당기고(7), 오른쪽은 더 멀리(18) 보내서 가로로 확장
        ctx.fillStyle = '#000000'; ctx.beginPath(); 
        ctx.moveTo(7, 1);      // 왼쪽 위 (11 -> 7로 확장)
        ctx.lineTo(18, 3);     // 오른쪽 위 (17 -> 18로 확장)
        ctx.lineTo(17, 10);    // 오른쪽 아래
        ctx.lineTo(6, 7);      // 왼쪽 아래
        ctx.fill();
        
        // 👅 혀 위치: 넓어진 입에 맞춰 조정
        ctx.fillStyle = '#F80000'; ctx.beginPath(); 
        ctx.arc(14, 8, 2.5, 0, Math.PI * 2); 
        ctx.fill();
    } else {
        // 일반 상태 수염 윤곽
        ctx.bezierCurveTo(20, -4, 24, 6, 12, 12); 
        ctx.bezierCurveTo(8, 16, -2, 12, -2, 4); 
        ctx.closePath(); ctx.fill(); ctx.stroke();
        
        // 👄 입 가로줄: 시작점을 중앙 근처(4)까지 당기고 끝을 밖으로(18) 빼서 길게 그림
        ctx.beginPath(); 
        ctx.moveTo(4, 7);      // 왼쪽 시작점 확장 (9 -> 4)
        ctx.bezierCurveTo(10, 10, 14, 9, 18, 5); // 오른쪽 끝점 확장 (17 -> 18)
        ctx.stroke();
    }
    
    // 👁️ 눈 (기존 유지)
    ctx.fillStyle = W; ctx.beginPath(); ctx.arc(10, -14, 5.5, 0, Math.PI*2); ctx.fill(); ctx.stroke();
    ctx.beginPath(); ctx.arc(0, -14, 6.5, 0, Math.PI*2); ctx.fill(); ctx.stroke();
    ctx.fillStyle = '#000000';
    let py = isSmash ? -12 : -14;
    ctx.beginPath(); ctx.arc(13.5, py, 1.2, 0, Math.PI*2); ctx.fill(); 
    ctx.beginPath(); ctx.arc(-1.5, py, 1.4, 0, Math.PI*2); ctx.fill(); 
    
    // 볼의 입체감 효과
    ctx.fillStyle = Y; ctx.beginPath(); ctx.ellipse(13, -9, 4.5, 3, 0, 0, Math.PI * 2); ctx.fill(); ctx.stroke();
}

function drawHomer(player, isPlayer2) {
    drawShadow(player.x, 278, 18); ctx.save(); ctx.translate(player.x, player.y);
    let scaleX = (player.state === 3 || player.state === 4) ? player.divingDirection : (isPlayer2 ? -1 : 1);
    ctx.scale(scaleX, 1); ctx.lineWidth = 1.5; ctx.strokeStyle = '#000000'; ctx.lineJoin = 'round'; ctx.lineCap = 'round';
    if (player.state === 0) {
        ctx.fillStyle = BP; ctx.fillRect(-12, 16, 10, 10); ctx.strokeRect(-12, 16, 10, 10); ctx.fillRect(-13, 26, 12, 4); ctx.strokeRect(-13, 26, 12, 4);
        ctx.fillStyle = S; ctx.beginPath(); ctx.ellipse(-7, 32, 8, 4, 0, Math.PI, 0); ctx.fill(); ctx.stroke(); ctx.fillRect(-15, 32, 16, 2);
        ctx.fillStyle = BP; ctx.fillRect(2, 16, 10, 10); ctx.strokeRect(2, 16, 10, 10); ctx.fillRect(1, 26, 12, 4); ctx.strokeRect(1, 26, 12, 4);
        ctx.fillStyle = S; ctx.beginPath(); ctx.ellipse(7, 32, 8, 4, 0, Math.PI, 0); ctx.fill(); ctx.stroke(); ctx.fillRect(-1, 32, 16, 2);
        ctx.fillStyle = W; ctx.beginPath(); ctx.moveTo(-14, -2); ctx.quadraticCurveTo(24, 6, 16, 18); ctx.lineTo(-12, 16); ctx.bezierCurveTo(-18, 10, -18, 0, -14, -2); ctx.fill(); ctx.stroke();
        ctx.fillStyle = W; poly([[-10, -5], [4, -1], [2, 4], [-12, -1]]); ctx.fill(); ctx.stroke();
        drawHomerHead(false);
        ctx.fillStyle = Y; poly([[-2, 6], [8, 12], [6, 16], [-4, 10]]); ctx.fill(); ctx.stroke();
        poly([[12, 6], [-2, 14], [0, 18], [14, 10]]); ctx.fill(); ctx.stroke();
        ctx.fillStyle = W; poly([[-6, 0], [4, 4], [2, 10], [-8, 6]]); ctx.fill(); ctx.stroke();
    } else if (player.state === 1) {
        ctx.fillStyle = BP; poly([[-6, 14], [-14, 22], [-8, 26], [0, 18]]); ctx.fill(); ctx.stroke();
        ctx.fillStyle = S; ctx.beginPath(); ctx.ellipse(-12, 28, 4, 7, 0, 0, Math.PI * 2); ctx.fill(); ctx.stroke();
        ctx.fillStyle = BP; poly([[4, 14], [-4, 22], [2, 26], [10, 18]]); ctx.fill(); ctx.stroke();
        ctx.fillStyle = S; ctx.beginPath(); ctx.ellipse(-2, 28, 4, 7, 0, 0, Math.PI * 2); ctx.fill(); ctx.stroke();
        ctx.fillStyle = W; ctx.beginPath(); ctx.moveTo(-14, -2); ctx.quadraticCurveTo(24, 6, 16, 18); ctx.lineTo(-12, 16); ctx.bezierCurveTo(-18, 10, -18, 0, -14, -2); ctx.fill(); ctx.stroke();
        ctx.fillStyle = W; poly([[-10, -5], [4, -1], [2, 4], [-12, -1]]); ctx.fill(); ctx.stroke();
        drawHomerHead(false);
        ctx.fillStyle = Y; poly([[-2, 0], [6, -14], [10, -12], [2, 2]]); ctx.fill(); ctx.stroke();
        ctx.fillStyle = W; poly([[-6, 0], [2, -6], [6, -2], [-2, 4]]); ctx.fill(); ctx.stroke();
    } else if (player.state === 2) {
        ctx.fillStyle = BP; poly([[-6, 14], [-14, 22], [-8, 26], [0, 18]]); ctx.fill(); ctx.stroke();
        ctx.fillStyle = S; ctx.beginPath(); ctx.ellipse(-12, 28, 4, 7, 0, 0, Math.PI * 2); ctx.fill(); ctx.stroke();
        ctx.fillStyle = BP; poly([[4, 14], [-4, 22], [2, 26], [10, 18]]); ctx.fill(); ctx.stroke();
        ctx.fillStyle = S; ctx.beginPath(); ctx.ellipse(-2, 28, 4, 7, 0, 0, Math.PI * 2); ctx.fill(); ctx.stroke();
        ctx.fillStyle = W; ctx.beginPath(); ctx.moveTo(-14, -2); ctx.quadraticCurveTo(24, 6, 16, 18); ctx.lineTo(-12, 16); ctx.bezierCurveTo(-18, 10, -18, 0, -14, -2); ctx.fill(); ctx.stroke();
        ctx.fillStyle = W; poly([[-10, -5], [4, -1], [2, 4], [-12, -1]]); ctx.fill(); ctx.stroke();
        drawHomerHead(true);
        ctx.fillStyle = Y; poly([[2, 0], [22, -6], [24, -2], [6, 6]]); ctx.fill(); ctx.stroke();
        ctx.fillStyle = W; poly([[-6, 0], [4, -4], [6, 2], [-2, 6]]); ctx.fill(); ctx.stroke();
        let hitX = 24; let hitY = -4; let globalHitX = player.x + (scaleX * hitX); let globalHitY = player.y + hitY;
        if (Math.hypot(physics.ball.x - globalHitX, physics.ball.y - globalHitY) < 45) drawSmashEffect(hitX, hitY);
    } else {
        ctx.rotate(Math.PI / 3);
        ctx.fillStyle = BP; poly([[-16, 12], [-24, 18], [-18, 24], [-10, 18]]); ctx.fill(); ctx.stroke();
        ctx.fillStyle = S; ctx.beginPath(); ctx.ellipse(-24, 22, 6, 3, -Math.PI/4, 0, Math.PI * 2); ctx.fill(); ctx.stroke();
        ctx.fillStyle = BP; poly([[-4, 16], [-8, 26], [0, 28], [4, 18]]); ctx.fill(); ctx.stroke();
        ctx.fillStyle = S; ctx.beginPath(); ctx.ellipse(-4, 30, 7, 3.5, -Math.PI/6, 0, Math.PI * 2); ctx.fill(); ctx.stroke();
        ctx.fillStyle = W; ctx.beginPath(); ctx.moveTo(-12, -4); ctx.lineTo(4, -2); ctx.lineTo(6, 2);
        ctx.quadraticCurveTo(26, 8, 16, 16); ctx.lineTo(8, 16); ctx.quadraticCurveTo(-2, 14, -16, 12); ctx.bezierCurveTo(-20, 6, -18, 0, -12, -4); ctx.fill(); ctx.stroke();
        ctx.fillStyle = W; poly([[-10, -5], [4, -1], [2, 4], [-12, -1]]); ctx.fill(); ctx.stroke();
        drawHomerHead(true);
        ctx.fillStyle = Y; ctx.beginPath(); ctx.moveTo(-1, 8); ctx.lineTo(4, 8); ctx.lineTo(16, 20);
        ctx.quadraticCurveTo(20, 24, 16, 26); ctx.quadraticCurveTo(10, 26, 8, 20); ctx.lineTo(-2, 12); ctx.closePath(); ctx.fill(); ctx.stroke();
        ctx.fillStyle = W; ctx.beginPath(); ctx.moveTo(-4, 0); ctx.lineTo(8, 2); ctx.quadraticCurveTo(10, 8, 6, 12); ctx.lineTo(-6, 10); ctx.closePath(); ctx.fill(); ctx.stroke();
    }
    ctx.restore();
}

// ---------------- 🛹 2P: 6개 스파이크 & 정밀 수치 적용 바트 ----------------
const BB = '#0073D1'; const R = '#F02A1D'; const LB = '#22A1E6';

function drawBartHead(isSmash) {
    ctx.fillStyle = Y; ctx.beginPath(); 
    ctx.moveTo(-8, -4); ctx.lineTo(-8, -34);
    
    for(let i = 0; i < 6; i++) { 
        let px = -8 + (22 / 6) * i; 
        ctx.lineTo(px + 1.8, -40); ctx.lineTo(px + 3.6, -34);
    }
    
    ctx.lineTo(14, -34); ctx.lineTo(14, -20);
    if (isSmash) {
        ctx.lineTo(14, -4); ctx.lineTo(12, 5); ctx.lineTo(7, 1); ctx.lineTo(8, 8);
    } else {
        ctx.lineTo(14, -2); 
        ctx.quadraticCurveTo(11, -2, 10, 3); 
        ctx.lineTo(6, 4); 
        ctx.lineTo(6, 6); 
        ctx.quadraticCurveTo(8, 8, 10, 8); 
    }
    ctx.closePath(); ctx.fill(); ctx.stroke();

    if (isSmash) {
        ctx.fillStyle = '#000000'; ctx.beginPath(); ctx.moveTo(14, -6); ctx.lineTo(16, -4); ctx.lineTo(12, 3); ctx.lineTo(8, 0); ctx.fill();
        ctx.fillStyle = '#F80000'; ctx.beginPath(); ctx.arc(12, -1, 1.8, 0, Math.PI * 2); ctx.fill(); ctx.fillStyle = Y;
    } else {
        ctx.beginPath(); ctx.moveTo(13, -4); ctx.quadraticCurveTo(10, -2, 7, -4); ctx.stroke();
    }
    ctx.beginPath(); ctx.arc(-8, -14, 3, Math.PI/2, -Math.PI/2); ctx.fill(); ctx.stroke();
    ctx.beginPath(); ctx.moveTo(-8, -15); ctx.lineTo(-6, -14); ctx.lineTo(-8, -13); ctx.stroke();

    let py = isSmash ? -18 : -20;

    ctx.fillStyle = W; ctx.beginPath(); ctx.arc(11, -20, 6.5, 0, Math.PI*2); ctx.fill(); ctx.stroke();
    ctx.fillStyle = '#000000'; ctx.beginPath(); ctx.arc(14, py, 1.5, 0, Math.PI*2); ctx.fill();
    
    // 1. 화면 기준 왼쪽 눈 (x = 0.5)
    ctx.fillStyle = W; ctx.beginPath(); ctx.arc(1.5, -20, 6.5, 0, Math.PI*2); ctx.fill(); ctx.stroke();
    ctx.fillStyle = '#000000'; ctx.beginPath(); ctx.arc(4, py, 1.5, 0, Math.PI*2); ctx.fill();

    // 2. 👃 코 (반지름 2.7, y = -13, 상단 길이 3, 하단 길이 1.5)
    ctx.fillStyle = Y; ctx.beginPath();
    ctx.moveTo(8, -13 - 2.7); // 상단 시작점
    ctx.lineTo(11, -13 - 2.7); // 상단 길이 3 적용
    ctx.arc(11, -13, 2.7, -Math.PI/2, Math.PI/2); // 코끝 곡선
    ctx.lineTo(9.5, -13 + 2.7); // 하단 길이를 1.5로 줄임 (11 - 1.5 = 9.5)
    ctx.fill();
    ctx.beginPath();
    ctx.moveTo(8, -13 - 2.7); ctx.lineTo(11, -13 - 2.7); 
    ctx.arc(11, -13, 2.7, -Math.PI/2, Math.PI/2); ctx.lineTo(9.5, -13 + 2.7);
    ctx.stroke();

    // 3. 화면 기준 오른쪽 눈 (x = 10) - 왼쪽 눈과 코를 가리도록 마지막에 렌더링
    
}

function drawBart(player, isPlayer2) {
    drawShadow(player.x, 278, 16); ctx.save(); ctx.translate(player.x, player.y);
    let scaleX = (player.state === 3 || player.state === 4) ? player.divingDirection : (isPlayer2 ? -1 : 1);
    ctx.scale(scaleX, 1); ctx.lineWidth = 1.5; ctx.strokeStyle = '#000000'; ctx.lineJoin = 'round'; ctx.lineCap = 'round';
    if (player.state === 0) {
        ctx.fillStyle = W; ctx.beginPath(); ctx.ellipse(-6, 28, 8, 3, 0, 0, Math.PI * 2); ctx.fill(); ctx.stroke();
        ctx.fillStyle = LB; ctx.beginPath(); ctx.ellipse(-6, 25, 6, 4, 0, Math.PI, 0); ctx.fill(); ctx.stroke();
        ctx.fillStyle = W; ctx.beginPath(); ctx.arc(-2, 26, 1.5, 0, Math.PI * 2); ctx.fill(); ctx.stroke();
        ctx.fillStyle = W; ctx.beginPath(); ctx.ellipse(8, 29, 8, 3, 0, 0, Math.PI * 2); ctx.fill(); ctx.stroke();
        ctx.fillStyle = LB; ctx.beginPath(); ctx.ellipse(8, 26, 6, 4, 0, Math.PI, 0); ctx.fill(); ctx.stroke();
        ctx.fillStyle = W; ctx.beginPath(); ctx.arc(12, 27, 1.5, 0, Math.PI * 2); ctx.fill(); ctx.stroke();
        ctx.fillStyle = Y; poly([[-10, 14], [-4, 14], [-4, 25], [-10, 25]]); ctx.fill(); ctx.stroke();
        poly([[6, 15], [12, 15], [12, 26], [6, 26]]); ctx.fill(); ctx.stroke();
        ctx.fillStyle = BB; poly([[-12, 12], [16, 12], [18, 19], [4, 19], [2, 15], [-6, 19], [-14, 18]]); ctx.fill(); ctx.stroke();
        ctx.fillStyle = R; ctx.beginPath(); ctx.moveTo(-10, -6); ctx.quadraticCurveTo(-14, 4, -12, 12); ctx.lineTo(16, 12); ctx.quadraticCurveTo(20, 4, 10, -6); ctx.fill(); ctx.stroke();
        ctx.fillStyle = Y; poly([[0, -4], [6, -4], [12, 10], [6, 10]]); ctx.fill(); ctx.stroke();
        ctx.fillStyle = R; poly([[-2, -6], [10, -6], [10, 0], [0, 0]]); ctx.fill(); ctx.stroke();
        drawBartHead(false);
    } else if (player.state === 1) {
        ctx.fillStyle = W; ctx.beginPath(); ctx.ellipse(-6, 22, 5, 8, 0, 0, Math.PI * 2); ctx.fill(); ctx.stroke();
        ctx.fillStyle = LB; ctx.beginPath(); ctx.ellipse(-6, 19, 4, 6, 0, Math.PI, 0); ctx.fill(); ctx.stroke();
        ctx.fillStyle = W; ctx.beginPath(); ctx.ellipse(8, 22, 5, 8, 0, 0, Math.PI * 2); ctx.fill(); ctx.stroke();
        ctx.fillStyle = LB; ctx.beginPath(); ctx.ellipse(8, 19, 4, 6, 0, Math.PI, 0); ctx.fill(); ctx.stroke();
        ctx.fillStyle = Y; poly([[-10, 12], [-4, 12], [-4, 18], [-10, 18]]); ctx.fill(); ctx.stroke();
        poly([[6, 12], [12, 12], [12, 18], [6, 18]]); ctx.fill(); ctx.stroke();
        ctx.fillStyle = BB; poly([[-12, 8], [16, 8], [18, 15], [4, 15], [2, 11], [-6, 15], [-14, 14]]); ctx.fill(); ctx.stroke();
        ctx.fillStyle = R; ctx.beginPath(); ctx.moveTo(-10, -6); ctx.quadraticCurveTo(-14, 2, -12, 8); ctx.lineTo(16, 8); ctx.quadraticCurveTo(20, 2, 10, -6); ctx.fill(); ctx.stroke();
        drawBartHead(false);
        ctx.fillStyle = Y; poly([[-2, -2], [4, -14], [8, -12], [4, 0]]); ctx.fill(); ctx.stroke();
        ctx.fillStyle = R; poly([[-2, -6], [4, -10], [6, -6], [0, -2]]); ctx.fill(); ctx.stroke();
    } else if (player.state === 2) {
        ctx.fillStyle = W; ctx.beginPath(); ctx.ellipse(-6, 22, 5, 8, 0, 0, Math.PI * 2); ctx.fill(); ctx.stroke();
        ctx.fillStyle = LB; ctx.beginPath(); ctx.ellipse(-6, 19, 4, 6, 0, Math.PI, 0); ctx.fill(); ctx.stroke();
        ctx.fillStyle = W; ctx.beginPath(); ctx.ellipse(8, 22, 5, 8, 0, 0, Math.PI * 2); ctx.fill(); ctx.stroke();
        ctx.fillStyle = LB; ctx.beginPath(); ctx.ellipse(8, 19, 4, 6, 0, Math.PI, 0); ctx.fill(); ctx.stroke();
        ctx.fillStyle = Y; poly([[-10, 12], [-4, 12], [-4, 18], [-10, 18]]); ctx.fill(); ctx.stroke();
        poly([[6, 12], [12, 12], [12, 18], [6, 18]]); ctx.fill(); ctx.stroke();
        ctx.fillStyle = BB; poly([[-12, 8], [16, 8], [18, 15], [4, 15], [2, 11], [-6, 15], [-14, 14]]); ctx.fill(); ctx.stroke();
        ctx.fillStyle = R; ctx.beginPath(); ctx.moveTo(-10, -6); ctx.quadraticCurveTo(-14, 2, -12, 8); ctx.lineTo(16, 8); ctx.quadraticCurveTo(20, 2, 10, -6); ctx.fill(); ctx.stroke();
        drawBartHead(true);
        ctx.fillStyle = Y; poly([[2, -2], [24, -10], [26, -6], [6, 2]]); ctx.fill(); ctx.stroke();
        ctx.fillStyle = R; poly([[-2, -6], [8, -8], [10, -4], [0, -2]]); ctx.fill(); ctx.stroke();
        let hitX = 26; let hitY = -8; let globalHitX = player.x + (scaleX * hitX); let globalHitY = player.y + hitY;
        if (Math.hypot(physics.ball.x - globalHitX, physics.ball.y - globalHitY) < 45) drawSmashEffect(hitX, hitY);
    } else {
        ctx.rotate(Math.PI / 3);
        ctx.fillStyle = W; ctx.beginPath(); ctx.ellipse(-20, 18, 8, 3, -Math.PI/4, 0, Math.PI * 2); ctx.fill(); ctx.stroke();
        ctx.fillStyle = LB; ctx.beginPath(); ctx.ellipse(-18, 16, 6, 4, -Math.PI/4, Math.PI, 0); ctx.fill(); ctx.stroke();
        ctx.fillStyle = W; ctx.beginPath(); ctx.ellipse(-6, 26, 8, 3, -Math.PI/6, 0, Math.PI * 2); ctx.fill(); ctx.stroke();
        ctx.fillStyle = LB; ctx.beginPath(); ctx.ellipse(-4, 24, 6, 4, -Math.PI/6, Math.PI, 0); ctx.fill(); ctx.stroke();
        ctx.fillStyle = Y; poly([[-16, 6], [-12, 6], [-16, 16], [-20, 16]]); ctx.fill(); ctx.stroke();
        poly([[-4, 12], [0, 12], [-2, 22], [-6, 22]]); ctx.fill(); ctx.stroke();
        ctx.fillStyle = BB; poly([[-18, 2], [6, 2], [8, 14], [-2, 12], [-6, 14], [-16, 10]]); ctx.fill(); ctx.stroke();
        ctx.fillStyle = R; ctx.beginPath(); ctx.moveTo(-12, -12); ctx.quadraticCurveTo(-18, -4, -18, 2); ctx.lineTo(6, 2); ctx.quadraticCurveTo(10, -4, 4, -12); ctx.fill(); ctx.stroke();
        ctx.fillStyle = Y; poly([[-2, -12], [2, -12], [16, -4], [14, 0]]); ctx.fill(); ctx.stroke();
        ctx.fillStyle = R; poly([[-4, -12], [4, -12], [6, -6], [-2, -6]]); ctx.fill(); ctx.stroke();
        drawBartHead(true);
    }
    ctx.restore();
}

// ---------------- UI & 루프 ----------------
function drawUI() {
    ctx.fillStyle = '#F80000'; ctx.strokeStyle = '#000000'; ctx.lineWidth = 3;
    ctx.font = 'italic bold 40px "Arial Black", Impact, sans-serif'; ctx.textAlign = 'center';
    ctx.strokeText(String(scores[0]), 58, 58); ctx.fillText(String(scores[0]), 58, 58);
    ctx.strokeText(String(scores[1]), 432 - 58, 58); ctx.fillText(String(scores[1]), 432 - 58, 58);
    if (gameEnded) {
        ctx.fillStyle = 'rgba(0, 0, 0, 0.6)'; ctx.fillRect(0, 0, 432, 304);
        ctx.fillStyle = 'yellow'; ctx.font = 'bold 40px Arial'; ctx.textAlign = 'center';
        ctx.fillText(scores[0] >= WINNING_SCORE ? '호머 심슨 WIN!' : '바트 심슨 WIN!', 216, 150);
        ctx.fillStyle = 'white'; ctx.font = '20px Arial'; ctx.fillText('F5로 재시작', 216, 190);
    }
}

function updatePhysics() {
    if (gameEnded || isRoundEndDelay || isPaused) return;
    keyboardArray[0].getInput(); keyboardArray[1].getInput();
    const isBallTouchingGround = physics.runEngineForNextFrame(keyboardArray);
    if (isBallTouchingGround) {
        isRoundEndDelay = true;
        if (physics.ball.punchEffectX < 216) { scores[1]++; isPlayer2Serve = true; }
        else { scores[0]++; isPlayer2Serve = false; }
        if (scores[0] >= WINNING_SCORE || scores[1] >= WINNING_SCORE) gameEnded = true;
        setTimeout(() => {
            if (!gameEnded) {
                physics.player1.initializeForNewRound(); physics.player2.initializeForNewRound();
                physics.ball.initializeForNewRound(isPlayer2Serve); isRoundEndDelay = false;
            }
        }, 1000);
    }
}

function gameLoop(timestamp) {
    const deltaTime = timestamp - lastTime;
    if (deltaTime >= INTERVAL) {
        lastTime = timestamp - (deltaTime % INTERVAL);
        updatePhysics(); drawBackground(); drawPokeball(physics.ball);
        drawHomer(physics.player1, false); drawBart(physics.player2, true);
        drawUI();
    }
    requestAnimationFrame(gameLoop);
}
requestAnimationFrame(gameLoop);