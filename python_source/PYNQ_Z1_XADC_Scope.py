from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np
import tkinter as tk
from UDP_Connect import *
from Scope_Control import *

# создаем фигуру для графика
fig = plt.Figure()

x = [a/512*0.5 for a in range(512)]
freq = [(a-256)/256*(10**6)*0.5 for a in range(512)]
y = [0 for a in range(512)]

# создаем основное окно 
root = tk.Tk()
root.title('PYNQ Z1 XADC Scope')

def animate(i):
    Data = [x/16384*0.5 for x in Net_Connect.RX_Data]
    npData = np.array(Data)
    # вычисляем спектр сигнала
    freqData = abs(np.fft.fft(npData))
    freqData = np.fft.fftshift(freqData)/len(npData)
    freqData = 20*np.log10(freqData/0.5)
    time_line.set_ydata(Data)
    freq_line.set_ydata(freqData)
    time_line.set_xdata(Scope.time_span)
    freq_line.set_xdata(Scope.freq_span)

canvas = FigureCanvasTkAgg(fig, master=root)
canvas.get_tk_widget().configure(width=1000, height=800)
canvas.get_tk_widget().grid(column=0, row=0, rowspan = 2, pady = 20)

time_space = fig.add_subplot(211)
time_line, = time_space.plot(x, y)
time_space.set_ylim(-0.5, 0.5)
time_space.set_xlim(0, 0.5)
time_space.grid()

freq_space = fig.add_subplot(212)
freq_line, = freq_space.plot(freq, y)
freq_space.set_ylim(-80, 0)
freq_space.set_xlim(-(10**6)*0.5, (10**6)*0.5)
freq_space.grid()

ani = animation.FuncAnimation(fig, animate, None, interval=200, blit=False)



# создаем и настраиваем widget управления Ethernet соединением
Net_Connect = UDP_Connect(root)
Net_Connect.get_frame().grid(column=1,row=1)

# создаем и настраиваем widget управления осчилографом
Scope = Scope_Ctrl(root, Net_Connect, time_space, freq_space)
Scope.get_frame().grid(column=1,row=0)

root.mainloop()
