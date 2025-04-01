import numpy as np

# 定义常量
OO_prime = 28  # mm
r = 44  # mm
l0 = 16  # mm
phi = 0  # rad

# 定义角度范围
angles_deg = np.arange(0, 91, 10)  # 0°到90°，步进10°
angles_rad = np.deg2rad(angles_deg)  # 转换为弧度

# 计算滑轨位移
displacements = []

for theta in angles_rad:
    # 计算OA
    OA_squared = OO_prime ** 2 + r ** 2 - 2 * OO_prime * r * np.cos(theta + phi)
    OA = np.sqrt(OA_squared)

    # 计算cos(gamma)
    cos_gamma = (2 * r ** 2 - OA_squared - l0 ** 2 + 2 * l0 * OA * np.cos(theta)) / (2 * r ** 2)

    # 计算gamma
    gamma = np.arccos(cos_gamma)

    # 计算滑轨位移d
    d = OA - l0
    displacements.append(d)

# 输出结果
for deg, disp in zip(angles_deg, displacements):
    print(f"角度: {deg}°, 滑轨位移: {disp:.2f} mm")