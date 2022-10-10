FROM ubuntu:22.04

RUN apt-get update && apt-get -y upgrade
RUN apt-get install --assume-yes --no-install-recommends ca-certificates build-essential cmake git
RUN apt-get install --assume-yes --no-install-recommends libboost-regex-dev libsqlite3-dev
RUN apt-get install --assume-yes --no-install-recommends texlive-latex-extra texlive-fonts-recommended texlive-latex-recommended texlive-fonts-extra

# Build sqlplot-tools
RUN git clone https://github.com/bingmann/sqlplot-tools.git /opt/sqlplot-tools
RUN mkdir /opt/sqlplot-tools/build
WORKDIR /opt/sqlplot-tools/build
RUN cmake -DCMAKE_BUILD_TYPE=Release -DWITH_POSTGRESQL=OFF -DWITH_MYSQL=OFF ..
RUN cmake --build . -j 8

# Build SicHash
COPY . /opt/sichash
RUN mkdir /opt/sichash/build
WORKDIR /opt/sichash/build
RUN cmake -DCMAKE_BUILD_TYPE=Release ..
RUN cmake --build . -j 8

# Actual benchmark
CMD bash /opt/dockerVolume/figure-1.sh
