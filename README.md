# RobotCar

Control a raspberry pi using a controller over a network.

# Dependencies
- cmake 
- opencv3
- postgresql
- wiringPi

# Build
Standard cmake
```
mkdir build
cd build
cmake ..
make
```

# Setup

Initialize the postgresql database
```
psql DATABASE_NAME -h HOST_NAME USERNAME < db/db.sql
```
