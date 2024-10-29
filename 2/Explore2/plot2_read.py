import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

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
    dp_matrix = np.loadtxt('prefix_20.csv', delimiter=',', dtype=int)

    print("Data loaded.")

    plot_edit_distance_matrix(dp_matrix)

if __name__ == "__main__":
    main()