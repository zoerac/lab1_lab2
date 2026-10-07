#!/usr/bin/env bash

# Please write you index name here
INDEX=$1

if [ ! -d "data" ]; then
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

echo $PATH

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
output_file=./results/real_lab1.csv
cnt=0

function execute_uint64_200M() {
    echo "Executing operations for $1 and index $2, op_cnt $3"
    op_cnt=$3
    case $op_cnt in 1)
    $BENCHMARK --keys_file=./data/$1 --keys_file_type=binary --table_size=-1 --thread_num=1 --output_path=$output_file --index=$2 --operations_num=100000000 --init_table_ratio=1.0 --read=1.0 --memory
    ;;    
    2)
    $BENCHMARK --keys_file=./data/$1 --keys_file_type=binary --table_size=-1 --thread_num=1 --output_path=$output_file --index=$2 --operations_num=500000000 --init_table_ratio=0.5 --read=0.8 --insert=0.2 --memory
    ;;
    3)
    $BENCHMARK --keys_file=./data/$1 --keys_file_type=binary --table_size=-1 --thread_num=1 --output_path=$output_file --index=$2 --operations_num=200000000 --init_table_ratio=0.5 --read=0.5 --insert=0.5 --memory
    ;;
    4)
    $BENCHMARK --keys_file=./data/$1 --keys_file_type=binary --table_size=-1 --thread_num=1 --output_path=$output_file --index=$2 --operations_num=125000000 --init_table_ratio=0.5 --read=0.2 --insert=0.8 --memory
    ;;
    5)
    $BENCHMARK --keys_file=./data/$1 --keys_file_type=binary --table_size=-1 --thread_num=1 --output_path=$output_file --index=$2 --operations_num=100000000 --init_table_ratio=0.5 --read=0.01 --insert=0.99 --memory
    ;;
    6)
    timeout 60s $BENCHMARK --keys_file=./data/$1 --keys_file_type=binary --table_size=1000000 --thread_num=1 --output_path=$output_file --index=$2 --operations_num=2000000 --init_table_ratio=0.5 --read=0.5 --insert=0.5 --verify
    #$BENCHMARK --keys_file=./data/$1 --keys_file_type=binary --table_size=10000 --thread_num=1 --output_path=$output_file --index=$2 --operations_num=10000 --init_table_ratio=0.5 --read=0.5 --insert=0.5 --verify
    esac
}

#for DATA in books
for DATA in books fb osm_cellids planet genome
do
    for i in {1..6};
    #for i in 7;
    do
	for j in {1..3}; 
	do
            execute_uint64_200M ${DATA}_200M_uint64 $INDEX $i
	done
    done
done
