FROM herocc/moos-ivp:19.8
LABEL maintainer = Michael Misha Novitzky <novitzky@mit.edu>

ENV MOOS="moos-ivp-aquaticus"

USER root
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y libncurses-dev sudo python2.7-dev python-pip python-tk && apt-get clean
RUN apt-get install -y vim emacs-nox 
RUN usermod -aG sudo moos
USER moos

RUN svn co "https://oceanai.mit.edu/svn/${MOOS}-aro/trunk" "${HOME}/${MOOS}"

RUN cd "${HOME}/${MOOS}" && ./build.sh

USER root
RUN echo "moos:moos" | chpasswd
USER moos

ENV PATH="/home/moos/${MOOS}/bin:${PATH}"
ENV IVP_BEHAVIOR_DIRS="${IVP_BEHAVIOR_DIRS}:/home/moos/${MOOS}/lib"

RUN pip2 install --no-cache-dir numpy matplotlib tensorflow keras

ENV PLEARN="moos-ivp-pLearn"


ENV PATH="/home/moos/${PLEARN}/bin:${PATH}"
ENV IVP_BEHAVIOR_DIRS="/home/moos/${PLEARN}/lib:${IVP_BEHAVIOR_DIRS}"
ENV PYTHONPATH="${PYTHONPATH}:/home/moos/${PLEARN}/pLearn/learning_code:/home/moos/${PLEARN}/src/lib_python"

#RUN git clone https://github.com/mnovitzky/moos-ivp-pLearn.git
COPY . moos-ivp-pLearn/

RUN cd ${PLEARN} && ./build.sh