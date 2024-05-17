# turret_motor_controller
Arduino code for controlling 2 steper-motors and a laser.

# COMMUNICATION
Arduino parses serial data.

Command format is [command byte] [data] [stop byte].

stop byte is '\n'

current commands

x [long] \n to change x coordinate target

y [long] \n to change y coordinate target

X c\n to set current x as zero

Y c\n to set current y as zero

q \n querry current position

a [float]\n to modify movement roughness, the inverse of smoothing

b [float]\n to modify maximum move speed

l [byte]\n to set laser brightness

example:
y 1000 \n
x 10000 \n
a 0.012 \n"
