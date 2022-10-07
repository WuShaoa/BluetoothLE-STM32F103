import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from scipy import signal


df_fall = pd.read_csv("./data.txt", header=None)
df = pd.read_csv("./leftwalk11.txt", header=None)

y1 = np.concatenate([np.array(df.iloc[:,6]), np.array(df_fall.iloc[:,6])])
y2 = np.concatenate([np.array(df.iloc[:,7]), np.array(df_fall.iloc[:,7])])
y3 = np.concatenate([np.array(df.iloc[:,8]), np.array(df_fall.iloc[:,8])])

# filtering
b, a = signal.butter(2, 0.55, 'lowpass')
y1 = signal.filtfilt(b, a, y1)
y2 = signal.filtfilt(b, a, y2)
y3 = signal.filtfilt(b, a, y3)

# plotting
#plt.plot(np.arange(len(y)), df.iloc[:,2], label="pressure") #压力

plt.plot(np.arange(len(y1)), y1, label="accx") 
plt.plot(np.arange(len(y2)), y2, label="accy")
plt.plot(np.arange(len(y3)), y3, label="accz")

#plt.plot(np.arange(len(y)), x, label="accx_filt")
#plt.plot(np.arange(len(y)), y, label="accy_filt")
# plt.plot(np.arange(len(y)), z, label="accz_filt")

plt.legend()
plt.show()