FROM ubuntu:16.04
RUN apt-get -y update
RUN apt-get -y upgrade
RUN apt-get install -y \
            software-properties-common\
            gcc-5-base\
            libgcc-5-dev\
            g++\
            cmake\
            git\
            curl
RUN add-apt-repository -y ppa:ubuntugis/ubuntugis-unstable
RUN apt-get -y update
RUN apt-get -y upgrade
RUN apt-get install -y \
            libboost-filesystem-dev\
            libboost-system-dev\
            lib3ds-dev\
            libcgal-dev\
            libcgal-qt5-dev\
            libgdal-dev\
            libopencv-dev
WORKDIR /home
RUN mkdir 3rParty
WORKDIR 3rParty
RUN curl -O http://imagine.enpc.fr/~monasse/Imagine++/downloads/Imagine++-4.3.1-Linux-x86_64.deb
RUN dpkg -i Imagine++-4.3.1-Linux-x86_64.deb
RUN echo "Imagine_DIR DEFAULT=/usr/share/Imagine++" >> $HOME/.pam_environment
RUN source $HOME/.pam_environment
WORKDIR /home
RUN git clone https://github.com/Ethiy/3DSceneModel.git
WORKDIR 3DSceneModel/
RUN git checkout build-system-trial
RUN mkdir build && mkdir build/linux
WORKDIR build/xenial
RUN cmake -DCGAL_DONT_OVERRIDE_CMAKE_FLAGS=ON ../..
RUN make -j4 all
RUN ./tests
RUN rm *.off *.3ds *.gml *.xsd *.geotiff
