<p align="center">
  <img src="https://github.com/nipscernlab/nipscernweb/blob/main/assets/icons/yanc.svg"
       alt="YANC Icon"
       width="160">
</p>

# YANC  
**Yet Another Compiler**

**YANC** is a collection of compilers developed by **NIPSCERN** that form the core toolchain powering the **SAPHO** ecosystem.

The YANC toolchain includes compilers such as:

- `cmmcomp` – CMM language compiler  
- `asmcomp` – assembly compiler  
- `APP` and auxiliary compilation tools  

Together, these components operate as a **processor programming pipeline**, enabling the full workflow of processor design, compilation, and execution within SAPHO.

All YANC compilers are implemented in **C**, using **Flex**, **Bison**, and **GCC**, with a focus on performance, portability, and precise control over the compilation process.

YANC acts as the **compilation backbone** of SAPHO, bridging high-level processor description and low-level executable representations.
