#!/bin/bash
sudo rm /mnt/persist-memory/pmem_yjb_1/test.pool
sudo ./btree -n 1000 -w 0 -p /mnt/persist-memory/pmem_yjb_1/test.pool -i ../sample_input.txt
