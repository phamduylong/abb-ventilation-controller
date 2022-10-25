# ABB Ventilation Control System

<em>The system is not yet a complete product and require slight modifications to function properly.</em>

## Description

[PDF](https://github.com/phamduylong/abb-ventilation-controller/blob/master/pdfs/Ventilation_controller_project_specification.pdf)

[GMP252 CO2 Sensor Manual](https://github.com/phamduylong/abb-ventilation-controller/blob/master/pdfs/GMP252-User-Guide-in-English-M211897EN.pdf)

[HMP60 Temperature and Humidity Probe Manual](https://github.com/phamduylong/abb-ventilation-controller/blob/master/pdfs/HMP60-User-guide.pdf)

[Produal MIO 12V Manual](https://github.com/phamduylong/abb-ventilation-controller/blob/master/pdfs/MIO12-V_181024_UG_en.pdf)

[SDP600 Differential Pressure Sensor Manual](https://github.com/phamduylong/abb-ventilation-controller/blob/master/pdfs/Sensirion_Differential_Pressure_SDP6x0series_Datas-767275.pdf)

The project is combination of a Ventilation Fan System with Vaisala's GMP252 Carbon Dioxide probe, HMP60 Temperature and Humidity sensor. Fan speed inside the duct can be controlled with Produal MIO 12V Actuator, while pressure difference between the inside and outside of the duct will be measured with Sensirion's SDP600 differential pressure sensor.




## User Manual

- #### Operation: 
  The idea of the system is to measure surrounding environments attributes, specifically CO2 level, relative humidity and temperature, then using the collected information as a feedback for the ventilation fan to adjust power level so as to create the ideal state in terms of air conditions. The system operates in 2 separate modes: manual and automatic. Automatic mode lets the user decide the desired parameter of pressure inside the ventilation duct, either through an LCD user interface implemented physically on the system or through a web interface. Controller measures the pressure difference between pressure level in the duct and in the environment, adjust the fan speed to keep it at a required level. On the other hand, user is free to control the fan speed by themselves during the operation in manual mode, giving them full access to modifying the pressure level arbitrarily. There are 2 interfaces available to the user: Web interface and LCD interface. These interfaces and the controller communicate with each other through MQTT.
  
    1. ###### Web Interface: 
 
         User is required to log in using their credentials. There is a page where users can create their account if they have not had one. A page of auto mode will be shown on default after user have logged in, which displays the current fan speed inside the duct. In addition, the pressure level will be shown and the user can modify it through an input field, and the fan speed will adjust until it reaches the desired pressure level. There is a switch in the page to change the operation mode of the system to manual and vice versa. Manual mode tab contains a chart that displays the current pressure value of the system. User can change the fan speed arbitrarily and the other components of the system shall act accordingly. User can inspect statistical data through the **See Full Statistics** button, where it will display the graph of pressure data, fan speed data and the user data. These graphs can be displayed in a better solution, separately. For user's activities analyctics, it will show the amount of time that individual user spent on the page in a scale of 24 hours. *User's activities will not operate correctly if the logout action was not performed.*
 
    2. ###### LCD Interface: 
 
         The LCD screen is physically connected to the system and can be controlled with buttons, where user can not modify remotely. LCD will display the values of the system measured in *both* operation modes. In auto mode, users can only configure the desired parameter of pressure inside the ventilation duct, the system will adjust the fan speed of the system and the LCD screen will display the updated value of other datas. On the other hand, user can only configure the fan speeed in the manual mode, in which the LCD screen will also continuously updating the remaining measurement results to the screen. If the operation mode is changed when the user is modifying a parameter of the system, **the pending changes will not be saved and the parameter will be resetted back to its original value.**
