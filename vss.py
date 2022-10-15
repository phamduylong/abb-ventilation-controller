import tkinter as tk
import numpy as np

import random
import sys
import json

from paho.mqtt import client as mqtt_client

# the following can be given as command line parameters
default_broker = '192.168.1.10' # 1st parameter
default_port = 1883  # 2nd parameter
default_pub_topic = "controller/status"  # 3rd parameter
default_sub_topic = "controller/settings" # 4th parameter



class MyApp:
    def __init__(self, parent, broker, port, pub_topic, sub_topic):
        self.myParent = parent  
        self.myContainer1 = tk.Frame(parent)
        self.myContainer1.grid()
        self.topic = pub_topic
        self.counter = 0
        self.error = False
        self.myParent.after(100, self.publish)
        self.myParent.after(1000, self.control)

        self.auto = True
        
        row = 0
        tk.Label(self.myContainer1, text="Mode").grid(row=row, column=0)
        self.button1 = tk.Button(self.myContainer1)
        self.button1.grid(row=row, column = 1, sticky='w')
        self.button1.bind("<Button-1>", self.button1Click) ### (1)
        
        row += 1
        tk.Label(self.myContainer1, text="Set point").grid(row=row, column=0)
        self.setpoint = tk.Scale(self.myContainer1, from_=0, to=140, tickinterval = 10, length = 400, orient=tk.HORIZONTAL)
        self.setpoint.set(10)
        self.setpoint.grid(row=row, column=1, sticky='w')

        row += 1
        tk.Label(self.myContainer1, text="Vent").grid(row=row, column=0)
        self.vent = tk.Scale(self.myContainer1, from_=0, to=10, orient=tk.HORIZONTAL, command=self.speed_changed)
        self.vent.set(1)
        self.vent.grid(row=row, column=1, sticky='w')

        row += 1 
        tk.Label(self.myContainer1, text="Speed").grid(row=row, column=0)
        self.speed = tk.Scale(self.myContainer1, from_=0, to=100, length = 400, orient=tk.HORIZONTAL, command=self.speed_changed, state = tk.DISABLED)
        self.speed.set(0)
        self.speed.grid(row=row, column=1, sticky='w')

        row += 1
        tk.Label(self.myContainer1, text="Pressure").grid(row=row, column=0)
        self.pressure = tk.Scale(self.myContainer1, from_=0, to=140, length = 400, orient=tk.HORIZONTAL,state = tk.DISABLED)
        self.pressure.set(0)
        self.pressure.grid(row=row, column=1, sticky='w')

        row += 1
        tk.Label(self.myContainer1, text="CO2").grid(row=row, column=0)
        self.co2 = tk.Scale(self.myContainer1, from_=0, to=5000, tickinterval = 1000, length = 400, orient=tk.HORIZONTAL)
        self.co2.set(300)
        self.co2.grid(row=row, column=1, sticky='w')

        row += 1
        tk.Label(self.myContainer1, text="RH").grid(row=row, column=0)
        self.rh = tk.Scale(self.myContainer1, from_=0, to=100, tickinterval = 10, length = 400, orient=tk.HORIZONTAL)
        self.rh.set(37)
        self.rh.grid(row=row, column=1, sticky='w')

        row += 1
        tk.Label(self.myContainer1, text="Temperature").grid(row=row, column=0)
        self.temp = tk.Scale(self.myContainer1, from_=-20, to= 100, tickinterval = 10, length = 400, orient=tk.HORIZONTAL)
        self.temp.set(20)
        self.temp.grid(row=row, column=1, sticky='w')

        self.setButton1()


        self.msg_count = 0
        self.dict = { "nr" : self.msg_count }

        # generate client ID with pub prefix randomly
        client_id = f'vss-{random.randint(0, 1000)}'
        # username = 'emqx'
        # password = 'public'
        
        self.client = mqtt_client.Client(client_id)
        #self.client.username_pw_set(username, password)
        self.client.on_connect = self.on_connect
        self.client.connect(broker, port)
        self.client.subscribe(sub_topic)
        self.client.on_message = self.on_message
        self.client.loop_start()

    def on_connect(self, client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
        else:
            print("Failed to connect, return code %d\n", rc)

    def on_message(self, client, userdata, msg):
        rd = msg.payload.decode()
        print(f"Received: `{rd}` from `{msg.topic}` topic")
        try:
            rcv = json.loads(rd)
            self.auto = rcv["auto"]
            if self.auto:
                self.setpoint.set(rcv["pressure"])
            else:
                self.setpoint.set(rcv["speed"])
            self.setButton1()
        except:
            print("Error while parsing JSON")

    def control(self):
        sp = self.setpoint.get()
        spd = self.speed.get()
        if self.auto: 
            if sp > self.pressure.get():
                self.counter += 1
                if spd < 100:
                    spd += 1
            elif sp < self.pressure.get():
                self.counter += 1
                if spd > 0:
                    spd -= 1
            else:
                self.counter = 0

            self.speed['state'] = tk.NORMAL
            self.speed.set(spd)
            self.speed['state'] = tk.DISABLED

        else:
            self.counter = 0
            if sp > 100:
                sp = 100
            self.speed['state'] = tk.NORMAL
            self.speed.set(self.setpoint.get())
            self.speed['state'] = tk.DISABLED
        self.error = self.counter > 100
        self.myParent.after(1000, self.control)


    def setButton1(self):
        if self.auto: 
            self.button1["background"] = "green"
            self.button1["text"] = "Auto"
            self.setpoint["to"] = 140
        else:
            self.button1["background"] = "yellow"
            self.button1["text"] = "Manual"
            if self.setpoint.get() > 100:
                self.setpoint.set(100)
            self.setpoint["to"] = 100
            self.speed['state'] = tk.NORMAL
            self.speed.set(self.setpoint.get())
            self.speed['state'] = tk.DISABLED
    
    def speed_changed(self, event):
        vnt = self.vent.get()
        spd = self.speed.get() * 200;
        V00 = np.interp(spd, [0, 5000, 10000, 15000, 20000],[0, 8, 29, 65, 90])
        V25 = np.interp(spd, [0, 5000, 10000, 15000, 20000],[0, 4, 13, 28, 40])
        V50 = np.interp(spd, [0, 5000, 10000, 15000, 20000],[0, 1, 4, 8, 11])
        V75 = np.interp(spd, [0, 5000, 10000, 15000, 20000],[0, 0, 1, 2, 3])
        V100 = np.interp(spd, [0, 5000, 10000, 15000, 20000],[0, 0, 0, 0, 0])
        pre = np.interp(vnt, [0, 2.5, 5, 7.5, 10],[V00, V25, V50, V75, V100])
        #print(V00, V25, V50, V75, V100, pre)
        self.pressure['state'] = tk.NORMAL
        self.pressure.set(pre)
        self.pressure['state'] = tk.DISABLED

        self.pressure.set(pre)

    def button1Click(self, event):    ### (3)
        self.auto = not self.auto
        self.setButton1()


    def publish(self):
        self.dict["nr"] = self.msg_count
        self.dict["speed"] = self.speed.get() 
        self.dict["setpoint"] = self.setpoint.get()
        self.dict["pressure"] = self.pressure.get()
        self.dict["auto"] = self.auto
        self.dict["error"] = self.error
        self.dict["co2"] = self.co2.get()
        self.dict["rh"] = self.rh.get()
        self.dict["temp"] = self.temp.get()

        msg = json.dumps(self.dict)
        result = self.client.publish(self.topic, msg)
        # result: [0, 1]
        status = result[0]
        if status == 0:
            print(f"Send `{msg}` to topic `{self.topic}`")
        else:
            print(f"Failed to send message to topic {self.topic}")
        self.msg_count += 1
        self.myParent.after(5000, self.publish)

        

        
def run(argv):
    if len(argv)> 0 : 
        broker = argv[0]
    else:
        broker = default_broker

    if len(argv)> 1 : 
        port = int(argv[1])
    else:
        port = default_port

    if len(argv)> 2 : 
        pub_topic = argv[2]
    else:
        pub_topic = default_pub_topic

    if len(argv)> 3 : 
        sub_topic = argv[3]
    else:
        sub_topic = default_sub_topic

    root = tk.Tk()
    root.title("Ventilation controller simulator")
    myapp = MyApp(root, broker, port, pub_topic, sub_topic)
    root.mainloop()


if __name__ == '__main__':
    run(sys.argv[1:])
