FROM ubuntu

MAINTAINER "Konstantin Belyalov"

# Compile openzwave-mqtt and it deps, then remove all unnecessary stuff.
# Must be done in single layer to reduce image size.
RUN apt-get -qq update && \
    apt-get install -y gcc g++ libudev-dev libmosquitto-dev git make cmake && \
    git clone https://github.com/OpenZWave/open-zwave.git && \
    make -C open-zwave install && \
    git clone https://github.com/belyalov/open-zwave-mqtt.git && \
    cd open-zwave-mqtt && \
    cmake . && \
    make && \
    cp ./ozw-mqtt / && \
    apt-get purge -y gcc g++ libudev-dev git make cmake && \
    apt-get autoremove -y --purge && \
    rm -rf /var/lib/apt/lists /open-zwave /open-zwave-mqtt

# Copy run script
COPY run_ozw-mqtt /

ENV MQTT_HOST "test.mosquitto.org"
ENV MQTT_PORT "1883"
ENV MQTT_CLIENT_ID "ozw-mqtt"
ENV MQTT_PREFIX "test"
ENV MQTT_USER ""
ENV MQTT_PASSWD ""
ENV DEBUG ""

# port ??

ENTRYPOINT ["/run_ozw-mqtt"]
