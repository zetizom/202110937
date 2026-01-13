"""
app.py - MedArm 대시보드 (실제 데이터 연동 버전)
공유 메모리에서 실시간 데이터를 읽어 표시

실행 순서:
1. python main.py --port-in COM8 --port-out COM3
2. streamlit run app.py
"""

import streamlit as st
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from datetime import datetime
import time
import math
from collections import deque
from shared_data import SharedMemoryReader

# === 페이지 설정 (Wide Mode 필수) ===
st.set_page_config(
    page_title="MedArm 대시보드",
    page_icon="🤖",
    layout="wide",
    initial_sidebar_state="collapsed"
)

# === 한글 폰트 설정 (Matplotlib) ===
plt.rcParams['font.family'] = 'sans-serif' 
plt.rcParams['axes.unicode_minus'] = False

# === 업데이트 주기 설정 ===
FAST_UPDATE_MS = 100
GRAPH_UPDATE_INTERVAL = 5
TRAJECTORY_LENGTH = 50

# === 스타일 ===
st.markdown('''
<style>
@import url('https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&display=swap');
* { font-family: 'Pretendard', 'Malgun Gothic', 'Apple SD Gothic Neo', sans-serif; }
.stApp { background: linear-gradient(135deg, #f8fafc 0%, #e2e8f0 100%); }

.block-container { padding-top: 3rem !important; padding-bottom: 1rem !important; }

.header {
    background: linear-gradient(135deg, #1e293b 0%, #334155 100%);
    color: white;
    padding: 0.5rem 1.5rem;
    border-radius: 12px;
    margin-bottom: 0.5rem;
    display: flex;
    justify-content: space-between;
    align-items: center;
    box-shadow: 0 2px 10px rgba(0,0,0,0.1);
}
.logo { font-size: 1.2rem; font-weight: 700; }
.logo span { color: #38bdf8; }
.status-badge {
    padding: 0.25rem 0.75rem; border-radius: 15px; font-size: 0.7rem; font-weight: 500;
    display: flex; align-items: center; gap: 0.4rem;
}
.status-ok { background: rgba(34, 197, 94, 0.2); color: #22c55e; }
.status-warn { background: rgba(251, 191, 36, 0.2); color: #fbbf24; }
.status-dot { width: 6px; height: 6px; border-radius: 50%; animation: pulse 2s infinite; }
@keyframes pulse { 0%, 100% { opacity: 1; } 50% { opacity: 0.5; } }
.dot-ok { background: #22c55e; }
.dot-warn { background: #fbbf24; }
.datetime { font-size: 0.8rem; text-align: right; line-height: 1.1; }

.card {
    background: white;
    border-radius: 10px;
    padding: 0.6rem;
    box-shadow: 0 1px 3px rgba(0,0,0,0.05);
    margin-bottom: 0.5rem;
    border: 1px solid #e2e8f0;
}
.card-title {
    font-size: 0.8rem;
    font-weight: 700;
    color: #64748b;
    margin-bottom: 0.4rem;
    text-transform: uppercase;
    border-bottom: 1px solid #f1f5f9;
    padding-bottom: 0.2rem;
}

.conn-grid { display: flex; flex-direction: column; gap: 0.3rem; }
.conn-item {
    display: flex; justify-content: space-between; align-items: center;
    padding: 0.3rem 0.5rem; background: #f8fafc; border-radius: 6px; border: 1px solid #e2e8f0;
}
.conn-label { font-size: 0.75rem; font-weight: 600; color: #64748b; }
.conn-value { font-size: 0.8rem; font-weight: 700; display: flex; align-items: center; gap: 0.4rem; }
.conn-dot { width: 6px; height: 6px; border-radius: 50%; }
.conn-ok { color: #22c55e; }
.conn-warn { color: #f59e0b; }

.metric-grid { display: grid; grid-template-columns: repeat(2, 1fr); gap: 0.4rem; }
.metric-box {
    background: #f8fafc; border-radius: 8px; padding: 0.4rem;
    text-align: center; border: 1px solid #e2e8f0;
}
.metric-label { font-size: 0.65rem; color: #64748b; margin-bottom: 0.1rem; }
.metric-value { font-size: 1.0rem; font-weight: 700; color: #1e293b; }
.metric-unit { font-size: 0.7rem; color: #94a3b8; margin-left:2px;}
.metric-highlight { color: #0ea5e9; }

.stat-grid { display: grid; grid-template-columns: repeat(3, 1fr); gap: 0.3rem; margin-top: 0.3rem; }
.stat-box {
    background: linear-gradient(135deg, #f0f9ff 0%, #e0f2fe 100%);
    border-radius: 6px; padding: 0.3rem; text-align: center;
    border: 1px solid #bae6fd;
}
.stat-label { font-size: 0.55rem; color: #0369a1; margin-bottom: 0.1rem; }
.stat-value { font-size: 0.75rem; font-weight: 700; color: #0c4a6e; }

.joint-grid { display: flex; flex-direction: column; gap: 0.3rem; }
.joint-row {
    display: flex; justify-content: space-between; align-items: center;
    padding: 0.3rem 0.5rem; background: #f8fafc; border-radius: 6px; border: 1px solid #e2e8f0;
}
.joint-name { font-size: 0.75rem; font-weight: 600; }
.joint-val { font-size: 0.85rem; font-weight: 700; font-family: 'SF Mono', monospace; }
.val-pos { color: #10b981; } .val-neg { color: #f43f5e; }

.finger-grid { display: grid; grid-template-columns: repeat(4, 1fr); gap: 0.3rem; }
.finger-item { padding: 0.4rem 0.2rem; border-radius: 8px; text-align: center; }
.finger-active { background: #dcfce7; border: 1px solid #22c55e; }
.finger-inactive { background: #f1f5f9; border: 1px solid #cbd5e1; }
.finger-name { font-size: 0.7rem; font-weight: 600; margin-bottom: 0.1rem; }
.finger-status { font-size: 0.6rem; font-weight: 700; padding: 0.1rem 0.3rem; border-radius: 4px; }
.finger-on { background: #22c55e; color: white; }
.finger-off { background: #94a3b8; color: white; }

.log-box { 
    font-family: 'SF Mono', monospace; 
    font-size: 0.7rem; 
    background: #1e293b; 
    color: #e2e8f0; 
    padding: 0.5rem; 
    border-radius: 6px; 
    height: 90px;
    overflow-y: auto; 
}
.log-line { margin-bottom: 0.1rem; }
.log-time { color: #64748b; }
.log-info { color: #38bdf8; }
.log-warn { color: #fbbf24; }
.log-error { color: #f43f5e; }
</style>
''', unsafe_allow_html=True)


# === 세션 상태 초기화 ===
if 'logs' not in st.session_state: st.session_state.logs = []
if 'graph_counter' not in st.session_state: st.session_state.graph_counter = 0
if 'e2e_history' not in st.session_state: st.session_state.e2e_history = []
if 'hz_history' not in st.session_state: st.session_state.hz_history = []
if 'packet_count' not in st.session_state: st.session_state.packet_count = 0
if 'start_time' not in st.session_state: st.session_state.start_time = time.time()

# 궤적 저장용
if 'trajectory' not in st.session_state: 
    st.session_state.trajectory = deque(maxlen=TRAJECTORY_LENGTH)

# 통계용
if 'e2e_stats' not in st.session_state:
    st.session_state.e2e_stats = {'min': 999, 'max': 0, 'sum': 0, 'count': 0}
if 'rtt_stats' not in st.session_state:
    st.session_state.rtt_stats = {'min': 999, 'max': 0, 'sum': 0, 'count': 0}

# 공유 메모리 연결
if 'shm_reader' not in st.session_state:
    try:
        st.session_state.shm_reader = SharedMemoryReader(timeout=5.0)
        st.session_state.hw_connected = True
    except:
        st.session_state.shm_reader = None
        st.session_state.hw_connected = False


def calculate_hand_position(yaw_deg, pitch1_deg, pitch2_deg):
    """손 끝 위치 계산"""
    yaw = np.radians(yaw_deg)
    pitch1 = np.radians(pitch1_deg)
    pitch2 = np.radians(pitch2_deg)
    
    L_UPPER = 2.8
    L_LOWER = 2.4
    
    r1_proj = L_UPPER * np.cos(pitch1)
    p1_x = r1_proj * np.cos(yaw)
    p1_y = r1_proj * np.sin(yaw)
    p1_z = L_UPPER * np.sin(pitch1)
    
    total_pitch = pitch1 + pitch2
    r2_proj = L_LOWER * np.cos(total_pitch)
    p2_x = p1_x + r2_proj * np.cos(yaw)
    p2_y = p1_y + r2_proj * np.sin(yaw)
    p2_z = p1_z + L_LOWER * np.sin(total_pitch)
    
    return (p2_x, p2_y, p2_z)


def create_arm_figure_with_trajectory(shoulder_yaw_deg, shoulder_pitch_deg, elbow_pitch_deg, fingers, trajectory):
    """3D Robot Arm + 궤적 시각화"""
    fig = plt.figure(figsize=(3.5, 2.5), facecolor='#ffffff') 
    ax = fig.add_subplot(111, projection='3d', facecolor='#ffffff')
    
    yaw = np.radians(shoulder_yaw_deg)
    pitch1 = np.radians(shoulder_pitch_deg)
    pitch2 = np.radians(elbow_pitch_deg)
    
    L_UPPER = 2.8
    L_LOWER = 2.4
    
    p0 = np.array([-5.0, 0.0, 0.0])
    
    r1_proj = L_UPPER * np.cos(pitch1)
    p1_x = r1_proj * np.cos(yaw)
    p1_y = r1_proj * np.sin(yaw)
    p1_z = L_UPPER * np.sin(pitch1)
    p1 = np.array([p1_x, p1_y, p1_z])
    
    total_pitch = pitch1 + pitch2
    r2_proj = L_LOWER * np.cos(total_pitch)
    p2_x = p1_x + r2_proj * np.cos(yaw)
    p2_y = p1_y + r2_proj * np.sin(yaw)
    p2_z = p1_z + L_LOWER * np.sin(total_pitch)
    p2 = np.array([p2_x, p2_y, p2_z])
    
    points = [p0, p1, p2]
    colors = ['#3b82f6', '#8b5cf6']
    
    for i in range(2):
        ax.plot([points[i][0], points[i+1][0]],
                [points[i][1], points[i+1][1]],
                [points[i][2], points[i+1][2]],
                color=colors[i], linewidth=4, solid_capstyle='round')
    
    for i, p in enumerate(points):
        size = 45 if i == 0 else 30
        color = '#1e293b' if i == 0 else colors[i-1]
        ax.scatter(*p, s=size, c=color, zorder=5)
    
    finger_colors = ['#22c55e' if f else '#94a3b8' for f in fingers]
    hand_vec = p2 - p1
    norm = np.linalg.norm(hand_vec)
    hand_dir = hand_vec / norm if norm != 0 else np.array([0,0,1])
    
    if np.abs(hand_dir[2]) > 0.99:
        right_vec = np.array([0, 1, 0])
    else:
        right_vec = np.cross(hand_dir, np.array([0, 0, 1]))
        right_vec = right_vec / np.linalg.norm(right_vec)
    
    for i, (f, c) in enumerate(zip(fingers, finger_colors)):
        offset_dist = (i - 1.5) * 0.2
        finger_base = p2 + right_vec * offset_dist
        f_len = 0.5 if f else 0.3
        finger_tip = finger_base + hand_dir * f_len
        ax.plot([finger_base[0], finger_tip[0]], 
                [finger_base[1], finger_tip[1]], 
                [finger_base[2], finger_tip[2]],
                color=c, linewidth=2, solid_capstyle='round')
    
    # 궤적 그리기
    if len(trajectory) > 1:
        traj = list(trajectory)
        xs = [p[0] for p in traj]
        ys = [p[1] for p in traj]
        zs = [p[2] for p in traj]
        
        n = len(traj)
        for i in range(n - 1):
            alpha = 0.1 + 0.9 * (i / n)
            ax.plot([xs[i], xs[i+1]], [ys[i], ys[i+1]], [zs[i], zs[i+1]],
                   color='#f97316', linewidth=2, alpha=alpha)
        
        ax.scatter(xs[-1], ys[-1], zs[-1], s=20, c='#f97316', marker='o', zorder=6)

    LIMIT = 6
    ax.set_xlim([-LIMIT, LIMIT])     
    ax.set_ylim([-LIMIT, LIMIT])    
    ax.set_zlim([-LIMIT, LIMIT])    
    ax.set_box_aspect([1, 1, 1])
    
    ax.xaxis.pane.fill = False
    ax.yaxis.pane.fill = False
    ax.zaxis.pane.fill = False
    ax.xaxis.pane.set_edgecolor('#e2e8f0')
    ax.yaxis.pane.set_edgecolor('#e2e8f0')
    ax.zaxis.pane.set_edgecolor('#e2e8f0')
    
    ticks = np.arange(-LIMIT, LIMIT+1, 2)
    ax.set_xticks(ticks)   
    ax.set_yticks(ticks)  
    ax.set_zticks(ticks)  
    
    ax.grid(True, alpha=0.4, color='#cbd5e1', linestyle='--')
    ax.set_xticklabels([])
    ax.set_yticklabels([])
    ax.set_zticklabels([])
    ax.view_init(elev=20, azim=-45)
    plt.tight_layout(pad=0)
    
    return fig


def create_latency_graph_improved(e2e_history, hz_history):
    """개선된 성능 그래프"""
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(3.5, 2.5), facecolor='#ffffff')
    
    ax1.set_facecolor('#ffffff')
    if e2e_history:
        x = list(range(len(e2e_history)))
        ax1.fill_between(x, e2e_history, alpha=0.3, color='#0ea5e9')
        ax1.plot(x, e2e_history, color='#0ea5e9', lw=1.5)
        avg = sum(e2e_history) / len(e2e_history)
        ax1.axhline(y=avg, color='#f97316', linestyle='--', lw=1, alpha=0.7)
        ax1.set_ylim([0, max(max(e2e_history) * 1.2, 20)])
    else:
        ax1.set_ylim([0, 30])
    
    ax1.set_xlim([0, 100])
    ax1.set_title('E2E Latency (ms)', fontsize=7, fontweight='600', color='#64748b', loc='left', pad=2)
    ax1.tick_params(axis='both', labelsize=6, colors='#94a3b8', length=0)
    ax1.spines['top'].set_visible(False)
    ax1.spines['right'].set_visible(False)
    ax1.spines['left'].set_color('#e2e8f0')
    ax1.spines['bottom'].set_color('#e2e8f0')
    ax1.grid(True, alpha=0.3, color='#e2e8f0')
    ax1.set_xticklabels([])
    
    ax2.set_facecolor('#ffffff')
    if hz_history:
        x = list(range(len(hz_history)))
        ax2.fill_between(x, hz_history, alpha=0.3, color='#10b981')
        ax2.plot(x, hz_history, color='#10b981', lw=1.5)
        avg = sum(hz_history) / len(hz_history)
        ax2.axhline(y=avg, color='#f97316', linestyle='--', lw=1, alpha=0.7)
        ax2.set_ylim([0, max(max(hz_history) * 1.2, 80)])
    else:
        ax2.set_ylim([0, 70])
    
    ax2.set_xlim([0, 100])
    ax2.set_title('Packet Rate (Hz)', fontsize=7, fontweight='600', color='#64748b', loc='left', pad=2)
    ax2.tick_params(axis='both', labelsize=6, colors='#94a3b8', length=0)
    ax2.spines['top'].set_visible(False)
    ax2.spines['right'].set_visible(False)
    ax2.spines['left'].set_color('#e2e8f0')
    ax2.spines['bottom'].set_color('#e2e8f0')
    ax2.grid(True, alpha=0.3, color='#e2e8f0')
    ax2.set_xticklabels([])
    
    plt.tight_layout(pad=0.2)
    return fig


def update_stats(stats, value):
    if value > 0:
        stats['min'] = min(stats['min'], value)
        stats['max'] = max(stats['max'], value)
        stats['sum'] += value
        stats['count'] += 1


def get_stats_avg(stats):
    if stats['count'] > 0:
        return stats['sum'] / stats['count']
    return 0


def get_joint_html_compact(angle1, angle2, angle3):
    joints = [
        ("어깨 (Yaw)", "J1", angle1), 
        ("어깨 (Pitch)", "J2", angle2), 
        ("팔꿈치 (Pitch)", "J3", angle3)
    ]
    
    h = ""
    for name, code, angle in joints:
        cls = "val-pos" if angle >= 0 else "val-neg"
        h += f'''<div class="joint-row">
            <span><span class="joint-name">{name}</span></span>
            <span class="joint-val {cls}">{angle:+.1f}°</span>
        </div>'''
    return f'<div class="joint-grid">{h}</div>'


def get_finger_html_compact(f1, f2, f3, f4):
    fingers = [("엄지", f1), ("검지", f2), ("중지", f3), ("약지", f4)]
    h = ""
    for name, state in fingers:
        active_cls = "finger-active" if state else "finger-inactive"
        status_cls = "finger-on" if state else "finger-off"
        status_text = "ON" if state else "OFF"
        h += f'''<div class="finger-item {active_cls}">
            <div class="finger-name">{name}</div>
            <span class="finger-status {status_cls}">{status_text}</span>
        </div>'''
    return f'<div class="finger-grid">{h}</div>'


def add_log(level, message):
    now_str = datetime.now().strftime('%M:%S')
    st.session_state.logs.insert(0, (now_str, level, message))
    st.session_state.logs = st.session_state.logs[:20]


# === 헤더 ===
header_ph = st.empty()

# === 메인 레이아웃 (3열 구조) ===
col1, col2, col3 = st.columns([1, 1.2, 0.9])

with col1:
    st.markdown('<div class="card"><div class="card-title">🤖 로봇 시뮬레이션 + 궤적</div>', unsafe_allow_html=True)
    graph_ph = st.empty()
    st.markdown('<div style="height:5px"></div>', unsafe_allow_html=True)
    joint_ph = st.empty()
    st.markdown('</div>', unsafe_allow_html=True)

with col2:
    st.markdown('<div class="card"><div class="card-title">📊 실시간 성능</div>', unsafe_allow_html=True)
    perf_graph_ph = st.empty()
    st.markdown('</div>', unsafe_allow_html=True)
    
    st.markdown('<div class="card"><div class="card-title">📋 시스템 로그</div>', unsafe_allow_html=True)
    log_ph = st.empty()
    st.markdown('</div>', unsafe_allow_html=True)

with col3:
    st.markdown('<div class="card"><div class="card-title">🔗 연결 상태</div>', unsafe_allow_html=True)
    conn_ph = st.empty()
    st.markdown('</div>', unsafe_allow_html=True)

    st.markdown('<div class="card"><div class="card-title">⚡ 핵심 지표</div>', unsafe_allow_html=True)
    metrics_ph = st.empty()
    stats_ph = st.empty()
    st.markdown('</div>', unsafe_allow_html=True)

    st.markdown('<div class="card"><div class="card-title">✋ 그리퍼</div>', unsafe_allow_html=True)
    finger_ph = st.empty()
    st.markdown('</div>', unsafe_allow_html=True)


# 초기 로그
if len(st.session_state.logs) == 0:
    if st.session_state.hw_connected:
        add_log("INFO", "공유 메모리 연결됨")
        add_log("INFO", "궤적 추적 활성화")
    else:
        add_log("WARN", "공유 메모리 연결 실패")
        add_log("INFO", "main.py 먼저 실행하세요")


# === 메인 루프 ===
while True:
    now = datetime.now()
    t = time.time() - st.session_state.start_time
    
    # 실제 데이터 읽기
    if st.session_state.shm_reader and st.session_state.hw_connected:
        try:
            data = st.session_state.shm_reader.read()
            angle1 = data.angle1
            angle2 = data.angle2
            angle3 = data.angle3
            f1 = data.finger1
            f2 = data.finger2
            f3 = data.finger3
            f4 = data.finger4
            e2e = data.e2e_latency_ms
            hz = data.hz
            latency_us = data.latency_us
            rtt = e2e * 2
            st.session_state.packet_count = data.packet_count
            is_connected = data.is_connected
        except:
            st.session_state.hw_connected = False
            is_connected = False
            angle1 = angle2 = angle3 = 0
            f1 = f2 = f3 = f4 = 0
            e2e = rtt = hz = latency_us = 0
    else:
        # 연결 안 됨
        angle1 = angle2 = angle3 = 0
        f1 = f2 = f3 = f4 = 0
        e2e = rtt = hz = latency_us = 0
        is_connected = False
    
    # 궤적 업데이트
    hand_pos = calculate_hand_position(angle1, angle2, angle3)
    st.session_state.trajectory.append(hand_pos)
    
    # 통계 업데이트
    update_stats(st.session_state.e2e_stats, e2e)
    update_stats(st.session_state.rtt_stats, rtt)
    
    # 상태 변수
    hw_conn = st.session_state.hw_connected
    status_class = "status-ok" if hw_conn else "status-warn"
    dot_class = "dot-ok" if hw_conn else "dot-warn"
    status_text = "Connected" if hw_conn else "Disconnected"
    
    # 헤더 업데이트
    header_ph.markdown(f'''
    <div class="header">
        <div class="logo">Med<span>Arm</span> Pro</div>
        <div class="status-badge {status_class}"><span class="status-dot {dot_class}"></span> {status_text}</div>
        <div class="datetime">{now.strftime("%H:%M:%S")}<br>{now.strftime("%Y-%m-%d")}</div>
    </div>
    ''', unsafe_allow_html=True)
    
    # 연결 상태 업데이트
    bt_class = "conn-ok" if hw_conn else "conn-warn"
    bt_dot = "dot-ok" if hw_conn else "dot-warn"
    bt_text = "연결됨" if hw_conn else "연결 안됨"
    
    conn_ph.markdown(f'''
    <div class="conn-grid">
        <div class="conn-item">
            <div class="conn-label">블루투스 (BT)</div>
            <div class="conn-value {bt_class}">
                <span class="conn-dot {bt_dot}"></span>
                {bt_text}
            </div>
        </div>
        <div class="conn-item">
            <div class="conn-label">공유 메모리 (IPC)</div>
            <div class="conn-value {bt_class}">
                <span class="conn-dot {bt_dot}"></span>
                {bt_text}
            </div>
        </div>
        <div class="conn-item">
            <div class="conn-label">현재 RTT</div>
            <div class="conn-value conn-ok">{rtt:.1f} ms</div>
        </div>
    </div>
    ''', unsafe_allow_html=True)
    
    # 메트릭 업데이트
    metrics_ph.markdown(f'''
    <div class="metric-grid">
        <div class="metric-box">
            <div class="metric-label">E2E Latency</div>
            <span class="metric-value metric-highlight">{e2e:.1f}<span class="metric-unit">ms</span></span>
        </div>
        <div class="metric-box">
            <div class="metric-label">Rate</div>
            <span class="metric-value">{hz:.1f}<span class="metric-unit">Hz</span></span>
        </div>
        <div class="metric-box">
            <div class="metric-label">RTT</div>
            <span class="metric-value">{rtt:.1f}<span class="metric-unit">ms</span></span>
        </div>
        <div class="metric-box">
            <div class="metric-label">Total Pkts</div>
            <span class="metric-value">{st.session_state.packet_count:,}</span>
        </div>
    </div>
    ''', unsafe_allow_html=True)
    
    # 통계 표시
    e2e_avg = get_stats_avg(st.session_state.e2e_stats)
    e2e_min = st.session_state.e2e_stats['min'] if st.session_state.e2e_stats['count'] > 0 else 0
    e2e_max = st.session_state.e2e_stats['max'] if st.session_state.e2e_stats['count'] > 0 else 0
    
    stats_ph.markdown(f'''
    <div class="stat-grid">
        <div class="stat-box">
            <div class="stat-label">E2E 최소</div>
            <div class="stat-value">{e2e_min:.1f}ms</div>
        </div>
        <div class="stat-box">
            <div class="stat-label">E2E 평균</div>
            <div class="stat-value">{e2e_avg:.1f}ms</div>
        </div>
        <div class="stat-box">
            <div class="stat-label">E2E 최대</div>
            <div class="stat-value">{e2e_max:.1f}ms</div>
        </div>
    </div>
    ''', unsafe_allow_html=True)
    
    # 컴포넌트 업데이트
    joint_ph.markdown(get_joint_html_compact(angle1, angle2, angle3), unsafe_allow_html=True)
    finger_ph.markdown(get_finger_html_compact(f1, f2, f3, f4), unsafe_allow_html=True)
    
    st.session_state.e2e_history.append(e2e)
    st.session_state.hz_history.append(hz)
    st.session_state.e2e_history = st.session_state.e2e_history[-80:]
    st.session_state.hz_history = st.session_state.hz_history[-80:]
    
    # 그래프 업데이트
    st.session_state.graph_counter += 1
    if st.session_state.graph_counter >= GRAPH_UPDATE_INTERVAL:
        st.session_state.graph_counter = 0
        
        fig = create_arm_figure_with_trajectory(
            angle1, angle2, angle3, 
            [f1, f2, f3, f4], 
            st.session_state.trajectory
        )
        graph_ph.pyplot(fig, use_container_width=True)
        plt.close(fig)
        
        perf_fig = create_latency_graph_improved(
            st.session_state.e2e_history, 
            st.session_state.hz_history
        )
        perf_graph_ph.pyplot(perf_fig, use_container_width=True)
        plt.close(perf_fig)
    
    # 로그
    if st.session_state.packet_count % 50 == 0 and st.session_state.packet_count > 0:
        msg = f"#{st.session_state.packet_count} | E2E={e2e:.1f}ms | RTT={rtt:.1f}ms"
        add_log("INFO", msg)
    
    log_html = ""
    for t_log, level, msg in st.session_state.logs[:10]:
        level_class = f"log-{level.lower()}"
        log_html += f'<div class="log-line"><span class="log-time">[{t_log}]</span> <span class="{level_class}">[{level}]</span> {msg}</div>'
    log_ph.markdown(f'<div class="log-box">{log_html}</div>', unsafe_allow_html=True)
    
    time.sleep(FAST_UPDATE_MS / 1000)