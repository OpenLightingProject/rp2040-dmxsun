EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "rp2040_dongle baseboard 4 slots"
Date "2021-03-19"
Rev "0.1"
Comp "https://www.openlighting.org/"
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L power:GND #PWR030
U 1 1 60531B3B
P 9050 2550
F 0 "#PWR030" H 9050 2300 50  0001 C CNN
F 1 "GND" H 9055 2377 50  0000 C CNN
F 2 "" H 9050 2550 50  0001 C CNN
F 3 "" H 9050 2550 50  0001 C CNN
	1    9050 2550
	1    0    0    -1  
$EndComp
$Comp
L Device:R R2
U 1 1 6053BED4
P 1750 2700
F 0 "R2" H 1820 2746 50  0000 L CNN
F 1 "2k" H 1820 2655 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 1680 2700 50  0001 C CNN
F 3 "~" H 1750 2700 50  0001 C CNN
	1    1750 2700
	1    0    0    -1  
$EndComp
$Comp
L Device:R R1
U 1 1 6053CEA4
P 1350 2700
F 0 "R1" H 1420 2746 50  0000 L CNN
F 1 "2k" H 1420 2655 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 1280 2700 50  0001 C CNN
F 3 "~" H 1350 2700 50  0001 C CNN
	1    1350 2700
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR02
U 1 1 6053E9EF
P 1750 2450
F 0 "#PWR02" H 1750 2300 50  0001 C CNN
F 1 "+3.3V" H 1765 2623 50  0000 C CNN
F 2 "" H 1750 2450 50  0001 C CNN
F 3 "" H 1750 2450 50  0001 C CNN
	1    1750 2450
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR01
U 1 1 6053EE63
P 1350 2450
F 0 "#PWR01" H 1350 2300 50  0001 C CNN
F 1 "+3.3V" H 1365 2623 50  0000 C CNN
F 2 "" H 1350 2450 50  0001 C CNN
F 3 "" H 1350 2450 50  0001 C CNN
	1    1350 2450
	1    0    0    -1  
$EndComp
Wire Wire Line
	1750 2450 1750 2550
$Comp
L Device:D_Schottky D1
U 1 1 60540187
P 9800 1800
F 0 "D1" H 9800 1583 50  0000 C CNN
F 1 "D_Schottky" H 9800 1674 50  0000 C CNN
F 2 "Diode_SMD:D_SMA" H 9800 1800 50  0001 C CNN
F 3 "~" H 9800 1800 50  0001 C CNN
	1    9800 1800
	-1   0    0    1   
$EndComp
Wire Wire Line
	9350 1800 9500 1800
Wire Wire Line
	9950 1800 10200 1800
Wire Wire Line
	10200 1800 10200 1750
Wire Wire Line
	9050 2400 9050 2550
NoConn ~ 9350 2000
NoConn ~ 9350 2100
NoConn ~ 9350 2200
$Comp
L Connector_Generic:Conn_02x06_Odd_Even J1
U 1 1 60575CEA
P 3400 3850
F 0 "J1" H 3450 4267 50  0000 C CNN
F 1 "IO-Board_00" H 3450 4176 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_2x06_P2.54mm_Vertical" H 3400 3850 50  0001 C CNN
F 3 "~" H 3400 3850 50  0001 C CNN
	1    3400 3850
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR05
U 1 1 60575E46
P 4050 3650
F 0 "#PWR05" H 4050 3400 50  0001 C CNN
F 1 "GND" V 4055 3522 50  0000 R CNN
F 2 "" H 4050 3650 50  0001 C CNN
F 3 "" H 4050 3650 50  0001 C CNN
	1    4050 3650
	0    -1   -1   0   
$EndComp
$Comp
L power:+3.3V #PWR07
U 1 1 60575E50
P 4050 3850
F 0 "#PWR07" H 4050 3700 50  0001 C CNN
F 1 "+3.3V" V 4065 3978 50  0000 L CNN
F 2 "" H 4050 3850 50  0001 C CNN
F 3 "" H 4050 3850 50  0001 C CNN
	1    4050 3850
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR06
U 1 1 60575E6D
P 4050 4050
F 0 "#PWR06" H 4050 3800 50  0001 C CNN
F 1 "GND" V 4055 3922 50  0000 R CNN
F 2 "" H 4050 4050 50  0001 C CNN
F 3 "" H 4050 4050 50  0001 C CNN
	1    4050 4050
	0    -1   -1   0   
$EndComp
Wire Notes Line
	2700 500  2700 7800
Wire Notes Line
	2700 7800 500  7800
Wire Notes Line
	500  7800 500  500 
Text Notes 550  600  0    50   ~ 0
Mounting holes
$Comp
L Mechanical:MountingHole H1
U 1 1 605BEA44
P 750 750
F 0 "H1" H 850 796 50  0000 L CNN
F 1 "MountingHole" H 850 705 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3" H 750 750 50  0001 C CNN
F 3 "~" H 750 750 50  0001 C CNN
	1    750  750 
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole H5
U 1 1 605BEE59
P 1750 750
F 0 "H5" H 1850 796 50  0000 L CNN
F 1 "MountingHole" H 1850 705 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3" H 1750 750 50  0001 C CNN
F 3 "~" H 1750 750 50  0001 C CNN
	1    1750 750 
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole H2
U 1 1 605BF8E7
P 750 1000
F 0 "H2" H 850 1046 50  0000 L CNN
F 1 "MountingHole" H 850 955 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3" H 750 1000 50  0001 C CNN
F 3 "~" H 750 1000 50  0001 C CNN
	1    750  1000
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole H6
U 1 1 605BFD55
P 1750 1000
F 0 "H6" H 1850 1046 50  0000 L CNN
F 1 "MountingHole" H 1850 955 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3" H 1750 1000 50  0001 C CNN
F 3 "~" H 1750 1000 50  0001 C CNN
	1    1750 1000
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole H3
U 1 1 605C02A5
P 750 1250
F 0 "H3" H 850 1296 50  0000 L CNN
F 1 "MountingHole" H 850 1205 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3" H 750 1250 50  0001 C CNN
F 3 "~" H 750 1250 50  0001 C CNN
	1    750  1250
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole H7
U 1 1 605C08C3
P 1750 1250
F 0 "H7" H 1850 1296 50  0000 L CNN
F 1 "MountingHole" H 1850 1205 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3" H 1750 1250 50  0001 C CNN
F 3 "~" H 1750 1250 50  0001 C CNN
	1    1750 1250
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole H4
U 1 1 605C0C36
P 750 1500
F 0 "H4" H 850 1546 50  0000 L CNN
F 1 "MountingHole" H 850 1455 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3" H 750 1500 50  0001 C CNN
F 3 "~" H 750 1500 50  0001 C CNN
	1    750  1500
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole H8
U 1 1 605C1175
P 1750 1500
F 0 "H8" H 1850 1546 50  0000 L CNN
F 1 "MountingHole" H 1850 1455 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3" H 1750 1500 50  0001 C CNN
F 3 "~" H 1750 1500 50  0001 C CNN
	1    1750 1500
	1    0    0    -1  
$EndComp
Wire Notes Line
	500  2000 2700 2000
Wire Notes Line
	8450 500  8450 3250
Text Notes 2750 600  0    50   ~ 0
RPi Pico Module via Pin Sockets. Debug pins not connected
NoConn ~ 5400 3050
NoConn ~ 5500 3050
NoConn ~ 5600 3050
NoConn ~ 6200 950 
$Comp
L power:GND #PWR019
U 1 1 605DA5B4
P 6500 1150
F 0 "#PWR019" H 6500 900 50  0001 C CNN
F 1 "GND" V 6505 1022 50  0000 R CNN
F 2 "" H 6500 1150 50  0001 C CNN
F 3 "" H 6500 1150 50  0001 C CNN
	1    6500 1150
	0    -1   -1   0   
$EndComp
NoConn ~ 6200 1250
$Comp
L power:+3.3V #PWR020
U 1 1 605DB42E
P 6500 1350
F 0 "#PWR020" H 6500 1200 50  0001 C CNN
F 1 "+3.3V" V 6515 1478 50  0000 L CNN
F 2 "" H 6500 1350 50  0001 C CNN
F 3 "" H 6500 1350 50  0001 C CNN
	1    6500 1350
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR021
U 1 1 605DC635
P 6500 1650
F 0 "#PWR021" H 6500 1400 50  0001 C CNN
F 1 "GND" V 6505 1522 50  0000 R CNN
F 2 "" H 6500 1650 50  0001 C CNN
F 3 "" H 6500 1650 50  0001 C CNN
	1    6500 1650
	0    -1   -1   0   
$EndComp
NoConn ~ 6200 1950
$Comp
L power:GND #PWR022
U 1 1 605DDAFA
P 6500 2150
F 0 "#PWR022" H 6500 1900 50  0001 C CNN
F 1 "GND" V 6505 2022 50  0000 R CNN
F 2 "" H 6500 2150 50  0001 C CNN
F 3 "" H 6500 2150 50  0001 C CNN
	1    6500 2150
	0    -1   -1   0   
$EndComp
$Comp
L power:GND #PWR023
U 1 1 605DE0C9
P 6500 2650
F 0 "#PWR023" H 6500 2400 50  0001 C CNN
F 1 "GND" V 6505 2522 50  0000 R CNN
F 2 "" H 6500 2650 50  0001 C CNN
F 3 "" H 6500 2650 50  0001 C CNN
	1    6500 2650
	0    -1   -1   0   
$EndComp
Wire Wire Line
	6200 1050 6500 1050
Wire Wire Line
	6200 1150 6500 1150
Wire Wire Line
	6200 1650 6500 1650
Wire Wire Line
	6200 2150 6500 2150
Wire Wire Line
	6200 2650 6500 2650
Wire Wire Line
	1750 2850 1750 3000
$Comp
L power:GND #PWR09
U 1 1 605F2F11
P 4450 1150
F 0 "#PWR09" H 4450 900 50  0001 C CNN
F 1 "GND" V 4455 1022 50  0000 R CNN
F 2 "" H 4450 1150 50  0001 C CNN
F 3 "" H 4450 1150 50  0001 C CNN
	1    4450 1150
	0    1    1    0   
$EndComp
Wire Wire Line
	4450 1150 4800 1150
Wire Wire Line
	1350 2450 1350 2550
Wire Wire Line
	1350 2850 1350 3000
Text Notes 550  2100 0    50   ~ 0
I2C Pull-Ups
$Comp
L power:GND #PWR010
U 1 1 60612B5F
P 4450 1650
F 0 "#PWR010" H 4450 1400 50  0001 C CNN
F 1 "GND" V 4455 1522 50  0000 R CNN
F 2 "" H 4450 1650 50  0001 C CNN
F 3 "" H 4450 1650 50  0001 C CNN
	1    4450 1650
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR011
U 1 1 60613054
P 4450 2150
F 0 "#PWR011" H 4450 1900 50  0001 C CNN
F 1 "GND" V 4455 2022 50  0000 R CNN
F 2 "" H 4450 2150 50  0001 C CNN
F 3 "" H 4450 2150 50  0001 C CNN
	1    4450 2150
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR012
U 1 1 6061349F
P 4450 2650
F 0 "#PWR012" H 4450 2400 50  0001 C CNN
F 1 "GND" V 4455 2522 50  0000 R CNN
F 2 "" H 4450 2650 50  0001 C CNN
F 3 "" H 4450 2650 50  0001 C CNN
	1    4450 2650
	0    1    1    0   
$EndComp
Wire Wire Line
	4450 1650 4800 1650
Wire Wire Line
	4450 2150 4800 2150
Wire Wire Line
	4450 2650 4800 2650
Wire Notes Line
	500  500  11100 500 
Wire Notes Line
	500  3250 11100 3250
Text Notes 8500 600  0    50   ~ 0
Additional power connector
Wire Wire Line
	9050 2400 8950 2400
Text Notes 2800 3450 0    50   ~ 0
IO Board 00
Wire Wire Line
	4050 3750 3700 3750
Wire Notes Line
	4800 3250 4800 4450
$Comp
L Connector_Generic:Conn_02x06_Odd_Even J2
U 1 1 606CF51A
P 5500 3900
F 0 "J2" H 5550 4317 50  0000 C CNN
F 1 "IO-Board_01" H 5550 4226 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_2x06_P2.54mm_Vertical" H 5500 3900 50  0001 C CNN
F 3 "~" H 5500 3900 50  0001 C CNN
	1    5500 3900
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR013
U 1 1 606CF676
P 6150 3700
F 0 "#PWR013" H 6150 3450 50  0001 C CNN
F 1 "GND" V 6155 3572 50  0000 R CNN
F 2 "" H 6150 3700 50  0001 C CNN
F 3 "" H 6150 3700 50  0001 C CNN
	1    6150 3700
	0    -1   -1   0   
$EndComp
$Comp
L power:+3.3V #PWR016
U 1 1 606CF680
P 6150 3900
F 0 "#PWR016" H 6150 3750 50  0001 C CNN
F 1 "+3.3V" V 6165 4028 50  0000 L CNN
F 2 "" H 6150 3900 50  0001 C CNN
F 3 "" H 6150 3900 50  0001 C CNN
	1    6150 3900
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR015
U 1 1 606CF694
P 6150 4150
F 0 "#PWR015" H 6150 3900 50  0001 C CNN
F 1 "GND" V 6155 4022 50  0000 R CNN
F 2 "" H 6150 4150 50  0001 C CNN
F 3 "" H 6150 4150 50  0001 C CNN
	1    6150 4150
	0    -1   -1   0   
$EndComp
Text Notes 4900 3450 0    50   ~ 0
IO Board 01
Wire Wire Line
	6150 3800 5800 3800
Wire Wire Line
	5800 3700 6150 3700
$Comp
L power:+3.3V #PWR014
U 1 1 606F27EB
P 6150 4000
F 0 "#PWR014" H 6150 3850 50  0001 C CNN
F 1 "+3.3V" V 6165 4128 50  0000 L CNN
F 2 "" H 6150 4000 50  0001 C CNN
F 3 "" H 6150 4000 50  0001 C CNN
	1    6150 4000
	0    1    1    0   
$EndComp
Wire Notes Line
	6900 3250 6900 4450
$Comp
L Connector_Generic:Conn_02x06_Odd_Even J3
U 1 1 607052B7
P 7550 3900
F 0 "J3" H 7600 4317 50  0000 C CNN
F 1 "IO-Board_10" H 7600 4226 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_2x06_P2.54mm_Vertical" H 7550 3900 50  0001 C CNN
F 3 "~" H 7550 3900 50  0001 C CNN
	1    7550 3900
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR024
U 1 1 6070543D
P 8200 3700
F 0 "#PWR024" H 8200 3450 50  0001 C CNN
F 1 "GND" V 8205 3572 50  0000 R CNN
F 2 "" H 8200 3700 50  0001 C CNN
F 3 "" H 8200 3700 50  0001 C CNN
	1    8200 3700
	0    -1   -1   0   
$EndComp
$Comp
L power:+3.3V #PWR028
U 1 1 60705447
P 8200 3900
F 0 "#PWR028" H 8200 3750 50  0001 C CNN
F 1 "+3.3V" V 8215 4028 50  0000 L CNN
F 2 "" H 8200 3900 50  0001 C CNN
F 3 "" H 8200 3900 50  0001 C CNN
	1    8200 3900
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR027
U 1 1 6070545B
P 8200 4200
F 0 "#PWR027" H 8200 3950 50  0001 C CNN
F 1 "GND" V 8205 4072 50  0000 R CNN
F 2 "" H 8200 4200 50  0001 C CNN
F 3 "" H 8200 4200 50  0001 C CNN
	1    8200 4200
	0    -1   -1   0   
$EndComp
Text Notes 7000 3450 0    50   ~ 0
IO Board 10
$Comp
L power:+3.3V #PWR026
U 1 1 6070547B
P 8200 4100
F 0 "#PWR026" H 8200 3950 50  0001 C CNN
F 1 "+3.3V" V 8215 4228 50  0000 L CNN
F 2 "" H 8200 4100 50  0001 C CNN
F 3 "" H 8200 4100 50  0001 C CNN
	1    8200 4100
	0    1    1    0   
$EndComp
Wire Notes Line
	9000 3250 9000 4450
$Comp
L power:GND #PWR025
U 1 1 6071575A
P 8200 4000
F 0 "#PWR025" H 8200 3750 50  0001 C CNN
F 1 "GND" V 8205 3872 50  0000 R CNN
F 2 "" H 8200 4000 50  0001 C CNN
F 3 "" H 8200 4000 50  0001 C CNN
	1    8200 4000
	0    -1   -1   0   
$EndComp
$Comp
L Connector_Generic:Conn_02x06_Odd_Even J4
U 1 1 60722953
P 9700 3900
F 0 "J4" H 9750 4317 50  0000 C CNN
F 1 "IO-Board_11" H 9750 4226 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_2x06_P2.54mm_Vertical" H 9700 3900 50  0001 C CNN
F 3 "~" H 9700 3900 50  0001 C CNN
	1    9700 3900
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR031
U 1 1 60722BD9
P 10350 3700
F 0 "#PWR031" H 10350 3450 50  0001 C CNN
F 1 "GND" V 10355 3572 50  0000 R CNN
F 2 "" H 10350 3700 50  0001 C CNN
F 3 "" H 10350 3700 50  0001 C CNN
	1    10350 3700
	0    -1   -1   0   
$EndComp
$Comp
L power:+3.3V #PWR035
U 1 1 60722BE3
P 10350 3900
F 0 "#PWR035" H 10350 3750 50  0001 C CNN
F 1 "+3.3V" V 10365 4028 50  0000 L CNN
F 2 "" H 10350 3900 50  0001 C CNN
F 3 "" H 10350 3900 50  0001 C CNN
	1    10350 3900
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR033
U 1 1 60722BF7
P 10350 4200
F 0 "#PWR033" H 10350 3950 50  0001 C CNN
F 1 "GND" V 10355 4072 50  0000 R CNN
F 2 "" H 10350 4200 50  0001 C CNN
F 3 "" H 10350 4200 50  0001 C CNN
	1    10350 4200
	0    -1   -1   0   
$EndComp
Text Notes 9100 3450 0    50   ~ 0
IO Board 11
$Comp
L power:+3.3V #PWR032
U 1 1 60722C11
P 10350 4050
F 0 "#PWR032" H 10350 3900 50  0001 C CNN
F 1 "+3.3V" V 10365 4178 50  0000 L CNN
F 2 "" H 10350 4050 50  0001 C CNN
F 3 "" H 10350 4050 50  0001 C CNN
	1    10350 4050
	0    1    1    0   
$EndComp
Wire Notes Line
	11100 500  11100 4450
Wire Notes Line
	2700 4450 11100 4450
$Comp
L RF:NRF24L01_Breakout U1
U 1 1 60745AEA
P 1500 4500
F 0 "U1" V 1900 4850 50  0000 L CNN
F 1 "NRF24L01_Breakout" V 1900 3950 50  0000 L CNN
F 2 "RF_Module:nRF24L01_Breakout" H 1650 5100 50  0001 L CIN
F 3 "http://www.nordicsemi.com/eng/content/download/2730/34105/file/nRF24L01_Product_Specification_v2_0.pdf" H 1500 4400 50  0001 C CNN
	1    1500 4500
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR04
U 1 1 60769A68
P 1500 5200
F 0 "#PWR04" H 1500 4950 50  0001 C CNN
F 1 "GND" H 1505 5027 50  0000 C CNN
F 2 "" H 1500 5200 50  0001 C CNN
F 3 "" H 1500 5200 50  0001 C CNN
	1    1500 5200
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR03
U 1 1 60776107
P 1500 3800
F 0 "#PWR03" H 1500 3650 50  0001 C CNN
F 1 "+3.3V" H 1515 3973 50  0000 C CNN
F 2 "" H 1500 3800 50  0001 C CNN
F 3 "" H 1500 3800 50  0001 C CNN
	1    1500 3800
	1    0    0    -1  
$EndComp
NoConn ~ 1000 4800
Text Notes 550  3450 0    50   ~ 0
Optional nRF24L01 board via pin sockets
$Comp
L MCU_RPi:Pico U2
U 1 1 6052D8F1
P 5500 1900
F 0 "U2" H 5500 3115 50  0000 C CNN
F 1 "Pico" H 5500 3024 50  0000 C CNN
F 2 "MCU_RPi:RPi_Pico_SMD_TH" V 5500 1900 50  0001 C CNN
F 3 "" H 5500 1900 50  0001 C CNN
	1    5500 1900
	1    0    0    -1  
$EndComp
Text Label 4800 950  2    50   ~ 0
SDA
Text Label 4800 1050 2    50   ~ 0
SCL
Text Label 4800 1250 2    50   ~ 0
SPI_CLK
Text Label 4800 1350 2    50   ~ 0
SPI_MOSI
Text Label 4800 1450 2    50   ~ 0
SPI_MISO
Text Label 4800 1550 2    50   ~ 0
SPI_CS0
Text Label 4800 1750 2    50   ~ 0
IO0_GP0
Text Label 4800 1850 2    50   ~ 0
IO0_GP1
Text Label 4800 1950 2    50   ~ 0
IO0_GP2
Text Label 4800 2050 2    50   ~ 0
IO0_GP3
Text Label 6200 1550 0    50   ~ 0
NRF_CE
Text Label 4800 2250 2    50   ~ 0
IO1_GP0
Text Label 4800 2350 2    50   ~ 0
IO1_GP1
Text Label 4800 2450 2    50   ~ 0
IO1_GP2
Text Label 4800 2550 2    50   ~ 0
IO1_GP3
Text Label 4800 2750 2    50   ~ 0
IO2_GP0
Text Label 4800 2850 2    50   ~ 0
IO2_GP1
Text Label 6200 2850 0    50   ~ 0
IO2_GP2
Text Label 6200 2750 0    50   ~ 0
IO2_GP3
Text Label 6200 2550 0    50   ~ 0
IO3_GP0
Text Label 6200 2450 0    50   ~ 0
IO3_GP1
Text Label 6200 2350 0    50   ~ 0
IO3_GP2
Text Label 6200 2250 0    50   ~ 0
IO3_GP3
Text Label 6200 2050 0    50   ~ 0
STATUS_LEDS
$Comp
L power:VDD #PWR018
U 1 1 605625A4
P 6500 1050
F 0 "#PWR018" H 6500 900 50  0001 C CNN
F 1 "VDD" V 6515 1178 50  0000 L CNN
F 2 "" H 6500 1050 50  0001 C CNN
F 3 "" H 6500 1050 50  0001 C CNN
	1    6500 1050
	0    1    1    0   
$EndComp
$Comp
L power:VDD #PWR034
U 1 1 605631A8
P 10200 1750
F 0 "#PWR034" H 10200 1600 50  0001 C CNN
F 1 "VDD" H 10215 1923 50  0000 C CNN
F 2 "" H 10200 1750 50  0001 C CNN
F 3 "" H 10200 1750 50  0001 C CNN
	1    10200 1750
	1    0    0    -1  
$EndComp
Text Label 1000 4200 2    50   ~ 0
SPI_MOSI
Text Label 1000 4300 2    50   ~ 0
SPI_MISO
Text Label 1000 4400 2    50   ~ 0
SPI_CLK
Text Label 1000 4500 2    50   ~ 0
SPI_CS0
Text Label 1000 4700 2    50   ~ 0
NRF_CE
Text Label 3200 3650 2    50   ~ 0
SDA
Text Label 3200 3750 2    50   ~ 0
SCL
$Comp
L power:VDD #PWR08
U 1 1 6056F660
P 4050 3750
F 0 "#PWR08" H 4050 3600 50  0001 C CNN
F 1 "VDD" V 4065 3878 50  0000 L CNN
F 2 "" H 4050 3750 50  0001 C CNN
F 3 "" H 4050 3750 50  0001 C CNN
	1    4050 3750
	0    1    1    0   
$EndComp
Text Label 3200 4150 2    50   ~ 0
IO0_GP0
Text Label 3200 4050 2    50   ~ 0
IO0_GP1
Text Label 3200 3950 2    50   ~ 0
IO0_GP2
Text Label 3200 3850 2    50   ~ 0
IO0_GP3
$Comp
L power:VDD #PWR017
U 1 1 605746E9
P 6150 3800
F 0 "#PWR017" H 6150 3650 50  0001 C CNN
F 1 "VDD" V 6165 3928 50  0000 L CNN
F 2 "" H 6150 3800 50  0001 C CNN
F 3 "" H 6150 3800 50  0001 C CNN
	1    6150 3800
	0    1    1    0   
$EndComp
Text Label 5300 3700 2    50   ~ 0
SDA
Text Label 7350 3800 2    50   ~ 0
SCL
Text Label 5300 4200 2    50   ~ 0
IO1_GP0
Text Label 5300 4100 2    50   ~ 0
IO1_GP1
Text Label 5300 4000 2    50   ~ 0
IO1_GP2
Text Label 5300 3900 2    50   ~ 0
IO1_GP3
$Comp
L power:VDD #PWR029
U 1 1 60578957
P 8200 3800
F 0 "#PWR029" H 8200 3650 50  0001 C CNN
F 1 "VDD" V 8215 3928 50  0000 L CNN
F 2 "" H 8200 3800 50  0001 C CNN
F 3 "" H 8200 3800 50  0001 C CNN
	1    8200 3800
	0    1    1    0   
$EndComp
Text Label 7350 3700 2    50   ~ 0
SDA
Text Label 7350 4200 2    50   ~ 0
IO2_GP0
Text Label 7350 4100 2    50   ~ 0
IO2_GP1
Text Label 7350 4000 2    50   ~ 0
IO2_GP2
Text Label 7350 3900 2    50   ~ 0
IO2_GP3
Text Label 5300 3800 2    50   ~ 0
SCL
Text Label 9500 3700 2    50   ~ 0
SDA
Text Label 9500 3800 2    50   ~ 0
SCL
Text Label 9500 4200 2    50   ~ 0
IO3_GP0
Text Label 9500 4100 2    50   ~ 0
IO3_GP1
Text Label 9500 4000 2    50   ~ 0
IO3_GP2
Text Label 9500 3900 2    50   ~ 0
IO3_GP3
$Comp
L power:VDD #PWR036
U 1 1 60586175
P 10350 3800
F 0 "#PWR036" H 10350 3650 50  0001 C CNN
F 1 "VDD" V 10365 3928 50  0000 L CNN
F 2 "" H 10350 3800 50  0001 C CNN
F 3 "" H 10350 3800 50  0001 C CNN
	1    10350 3800
	0    1    1    0   
$EndComp
Connection ~ 9050 2400
$Comp
L Connector:USB_B_Micro J5
U 1 1 6053037D
P 9050 2000
F 0 "J5" H 9107 2467 50  0000 C CNN
F 1 "USB_B_Micro" H 9107 2376 50  0000 C CNN
F 2 "Connector_USB:USB_Micro-B_Wuerth_629105150521" H 9200 1950 50  0001 C CNN
F 3 "~" H 9200 1950 50  0001 C CNN
	1    9050 2000
	1    0    0    -1  
$EndComp
Text Label 1350 3000 3    50   ~ 0
SDA
Text Label 1750 3000 3    50   ~ 0
SCL
$Comp
L Connector_Generic:Conn_01x05 J6
U 1 1 605B8435
P 9300 1050
F 0 "J6" V 9264 762 50  0000 R CNN
F 1 "Conn_01x05: Adafruit 1833" V 9173 762 50  0000 R CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x05_P2.54mm_Vertical" H 9300 1050 50  0001 C CNN
F 3 "~" H 9300 1050 50  0001 C CNN
	1    9300 1050
	0    -1   -1   0   
$EndComp
Connection ~ 9500 1800
Wire Wire Line
	9500 1800 9650 1800
Wire Wire Line
	9500 1250 9500 1800
NoConn ~ 6200 1850
NoConn ~ 6200 1750
NoConn ~ 9100 1250
NoConn ~ 9200 1250
NoConn ~ 9300 1250
NoConn ~ 9400 1250
$Comp
L Mechanical:MountingHole H9
U 1 1 605FD33B
P 750 1750
F 0 "H9" H 850 1796 50  0000 L CNN
F 1 "MountingHole" H 850 1705 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3" H 750 1750 50  0001 C CNN
F 3 "~" H 750 1750 50  0001 C CNN
	1    750  1750
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole H10
U 1 1 605FDAC1
P 1750 1750
F 0 "H10" H 1850 1796 50  0000 L CNN
F 1 "MountingHole" H 1850 1705 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3" H 1750 1750 50  0001 C CNN
F 3 "~" H 1750 1750 50  0001 C CNN
	1    1750 1750
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0101
U 1 1 60611674
P 1500 6050
F 0 "#PWR0101" H 1500 5800 50  0001 C CNN
F 1 "GND" H 1505 5877 50  0000 C CNN
F 2 "" H 1500 6050 50  0001 C CNN
F 3 "" H 1500 6050 50  0001 C CNN
	1    1500 6050
	0    1    1    0   
$EndComp
Wire Wire Line
	1500 6050 1950 6050
$Comp
L power:VDD #PWR0102
U 1 1 6061584A
P 1500 6250
F 0 "#PWR0102" H 1500 6100 50  0001 C CNN
F 1 "VDD" V 1515 6378 50  0000 L CNN
F 2 "" H 1500 6250 50  0001 C CNN
F 3 "" H 1500 6250 50  0001 C CNN
	1    1500 6250
	0    -1   -1   0   
$EndComp
Wire Wire Line
	1500 6250 1950 6250
Text Label 1950 6150 2    50   ~ 0
STATUS_LEDS
Text Notes 550  5700 0    50   ~ 0
Status LEDs
Wire Notes Line
	2700 5500 500  5500
$Comp
L Connector_Generic:Conn_02x02_Odd_Even J8
U 1 1 606324DE
P 1100 7150
F 0 "J8" H 1150 7367 50  0000 C CNN
F 1 "Conn_02x02_Odd_Even" H 1150 7276 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_2x02_P2.54mm_Vertical" H 1100 7150 50  0001 C CNN
F 3 "~" H 1100 7150 50  0001 C CNN
	1    1100 7150
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_02x02_Odd_Even J9
U 1 1 60632A27
P 2050 7150
F 0 "J9" H 2100 7367 50  0000 C CNN
F 1 "Conn_02x02_Odd_Even" H 2100 7276 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_2x02_P2.54mm_Vertical" H 2050 7150 50  0001 C CNN
F 3 "~" H 2050 7150 50  0001 C CNN
	1    2050 7150
	1    0    0    -1  
$EndComp
NoConn ~ 2350 7150
NoConn ~ 2350 7250
NoConn ~ 1850 7250
NoConn ~ 1850 7150
NoConn ~ 900  7150
NoConn ~ 900  7250
NoConn ~ 1400 7150
NoConn ~ 1400 7250
Text Notes 550  6850 0    50   ~ 0
Mechanical support / stands
Wire Notes Line
	2700 6650 500  6650
Wire Wire Line
	3700 3650 4050 3650
Wire Wire Line
	4050 3850 3700 3850
Wire Wire Line
	3700 4050 3950 4050
Wire Wire Line
	3700 3950 3950 3950
Wire Wire Line
	3950 3950 3950 4050
Connection ~ 3950 4050
Wire Wire Line
	3950 4050 4050 4050
Wire Wire Line
	3700 4150 3950 4150
Wire Wire Line
	3950 4150 3950 4050
Wire Wire Line
	5800 3900 6150 3900
Wire Wire Line
	6150 4000 5800 4000
Wire Wire Line
	5800 4100 6050 4100
Wire Wire Line
	6050 4100 6050 4150
Wire Wire Line
	6050 4150 6150 4150
Wire Wire Line
	5800 4200 6050 4200
Wire Wire Line
	6050 4200 6050 4150
Connection ~ 6050 4150
Wire Wire Line
	8200 3900 7850 3900
Wire Wire Line
	7850 3800 8200 3800
Wire Wire Line
	8200 4200 7850 4200
Wire Wire Line
	7850 4100 8200 4100
Wire Wire Line
	8200 4000 7850 4000
Wire Wire Line
	7850 3700 8200 3700
Wire Wire Line
	10350 4200 10000 4200
Wire Wire Line
	10000 4100 10250 4100
Wire Wire Line
	10250 4100 10250 4050
Wire Wire Line
	10250 4000 10000 4000
Wire Wire Line
	10350 4050 10250 4050
Connection ~ 10250 4050
Wire Wire Line
	10250 4050 10250 4000
Wire Wire Line
	10350 3900 10000 3900
Wire Wire Line
	10000 3800 10350 3800
Wire Wire Line
	10000 3700 10350 3700
Text Notes 2750 4350 0    50   ~ 0
Pins 2, 4, 6: Supply; Pins 8, 10, 12: I2C ID
Text Notes 4850 4350 0    50   ~ 0
Pins 2, 4, 6: Supply; Pins 8, 10, 12: I2C ID
Text Notes 6950 4350 0    50   ~ 0
Pins 2, 4, 6: Supply; Pins 8, 10, 12: I2C ID
Text Notes 9050 4350 0    50   ~ 0
Pins 2, 4, 6: Supply; Pins 8, 10, 12: I2C ID
Wire Wire Line
	1500 5100 1500 5200
Wire Wire Line
	1500 3800 1500 3900
$Comp
L Connector:Screw_Terminal_01x04 J7
U 1 1 6056C000
P 2150 6150
F 0 "J7" H 2230 6142 50  0000 L CNN
F 1 "Screw_Terminal_01x04" V 2400 5800 50  0000 L CNN
F 2 "TerminalBlock:TerminalBlock_Altech_AK300-4_P5.00mm" H 2150 6150 50  0001 C CNN
F 3 "~" H 2150 6150 50  0001 C CNN
	1    2150 6150
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR037
U 1 1 6056D4FD
P 1500 6350
F 0 "#PWR037" H 1500 6100 50  0001 C CNN
F 1 "GND" H 1505 6177 50  0000 C CNN
F 2 "" H 1500 6350 50  0001 C CNN
F 3 "" H 1500 6350 50  0001 C CNN
	1    1500 6350
	0    1    1    0   
$EndComp
Wire Wire Line
	1500 6350 1950 6350
Wire Wire Line
	6200 1350 6500 1350
NoConn ~ 6200 1450
$Comp
L Device:C C1
U 1 1 6057FC08
P 2300 4500
F 0 "C1" H 2415 4546 50  0000 L CNN
F 1 "100nF" H 2415 4455 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 2338 4350 50  0001 C CNN
F 3 "~" H 2300 4500 50  0001 C CNN
	1    2300 4500
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR038
U 1 1 60580399
P 2300 4250
F 0 "#PWR038" H 2300 4100 50  0001 C CNN
F 1 "+3.3V" H 2315 4423 50  0000 C CNN
F 2 "" H 2300 4250 50  0001 C CNN
F 3 "" H 2300 4250 50  0001 C CNN
	1    2300 4250
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR039
U 1 1 60580C0E
P 2300 4750
F 0 "#PWR039" H 2300 4500 50  0001 C CNN
F 1 "GND" H 2305 4577 50  0000 C CNN
F 2 "" H 2300 4750 50  0001 C CNN
F 3 "" H 2300 4750 50  0001 C CNN
	1    2300 4750
	1    0    0    -1  
$EndComp
Wire Wire Line
	2300 4750 2300 4650
Wire Wire Line
	2300 4350 2300 4250
$Comp
L Device:C C2
U 1 1 605933C7
P 850 6250
F 0 "C2" H 965 6296 50  0000 L CNN
F 1 "100nF" H 965 6205 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 888 6100 50  0001 C CNN
F 3 "~" H 850 6250 50  0001 C CNN
	1    850  6250
	1    0    0    -1  
$EndComp
$Comp
L power:VDD #PWR040
U 1 1 60593AD5
P 850 6050
F 0 "#PWR040" H 850 5900 50  0001 C CNN
F 1 "VDD" V 865 6178 50  0000 L CNN
F 2 "" H 850 6050 50  0001 C CNN
F 3 "" H 850 6050 50  0001 C CNN
	1    850  6050
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR041
U 1 1 6059884D
P 850 6450
F 0 "#PWR041" H 850 6200 50  0001 C CNN
F 1 "GND" H 855 6277 50  0000 C CNN
F 2 "" H 850 6450 50  0001 C CNN
F 3 "" H 850 6450 50  0001 C CNN
	1    850  6450
	1    0    0    -1  
$EndComp
Wire Wire Line
	850  6400 850  6450
Wire Wire Line
	850  6050 850  6100
$Comp
L Device:C C3
U 1 1 605A2EB9
P 7900 1250
F 0 "C3" H 8015 1296 50  0000 L CNN
F 1 "100nF" H 8015 1205 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 7938 1100 50  0001 C CNN
F 3 "~" H 7900 1250 50  0001 C CNN
	1    7900 1250
	1    0    0    -1  
$EndComp
$Comp
L power:VDD #PWR042
U 1 1 605A70D5
P 7900 1000
F 0 "#PWR042" H 7900 850 50  0001 C CNN
F 1 "VDD" V 7915 1128 50  0000 L CNN
F 2 "" H 7900 1000 50  0001 C CNN
F 3 "" H 7900 1000 50  0001 C CNN
	1    7900 1000
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR043
U 1 1 605A771F
P 7900 1550
F 0 "#PWR043" H 7900 1300 50  0001 C CNN
F 1 "GND" V 7905 1422 50  0000 R CNN
F 2 "" H 7900 1550 50  0001 C CNN
F 3 "" H 7900 1550 50  0001 C CNN
	1    7900 1550
	1    0    0    -1  
$EndComp
Wire Wire Line
	7900 1000 7900 1100
Wire Wire Line
	7900 1400 7900 1550
$Comp
L Device:C C4
U 1 1 605DAA1B
P 4600 3800
F 0 "C4" H 4500 3900 50  0000 L CNN
F 1 "100nF" V 4750 3700 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 4638 3650 50  0001 C CNN
F 3 "~" H 4600 3800 50  0001 C CNN
	1    4600 3800
	1    0    0    -1  
$EndComp
$Comp
L power:VDD #PWR044
U 1 1 605DAB43
P 4600 3550
F 0 "#PWR044" H 4600 3400 50  0001 C CNN
F 1 "VDD" V 4615 3678 50  0000 L CNN
F 2 "" H 4600 3550 50  0001 C CNN
F 3 "" H 4600 3550 50  0001 C CNN
	1    4600 3550
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR045
U 1 1 605DAB4D
P 4600 4100
F 0 "#PWR045" H 4600 3850 50  0001 C CNN
F 1 "GND" V 4605 3972 50  0000 R CNN
F 2 "" H 4600 4100 50  0001 C CNN
F 3 "" H 4600 4100 50  0001 C CNN
	1    4600 4100
	1    0    0    -1  
$EndComp
Wire Wire Line
	4600 3550 4600 3650
Wire Wire Line
	4600 3950 4600 4100
$Comp
L Device:C C5
U 1 1 606061B2
P 6700 3800
F 0 "C5" H 6600 3900 50  0000 L CNN
F 1 "100nF" V 6850 3700 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 6738 3650 50  0001 C CNN
F 3 "~" H 6700 3800 50  0001 C CNN
	1    6700 3800
	1    0    0    -1  
$EndComp
$Comp
L power:VDD #PWR046
U 1 1 60606588
P 6700 3550
F 0 "#PWR046" H 6700 3400 50  0001 C CNN
F 1 "VDD" V 6715 3678 50  0000 L CNN
F 2 "" H 6700 3550 50  0001 C CNN
F 3 "" H 6700 3550 50  0001 C CNN
	1    6700 3550
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR047
U 1 1 60606592
P 6700 4100
F 0 "#PWR047" H 6700 3850 50  0001 C CNN
F 1 "GND" V 6705 3972 50  0000 R CNN
F 2 "" H 6700 4100 50  0001 C CNN
F 3 "" H 6700 4100 50  0001 C CNN
	1    6700 4100
	1    0    0    -1  
$EndComp
Wire Wire Line
	6700 3550 6700 3650
Wire Wire Line
	6700 3950 6700 4100
$Comp
L Device:C C6
U 1 1 6061B651
P 8750 3800
F 0 "C6" H 8650 3900 50  0000 L CNN
F 1 "100nF" V 8900 3700 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 8788 3650 50  0001 C CNN
F 3 "~" H 8750 3800 50  0001 C CNN
	1    8750 3800
	1    0    0    -1  
$EndComp
$Comp
L power:VDD #PWR048
U 1 1 6061B91D
P 8750 3550
F 0 "#PWR048" H 8750 3400 50  0001 C CNN
F 1 "VDD" V 8765 3678 50  0000 L CNN
F 2 "" H 8750 3550 50  0001 C CNN
F 3 "" H 8750 3550 50  0001 C CNN
	1    8750 3550
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR049
U 1 1 6061B927
P 8750 4100
F 0 "#PWR049" H 8750 3850 50  0001 C CNN
F 1 "GND" V 8755 3972 50  0000 R CNN
F 2 "" H 8750 4100 50  0001 C CNN
F 3 "" H 8750 4100 50  0001 C CNN
	1    8750 4100
	1    0    0    -1  
$EndComp
Wire Wire Line
	8750 3550 8750 3650
Wire Wire Line
	8750 3950 8750 4100
$Comp
L Device:C C7
U 1 1 6062B8FA
P 10850 3800
F 0 "C7" H 10750 3900 50  0000 L CNN
F 1 "100nF" V 11000 3700 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 10888 3650 50  0001 C CNN
F 3 "~" H 10850 3800 50  0001 C CNN
	1    10850 3800
	1    0    0    -1  
$EndComp
$Comp
L power:VDD #PWR050
U 1 1 6062BA32
P 10850 3550
F 0 "#PWR050" H 10850 3400 50  0001 C CNN
F 1 "VDD" V 10865 3678 50  0000 L CNN
F 2 "" H 10850 3550 50  0001 C CNN
F 3 "" H 10850 3550 50  0001 C CNN
	1    10850 3550
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR051
U 1 1 6062BA3C
P 10850 4100
F 0 "#PWR051" H 10850 3850 50  0001 C CNN
F 1 "GND" V 10855 3972 50  0000 R CNN
F 2 "" H 10850 4100 50  0001 C CNN
F 3 "" H 10850 4100 50  0001 C CNN
	1    10850 4100
	1    0    0    -1  
$EndComp
Wire Wire Line
	10850 3550 10850 3650
Wire Wire Line
	10850 3950 10850 4100
$Comp
L Device:C C8
U 1 1 6063F026
P 7350 1250
F 0 "C8" H 7465 1296 50  0000 L CNN
F 1 "100nF" H 7465 1205 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 7388 1100 50  0001 C CNN
F 3 "~" H 7350 1250 50  0001 C CNN
	1    7350 1250
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR053
U 1 1 6063F284
P 7350 1550
F 0 "#PWR053" H 7350 1300 50  0001 C CNN
F 1 "GND" V 7355 1422 50  0000 R CNN
F 2 "" H 7350 1550 50  0001 C CNN
F 3 "" H 7350 1550 50  0001 C CNN
	1    7350 1550
	1    0    0    -1  
$EndComp
Wire Wire Line
	7350 1400 7350 1550
$Comp
L power:+3.3V #PWR052
U 1 1 6064501B
P 7350 1000
F 0 "#PWR052" H 7350 850 50  0001 C CNN
F 1 "+3.3V" V 7365 1128 50  0000 L CNN
F 2 "" H 7350 1000 50  0001 C CNN
F 3 "" H 7350 1000 50  0001 C CNN
	1    7350 1000
	1    0    0    -1  
$EndComp
Wire Wire Line
	7350 1000 7350 1100
$EndSCHEMATC