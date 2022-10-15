import tkinter as tk

import random
import sys
import json

from paho.mqtt import client as mqtt_client

# the following can be given as command line parameters
default_broker = '192.168.1.10'  # 1st parameter
default_port = 1883  # 2nd parameter
default_pub_topic = "controller/settings"  # 3rd parameter
default_sub_topic = "controller/status"  # 4th parameter


class MyApp:
    def __init__(self, parent, broker, port, pub_topic, sub_topic):
        self.myParent = parent
        self.myContainer1 = tk.Frame(parent)
        self.myContainer1.grid()
        self.topic = pub_topic

        self.auto = tk.IntVar()
        self.auto.set(1)


        row = 0
        tk.Label(self.myContainer1, text="Control message", font = ("Arial",15)).grid(row=row, column=0, sticky = 'w')

        row += 1
        self.button2 = tk.Button(self.myContainer1, text="Send")
        self.button2.grid(row=row, column=1)
        self.button2.bind("<Button-1>", self.button2Click)  # (1)

        tk.Label(self.myContainer1, text="Mode").grid(row=row, column=0)
        self.radiobutton1 = tk.Radiobutton(self.myContainer1, text = 'Pressure', variable = self.auto, value = 1, command = self.setButton1)
        self.radiobutton1.grid(row=row, column=1, sticky='w')

        row += 1
        self.radiobutton2 = tk.Radiobutton(self.myContainer1, text = 'Speed', variable = self.auto, value = 0, command = self.setButton1)
        self.radiobutton2.grid(row=row, column=1, sticky='w')

        row += 1
        tk.Label(self.myContainer1, text="Set point").grid(row=row, column=0)
        self.setpoint = tk.Scale(self.myContainer1, from_=0, to=140,
                                 tickinterval=10, length=400, orient=tk.HORIZONTAL)
        self.setpoint.set(10)
        self.setpoint.grid(row=row, column=1, sticky='w')

        row += 1
        tk.Label(self.myContainer1, text="Current state", font = ("Arial",15)).grid(row=row, column=0, sticky = 'w')

        self.mode = tk.IntVar()
        self.mode.set(1)
        row += 1
        self.radiobutton3 = tk.Radiobutton(self.myContainer1, text = 'Pressure', variable = self.mode, value = 1)
        self.radiobutton3.grid(row=row, column=1, sticky='w')

        row += 1
        self.radiobutton4 = tk.Radiobutton(self.myContainer1, text = 'Speed', variable = self.mode, value = 0)
        self.radiobutton4.grid(row=row, column=1, sticky='w')


        row += 1
        tk.Label(self.myContainer1, text="Speed").grid(row=row, column=0)
        self.speed = tk.Scale(self.myContainer1, from_=0, to=100,
                              length=400, orient=tk.HORIZONTAL, state = tk.DISABLED)
        self.speed.set(0)
        self.speed.grid(row=row, column=1, sticky='w')

        row += 1
        tk.Label(self.myContainer1, text="Pressure").grid(row=row, column=0)
        self.pressure = tk.Scale(self.myContainer1, from_=0, to=140,
                                 length=400, orient=tk.HORIZONTAL, state = tk.DISABLED)
        self.pressure.set(0)
        self.pressure.grid(row=row, column=1, sticky='w')

        row += 1
        t = tk.Label(self.myContainer1, text="CO2").grid(row=row, column=0)
        self.co2 = tk.Entry(self.myContainer1)
        self.co2.insert(0, "400")
        self.co2.grid(row=row, column=1, sticky = 'w')
        self.co2['state'] = tk.DISABLED;

        row += 1
        t = tk.Label(self.myContainer1, text="RH").grid(row=row, column=0)
        self.rh = tk.Entry(self.myContainer1)
        self.rh.insert(0, "42.0")
        self.rh.grid(row=row, column=1, sticky = 'w')
        self.rh['state'] = tk.DISABLED;

        row += 1
        t = tk.Label(self.myContainer1, text="Temp").grid(row=row, column=0)
        self.temp = tk.Entry(self.myContainer1)
        self.temp.insert(0, "21.2")
        self.temp.grid(row=row, column=1, sticky = 'w')
        self.temp['state'] = tk.DISABLED;

        self.setButton1()

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
            self.mode.set(int(rcv["auto"]))
            self.pressure['state'] = tk.NORMAL
            self.speed['state'] = tk.NORMAL
            self.co2['state'] = tk.NORMAL
            self.rh['state'] = tk.NORMAL
            self.temp['state'] = tk.NORMAL
            self.pressure.set(int(rcv["pressure"]))
            self.speed.set(int(rcv["speed"]))
            self.co2.delete(0,tk.END)
            self.co2.insert(0, rcv['co2'])
            self.rh.delete(0,tk.END)
            self.rh.insert(0, rcv['rh'])
            self.temp.delete(0,tk.END)
            self.temp.insert(0, rcv['temp'])
        except:
            print("Error while parsing JSON")
        self.pressure['state'] = tk.DISABLED
        self.speed['state'] = tk.DISABLED
        self.co2['state'] = tk.DISABLED
        self.rh['state'] = tk.DISABLED
        self.temp['state'] = tk.DISABLED

    def setButton1(self):
        if self.auto.get():  # (4)
            self.setpoint["to"] = 140
        else:
            if self.setpoint.get() > 100:
                self.setpoint.set(100)
            self.setpoint["to"] = 100

    def button2Click(self, event):  # (3)
        self.publish()

    def publish(self):
        dict = {"auto": bool(self.auto.get())}
        if self.auto.get():
            dict["pressure"] = self.setpoint.get()
        else:
            dict["speed"] = self.setpoint.get()

        msg = json.dumps(dict)
        result = self.client.publish(self.topic, msg)
        # result: [0, 1]
        status = result[0]
        if status == 0:
            print(f"Send `{msg}` to topic `{self.topic}`")
        else:
            print(f"Failed to send message to topic {self.topic}")


def run(argv):
    if len(argv) > 0:
        broker = argv[0]
    else:
        broker = default_broker

    if len(argv) > 1:
        port = int(argv[1])
    else:
        port = default_port

    if len(argv) > 2:
        pub_topic = argv[2]
    else:
        pub_topic = default_pub_topic

    if len(argv) > 3:
        sub_topic = argv[3]
    else:
        sub_topic = default_sub_topic

    root = tk.Tk()
    root.title("Ventilation WEB-UI simulator")
    myapp = MyApp(root, broker, port, pub_topic, sub_topic)
    root.mainloop()


if __name__ == '__main__':
    run(sys.argv[1:])
