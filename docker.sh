#!/bin/bash

# ejecuta docker y bindea el direactorio actual a root
docker run --security-opt seccomp:unconfined -v ${PWD}:/root -ti agodio/itba-so:1.0
