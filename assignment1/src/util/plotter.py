import numpy as np
import pandas as pd
import argparse
import matplotlib.pyplot as plt

output_dir = "../../output_files/"

def parse():
  args = argparse.ArgumentParser()
  args.add_argument("-i", "--input", dest="input", type=str, help="Input CSV file name", default="density.csv")
  args.add_argument("-e", "--error", dest="e",action="store_true", help="Calculate error between two files", default=False)
  args.add_argument("-f1", "--file1", dest="f1", type=str, help="Diff 1 CSV file name", default="density.csv")
  args.add_argument("-f2", "--file2", dest="f2", type=str, help="Diff 2 CSV file name", default="density.csv")

  args.add_argument("-o", "--output", dest="output", type=str, help="Output PNG file name", default="plot.png")
  return args.parse_args()

args = parse()

def get_data(filename):
  df = pd.read_csv(output_dir + filename, delimiter=',')
  data = df.to_numpy()
  data = np.transpose(data)
  return data

def plot(data):
  x = data[0]
  y1 = data[1]
  y2 = data[2]

  red_lights = [180, 350]
  green_lights = [100, 275]


  plt.plot(x, y1, 'b', label="Queue Density")
  plt.plot(x, y2, 'tab:orange', label="Dynamic Density")
  plt.xlabel("Time (s)")
  if args.e:
    plt.ylabel("Density error")
  else:
    plt.ylabel("Density")

  for r in red_lights:
    plt.axvline(x=r, c='r')

  for g in green_lights:
    plt.axvline(x=g, c='g')

  plt.legend()
  plt.savefig(output_dir+args.output)
  plt.show()


if args.e:
  frames, q1, d1 = get_data(args.f1)
  frames, q2, d2 = get_data(args.f2)

  q_err = (q1 - q2)**2
  d_err = (d1 - d2)**2
  plot([frames,q_err,d_err])
else:
  plot(get_data(args.input))
