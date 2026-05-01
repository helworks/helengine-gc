# syntax=docker/dockerfile:1

FROM devkitpro/devkitppc:latest

RUN dkp-pacman -Syu --noconfirm \
    && dkp-pacman -S --noconfirm gamecube-dev

WORKDIR /workspace
CMD ["/bin/bash"]
