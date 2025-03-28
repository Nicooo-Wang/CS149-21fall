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
    Instruction4 - satisfies -> Instruction2
    Instruction4 - satisfies -> Instruction3
    Instruction5 - satisfies -> Instruction2
    Instruction5 - satisfies -> Instruction3
    Instruction6 - satisfies -> Instruction4
    Instruction6 - satisfies -> Instruction5
    Instruction7 - satisfies -> Instruction4
    Instruction7 - satisfies -> Instruction5
    Instruction8 - satisfies -> Instruction6
    Instruction8 - satisfies -> Instruction7
    Instruction9 - satisfies -> Instruction6
    Instruction9 - satisfies -> Instruction7
    Instruction10 - satisfies -> Instruction8
    Instruction10 - satisfies -> Instruction9
    Instruction11 - satisfies -> Instruction10

```

### B. steay-state utilization of the processor
10 / 70 = 14.28%

### C. total hardware threads are needed
70 / 10 = 7 threads

### D. speedup of for-way superscalar proceessor
```mermaid
gantt
    title CPU 乱序执行 Gantt 图
    dateFormat  DDD
    axisFormat  %j
    section Thread 1
    LOAD         :l1, 2000-01-01, 60d
    EXEC         :after l1  , 10d
    section Thread 2
    LOAD         :l2, 2000-01-11, 60d
    EXEC         :after l2  , 10d
    section Thread 3
    LOAD         :l3, 2000-01-21, 60d
    EXEC         :after l3  , 10d
    section Thread 4
    LOAD         :l4, 2000-01-31, 60d
    EXEC         :after l4  , 10d
```
（70 + 60 + 50 + 40 ）/ 70 = 3.1428571429
## Problem 2: Picking the Right CPU for the Job