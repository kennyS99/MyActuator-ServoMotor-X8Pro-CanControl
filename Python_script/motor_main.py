from motor import *  # 导入串口包
import serial

import time  # 导入时间包 



def main():
    motor1 = Motor_control(1)
    motor1.angle_control(360,0)
    while True:
        # line = ser.readline()
        # line = ser.read(ser.in_waiting)
        # print("now-->",line)
        # print(uart_get(line))
        uart_get()
        time.sleep(0.5)

    

if __name__ == '__main__':
    main()

