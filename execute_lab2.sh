#!/usr/bin/env bash

# Please write you index name here
INDEX=btree
#INDEX=stl_map

if [ ! -d "./data" ]; then
    echo "ln -s ../lab1_lab2_data/ data"
    ln -s ../lab1_lab2_data/ data
fi

#Please write env directory here
ENV_PATH=../env/lib
MKL_PATH=../env/intel

if [ ! -d "./lib" ]; then
    mkdir -p lib
fi

CPLUS_INCLUDE_PATH=$ENV_PATH/lib/include:$:$CPLUS_INCLUDE_PATH
LD_LIBRARY_PATH=$ENV_PATH/lib/lib:$LD_LIBRARY_PATH
LIBRARY_PATH=$ENV_PATH/lib/lib:$LIBRARY_PATH
PATH=$ENV_PATH/lib:$ENV_PATH/lib/bin:$ENV_PATH/lib/include:$ENV_PATH/lib/lib:$PATH
C_INCLUDE_PATH=$ENV_PATH/lib/include:$C_INCLUDE_PATH
CPATH=$ENV_PATH/lib/include:$CPATH

echo "#!/bin/sh"
echo "echo Executing benchmark and saving results..."

echo "bash run.sh build_release"
bash run.sh build_release
#bash run.sh run

BENCHMARK=build/microbench
if [ ! -f $BENCHMARK ]; then
    echo "benchmark binary does not exist"
    exit
fi
output_file=./results/lab2.csv
cnt=0

function execute_uint64_200M() {
    echo "Executing operations for $1 and index $2, op_cnt $3"
    op_cnt=$3
    case $op_cnt in 1)
    $BENCHMARK --keys_file=./data/$1 --keys_file_type=binary --table_size=-1 --thread_num=1 --output_path=$output_file --index=$2 --operations_num=100000000 --memory --init_table_ratio=1.0 --read=0.0 --scan=0.01 --lower_bound=0.99
    ;;
    2)
    $BENCHMARK --keys_file=./data/$1 --keys_file_type=binary --table_size=-1 --thread_num=1 --output_path=$output_file --index=$2 --operations_num=20000000 --memory --init_table_ratio=1.0 --read=0.0 --scan=1.0
    ;;    
    3)
    $BENCHMARK --keys_file=./data/$1 --keys_file_type=binary --table_size=-1 --thread_num=1 --output_path=$output_file --index=$2 --operations_num=500000000 --memory --init_table_ratio=0.5 --read=0.76 --insert=0.2 --scan=0.04
    ;;
    4)
    $BENCHMARK --keys_file=./data/$1 --keys_file_type=binary --table_size=-1 --thread_num=1 --output_path=$output_file --index=$2 --operations_num=200000000 --memory --init_table_ratio=0.5 --read=0.475 --insert=0.5 --scan=0.025
    ;;
    5)
    $BENCHMARK --keys_file=./data/$1 --keys_file_type=binary --table_size=-1 --thread_num=1 --output_path=$output_file --index=$2 --operations_num=125000000 --memory --init_table_ratio=0.5 --read=0.19 --insert=0.8 --scan=0.01
    ;;
    6)
    $BENCHMARK --keys_file=./data/$1 --keys_file_type=binary --table_size=-1 --thread_num=1 --output_path=$output_file --index=$2 --operations_num=100000000 --memory --init_table_ratio=0.5 --read=0.0 --insert=0.99 --scan=0.01
    ;;
    7)
    $BENCHMARK --keys_file=./data/$1 --keys_file_type=binary --table_size=200000 --thread_num=1 --output_path=$output_file --index=$2 --operations_num=200000 --memory --init_table_ratio=0.5 --read=0.375 --insert=0.5 --scan=0.025 --lower_bound=0.1 --verify
    ;;
    8)
    $BENCHMARK --keys_file=./data/$1 --keys_file_type=binary --table_size=20000 --thread_num=1 --output_path=$output_file --index=$2 --operations_num=20000 --memory --init_table_ratio=0.5 --read=0.0 --insert=0.0 --scan=0.0 --lower_bound=1.0
    esac
}

#for DATA in fb
for DATA in books fb osm_cellids planet genome
do
    for i in {1..6};
    #for i in 7;
    do
        execute_uint64_200M ${DATA}_200M_uint64 $INDEX $i
    done
done
