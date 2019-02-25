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

## Discover MQTT topics
Ok, now you know how to run `ozw-mqtt`, and so you may ask? How to I know where ZWave device will report to?
This is where `--print-topics-only` comes into picture, simply run `ozw-mqtt` with this option and you'll get something like this:
```
Print MQTT topics only mode selected, gathering nodes information.
This will take awhile (up to few minutes, depending on amount of nodes)...
2019-02-24 16:03:00.758 Always, OpenZwave Version 1.4.3311 Starting Up

(R/W) home/controller/basic/basic (1/32/0)
(R/W) home/living/window1/sensor/basic/basic (3/32/0)
(R) home/living/window1/sensor/sensor_binary/sensor (3/48/0)
(R) home/living/window1/sensor/alarm/access_control (3/113/9)
(R) home/living/window1/sensor/alarm/burglar (3/113/10)
(R) home/living/window1/sensor/alarm/power_management (3/113/11)
(R/W) home/living/window1/sensor/powerlevel/powerlevel (3/115/0)
(R/W) home/living/window1/sensor/powerlevel/timeout (3/115/1)
.....
```

## Topic Map
By default application will publish **all** topics from your OpenZWave configuration. However, sometimes you may want to filter some topics our / rename them.
That could be done by creating simple text file with mappings, like:
```
# Comments start with "#"

# Master bedroom
home/master/lights/alarm/burglar                    = home/master/motion
home/master/lights/switch_multilevel/1/level        = home/master/lights
home/master/wall_lights/switch_multilevel/1/level   = home/master/wall_lights
home/master/window/sensor/battery/battery_level     = home/master/window/battery
home/master/window/sensor/sensor_binary/sensor      = home/master/window/state

# ^ empty lines are ignored. Spaces between topics are ignored too

# You can just specify topic, without mapping, like:
home/living/window1/sensor/battery/battery_level

```
Then simply run:
```bash
$ ./ozw-mqtt --topic-filter-file mytopiclist.txt
```
... And you'll see only topics defined in map! :-)

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
* `--topic-filter-file` - Specifies file contains topic map/filter.
* `--print-topics-only` - Run OpenZwave, discover all nodes, then print all MQTT topics and exit.

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
