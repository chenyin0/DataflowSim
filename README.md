# DFSim

A modularized dataflow simulator for agile design space exploration in micro-architecture level



## Overview

***DFSim*** is a high level ***micro-architecture design space explore(DSE) platform***. This platform aims to help the designer estimate the micro-architecture before implementing the hardware details. In this way, it cuts off the costly and complicated development iteration between the architecture design and RTL implement. 

To accomplish this goal, this framework abstracts the architecture properties into several primitives (in [section 2](# Supported hardware ISA)) from three design spaces: *execution*, *memory access* and *interconnection*, which are the fundamental properties of a spatial architecture. In a sense, these primitives work as ***Hardware ISA*** (or called ***Hardware IR***), by which almost all of the data-driven architectures can be described in a abstraction level with performance equivalence. With annotating parameters of area and power on each behavior primitive like [DSAGEN](https://github.com/PolyArch/dsa-framework)<a href="#ref-dsagen">[1]</a>, a rough hardware overhead is also assessable. However, as the array size increasing (e.g. from 1k to 10k processor units in our project), the latency and energy consumption of the interconnect network become vital. So another framework [DFMapper](https://github.com/chenyin0/DFMapper) was developed to generate placing and routing schemes for a diversity of customizable network topologies, by which we can get the real transmission distance between any two processing units and evaluate the data communication overhead in a precise way.



## Supported hardware ISA 

### 1. Execution

|      Hardware ISA      | μ-Architecture Design | Representative Hardware |
| :--------------------: | :-------------------: | :---------------------: |
|   Dataflow pipeline    |    Data buffering     |       TPU & CGRA        |
| Data-level concurrency |    Multi-lane ALU     |   Plasticine@ISCA'17    |
|    Thread-level O3     |    Token matching     |      SGMF@ISCA'16       |
|  Instruction-level O3  | Instruction scheduler |       TIA@ISCA'13       |
| Asynchronous execution | Cross-clock handshake |  Elastic CGRA@HPCA'21   |
|          ...           |          ...          |           ...           |

### 2. Memory

|          Hardware ISA           |   μ-Architecture Design   | Representative Hardware |
| :-----------------------------: | :-----------------------: | :---------------------: |
|      Prediction (locality)      | Tag Comparison (Miss/Hit) |      Stash@ISCA'15      |
|            Hierarchy            |     Replace strategy      |      Jenga@ISCA'17      |
|           Prefetching           |    Localize buffering     |    Tao Chen@MICRO'16    |
|         Ping-Pong load          |         N-buffer          |   Plasticine@ISCA'17    |
|           Data reuse            |   Update/Shrink control   |   Patch Mem@MICRO'16    |
| Decouple Access/Execution (DAE) |    A/E synchronization    |    Buffet@ASPLOS'19     |
|               ...               |            ...            |           ...           |

### 3. Interconnection

|  Hardware ISA   |  μ-Architecture Design  |   Representative Hardware   |
| :-------------: | :---------------------: | :-------------------------: |
|    Bus-based    | Bus controller/arbiter  |    Multi-core processor     |
| Static routing  |       Relay node        |        most of CGRAs        |
| Dynamic routing |       Switch box        | HyCube@DAC'17/DySer@HPCA'11 |
|     Mixture     | Relay node + switch box |     Yaqi Zhang@ISCA'19      |
|       NoC       |  Packet/deadlock-free   |        Sigma@HPCA'20        |
|       ...       |           ...           |             ...             |



## Target workloads

We focus on the benchmark suits with sufficient parallelism, which can be unrolled in spatial for speedup (such as MachSuite<a href="#ref-machsuite">[2]</a>, Rodinia<a href="#ref-rodinia">[3]</a>and PolyBench<a href="#ref-polybench">[4]</a>).



## Software stacks

1. Use Clang to generate the target benchmark's LLVM IR, and generate the corresponding dataflow graph (DFG) and control flow graph (CFG) by the LLVM built-in pass.
2. Separate the original dataflow graph (DFG) according to control regions to get multiple independent sub-DFGs (Details are in this paper<a href="#ref-subgraph">[5]</a>).
3. Define the network topology and mapping the intact DFG or sub-DFGs under this topology to get the interconnection information (including the hop times, path-balance buffer size, the minimum array size that can be routed etc. ) by [DFMapper](https://github.com/chenyin0/DFMapper).
4. Use the hardware ISAs provided in [DFSim](https://github.com/chenyin0/DataflowSim) (or define customized ones) to build the micro-architecture of your own accelerators and evaluate the PPA (perf, power and area).
5. Change different hardware ISAs of the execution mode, memory behavior and interconnect topology to help designers realize a design space exploration. This is an agile development framework in the micro-architecture level.



## Build usages

TBD



## References

<span name="ref-dsagen">[1] Weng, Jian, et al. "Dsagen: Synthesizing programmable spatial accelerators." *2020 ACM/IEEE 47th Annual International Symposium on Computer Architecture (ISCA)*. IEEE, 2020.</span>

<span name="ref-machsuite">[2] Reagen, Brandon, et al. "Machsuite: Benchmarks for accelerator design and customized architectures." *2014 IEEE International Symposium on Workload Characterization (IISWC)*. IEEE, 2014.</span>

<span name="ref-rodinia">[3] Che, Shuai, et al. "Rodinia: A benchmark suite for heterogeneous computing." *2009 IEEE international symposium on workload characterization (IISWC)*. IEEE, 2009.</span>

<span name="ref-polybench">[4] Pouchet, Louis-Noël. "Polybench: The polyhedral benchmark suite." *URL: http://www.cs.ucla.edu/pouchet/software/polybench* 437 (2012).</span>

<span name="ref-subgraph">[5] Chen Yin, et al. "Subgraph Decoupling and Rescheduling for Increased Utilization in CGRA Architecture", DATE 2021</span>.

