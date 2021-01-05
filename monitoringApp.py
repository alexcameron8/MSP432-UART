import serial
import threading
import time
import sys

#initializing the state
state = "0"

#Serial port config
ser = serial.Serial('COM4') #Opens com port
print("Serial port: " + ser.name + " Is serial port open?: " + str(ser.is_open))             #the port in which was opened 
ser.baudrate =9600
ser.bytesize = 8
ser.parity = 'N'
ser.stopbits = 1
ser.timeout = 0
print(ser.is_open)

def receive_uart(ser):
    while True:
        for receive in ser.read(): 
            if(receive==49): #When board is in state 1, TXBUF = 49
                receive=1
            elif(receive==50): #When board is in state 2, TXBUF = 50
                receive =2
            elif(receive==51):#When board is in state 3, TXBUF = 51
                receive =3
            elif(receive==52):#When board is in state 4, TXBUF = 52
                receive =4
            
            print("Current State:" + str(receive))

#Run a thread which always checks to see if the msp board is trying to send data while thse monitoring program always waits for the users input to write to UART
readThread = threading.Thread(target=receive_uart, args=(ser,))
readThread.start()

def getInput():
    global state
    while(1):   
        state = input("Enter State '1' , '2', '3' or '4' or 'quit' :\n")
        if(state == "1"):
            ser.write(b'1')
            time.sleep(1) # wait 1 second to allow the receive_uart() function to print the current state
        elif(state == "2"):
            ser.write(b'2')
            time.sleep(1)
        elif(state =="3"):
            ser.write(b'3')
            time.sleep(1)
        elif(state =="4"):
            ser.write(b'4')
            time.sleep(1)
        elif(state =="quit"):
            sys.exit("Monitor Program aborted.") 
        else:
            ser.write(b'1') #reset 
            print("unknown state")

while True:
    getInput()