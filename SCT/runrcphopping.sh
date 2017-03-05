#!/bin/sh

mkdir logs

# echo init master...
# ./testing 0 4 > logs/testing_0_4.txt &
# sleep 10
# echo init slaves ...
# ./testing 1 4 > logs/testing_1_4.txt &
# ./testing 2 4 > logs/testing_2_4.txt &
# ./testing 3 4 > logs/testing_3_4.txt &

echo init master...
./testing 0 16 > logs/testing_0_16.txt &
sleep 10
echo init slaves ...
./testing 1 16 > logs/testing_1_16.txt &
./testing 2 16 > logs/testing_2_16.txt &
./testing 3 16 > logs/testing_3_16.txt &
./testing 4 16 > logs/testing_4_16.txt &
./testing 5 16 > logs/testing_5_16.txt &
./testing 6 16 > logs/testing_6_16.txt &
./testing 7 16 > logs/testing_7_16.txt &
./testing 8 16 > logs/testing_8_16.txt &
./testing 9 16 > logs/testing_9_16.txt &
./testing 10 16 > logs/testing_10_16.txt &
./testing 11 16 > logs/testing_11_16.txt &
./testing 12 16 > logs/testing_12_16.txt &
./testing 13 16 > logs/testing_13_16.txt &
./testing 14 16 > logs/testing_14_16.txt &
./testing 15 16 > logs/testing_15_16.txt &