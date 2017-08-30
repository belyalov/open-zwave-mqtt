ARG BASE=ubuntu:latest
FROM ${BASE}

LABEL maintainer="Konstantin Belyalov"

# Compile openzwave-mqtt and it deps, then remove all unnecessary stuff.
# Must be done in single layer to reduce image size
RUN apt-get -qq update && \
    apt-get install -y gcc g++ libudev-dev libmosquitto-dev git make cmake && \
    git clone https://github.com/OpenZWave/open-zwave.git && \
    make -C open-zwave install && \
    rm -rf open-zwave && \
    git clone https://github.com/belyalov/open-zwave-mqtt.git && \
    cd open-zwave-mqtt && \
    cmake . && \
    make && \
    cp ./ozw-mqtt / && \
    cd .. && \
    apt-get purge -y gcc g++ libudev-dev git make cmake && \
    apt-get autoremove -y --purge && \
    rm -rf /var/lib/apt/lists/ /open-zwave /open-zwave-mqtt

# WORKDIR /distr

# Compile/Install OpenZWave
# RUN git clone https://github.com/OpenZWave/open-zwave.git
# RUN make -C open-zwave install

# Compile OpenZWave-MQTT
# RUN git clone https://github.com/belyalov/open-zwave-mqtt.git
# WORKDIR open-zwave-mqtt
# RUN cmake .
# RUN make
# RUN cp ozw-mqtt /

# Cleanup
# WORKDIR /
# RUN apt-get purge -y gcc g++ libudev-dev git make cmake
# RUN apt-get autoremove -y --purge
# RUN rm -rf /distr
# RUN rm -rf /var/lib/apt/lists/*

ENTRYPOINT ["/bin/bash"]
