---
finished_date: 2020-06-22
tags:
  - computer_architecture
  - cache_miss
  - personal_project
  - C
  - cloud_server
  - Makefile
---
# cache_simulator
- cache simulator that shows the number of cache miss and memory write in C.
- 2 types of simulator
  - type 0
    - total data capacity: 16KB
    - 16 bytes per block
    - direct-mapped, write-through, no write allocate policy
    - 32 bit mem addr
  - type 1
    - total data capacity: 64KB
    - 64 bytes per block
    - 2-way set associative, LRU replacement policy, write-back, write allocate policy
    - 32 bit mem addr

## Interface
```
./cache-sim [cache type] [testfile no.]
example
./cache-sim 0 1
```
- test files should be placed in the same directory with source files
## Input file
- test file name: trace[testfile no.].txt
```
[L/S] address in hex
example
...
L 00000048
L 0000004C
S 00000000
S 00000004
...
```
- L: load, S: store
## Result
```
./cache-sim 1 100
2231 233
```
## Filestructure
```
|-- src
  |-- main.c
  |-- Makefile
```
## How to run
1. get main.c, Makefile in the same directory
2. enter **make* to prompt to compile
3. enter **make run [cache type] [testfile no.]** to execute
4. enter **make clean** to erase all generated files
## 배운 점
1. direct mapped, write-through, LRU replacement, 2-way set, write-back에 대해 어떻게 메모리를 관리하게 되는지 알 수 있었다.
2. 어떤 경우에 type0, type1이 좋은 성능을 내는지 확인할 수 있었다.
  - 주로 type 0보다 type1이 miss rate이 작다.
  - memory write 또한 type1이 더 작다.
  - 실행 속도 및 메모리 관리에 있어 LRU replacement, write-back 방식을 사용하는 것이 비교적 효율적이다.

## 한계점
1. main.c 로 source code를 통일했기 때문에 가독성 및 관리의 어려움이 있다. 차후 프로젝트를 진행할 때는 source code를 적절하게 분배하는 것이 필요하다.
2. comment를 작성하지 않았기 때문에 프로젝트 관리 및 차후 이해에 어려움이 있다.
3. 코드를 정돈하지 않고 보관하여 불필요한 부분이 많다(불필요한 주석 등)
4. 디버깅 시 디버그보다 printf를 주로 사용하여 진행하였기 때문에 효율적인 디버깅이 어려웠다.
5. document를 작성하지 않아 프로젝트를 전반적으로 이해하기 어렵다.
