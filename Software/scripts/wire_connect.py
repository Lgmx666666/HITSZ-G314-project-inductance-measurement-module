import serial
import pandas as pd
from pynput import keyboard
import threading
import time
from datetime import datetime
from seed_actuators import SeedActuators
import queue
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# 初始化 SEED 机械手  右手
# my_seed_hand = SeedActuators('COM3', baudrate=1000000, protocol_version=1.0)  # 请替换为实际的串口号
# my_seed_hand.init_seed_robotic_hand()

# 初始化串口
ser = serial.Serial('COM8', 115200)  # 根据实际情况修改串口号和波特率

# 传感器限值
sensor_limits = {
    'sensor 1': (1.610, 1.90),
    'sensor 2': (1.620, 1.90),
    'sensor 3': (1.615, 1.75),
    'sensor 4': (1.615, 1.75)
}

# 数据记录相关变量
recording = False
data_records = []
lock = threading.Lock()  # 用于线程同步

# 存储手指位置百分比
ring_pos_percent = 0
middle_pos_percent = 0
index_pos_percent = 0
thumb_pos_percent = 0

# 信号量，用于线程间通信
update_finger_positions_event = threading.Event()

# 全局变量
inductance_data = {
    'Sensor 1': [],
    'Sensor 2': [],
    'Sensor 3': [],
    'Sensor 4': []
}
# 数据队列，用于线程间通信
data_queue = queue.Queue(maxsize=100)  # 限制队列大小，避免内存溢出

# 实时绘图的数据和变量
sensor_values = [0.0, 0.0, 0.0, 0.0]
fig, axs = plt.subplots(2, 2, figsize=(10, 8))
fig.suptitle("Real-time Sensor Data")
plot_data = [[0.0] * 101 for _ in range(4)]

lines = []
texts = []
for i, ax in enumerate(axs.flat):
    line, = ax.plot([], [], label=f"Channel {i+1}")
    lines.append(line)
    text = ax.text(0.85, 0.9, "", transform=ax.transAxes, fontsize=10, color='red')
    texts.append(text)
    ax.set_xlim(0, 100)
    ax.legend()
    ax.grid()


def update(frame):
    global plot_data
    new_data = None

    # 获取新数据
    while not data_queue.empty():
        new_data = data_queue.get()
        current_time = time.time()  # 获取当前时间（秒，浮点数）

        for i in range(4):
            # 确保队列长度始终为 101
            plot_data[i].append(new_data[i])
            if len(plot_data[i]) > 101:  # 如果超过长度限制，移除最旧的数据
                plot_data[i].pop(0)

    if new_data is not None:
        for i, ax in enumerate(axs.flat):
            # 确保横坐标始终为 0-100
            x_coords = range(101)

            # 更新线条数据
            lines[i].set_data(x_coords, plot_data[i])

            # 动态调整纵坐标范围
            min_val = min(plot_data[i])
            max_val = max(plot_data[i])
            ax.set_ylim(min_val - 0.1, max_val + 0.1)

            # 更新实时值显示
            texts[i].set_text(f"{new_data[i]:.4f}")

    # 返回更新后的元素
    return lines + texts


# 串口数据接收线程
def read_serial_data():
    global recording, data_records, ring_pos_percent, middle_pos_percent, index_pos_percent, thumb_pos_percent
    while True:
        if ser.in_waiting > 0:
            # 读取串口数据
            data = ser.readline().decode('utf-8', errors='ignore').strip()
            if data.startswith("inductance:"):
                # 解析数据
                inductance_values = [float(value) for value in data[len("inductance:"):].split(',')]
                data_queue.put(inductance_values)
                ring_pos_percent = max(0, min(100, int((sensor_limits['sensor 1'][1] - inductance_values[0]) /
                                                       (sensor_limits['sensor 1'][1] - sensor_limits['sensor 1'][
                                                           0]) * 100)))
                middle_pos_percent = max(0, min(100, int((sensor_limits['sensor 2'][1] - inductance_values[1]) /
                                                         (sensor_limits['sensor 2'][1] - sensor_limits['sensor 2'][
                                                             0]) * 100)))
                index_pos_percent = max(0, min(100, int((sensor_limits['sensor 3'][1] - inductance_values[2]) /
                                                        (sensor_limits['sensor 3'][1] - sensor_limits['sensor 3'][
                                                            0]) * 100)))
                thumb_pos_percent = max(0, min(100, int((sensor_limits['sensor 4'][1] - inductance_values[3]) /
                                                        (sensor_limits['sensor 4'][1] - sensor_limits['sensor 4'][
                                                            0]) * 100)))

                # 如果正在记录，则保存数据和时间戳
                if recording:
                    with lock:
                        timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]  # 精确到毫秒
                        data_records.append({
                            'Timestamp': timestamp,
                            'Sensor 1 (Ring)': inductance_values[0],
                            'Ring': ring_pos_percent,
                            'Sensor 2 (Middle)': inductance_values[1],
                            'Middle': middle_pos_percent,
                            'Sensor 3 (Index)': inductance_values[2],
                            'Index': index_pos_percent,
                            'Sensor 4 (Thumb)': inductance_values[3],
                            'Thumb': thumb_pos_percent
                        })
                # 设置手指位置更新事件，触发 ctl_fingers 线程
                update_finger_positions_event.set()

            else:
                print("Invalid data received:", data)




# 键盘监听线程
def monitor_keyboard():
    global recording, data_records

    def on_press(key):
        global recording
        try:
            if key.char == 'r':  # 开始记录
                if not recording:
                    print("Started recording...")
                    with lock:
                        data_records = []  # 清空旧数据
                    recording = True
            elif key.char == 's':  # 停止记录并保存s
                if recording:
                    print("Stopped recording. Saving data...")
                    recording = False
                    with lock:
                        save_data_to_excel()
        except AttributeError:
            pass

    with keyboard.Listener(on_press=on_press) as listener:
        listener.join()


# 保存数据到 Excel 文件
def save_data_to_excel():
    global data_records
    if data_records:
        df = pd.DataFrame(data_records)
        filename = f"./data/20250217exp-soft-1-sensor_data_{datetime.now().strftime('%Y%m%d_%H%M%S')}.xlsx"
        df.to_excel(filename, index=False)
        print(f"Data saved to {filename}")
    else:
        print("No data to save.")


# ctl_fingers 控制手指函数，50ms周期运行
# def ctl_fingers_thread():
#     global ring_pos_percent, middle_pos_percent, index_pos_percent, thumb_pos_percent
#     while True:
#         # 等待数据更新信号
#         update_finger_positions_event.wait()  # 等待直到触发更新
#         # 调用 SEED 机械手的 ctl_fingers 方法
#         my_seed_hand.ctl_fingers(thumb_pos_percent, index_pos_percent, middle_pos_percent, ring_pos_percent)
#         print(
#             f"Fingers controlled - Ring: {ring_pos_percent}%, Middle: {middle_pos_percent}%, Index: {index_pos_percent}%, Thumb: {thumb_pos_percent}%")
#         # 重置事件，准备下一次更新
#         update_finger_positions_event.clear()
#         # 等待50ms再进行下一次更新
#         time.sleep(0.01)


# 主函数
if __name__ == "__main__":
    # 启动串口数据接收线程
    serial_thread = threading.Thread(target=read_serial_data, daemon=True)
    serial_thread.start()

    # 启动键盘监听线程
    keyboard_thread = threading.Thread(target=monitor_keyboard, daemon=True)
    keyboard_thread.start()

    # 启动 ctl_fingers 控制线程，每50ms执行一次
    # ctl_fingers_thread = threading.Thread(target=ctl_fingers_thread, daemon=True)
    # ctl_fingers_thread.start()

    ani = FuncAnimation(fig, update, interval=50, blit=True)

    # 主线程保持运行
    try:
        plt.show()
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("Program terminated.")
