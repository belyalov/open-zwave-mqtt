# open-zwave-mqtt
A bridge between ZWave and MQTT networks.
This is a first, initial implementation. A lot of work needs to be done, however main functionality works well.
## Prerequisites
* CMake 2.8+
* [OpenZwave library 1.5+](https://github.com/OpenZWave/open-zwave)
* [Mosquitton (MQTT) library](https://github.com/eclipse/mosquitto)
* GCC 4.8+ or CLang
## Quick start
### Install dependecies
Before compile ozw-mqtt please be sure to install libraries mentioned before.
### Compile ozw-mqtt

Get it!
```
git clone https://github.com/belyalov/open-zwave-mqtt.git
```

Then compile it:
```
cd open-zwave-mqtt
mkdir build
cd build
cmake ../
make
````

And eventuall run it:
```
./ozw-mqtt -d /dev/zwave -h 192.168.1.1
```

## More documentation coming soon!
