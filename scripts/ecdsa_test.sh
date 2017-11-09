#!/bin/bash

sudo output/ed25519_test.o | tee data/ecdsa_test/ecdsa_test_$(date +"%Y%m%d%H%M").csv
