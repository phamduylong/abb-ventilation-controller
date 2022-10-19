<h2>ABB Ventilation Control System with Web UI Implemented</h2>

<em>The system is not yet a complete product and require slight modifications to function properly.</em>
<h3>User Manual</h3>
<ul>
  <li><h5>Operation:</h5><p>The idea of the system is to measure surrounding environments attributes, specifically CO2 level, relative humidity and temperature, then using the collected information as a feedback for the ventilation fan to adjust power level so as to create the ideal state in terms of air conditions. The system operates in 2 separate modes: manual and automatic. Automatic mode lets the user decide the desired parameter of pressure inside the ventilation duct, either through an LCD user interface implemented physically on the system or through a web interface. Controller measures the pressure difference between pressure level in the duct and in the environment, adjust the fan speed to keep it at a required level. On the other hand, user is free to control the fan speed by themselves during the operation in manual mode, giving them full access to modifying the pressure level arbitrarily. There are 2 interfaces available to the user: Web interface and LCD interface. These interfaces and the controller communicate with each other through MQTT.</p></li>
</ul>
