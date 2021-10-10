from bluetooth import *
import RPi.GPIO as GPIO  #RPi.GPIO 라이브러리를 GPIO로 사용
from time import sleep  #time 라이브러리의 sleep함수 사용


switch1 = 37
switch2 = 35
switch3 = 33
switch4 = 31 
state = 1
Map_Num = 0
Select_Num = 0
Root_Count = 0
Go = 0
Back_ON = 0
Keep_Space = 'z'
Back_Space = 0
Back = 0
Ex_Root_Count = 0
CCC = 0
Count = 0

GPIO.setmode(GPIO.BOARD)
#GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)
GPIO.setup(switch1,GPIO.IN, GPIO.PUD_UP)
GPIO.setup(switch2,GPIO.IN, GPIO.PUD_UP)
GPIO.setup(switch3,GPIO.IN, GPIO.PUD_UP)
GPIO.setup(switch4,GPIO.IN, GPIO.PUD_UP)

pin1 = 38
pin2 = 40
GPIO.setup(pin1, GPIO.OUT)
GPIO.setup(pin2, GPIO.OUT)


servoPin = 12
SERVO_MAX_DUTY = 12
SERVO_MIN_DUTY = 3

#GPIO.setmode(GPIO.BOARD)        # GPIO 설정
GPIO.setup(servoPin, GPIO.OUT)  # 서보핀 출력으로 설정

servo = GPIO.PWM(servoPin, 50)  # 서보핀을 PWM 모드 50Hz로 사용하기 (50Hz > 20ms)
servo.start(90)  # 서보 PWM 시작 duty = 0, duty가 0이면 서보는 동작하지 않는다.

def setServoPos(degree):
    if degree > 180:
      degree = 180
    
    duty = SERVO_MIN_DUTY+(degree*(SERVO_MAX_DUTY-SERVO_MIN_DUTY)/180.0)
    #print("Degree: {} to {}(Duty)".format(degree, duty))
    
    servo.ChangeDutyCycle(duty)
    
client_socket=BluetoothSocket(RFCOMM)

client_socket.connect(("20:21:02:02:04:07", 1))
print("bluetooth connected!")

while True:
    
    #this is ckecking about DC Motor 
          
    msg = client_socket.recv(1024)
    data = msg.decode()
    print(data)
    print("recived message : {}".format(data))
    
    if data == 'J':
        Map_Num = 1
        print("1")
        for i in range(4) : 
            GPIO.output(pin1,True)
            GPIO.output(pin2,False)
            sleep(0.5)
            GPIO.output(pin1,True)
            GPIO.output(pin2,True)
            sleep(0.5)
    else :
        setServoPos(90)
        GPIO.output(pin1,True)
        GPIO.output(pin2,True)


    ####################################################Jangwnag Station#######################################################
    if(Map_Num == 1) :
        if GPIO.input(switch1) == 0 :
            Select_Num = 1
            print("switch1")
        elif GPIO.input(switch2) == 0 :
            Select_Num = 2
            print("switch2")
        elif GPIO.input(switch3) == 0 :
            if Count == 0 :
                Back_ON = 1
                print("switch3")                        
                Count = 1
        elif GPIO.input(switch4) == 0 : # Select_Num = 4 # this switch on -> reset
            print("switch4")
            Map_Num = 0
            Select_Num = 0
            Root_Count = 0
            Go = 0
            Count = 0
            setServoPos(0)
            GPIO.output(pin1,True)
            GPIO.output(pin2,False)
            sleep(2)
           #every variable number reset 
        #######################################Select_Num = 1###############################################
        if  Select_Num == 1 :
            if(Root_Count == 0) :
                    if(Go == 0) :
                        setServoPos(0)  #right
                        if(data == 'e' or  data == 'f') :  # after modify  (sensor value)
                            GPIO.output(pin1,True)
                            GPIO.output(pin2,False)
                            sleep(2)
                            print("1")
                            Go = 1
                    elif (Go == 1) :
                        setServoPos(90)  #straight
                        CCC = data
                        if(data == 'A') :  # after modify (RFID value)
                            GPIO.output(pin1,True)
                            GPIO.output(pin2,False)
                            sleep(1)
                            GPIO.output(pin1,True)
                            GPIO.output(pin2,True)
                            sleep(0.5)
                            GPIO.output(pin1,True)
                            GPIO.output(pin2,False)
                            sleep(1)
                            GPIO.output(pin1,True)
                            GPIO.output(pin2,True)
                            sleep(0.5)
                            print("2")
                            Go = 0
                            Root_Count = 1
            elif Root_Count == 1  :
                    if(Go == 0) :
                        setServoPos(180)  #Left
                        if(data == 'h') :  # after modify  (sensor value)
                            GPIO.output(pin1,True)
                            GPIO.output(pin2,False)
                            sleep(2)
                            print("3")
                            Go = 1
                    elif (Go == 1) :
                        setServoPos(90)  #straight
                        CCC = data
                        if(data == 'B') :  # after modify (RFID value)
                            GPIO.output(pin1,True)
                            GPIO.output(pin2,False)
                            sleep(1)
                            GPIO.output(pin1,True)
                            GPIO.output(pin2,True)
                            sleep(0.5)
                            GPIO.output(pin1,True)
                            GPIO.output(pin2,False)
                            sleep(1)
                            GPIO.output(pin1,True)
                            GPIO.output(pin2,True)
                            sleep(0.5)
                            print("4")
                            Go = 0
                            Root_Count = 2
            elif(Root_Count == 2) : 
                    if(Go == 0) :
                        setServoPos(180)  #right
                        if(data == 'b' or data == 'a') :  # after modify  (sensor value)
                            GPIO.output(pin1,True)
                            GPIO.output(pin2,False)
                            sleep(2)
                            print("5")
                            Go = 1
                    elif (Go == 1) :
                        setServoPos(90)  #straight
                        CCC = data
                        if(data == 'C') :  # after modify (RFID value)
                            for i in range(4) : 
                                GPIO.output(pin1,True)
                                GPIO.output(pin2,False)
                                sleep(0.5)
                                GPIO.output(pin1,True)
                                GPIO.output(pin2,True)
                                sleep(0.5)
                        print("6")
                        Go = 0     
        #######################################Select_Num = 2###########################################
        elif Select_Num == 2 : 
            if(Root_Count == 0) :
                    if(Go == 0) :
                        setServoPos(0)  #right
                        if(data == 'B') :  # after modify  (sensor value)
                            GPIO.output(pin1,True)
                            GPIO.output(pin2,False)
                            sleep(2)
                            print("B")
                            Go = 1
                    elif (Go == 1) :
                        setServoPos(90)  #straight
                        CCC = data
                        if(data == 'B') :  # after modify (RFID value)
                            GPIO.output(pin1,True)
                            GPIO.output(pin2,False)
                            sleep(1)
                            GPIO.output(pin1,True)
                            GPIO.output(pin2,True)
                            sleep(0.5)
                            GPIO.output(pin1,True)
                            GPIO.output(pin2,False)
                            sleep(1)
                            GPIO.output(pin1,True)
                            GPIO.output(pin2,True)
                            sleep(0.5)
                            print("B")
                            Go = 0
                            Root_Count = 1;   
            elif Root_Count == 1  :
                    if(Go == 0) :
                        setServoPos(180)  #right
                        if(data == 'B') :  # after modify  (sensor value)
                            GPIO.output(pin1,True)
                            GPIO.output(pin2,False)
                            sleep(2)
                            print("B")
                            Go = 1
                    elif (Go == 1) :
                        setServoPos(90)  #straight
                        CCC = data
                        if(data == 'B') :  # after modify (RFID value)
                            GPIO.output(pin1,True)
                            GPIO.output(pin2,False)
                            sleep(1)
                            GPIO.output(pin1,True)
                            GPIO.output(pin2,True)
                            sleep(0.5)
                            GPIO.output(pin1,True)
                            GPIO.output(pin2,False)
                            sleep(1)
                            GPIO.output(pin1,True)
                            GPIO.output(pin2,True)
                            sleep(0.5)
                            print("B")
                            Go = 0
                            Root_Count = 2
            elif(Root_Count == 2) : 
                    if(Go == 0) :
                        setServoPos(0)  #right
                        if(data == 'B') :  # after modify  (sensor value)
                            GPIO.output(pin1,True)
                            GPIO.output(pin2,False)
                            sleep(2)
                            print("B")
                            Go = 1
                    elif (Go == 1) :
                        setServoPos(90)  #straight
                        CCC = data
                        if(data == 'B') :  # after modify (RFID value)
                            for i in range(4) : 
                                GPIO.output(pin1,True)
                                GPIO.output(pin2,False)
                                sleep(0.5)
                                GPIO.output(pin1,True)
                                GPIO.output(pin2,True)
                                sleep(0.5)
                            print("B")
                            Go = 0
                            
                            
   #############################################################
    if(Back_ON == 1) :
        Keep_Space = CCC
        Ex_Root_Count = Root_Count
        Root_Count = 7 #every root is not work
        Back_ON = 0
        print("777")
    
    #if(Keep_Space != 0) :
    if(Keep_Space == 'a') :
        print("Ex_RootCount = ",Ex_Root_Count)
        if(data == 'e') :
            GPIO.output(pin1,True)
            GPIO.output(pin2,False)
            sleep(0.5)
            GPIO.output(pin1,True)
            GPIO.output(pin2,True)
            sleep(0.5)
            Back = 1
            Keep_Space = 0
            print("8888")
    elif(Keep_Space == 'b') :
        if(data == 'f') :
            GPIO.output(pin1,True)
            GPIO.output(pin2,False)
            sleep(0.5)
            GPIO.output(pin1,True)
            GPIO.output(pin2,True)
            sleep(0.5)
            Back = 1
            Keep_Space = 0
            print("8888")
    elif(Keep_Space == 'c') :
        if(data == 'g') :
            GPIO.output(pin1,True)
            GPIO.output(pin2,False)
            sleep(0.5)
            GPIO.output(pin1,True)
            GPIO.output(pin2,True)
            sleep(0.5)
            Back = 1
            Keep_Space = 0
            print("8888")
    elif(Keep_Space == 'd') :
        print("8888")
        if(data == 'h') :
            GPIO.output(pin1,True)
            GPIO.output(pin2,False)
            sleep(0.5)
            GPIO.output(pin1,True)
            GPIO.output(pin2,True)
            sleep(0.5)
            Back = 1
            Keep_Space = 0
            print("8888")
    elif(Keep_Space == 'e') :
        print("uuuuu")
        if(data == 'a') :
            GPIO.output(pin1,True)
            GPIO.output(pin2,False)
            sleep(0.5)
            GPIO.output(pin1,True)
            GPIO.output(pin2,True)
            sleep(0.5)
            Back = 1
            Keep_Space = 0
            print("8888")
    elif(Keep_Space == 'f') :
        print("uuuuu")
        if(data == 'b') :
            GPIO.output(pin1,True)
            GPIO.output(pin2,False)
            sleep(0.5)
            GPIO.output(pin1,True)
            GPIO.output(pin2,True)
            sleep(0.5)
            Back = 1
            Keep_Space = 0
            print("8888")
    elif(Keep_Space == 'g') :
        if(data == 'c') :
            GPIO.output(pin1,True)
            GPIO.output(pin2,False)
            sleep(0.5)
            GPIO.output(pin1,True)
            GPIO.output(pin2,True)
            sleep(0.5)
            Back = 1
            Keep_Space = 0
            print("8888")
    elif(Keep_Space == 'h') :
        if(data == 'd') :
            GPIO.output(pin1,True)
            GPIO.output(pin2,False)
            sleep(0.5)
            GPIO.output(pin1,True)
            GPIO.output(pin2,True)
            sleep(0.5)
            Back = 1
            Keep_Space = 0
            print("8888")           
    ######################Back################################
    if(Back == 1) :
        Root_Count = Ex_Root_Count  #nothing number
        Ex_Root_Count = 6
        Back = 0
        Count = 0
        print("9999")  
            
        
                



# 서보 PWM 정지
servo.stop()
  # GPIO 모드 초기화
GPIO.cleanup()

print("Finished")
client_socket.close()



if __name__ == '__main__' :
    main()