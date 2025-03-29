[toc]

# [Write Assignment 1](https://gfxcourses.stanford.edu/cs149/fall21content/static/pdfs/written_asst1.pdf)

## Problem 1: Superscalar and Hardware Multi-Threading
### A. Instruction Dependency

 1. LD R1 <- [R0]
 2. MUL R2 <- R1, R1
 3. ADD R3 <- R1, R1
 4. MUL R4 <- R2, R3
 5. ADD R5 <- R2, R3
 6. MUL R2 <- R4, R5
 7. ADD R3 <- R4, R5
 8. MUL R4 <- R2, R3
 9. ADD R5 <- R2, R3
 10. ADD R1 <- R4, R5
 11. ST [R0] <- R1  

 Please draw the dependency graph for the instruction sequence.

```mermaid

    requirementDiagram

    element Instruction1 {
        type: simulation
    }

    element Instruction2 {
        type: simulation
    }

    element Instruction3 {
        type: simulation
    }

    element Instruction4 {
        type: simulation
    }

    element Instruction5 {
        type: simulation
    }

    element Instruction6 {
        type: simulation
    }

    element Instruction7 {
        type: simulation
    }

    element Instruction8 {
        type: simulation
    }

    element Instruction9 {
        type: simulation
    }

    element Instruction10 {
        type: simulation
    }

    element Instruction11 {
        type: simulation
    }

    Instruction1 - satisfies -> Instruction2
    Instruction1 - satisfies -> Instruction3
    Instruction2 - satisfies -> Instruction4
    Instruction3 - satisfies -> Instruction4
    Instruction2 - satisfies -> Instruction5
    Instruction3 - satisfies -> Instruction5
    Instruction4 - satisfies -> Instruction6
    Instruction5 - satisfies -> Instruction6
    Instruction4 - satisfies -> Instruction7
    Instruction5 - satisfies -> Instruction7
    Instruction6 - satisfies -> Instruction8
    Instruction7 - satisfies -> Instruction8
    Instruction6 - satisfies -> Instruction9
    Instruction7 - satisfies -> Instruction9
    Instruction8 - satisfies -> Instruction10
    Instruction9 - satisfies -> Instruction10
    Instruction10 - satisfies -> Instruction11

```

### B. steay-state utilization of the processor
10 / 70 = 14.28%

### C. total hardware threads are needed
70 / 10 = 7 threads

### D. speedup of for-way superscalar proceessor
70 / 66

### E. minimum number of hardware threads needed
考虑极限情况，两个thread分别在0, 1时序发射，在arithmetic calculation时会占满4个superscaler
因此答案为2

### F. think ur friend's suggestion
在数据局部性好的场景下是合理的（16MB缓存可显著降低延迟，避免多线程开销）。
但在数据访问随机或高并发场景下，单纯依赖缓存可能不够，仍需多线程辅助隐藏延迟。
最佳方案是结合缓存优化和多线程，根据实际应用特点权衡设计。

## Problem 2: Picking the Right CPU for the Job