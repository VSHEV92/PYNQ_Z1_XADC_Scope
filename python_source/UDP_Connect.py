import tkinter as tk
from tkinter import ttk
import socket
from threading import Thread
from time import sleep

class UDP_Connect:
    def method_in_a_thread(self):
        while True:
            data, addr = self.sock.recvfrom(1024)
            samp_bin = [data[idx*2:(idx+1)*2] for idx in range(512) ]
            self.RX_Data = [int.from_bytes(samp, byteorder='little', signed=True) for samp in samp_bin]
            if self.stop_threads: break

    def Start_Button_Pressed(self):
        self.sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
        self.sock.bind(("192.168.1.11", 5001))
        self.sock.connect((self.IP_Addr_var.get(), self.Port_Num_var.get()))
        self.Start_Button['state'] = 'disabled'
        self.Stop_Button['state'] = '!disabled'
        self.stop_threads = False
        self.run_thread = Thread(target = self.method_in_a_thread)
        self.run_thread.setDaemon(True)
        self.run_thread.start()
            
    def Stop_Button_Pressed(self):
        # обработчик нажатия Stop_Button
        self.sock.close()
        self.Start_Button['state'] = '!disabled'
        self.Stop_Button['state'] = 'disabled'
        self.stop_threads = True
        sleep(0.5)
        self.run_thread.join()
        
    def __init__(self, master):
        self.sock = None
        self.RX_Data = [1 for a in range(512)]
        self.stop_threads = False
        
        # создаем Frame
        self.f = ttk.LabelFrame(master, text = 'UDP Connection', width = 40, height = 50)

        # создаем Entry для настройки TCP/UDP соединения
        self.IP_Addr_var = tk.StringVar()
        self.IP_Addr_var.set('192.168.1.10')
        self.IP_Addr_Entry = ttk.Entry(self.f, width = 15, textvariable = self.IP_Addr_var, justify = 'center')
        ttk.Label(self.f, text = 'IP Address').grid(padx = 5, row = 0, column = 0, columnspan = 2)
        self.IP_Addr_Entry.grid(pady = 4, padx = 5, row = 1, column = 0, columnspan = 2)

        self.Port_Num_var = tk.IntVar()
        self.Port_Num_var.set(5001)
        self.Port_Num_Entry = ttk.Entry(self.f, width = 15, textvariable = self.Port_Num_var, justify = 'center')
        ttk.Label(self.f, text = 'Port Number').grid(padx = 5, row = 2, column = 0, columnspan = 2)
        self.Port_Num_Entry.grid(pady = 4, padx = 5, row = 3, column = 0, columnspan = 2)

        # создаем Buttons для начала и окончания соединения
        self.Start_Button = ttk.Button(self.f, text = '     Start\nConnection', state = '!disabled', command = self.Start_Button_Pressed)
        self.Start_Button.grid(pady = 12, padx = 8, row = 4, column = 0)
        
        self.Stop_Button = ttk.Button(self.f, text = '     Stop\nConnection', state = 'disabled', command = self.Stop_Button_Pressed)
        self.Stop_Button.grid(pady = 12, padx = 8, row = 4, column = 1)
 
    def get_frame(self):
        # метод возвращает Frame
        return self.f
       
