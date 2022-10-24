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
<ul>
  <li><h5>Operation:</h5><p>The idea of the system is to measure surrounding environments attributes, specifically CO2 level, relative humidity and temperature, then using the collected information as a feedback for the ventilation fan to adjust power level so as to create the ideal state in terms of air conditions. The system operates in 2 separate modes: manual and automatic. Automatic mode lets the user decide the desired parameter of pressure inside the ventilation duct, either through an LCD user interface implemented physically on the system or through a web interface. Controller measures the pressure difference between pressure level in the duct and in the environment, adjust the fan speed to keep it at a required level. On the other hand, user is free to control the fan speed by themselves during the operation in manual mode, giving them full access to modifying the pressure level arbitrarily. There are 2 interfaces available to the user: Web interface and LCD interface. These interfaces and the controller communicate with each other through MQTT.</p></li>
</ul>
