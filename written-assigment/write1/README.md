[toc]

# [Write Assignment 1](https://gfxcourses.stanford.edu/cs149/fall21content/static/pdfs/written_asst1.pdf)

## Problem 1: Superscalar and Hardware Multi-Threading

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

## Problem 2: Picking the Right CPU for the Job