#!/bin/bash

set -e

BASE_OUT_DIR=../../../bazel-out/k8-opt/bin

bazel build --compilation_mode opt :speedtest1-sapi
rsync -L "$BASE_OUT_DIR"/_solib_k8/lib* "$BASE_OUT_DIR"/sandboxed_api/examples/speedtest1/speedtest1-sapi Xeon-gold-0:/local/msammler/sandboxed-api
ssh Xeon-gold-0 "GLOG_logtostderr=1 LD_LIBRARY_PATH=/local/msammler/sandboxed-api/ /local/msammler/sandboxed-api/speedtest1-sapi :memory:"
