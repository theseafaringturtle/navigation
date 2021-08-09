PROGRAM="./bin/LSM9DS1_Sensor"
RESTART_FILE="/home/pi/restart_imu"

while [ -e $RESTART_FILE ] 
do
echo "Starting $PROGRAM"
$PROGRAM
echo "Finished with status $?"
sleep 2
done
