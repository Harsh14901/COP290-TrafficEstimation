import argparse
import matplotlib.pyplot as plt


def parse():
  args = argparse.ArgumentParser()
  args.add_argument("-i", "--input", dest="input", type=str, help="Input CSV file")
  return args.parse_args()


args = parse()
infile = args.input

inname = infile.split("/")[-1]
outname = "plot_" + inname[:-3] + "png"
outfile = "/".join(infile.split("/")[:-1] + [outname])
a = inname[:-8]


labels = []
data = []
with open(infile, 'r') as f:
  labels = f.readline().split(',')
  for line in f.readlines():
    x,y = line.split(',')
    # data.append([float(x.strip()),float(y.strip())])
    data.append([x.strip(),float(y.strip())])



print(outfile)
print(data)

plt_title = a
if(a == "skip_frames"):
  plt_title = "Frame subsampling"
elif a == "sparse_optical":
  plt_title = "Sparse Optical Flow VS Dense optical flow"
elif a == "split_frame":
  plt_title = "Spatial Multi threading"
elif a == "split_video":
  plt_title = "Temporal Multi threading"
elif a == "resolution":
  plt_title = "Resolution Area"
  # for i in range(len(data)):
  #   res = data[i][0]
  #   area = int(res.split("X")[0])*int(res.split("X")[1])
  #   data[i][0] = area

# data = sorted(data, key=lambda x: x[0])

x = [d[0] for d in data]
y = [d[1] for d in data]

plt.title(plt_title)
plt.xlabel(labels[0])
plt.ylabel(labels[1])
# plt.plot(x,y, "-o")
plt.bar(x,y)



plt.savefig(outfile)
plt.show()


