# open-zwave-mqtt [![Build Status](https://travis-ci.org/belyalov/open-zwave-mqtt.svg?branch=master)](https://travis-ci.org/belyalov/open-zwave-mqtt)
A bridge between ZWave and MQTT networks.

A lot of work needs to be done, so your contributions are warmly welcomed! :)

## Quick start
If you're running linux the simplest way to start by using **docker** container:
```
docker run --rm -ti --device=<your_zwave_device> -e "DEVICE=<your_zwave_device>" arsenicus/open-zwave-mqtt
```

Some explanations:
* `--rm` - auto delete container after it finished.
* `-ti` - **t** - terminal, **i** - interactive, i.e. run container in interactive terminal mode.
* `--device` - Allow container to use specified device
* `-e`  - Define environment variable

After run you'll see something like:
```
$ docker run --rm -ti -e --device=/dev/zzz "DEVICE=/dev/zzz" arsenicus/open-zwave-mqtt
+ /ozw-mqtt --device /dev/zzz --config /config --system-config /usr/local/etc/openzwave --mqtt-host test.mosquitto.org --mqtt-port 1883 --mqtt-client-id ozw-mqtt --mqtt-prefix '' --mqtt-user '' --mqtt-passwd '' --log-level info
Starting OpenZWave to MQTT bridge.
Connecting to MQTT Broker test.mosquitto.org:1883...
2018-01-21 20:58:21.938 Always, OpenZwave Version 1.4.0 Starting Up
2018-01-21 20:58:21.939 Info, Setting Up Provided Network Key for Secure Communications
2018-01-21 20:58:21.939 Warning, Failed - Network Key Not Set
2018-01-21 20:58:21.939 Info, mgr,     Added driver for controller /dev/zzz
2018-01-21 20:58:21.939 Info,   Opening controller /dev/zzz
2018-01-21 20:58:21.939 Info, Trying to open serial port /dev/zzz (attempt 1)
```

## Build
### Dependencies
* CMake 2.8+
* [OpenZwave library 1.5+](https://github.com/OpenZWave/open-zwave)
* [Mosquitto (MQTT) library](https://github.com/eclipse/mosquitto)
* [Gtest](https://github.com/google/googletest) - [optional] to be able to run tests
* GCC 4.8+ or CLang 3.8+

### Compile
Clone repository:
```
git clone https://github.com/belyalov/open-zwave-mqtt.git
```

Create Makefiles / compile:
```
cd open-zwave-mqtt
mkdir build
cd build
cmake ../
make
```

Run test to be sure that it works on your setup / platform:
```
./tests
```

## Run
```
./ozw-mqtt -d <ZWave device> -h <MQTT HOST>
```
E.g.:
```
./ozw-mqtt -d /dev/zwave -h 192.168.1.1
```

Now all your ZWave messages are replicated into MQTT network, e.g. let's say you have motion sensor named `motion` located at `living_room`, i.e. openzwave config entry may looks like:
```
<Node id="4" name="motion" location="living" .... >
...
    <CommandClasses>
        ...
        <CommandClass id="48" name="COMMAND_CLASS_SENSOR_BINARY"...>
            <Instance index="1" />
                <Value type="bool" label="Sensor" ... />
        ...
```
Whenever sensor detects movement a MQTT message will be sent to topic `living/motion/sensor_binary/sensor` with `True` or `False` string payload.

## Options
#### Mandatory parameters
* `-d [--device]` - ZWave Device location (defaults to `/dev/zwave`)
* `-h [--mqtt-host]` - Address of MQTT Broker. (defaults to `localhost`)

#### Optional parameters
* `-c [--config]` - *User* configuration file for OpenZWave library (defaults to current directory - `./`)
* `-u [--mqtt-user]` - MQTT Username (defaults to empty)
* `-p [--mqtt-passwd]` - MQTT Password (defaults to empty)
* `--mqtt-port` - MQTT Broker port (defaults to `1883`)
* `--mqtt-client-id` - Set MQTT Client-ID (defaults to `ozw-mqtt`)
* `--mqtt-prefix` - Add prefix to all ZWave subscriptions / publications topics. E.g. `living/motion/sensor_binary/sensor` will be prefixed to `<prefix>/living/motion/sensor_binary/sensor`. Defaults to no prefix.
* `--system-config` - OpenZWave library system configuration dir (defaults to `/usr/local/etc/openzwave`)
* `--log-level` - Set OpenZWave library log level (can be `error`, `warning`, `info`, `debug`). Defaults to `info`.
* `--mqtt-no-name-topics` - Disables subscribe / publish to name-based topics (like `home/room/light`)
* `--mqtt-no-id-topics` - Disables subscribe / publish on id-based topics (like `1/2/33`).
