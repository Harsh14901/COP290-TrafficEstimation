import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("../../output_files/density.csv")
data = df.to_numpy()

x = [d[0] for d in data]
y1 = [d[1] for d in data]
y2 = [d[2] for d in data]

red_lights = [180, 350]
green_lights = [100, 275]


plt.plot(x, y1, 'b', label="Queue Density")
plt.plot(x, y2, 'tab:orange', label="Dynamic Density")
plt.xlabel("Time (s)")
plt.ylabel("Density")

for r in red_lights:
  plt.axvline(x=r, c='r')

for g in green_lights:
  plt.axvline(x=g, c='g')

plt.legend()
plt.savefig("../../output_files/plot.png")
plt.show()