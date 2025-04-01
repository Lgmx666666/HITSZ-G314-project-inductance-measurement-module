import asyncio
import threading
import queue
import signal
import atexit
from bleak import BleakClient
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import pandas as pd
import time
from pynput import keyboard
from seed_actuators import SeedActuators
from bleak import BleakScanner  # 添加必要的导入

my_seed_hand = SeedActuators('COM3', baudrate=1000000, protocol_version=1.0)  # 请替换为实际的串口号
my_seed_hand.init_seed_robotic_hand()

# 初始化数据队列
data_queue = queue.Queue(maxsize=100)

# 定义每个通道的实际变化范围
sensor_limits = {'sensor 1':(1.625, 1.73),
                 'sensor 2':(1.625, 1.73),
                 'sensor 3':(1.620, 1.75),
                 'sensor 4':(1.620, 1.73)}

# 替换为目标设备的地址
device_address = {'device1': "00:80:E1:26:9D:DB",
                  'device2': "00:80:E1:26:9F:4E",
                  'device3': "00:80:E1:26:9A:5A",
                  'device4': "00:80:E1:26:9D:D7",
                  'device5': "00:80:E1:26:9F:57",  #疑似坏的，ldc1614通道测量有问题
                  'device6': "00:80:E1:26:9B:BD",
                  'device7': "00:80:E1:26:99:A4",
                  'device8': "00:80:E1:26:9E:54",
                  'device9': "00:80:E1:26:9C:AC",
                  'device10': "00:80:E1:26:9F:5F"  #疑似坏的，ldc1614通道测量有问题
                  }

DEVICE_ADDRESS = device_address['device2']

# 心率测量特征的 UUID
HEART_RATE_MEASUREMENT_UUID = "00002A37-0000-1000-8000-00805F9B34FB"

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

# 初始化 BleakClient
client = BleakClient(DEVICE_ADDRESS)

# 数据记录状态和存储
recording = False
recorded_data = []

# 更新图表函数
def update(frame):
    global plot_data, recorded_data, recording
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

        # 如果正在记录数据，保存到列表
        if recording:
            recorded_data.append([current_time] + new_data)

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


# 蓝牙通知处理函数
def notification_handler(sender, data):
    byte_list = list(data)
    reversed_byte_list = byte_list[::-1]
    sensor_data_bytes = reversed_byte_list[4:12]

    sensor_values = []
    for i in range(0, len(sensor_data_bytes), 2):
        sensor_raw = (sensor_data_bytes[i] << 8) | sensor_data_bytes[i + 1]
        sensor_value = sensor_raw / 10000.0
        sensor_values.append(sensor_value)

    if len(sensor_values) == 4 and not data_queue.full():
        data_queue.put(sensor_values)

# 异步函数：连接蓝牙设备
from bleak import BleakScanner  # 添加必要的导入


# 异步函数：连接蓝牙设备
async def connect_and_receive_data():
    global client

    print("正在扫描蓝牙设备...")
    try:
        devices = await BleakScanner.discover(timeout=10.0)
        print("扫描到的设备:")
        for d in devices:
            print(f"设备地址: {d.address}, 名称: {d.name}, RSSI: {d.rssi}")

        print(f"正在连接设备 {DEVICE_ADDRESS}...")
        await client.connect(timeout=10.0)
        print("蓝牙设备连接成功")

        await client.start_notify(HEART_RATE_MEASUREMENT_UUID, notification_handler)
        print("已启用通知，开始接收数据...")

        while True:
            await asyncio.sleep(1)
    except asyncio.CancelledError:
        print("程序终止，正在停止通知并断开蓝牙连接...")
    except Exception as e:
        print(f"发生错误: {e}")
    finally:
        await cleanup()


# 蓝牙清理操作
async def cleanup():
    if client.is_connected:
        await client.stop_notify(HEART_RATE_MEASUREMENT_UUID)
        print("已停止通知")
        await client.disconnect()
        print("蓝牙连接已断开")

# 捕获退出信号
def handle_exit(*args):
    print("程序终止，正在清理资源...")
    loop = asyncio.get_event_loop()
    loop.run_until_complete(cleanup())
    loop.stop()

# 绑定退出信号
signal.signal(signal.SIGINT, handle_exit)
signal.signal(signal.SIGTERM, handle_exit)
atexit.register(lambda: asyncio.run(cleanup()))

# 启动蓝牙后台线程
def bluetooth_thread():
    asyncio.run(connect_and_receive_data())

bluetooth_worker = threading.Thread(target=bluetooth_thread, daemon=True)
bluetooth_worker.start()

#  ------------------------------------------------------------------------------------------- 用户定义的数据处理线程
def user_defined_thread():
    while True:
        if not data_queue.empty():
            # 获取最新数据
            latest_data = data_queue.get()
            # print(f"User-defined thread received data: {latest_data}")
            thumb_pos_percent = max(0, min(100, int((sensor_limits['sensor 1'][1] - latest_data[0]) /
                                                    (sensor_limits['sensor 1'][1] - sensor_limits['sensor 1'][
                                                        0]) * 100)))

            index_pos_percent = max(0, min(100, int((sensor_limits['sensor 2'][1] - latest_data[1]) /
                                                    (sensor_limits['sensor 2'][1] - sensor_limits['sensor 2'][
                                                        0]) * 100)))

            middle_pos_percent = max(0, min(100, int((sensor_limits['sensor 3'][1] - latest_data[2]) /
                                                     (sensor_limits['sensor 3'][1] - sensor_limits['sensor 3'][
                                                         0]) * 100)))

            ring_pos_percent = max(0, min(100, int((sensor_limits['sensor 4'][1] - latest_data[3]) /
                                                   (sensor_limits['sensor 4'][1] - sensor_limits['sensor 4'][
                                                       0]) * 100)))
            my_seed_hand.ctl_fingers(thumb_pos_percent, index_pos_percent, middle_pos_percent, ring_pos_percent)
            print(f"Thumb: {thumb_pos_percent}%, Index: {index_pos_percent}%, Middle: {middle_pos_percent}%, Ring: {ring_pos_percent}%")
        time.sleep(0.1)  # 调整运行频率

# 启动用户定义线程
user_thread = threading.Thread(target=user_defined_thread, daemon=True)
user_thread.start()

# 键盘监听事件
def on_press(key):
    global recording, recorded_data

    try:
        if key.char == 'r':
            recording = True
            recorded_data = []  # 清空之前的数据
            print("开始记录数据...")
        elif key.char == 's':
            recording = False
            print("停止记录数据，正在保存到文件...")
            if recorded_data:
                # 更新时间戳处理，精确到毫秒
                df = pd.DataFrame(recorded_data, columns=["Timestamp", "Sensor1", "Sensor2", "Sensor3", "Sensor4"])
                df["Timestamp"] = pd.to_datetime(df["Timestamp"], unit="s")  # 转换为时间戳（秒）
                df["Timestamp"] = df["Timestamp"].apply(lambda x: x.strftime('%Y-%m-%d %H:%M:%S.%f')[:-3])  # 精确到毫秒
                df.to_excel("./data/robotic-hand-6-repeat.xlsx", index=False)
                print("数据已保存为xlsx")
            else:
                print("没有记录到数据，文件未生成。")
    except AttributeError:
        pass

# 启动键盘监听线程
keyboard_listener = keyboard.Listener(on_press=on_press)
keyboard_listener.start()

# 实现动态更新图表
ani = FuncAnimation(fig, update, interval=50, blit=True)

try:
    plt.show()
except KeyboardInterrupt:
    print("程序已被用户中止")
