from flask import Flask, send_from_directory
from flask_socketio import SocketIO
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
import pandas as pd
import threading
import eventlet
import numpy as np

# Flask 앱 및 WebSocket 초기화
app = Flask(__name__, static_folder='static')
socketio = SocketIO(app, cors_allowed_origins="*")

# 전역 데이터 변수
cached_data = {}
previous_data = {}

# CSV 변경 감지 핸들러
class CSVChangeHandler(FileSystemEventHandler):
    def on_modified(self, event):
        if event.src_path.endswith("data.csv"):
            print("[DEBUG] CSV 파일 변경 감지:", event.src_path)
            global cached_data
            try:
                # CSV 파일 읽기
                data = pd.read_csv("data.csv", header=0, on_bad_lines="skip")
                print("[DEBUG] CSV 데이터:", data)

                # 데이터 처리 및 업데이트
                arduino1 = data[data["Arduino"] == 1].iloc[-1] if not data[data["Arduino"] == 1].empty else {}
                arduino2 = data[data["Arduino"] == 2].iloc[-1] if not data[data["Arduino"] == 2].empty else {}

                cached_data["arduino_1"] = {
                    "ph": arduino1.get("PH", None),
                    "light": arduino1.get("Light", None),
                    "soil": arduino1.get("Soil", None),
                    "humidity": arduino1.get("Humidity", None),
                    "temperature": arduino1.get("Temperature", None),
                }
                cached_data["arduino_2"] = {
                    "ph": arduino2.get("PH", None),
                    "light": arduino2.get("Light", None),
                    "soil": arduino2.get("Soil", None),
                    "humidity": arduino2.get("Humidity", None),
                    "temperature": arduino2.get("Temperature", None),
                }
                print("[DEBUG] 업데이트된 cached_data:", cached_data)
            except Exception as e:
                print("[ERROR] CSV 처리 중 오류:", e)

# Watchdog 실행 함수
def start_watchdog():
    event_handler = CSVChangeHandler()
    observer = Observer()
    observer.schedule(event_handler, path=".", recursive=False)
    observer.start()
    try:
        while True:
            eventlet.sleep(1)  # Eventlet sleep을 사용하여 서버 동작 유지
    except KeyboardInterrupt:
        observer.stop()
    observer.join()

def broadcast_data():
    global previous_data
    while True:
        socketio.sleep(1)
        if cached_data != previous_data:  # 데이터 변경 여부 확인
            # 데이터 직렬화 문제 해결: numpy 데이터 타입을 Python 기본 타입으로 변환
            serializable_data = {
                "arduino_1": {k: int(v) if isinstance(v, (np.integer, int)) else v for k, v in cached_data["arduino_1"].items()},
                "arduino_2": {k: int(v) if isinstance(v, (np.integer, int)) else v for k, v in cached_data["arduino_2"].items()}
            }
            previous_data = cached_data.copy()
            print("[INFO] 데이터가 변경되어 전송됩니다:", serializable_data)
            socketio.emit('update_data', serializable_data)  # 수정된 데이터를 전송


# 라우팅
@app.route('/')
def serve_index():
    return send_from_directory(app.static_folder, 'index.html')

@app.route('/about')
def serve_about():
    return send_from_directory(app.static_folder, 'about.html')

@app.route('/member')
def serve_member():
    return send_from_directory(app.static_folder, 'member.html')


# WebSocket 연결
@socketio.on('connect')
def handle_connect():
    print("[INFO] 클라이언트가 연결되었습니다.")

# 메인 실행
if __name__ == '__main__':
    # Watchdog 스레드 실행
    watchdog_thread = threading.Thread(target=start_watchdog, daemon=True)
    watchdog_thread.start()

    # WebSocket 데이터 전송 백그라운드 태스크 실행
    socketio.start_background_task(broadcast_data)

    # 서버 실행
    eventlet.wsgi.server(eventlet.listen(('', 9999)), app)