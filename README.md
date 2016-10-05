# RobotCar

Control a raspberry pi using a controller over a network.

# Dependencies
- cmake 
- opencv3
- postgresql
- wiringPi
- UnitTest++ (build/test)

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

Create a carrc file at /etc/carrc
```
ServerAddr = HOST_NAME
DbAddr     = DB_HOST_NAME
DbPort     = 5432
DbUser     = DB_USER_NAME
DbName     = DATABASE_NAME
```

# Running
Start up the local server with `build/gcontrol` then start up
the car client with `./car --train=false` to just control the car
