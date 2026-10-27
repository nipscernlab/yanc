# Protecting SAPHO: synthesis without showing the source

Status: notes, 2026-10-14. Nothing decided, nothing built. Parked until a
partnership between UFJF and a company asks for it (Luciano). Marks:
**fact** (read or checked), **from the docs** (vendor documentation or
forum answers found on 2026-10-14, quoted through a search where the page
itself could not be opened), **deduced**.

## 1. The question

`yanc` writes a readable `.v`: a top module that instantiates `processor`
with the program's parameters (which operations to build, memory sizes,
widths) and names its two `.mif`. To put it in an FPGA the synthesis tool
also needs `SAPHO/`. The question: hand `SAPHO/` to Quartus or Vivado so it
synthesizes, without anyone being able to read it, the way vendors ship
their own soft cores and IP. Simulation is a separate question, left out
here (section 6).

## 2. What does not fit SAPHO

**A pre-synthesized netlist** (a Quartus exported partition, `.qxp` in
Standard or `.qdb` in Pro; a Vivado checkpoint). **Deduced:** it freezes the
parameters. SAPHO is generated per program (pay only for what you use), so
one netlist serves one program. It fits only as a service (section 5).

## 3. Encrypted source, IEEE 1735

How it works (**fact**, the files in Quartus Lite 24.1 show the structure:
`key_method = "rsa"`, `data_method = "aes128-cbc"`): the IP is encrypted once
with a random AES key; that key is wrapped once per tool with the tool
vendor's RSA public key; the tool holds the private key in its executable,
decrypts in memory, applies the parameters and synthesizes; the decrypted
source never reaches the disk. **Deduced:** the parameters keep working, so
`yanc`'s `.v` would not change.

**Quartus.**

- **From the docs:** the Quartus Prime Pro Edition guide says Pro supports
  IEEE 1735 v1 decryption and that Standard does not; Pro ships the public
  key (`quartus/common/misc/public_key`) and an encryptor `encrypt_1735`.
  Nothing direct was found for Lite, which sits below Standard.
- **Fact (this machine, Quartus Lite 24.1):** no `public_key`, no
  `encrypt_1735`. Its 9 012 IEEE 1735 files carry keys for simulators only
  (Siemens MTI 5 595, Aldec 3 759, Synopsys VCS 2), none for Quartus:
  Intel's own IP for synthesis in Lite uses another scheme.
- **From the docs (forum, Intel staff):** for Standard there is Intel's
  proprietary encryption, Ampcrypt/Amplic, obtained through Intel sales.
- **The catch for the lab's boards:** as far as we know, Cyclone V (the
  DE10-Nano) is supported by Lite and Standard only, not by Pro, so IEEE
  1735 is not a path there. To be confirmed in Intel's device support
  list.

**Vivado.**

- **From the docs (AMD UG835, UG1118):** the Tcl command
  `encrypt -lang verilog -key <keyfile> ...` encrypts in IEEE 1735; it needs
  an encryption license, and the Xilinx public key is given to members of
  the IEEE P1735 group or through an AMD representative.
- Not tested: no key and no license here.

**Strength.** **From the docs:** a 2021 paper (arXiv 2112.04838) reports key
recovery from IEEE 1735 implementations of several vendors, Intel among
them, and calls the weaknesses structural. Encryption keeps out casual
readers and companies that will not risk a lawsuit; it is not a vault.

## 4. What was already published

**Fact:** SAPHO up to v7.1 is public, in the yanc repository and in every
release, under the MIT license, and described in the IEEE paper. Those
versions stay free for whoever has a copy. Only future versions could be
protected, and only if their source leaves the public repository.
**Deduced:** a patent on what the paper describes is unlikely; what UFJF
could license is the future versions, the group's know-how, the compiler
and support. Dual licensing (a copyleft free version plus a paid
commercial one) is possible only from a new version on.

## 5. Ways a partnership could deliver SAPHO closed

1. **Through the FPGA vendor:** IEEE 1735 (Quartus Pro, Vivado with the AMD
   key and license) or Intel's Ampcrypt for Standard/Lite. Needs an
   agreement with each vendor; `yanc` unchanged.
2. **As a service:** the client sends the program (or `yanc`'s `.v`); the
   holder of SAPHO synthesizes that processor and returns a netlist for the
   client's family (a Quartus partition, a Vivado checkpoint). Solves the
   per-program parameters; needs the holder to run the synthesis for each
   program and each FPGA family.
3. **Obfuscation only:** renamed signals, no comments, one flat file. Works
   in every tool, simulators included; delays a reader, does not stop one.

## 6. Left out here

Simulation of a closed SAPHO: Icarus and Verilator cannot decrypt IEEE 1735
(open tools cannot hold the vendors' private keys); commercial simulators
can, but the inside of the processor is hidden in the waveform. An
instruction-set simulator in `yanc` (issue #32) would let users
run and debug programs without the HDL.

## 7. To do if a partnership comes

- Ask the vendors: Intel (Ampcrypt/Amplic for Standard/Lite and the Cyclone
  V question; IEEE 1735 key for Pro), AMD (the IEEE 1735 key and encryption
  license).
- With a key in hand: encrypt a small module, then SAPHO, and synthesize the
  `yanc` output of `sapho_all`; compare Fmax and area with the plain source.
- Decide the licence of future versions with UFJF's technology transfer
  office before any SAPHO source leaves the public repository.

Sources (2026-10-14): Quartus Prime Pro Edition User Guide, "Support for the
IEEE 1735 Encryption Standard" (683463, 24.1 and 25.1); Altera Community
threads "Quartus Prime Standard encryption public key", "Decrypt problem in
Std version of Quartus", "How to use IEEE 1735 encryption on a design with
Cyclone V?"; Macnica FAQ 141642; AMD UG835 `encrypt`, UG1118 "Encrypting IP
with Vivado"; arXiv 2112.04838.
