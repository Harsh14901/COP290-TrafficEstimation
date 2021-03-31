import pandas as pd
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt
import numpy as np

class DataField:
    
    def __init__(self,error,runtime):
        self.error = error
        self.runtime = runtime

class Params:
    
    def __init__(self,param_string):
        print(param_string)

        params : str = param_string.split(';')
        self.skip_frames = int(params[0])
        self.sparse = int(params[1])
        self.res_factor = 1920//int(params[3][:params[3].rfind('X')])
        self.split_vid = int(params[-1])
        

def three_d_plots(data):
    
    x,y,z = [],[],[]
    c_rt,c_err = [],[]

    
    for key in data:
        param = Params(key)
        if param.sparse==0:
            continue
        x.append(param.skip_frames)
        y.append(param.res_factor)
        z.append(param.split_vid)

        c_rt.append(data[key].runtime)
        c_err.append(data[key].error/3.284)
        
        

    for i in range(2):
        fig = plt.figure()
        ax = fig.add_subplot(111, projection='3d')

        ax.set_xlabel('Frame Sampling')
        ax.set_ylabel('Resolution Scaling')
        ax.set_zlabel('Multi-Threading')      
        if i==0:
            img = ax.scatter(x, y, z, c=c_rt, cmap=plt.hot())
            fig.colorbar(img)

            plt.title('Runtime vs Parameters')
            plt.savefig("Runtime_vs_Params.png")
            plt.savefig("../../output_files/Runtime_vs_Params.png")
        else:
            img = ax.scatter(x, y, z, c=c_err, cmap=plt.hot())

            fig.colorbar(img)

            plt.title('Error vs Parameters')
            plt.savefig("../../output_files/Error_vs_Params.png")
        plt.show()

error_data = pd.read_csv('../../output_files/complete_analysis_pVe.csv')
runtime_data = pd.read_csv('../../output_files/complete_analysis_pVt.csv')

data = dict()
for i,row in error_data.iterrows():
    data[row['param']] = DataField(row['error'],0)

for i,row in runtime_data.iterrows():
    data[row['param']].runtime = row['time']

three_d_plots(data)
