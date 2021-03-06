import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("../output_files/density.csv")
data = df.to_numpy()

x = [d[0] for d in data]
y1 = [d[1] for d in data]
y2 = [d[2] for d in data]

plt.plot(x, y1, 'b', label="Queue Density")
plt.plot(x, y2, 'r', label="Dynamic Density")
plt.legend()
plt.savefig("../output_files/plot.png")
plt.show()