import matplotlib.pyplot as plt
import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns
from glob import glob
import os


def main():
    for bin_file in glob('./data/*.bin'):
        tick = int(os.path.basename(bin_file)[:-4])
        print(tick)
        x = np.fromfile(bin_file, dtype=np.float32).reshape(80, 80).T
        plt.subplots(figsize=(10, 10))
        x = np.flip(x, 0)
        sns_plot = sns.heatmap(x)
        sns_plot.get_figure().savefig(f'./data/pic/{tick}.png')


if __name__ == '__main__':
    main()