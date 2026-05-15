FROM devkitpro/devkitppc:latest

ENV DEVKITPRO=/opt/devkitpro
ENV DEVKITPPC=${DEVKITPRO}/devkitPPC
ENV LIBOGC=${DEVKITPRO}/libogc
ENV PATH=${DEVKITPPC}/bin:${DEVKITPRO}/tools/bin:${PATH}

RUN apt-get update \
    && apt-get install -y --no-install-recommends xorriso \
    && rm -rf /var/lib/apt/lists/* \
    && dkp-pacman -Syu --noconfirm \
    && dkp-pacman -S --noconfirm gamecube-dev

WORKDIR /workspace
CMD ["/bin/bash"]
