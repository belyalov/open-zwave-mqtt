FROM ubuntu

MAINTAINER "Konstantin Belyalov"

# Compile openzwave-mqtt and it deps, then remove all unnecessary stuff.
# Must be done in single layer to reduce image size.
RUN apt-get -qq update && \
    apt-get install -y gcc g++ libudev-dev libmosquitto-dev git make cmake && \
    git clone https://github.com/OpenZWave/open-zwave.git && \
    cd open-zwave && \
    git checkout V1.5 && \
    make install && \
    cd .. && \
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

# Variables to control the bridge
ENV MQTT_HOST "test.mosquitto.org"
ENV MQTT_PORT "1883"
ENV MQTT_CLIENT_ID "ozw-mqtt"
ENV MQTT_PREFIX ""
ENV MQTT_USER ""
ENV MQTT_PASSWD ""
ENV DEVICE "/dev/zwave"
ENV OZW_SYSTEM_CONFIG "/usr/local/etc/openzwave"
ENV OZW_USER_CONFIG "/config"
ENV LOG_LEVEL "info"
ENV TOPIC_FILTER_FILE ""

# User config (home config data)
VOLUME /config

ENTRYPOINT ["/run_ozw-mqtt"]
