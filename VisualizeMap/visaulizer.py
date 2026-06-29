import numpy as np
import matplotlib.pyplot as plt


file_path = 'C:/Unreal/MMOGameServer/VisualizeMap/LobbyMap.dat'
y_size = 12000
x_size = 12000

# uint8 형식으로 파일 읽기
with open(file_path, 'rb') as f:
    data = np.fromfile(f, dtype=np.uint8)

data = data.reshape((y_size, x_size))

plt.imshow(data, cmap='gray', interpolation='nearest')
plt.title('ap Visualization')
plt.axis('off')  # 축 제거
plt.show()