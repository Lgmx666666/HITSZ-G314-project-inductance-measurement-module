from dynamixel_sdk import PortHandler, PacketHandler
class SeedActuators:
    def __init__(self, port='COM3', baudrate=1000000, protocol_version=1.0):
        # 设置通信参数
        self.PORT = port
        self.BAUDRATE = baudrate
        self.PROTOCOL_VERSION = protocol_version

        # SEED Actuators 控制表地址
        self.ADDR_TORQUE_ENABLE = 24
        self.ADDR_OPERATING_MODE = 90
        self.ADDR_GOAL_POSITION = 30
        self.ADDR_PRESENT_POSITION = 36
        self.ADDR_GOAL_VELOCITY = 32
        self.ADDR_PRESENT_VELOCITY = 34

        # SEED Actuators 电机ID
        self.actuators_id = {'wrist rotation': 31,
                             'wrist abduction': 32,
                             'wrist flexion': 33,
                             'thumb abduction': 34,
                             'thumb flexion': 35,
                             'index flexion': 36,
                             'middle flexion': 37,
                             'ring flexion': 38}

        # SEED actuators 电机初始化位置和极限位置
        self.actuators_init_position = {'wrist rotation': 3200,
                                        'wrist abduction': 2048,
                                        'wrist flexion': 2048,
                                        'thumb abduction': 1300,
                                        'thumb flexion': 0,
                                        'index flexion': 0,
                                        'middle flexion': 0,
                                        'ring flexion': 0}

        self.actuators_limit_position = {'wrist rotation': (0, 4095),
                                         'wrist abduction': (0, 4095),
                                         'wrist flexion': (0, 4095),
                                         'thumb abduction': (0, 4095),
                                         'thumb flexion': (0, 2000),
                                         'index flexion': (0, 3000),
                                         'middle flexion': (0, 3000),
                                         'ring flexion': (0, 3000)}

        # 初始化 PortHandler 和 PacketHandler
        self.port_handler = PortHandler(self.PORT)
        self.packet_handler = PacketHandler(self.PROTOCOL_VERSION)

        # 尝试打开端口
        try:
            if self.port_handler.openPort():
                print("端口已打开")
            else:
                print("无法打开端口，跳过初始化")
                self.port_handler = None  # 标记端口未打开
                return

            # 设置波特率
            if self.port_handler.setBaudRate(self.BAUDRATE):
                print("波特率已设置")
            else:
                print("无法设置波特率，跳过初始化")
                self.port_handler.closePort()
                self.port_handler = None  # 标记端口未打开
                return
        except Exception as e:
            print(f"连接端口时发生错误: {e}，跳过初始化")
            self.port_handler = None  # 标记端口未打开
            return

        # 初始化所有电机
        self.init_seed_robotic_hand()

    def enable_torque(self, motor_id):
        """启用电机扭矩"""
        if self.port_handler is None:
            print("端口未打开，无法启用扭矩")
            return
        self.packet_handler.write1ByteTxRx(self.port_handler, motor_id, self.ADDR_TORQUE_ENABLE, 1)

    def disable_torque(self, motor_id):
        """禁用电机扭矩"""
        if self.port_handler is None:
            print("端口未打开，无法禁用扭矩")
            return
        self.packet_handler.write1ByteTxRx(self.port_handler, motor_id, self.ADDR_TORQUE_ENABLE, 0)

    def set_position_mode(self, motor_id):
        """设置操作模式为位置模式"""
        if self.port_handler is None:
            print("端口未打开，无法设置位置模式")
            return
        self.packet_handler.write1ByteTxRx(self.port_handler, motor_id, self.ADDR_OPERATING_MODE, 3)  # 3 表示位置模式

    def set_goal_velocity(self, motor_id, velocity):
        """设置目标速度"""
        if self.port_handler is None:
            print("端口未打开，无法设置目标速度")
            return
        self.packet_handler.write4ByteTxRx(self.port_handler, motor_id, self.ADDR_GOAL_VELOCITY, velocity)

    def set_goal_position(self, motor_id, position):
        """设置目标位置"""
        if self.port_handler is None:
            print("端口未打开，无法设置目标位置")
            return
        self.packet_handler.write4ByteTxRx(self.port_handler, motor_id, self.ADDR_GOAL_POSITION, position)

    def get_present_position(self, motor_id):
        """读取当前位置"""
        if self.port_handler is None:
            print("端口未打开，无法读取位置")
            return None
        position, result, error = self.packet_handler.read4ByteTxRx(self.port_handler, motor_id, self.ADDR_PRESENT_POSITION)
        if result == 0:
            return position
        else:
            print(f"读取位置失败，电机 ID: {motor_id}")
            return None

    def set_motor_position(self, motor_id, target_position):
        """设置电机位置"""
        if self.port_handler is None:
            print("端口未打开，无法设置电机位置")
            return

        # 设置目标位置
        self.set_goal_position(motor_id, target_position)

    def close(self):
        """关闭端口并禁用所有电机扭矩"""
        if self.port_handler is None:
            print("端口未打开，无需关闭")
            return

        for motor_id in range(41, 49):  # 假设电机编号为 41-48
            self.disable_torque(motor_id)
        self.port_handler.closePort()
        print("端口已关闭")

    def init_seed_robotic_hand(self):
        """初始化 SEED 手部"""
        for actuator_name, motor_id in self.actuators_id.items():
            init_pos = self.actuators_init_position.get(actuator_name, 0)
            self.set_position_mode(motor_id)  # 设置为位置模式
            self.enable_torque(motor_id)  # 启用扭矩
            self.set_motor_position(motor_id, init_pos)  # 设置电机位置

    def ctl_fingers(self, thumb_target_pos, index_target_pos, middle_target_pos, ring_target_pos):
        """控制手指位置
        "@param thumb_target_pos: 大拇指目标位置,百分比表示   0-100
        "@param index_target_pos: 食指目标位置,百分比表示     0-100
        "@param middle_target_pos: 中指目标位置,百分比表示    0-100
        "@param ring_target_pos: 无名指目标位置,百分比表示     0-100
        """
        # 计算目标位置
        thumb_absolute_pos = int(thumb_target_pos * (self.actuators_limit_position['thumb flexion'][1] - self.actuators_limit_position['thumb flexion'][0]) / 100)
        index_absolute_pos = int(index_target_pos * (self.actuators_limit_position['index flexion'][1] - self.actuators_limit_position['index flexion'][0]) / 100)
        middle_absolute_pos = int(middle_target_pos * (self.actuators_limit_position['middle flexion'][1] - self.actuators_limit_position['middle flexion'][0]) / 100)
        ring_absolute_pos = int(ring_target_pos * (self.actuators_limit_position['ring flexion'][1] - self.actuators_limit_position['ring flexion'][0]) / 100)

        # 更新电机位置
        self.set_motor_position(self.actuators_id['thumb flexion'], thumb_absolute_pos)
        self.set_motor_position(self.actuators_id['index flexion'], index_absolute_pos)
        self.set_motor_position(self.actuators_id['middle flexion'], middle_absolute_pos)
        self.set_motor_position(self.actuators_id['ring flexion'], ring_absolute_pos)
