# Notes for SEC related command in [berkeley-abc](/engine/abc/)

--- 
## Commands in ABC
#### 1. `scleanup`
- usage: structural cleanup by **removing nodes and latches that do not feed into POs**
- merge registers with identical drivers, and replace stuck-at registers by constants
- only works for sequential strashed ntk
- core functions: in `Abc_Ntk_t * Abc_NtkDarLatchSweep()`, do `Abc_NtkFromDarSeqSweep()`
#### 2. `ssweep`[^1]
- usage: performs sequential sweep using K-step induction
- detecting and merging sequencially equivalent nodes, only works for sequential strashed ntk
- core functions: in `Abc_Ntk_t * Abc_NtkDarSeqSweep()`, do `Fra_FraigInduction()`. Then if there redundant latches, do `Abc_NtkFromDarSeqSweep` (same as the one in `scleanup`)
#### 3. `scorr`
- usage: performs sequential sweep using K-step induction
- detecting and merging sequencially equivalent nodes, only works for sequential strashed ntk
- core functions: in `Abc_Ntk_t * Abc_NtkDarSeqSweep2()`, do `Ssw_SignalCorrespondence()`
- compared with `sweep`: 
    - more detailed options
    - inductive prover with constraints
    - newer, and used by _yosys_
#### 4. `dsec` 
- usage: performs inductive sequential equivalence checking
- it includes `scl` and `scorr`.
- core functions: in `int Abc_NtkDarSec()`, build `Abc_NtkMiter()` and then do `Fra_FraigSec()`
#### 5. `dprove`
- usage: performs SEC on a sequential miter
- more options to select, like bmc, cec before `Fra_FraigSec()`
---
## Commands in ABC9
#### 1. `&scl`
- usage: performs structural sequntial cleanup
- core functions: in `Gia_Man_t * Gia_ManSeqStructSweep()`, `Gia_ManSeqCleanup()`
#### 2. `&scorr`
- usage: performs signal correspondence computation
- core functions: `Gia_Man_t * Cec_ManLSCorrespondence()`, 
#### 3. `&iso` [^2]
- usage: removes POs with isomorphic sequential COI
- core functions: `Gia_ManIsoReduce`

---
## Between old ABC and ABC9
 The ABC9 packeage with a new AIG manager called "GIA" is the new version of ABC. 
 ABC9 allows technology dependent mapping using custom propertieis for each type of LUT, and introduces the possibility to use XOR or MUX in addition to standard AIG nodes. [^3]
1. `&put`
    - usage: **transfer the current network to old ABC**
    - core functions: `Aig_Man_t * Gia_ManToAig()`
2. `&get`
    - usage: **converts the current network into GIA** and moves it to the &-space (if the network is a sequential logic network, normalizes the flops to have const-0 initial values, equivalent to `undc; strash; zero`)
    - core functions: `Gia_Man_t * Gia_ManFromAig()`
---
## Seqential AIG [^4]
Sequential AIGs extend combinational AIGs with dechnology-independent D-flip-flops with one input and one output, controlled by the same clock, omitted in the AIG representations. 
**Flip-flops represented in the AIG explicitly as additional PI/POs pairs.** The PIs and register outputs are called combinational inputs (CIs) and the POs and register inputs are called combinational outputs (COs). The additional pairs of CI/CO nodes follow the regular PIs/POs, and are in one to one correspondence with each other.
The chosen representation of sequential AIGs allows us to **work with the AIG manager as if it was a combinational AIG, and only utilize its sequential properties when sequential transformations are applied**. For example, combinational AIG rewriting works uniformly on combinational and sequential AIGs, while **sequential cleanup, which removes structurally equivalent flip-flops, exploits the fact that they are represented as additional PIs and POs**. Sequential transformation, such as retiming, can add and remove latches as needed.

---
## Experiment: `scorr` vs `ssweep`
| testcases |  io   | registers | nodes after scl | scorr | ssweep |
| ---| -- | --------- | --------------- | ----- | ------ |
| a.aig | 1/4 | 16 | 180 | 155 | 155 |
|b.aig | 2/11 | 10 | 72 | 45 | 45 |
|c.aig | 2/4 | 6 | 28 | 22 | 22 |
|s1238.aig | 15/14 | 18 | 488 | 488 | 488 |
|s1423.aig | 18/5 | 74 | 536 | 513 | 513 |
|s9234.aig | 37/39 | 132 | 951 | 881 | 881 |
|s13207.aig | 63/152 | 420 | 1993 | ***997*** | 998 |
|s15850.aig | 78/150 | 450 | 2931 | 2784 | 2784 |
|s38584.aig | 39/304 | 1233 | 9623 | 9262 | 9262 |
|s35932.aig | 36/320 | 1728 | 12002 | 8945 | 8945 |

### Useful script for synthesis
```
read <file>; strash; ifraig; scorr; dc2; dretime; strash; &get -n; &dch -f; &nf {D}; &put;
```

[^1]: https://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.152.6465&rep=rep1&type=pdf "Scalable and Scalably-Verifiable Sequential Synthesis (ICCAD 2008)"
[^2]: http://people.eecs.berkeley.edu/~alanmi/publications/2013/date13_iso.pdf "A Semi-Canonical Form for Sequential AIGs (DATE 2013)"
[^3]: https://www.researchgate.net/publication/317244971_Are_XORs_in_logic_synthesis_really_necessary "Are XORs in logic synthesis really necessary? (DDECS 2017)"
[^4]: https://people.eecs.berkeley.edu/~alanmi/publications/2010/cav10_abc.pdf "ABC: An Academic Industrial-Strength Verification Tool (CAV 2010)"