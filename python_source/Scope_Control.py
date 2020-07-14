import tkinter as tk
from tkinter import ttk

class Scope_Ctrl:
    
    def __init__(self, master, sock, time_space):
        self.sock = sock
        self.time_space = time_space

        # создаем Frame
        self.f = ttk.LabelFrame(master, text = 'Scope Control')
      
        # создаем Scales для управления уровнем триггра
        self.frame_trig = ttk.Frame(self.f)
        self.frame_trig.grid(row = 0, column = 0)
      
        self.trig_scale_var = tk.IntVar()
        self.trig_scale_var.set(0)
        self.max_scale = 16384
        self.min_scale = -16384
        self.trig_scale = ttk.Scale(self.frame_trig, from_ = self.min_scale, to = self.max_scale, orient = 'vertical', length = 275, variable = self.trig_scale_var)
        self.trig_label = ttk.Label(self.frame_trig, text = '0', justify = 'right')
        self.trig_scale.grid(padx = 10, row = 1, column = 0)
        self.trig_label.grid(padx = 10, row = 2, column = 0)
        ttk.Label(self.frame_trig, text = 'Trigger Level').grid(row = 0, column = 0)

        self.frame_other = ttk.Frame(self.f)
        self.frame_other.grid(row = 0, column = 1)
        
        # создаем ComboBox для настройки режима триггера
        self.Trig_Comb_var = tk.StringVar()
        self.Trig_Comb_var.set('AUTO')
        self.Trig_Comb = ttk.Combobox(self.frame_other, width = 13, textvariable = self.Trig_Comb_var, values=["AUTO", "TRIGGER", "ONCE", "TRIG ONCE"])
        ttk.Label(self.frame_other, text = 'Triggre Mode:').grid(padx = 5, row = 0, column = 0, columnspan = 2)
        self.Trig_Comb.grid(pady = 4, padx = 5, row = 1, column = 0, columnspan = 2)

        # создаем ComboBox для настройки разрешения по амплитуде
        self.Level_Value = 0.5
        self.Level_Comb_var = tk.StringVar()
        self.Level_Comb_var.set('-0.5...0.5')
        self.Level_Comb = ttk.Combobox(self.frame_other, width = 13, textvariable = self.Level_Comb_var, values=["-0.5...0.5",
                                                                                                                 "-0.4...0.4",
                                                                                                                 "-0.3...0.3",
                                                                                                                 "-0.2...0.2",
                                                                                                                 "-0.1...0.1"])
        
        
        ttk.Label(self.frame_other, text = 'Level Span:').grid(padx = 5, row = 2, column = 0, columnspan = 2)
        self.Level_Comb.grid(pady = 4, padx = 5, row = 3, column = 0, columnspan = 2)

        # создаем ComboBox для настройки разрешения по времени
        self.Time_Comb_var = tk.StringVar()
        self.Time_Comb_var.set('0.5 ms')
        self.Time_Comb = ttk.Combobox(self.frame_other, width = 13, textvariable = self.Time_Comb_var, values=["0.5 ms", "1 ms", "2 ms", "5 ms", "10 ms"])
        ttk.Label(self.frame_other, text = 'Time Span:').grid(padx = 5, row = 4, column = 0, columnspan = 2)
        self.Time_Comb.grid(pady = 4, padx = 5, row = 5, column = 0, columnspan = 2)

        # создаем Buttons для начала триггера
        self.Trig_Button = ttk.Button(self.frame_other, text = '   Start\nTrigger', command = self.trigger_start)
        self.Trig_Button.grid(pady = 12, padx = 8, row = 6, column = 0)

         # создаем Buttons для установки парамтров
        self.Param_Button = ttk.Button(self.frame_other, text = '   Set\nParams', command = self.set_parameters)
        self.Param_Button.grid(pady = 12, padx = 8, row = 6, column = 1)

    def set_parameters(self):
        if   self.Level_Comb_var.get() == "-0.5...0.5": self.Level_Value = 0.5
        elif self.Level_Comb_var.get() == "-0.4...0.4": self.Level_Value = 0.4
        elif self.Level_Comb_var.get() == "-0.3...0.3": self.Level_Value = 0.3
        elif self.Level_Comb_var.get() == "-0.2...0.2": self.Level_Value = 0.2
        elif self.Level_Comb_var.get() == "-0.1...0.1": self.Level_Value = 0.1
        self.time_space.set_ylim(-self.Level_Value, self.Level_Value)
        
        ms_val = float(self.Time_Comb_var.get().split()[0])
        self.time_space.set_xlim(0, ms_val)
        downsamp = ms_val*2
        
    def trigger_start(self):
        print('hello')

    def get_frame(self):
        # метод возвращает Frame
        return self.f
