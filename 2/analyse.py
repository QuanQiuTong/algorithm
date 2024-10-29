import matplotlib.pyplot as plt
from threading import Thread


def draw(filename):
    p = []

    # lines of integers
    with open(filename, "r") as f:
        for line in f:
            p.append(int(line))

    # plot

    plt.plot(p)
    plt.xlabel("Prefix Length")
    plt.ylabel("Edit Distance")
    plt.title("Edit Distance vs Prefix Length")
    plt.grid(True)
    plt.show()


if __name__ == "__main__":
    draw("prefix.txt")
    # draw("suffix.txt")
