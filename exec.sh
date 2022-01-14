#! /bin/bash

cd server
make clean
make server

cd ../client
make clean
make client

#clear
