import argparse
import matplotlib.pyplot as plt

data_dir = "../../output_files/"
plt.figure(figsize=(10,10))

def parse():
  args = argparse.ArgumentParser()
  args.add_argument("-m", "--method", dest="method", type=str, help="Method name")
  return args.parse_args()

def read_csv(infile, first_str=False):
  labels = []
  data = []
  with open(infile, 'r') as f:
    labels = f.readline().split(',')
    for line in f.readlines():
      x,y = line.split(',')
      
      if not first_str:
        data.append([float(x.strip()),float(y.strip())])
      else:
        data.append([x.strip(),float(y.strip())])
  return labels, data

def plot(method):
  input_path = get_input(method)
  plt_title = get_title(method)


  pvt_file = input_path + "_pVt.csv"
  evt_file = input_path + "_eVt.csv"

  _ , pvt_data = read_csv(pvt_file, True)
  _ , evt_data = read_csv(evt_file, False)
  data = []
  assert len(pvt_data) == len(evt_data)
  n = len(pvt_data)

  for i in range(n):
    data.append([pvt_data[i][0], pvt_data[i][1], evt_data[i][0]])

  data = sorted(data, key=lambda x: x[2])
  print(data)

  params = [d[0] for d in data]
  time = [d[1] for d in data]
  error = [d[2] for d in data]
  print(time, error)
  plt.title(plt_title)
  plt.xlabel("error in utility")
  plt.ylabel("time (s)")
  for i in range(n):
    plt.annotate(params[i], [error[i], time[i]], xytext=(0,10),textcoords="offset pixels")
  plt.plot(error,time, "-o", label=method)

args = parse()
method_name = args.method
output_path = data_dir + "plot_" + method_name + ".png"

def get_input(method_name):
  input_path = data_dir + method_name
  return input_path

def get_title(method_name):
  plt_title = method_name
  if(method_name == "skip_frames"):
    plt_title = "Frame subsampling"
  elif method_name == "sparse_optical":
    plt_title = "Sparse VS Dense optical flow"
  elif method_name == "split_frame":
    plt_title = "Spatial Multi Threading"
  elif method_name == "split_video":
    plt_title = "Temporal Multi Threading"
  elif method_name == "resolution":
    plt_title = "Resolution Scaling"
  elif method_name == "all":
    plt_title = "Runtime parameter Vs error analysis"
  return plt_title


if(method_name != "all"):
  plot(method_name)
else:
  methods = ["skip_frames","split_frame","split_video","resolution"]
  for m in methods:
    plot(m)

plt.legend()
plt.savefig(output_path)
plt.show()


