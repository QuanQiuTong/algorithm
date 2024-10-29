import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

def compute_edit_distance_matrix(ref, query):
    m, n = len(ref), len(query)
    dp = np.zeros((m + 1, n + 1), dtype=int)

    # 初始化第一行和第一列
    for i in range(m + 1):
        dp[i][0] = i
    for j in range(n + 1):
        dp[0][j] = j

    # 填充编辑距离矩阵
    for i in range(1, m + 1):
        for j in range(1, n + 1):
            if ref[i - 1] == query[j - 1]:
                dp[i][j] = dp[i - 1][j - 1]
            else:
                dp[i][j] = min(dp[i - 1][j] + 1,    # 删除
                               dp[i][j - 1] + 1,    # 插入
                               dp[i - 1][j - 1] + 1) # 替换
        
        print(f"Processed {i} rows.")

    return dp

def plot_edit_distance_matrix(dp):
    m, n = dp.shape
    x, y = np.meshgrid(range(n), range(m))
    z = dp

    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')
    ax.plot_surface(x, y, z, cmap='viridis')

    ax.set_xlabel('j')
    ax.set_ylabel('i')
    ax.set_zlabel('dp[i][j]')
    ax.set_title('编辑距离矩阵')

    plt.show()

def main():
    with open('reference.txt', 'r') as f:
        ref = f.read().strip()
    
    with open('query.txt', 'r') as f:
        query = f.read().strip()

    print("Loaded.")

    dp_matrix = compute_edit_distance_matrix(ref, query)

    print("Computed.")

    plot_edit_distance_matrix(dp_matrix)

if __name__ == "__main__":
    main()