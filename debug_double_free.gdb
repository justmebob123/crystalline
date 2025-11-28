set pagination off
set logging file gdb_output.txt
set logging on
catch syscall exit_group
run data/smoke_test/ --threads 2 --epochs 1 --batch-size 4 --seq-len 16 --vocab-size 1000
bt
quit
