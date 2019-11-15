# What is AFL and What is it Good for?

AFL is a coverage-guided, or feedback-based, fuzzer. More about these concepts can be found in a cool paper, [Fuzzing: Art, Science, and Engineering](https://arxiv.org/pdf/1812.00140.pdf). Let's wrap up general information about AFL:

- It modifies the executable file to find out how it influences coverage.
- Mutates input data to maximize coverage.
- Repeats the preceding step to find where the program crashes.
- It’s highly effective, which is proven by practice.
- It’s very easy to use.

Here's a graphic representation:

![Graphic representation of AFL work](/images/graphic_representation_afl.png)

If you don't know what AFL is, here is a list of helpful resources for you to start:

1. [The official page of the project](http://lcamtuf.coredump.cx/afl/)
2. [afl-training](https://github.com/ThalesIgnite/afl-training) - a short intro to AFL.
3. [afl-demo](https://gitlab.com/wolframroesler/afl-demo) - a simple demo of fuzzing C++ programs with AFL.
4. [afl-cve](https://github.com/mrash/afl-cve) - a collection of the vulnerabilities found with AFL (hasn't been updated since 2017).
5. [Here](https://tunnelshade.in/blog/2018/01/afl-internals-compile-time-instrumentation/) - you can read about the stuff AFL adds to a program during its build.
6. [A few](https://copyninja.info/blog/afl-and-network-programs.html) useful tips about fuzzing network applications.

At the moment this article was being written, the latest version of AFL was 2.52b. The fuzzer is in active development, and with time some side developments are being incorporated into the main AFL branch and grow irrelevant. Today, we can name several useful accessory tools, which are listed in the following chapter.

It's also worth to mention the monthly [Rode0day](https://rode0day.mit.edu/) competition – an event where fuzzers try to find the biggest number of bugs with less time than their opponents in pre-made corpora both with and without access to the source code. In its nature, Rode0day is a battle between different modifications and forks of AFL.

![AFL demo](/images/afl_demo.png)

There are also different variations and derivates of AFL, which allows fuzzing Python, Go, Rust, OCaml, GCJ Java, kernel syscalls, or even entire VMs.

AFL for other programming languages:

- [python-afl](https://github.com/jwilk/python-afl) - for Python.
- [afl.rs](https://github.com/rust-fuzz/afl.rs) - for fuzzing programs written on Rust.
- [afl-fuzz-js](https://github.com/connor4312/js-fuzz) - for JavaScript.
- [java-afl](https://github.com/Barro/java-afl) - for Java.
- [kelinci](https://github.com/isstac/kelinci) - another fuzzer for Java (an [article](https://www.modzero.ch/modlog/archives/2018/09/20/java_bugs_with_and_without_fuzzing/index.html) on the topic).
- [javan-warty-pig](https://github.com/cretz/javan-warty-pig) - fuzzer for JVM.
- [afl-swift](https://github.com/Proteas/afl-swift) - for Swift.
- [ocamlopt-afl](https://github.com/kayceesrk/ocamlopt-afl) - for OCaml.
- [sharpfuzz](https://github.com/Metalnem/sharpfuzz) - fuzzer based on afl for .net.

# Accessory tools

For this chapter, we've collected various scripts and tools for AFL and divided them into several categories:

## Crash processing

- [afl-utils](https://gitlab.com/rc0r/afl-utils) - a set of utilities for automatic processing/analysis of crashes and reducing the number of test cases.
- [afl-crash-analyzer](https://github.com/floyd-fuh/afl-crash-analyzer) - another crash analyzer for AFL.
- [fuzzer-utils](https://github.com/ThePatrickStar/fuzzer-utils) - a set of scripts for the analysis of results.
- [atriage](https://github.com/Ayrx/atriage) - a simple triage tool.
- [afl-kit](https://github.com/kcwu/afl-kit) - afl-cmin on Python.
- [AFLize](https://github.com/d33tah/aflize) - a tool that automatically generates builds of debian packages suitable for AFL.
- [afl-fid](https://github.com/FoRTE-Research/afl-fid) - a set of tools for working with input data.

## Work with code coverage

- [afl-cov](https://github.com/mrash/afl-cov) - provides human-friendly data about coverage.
- [count-afl-calls](https://github.com/Barro/count-afl-calls) - ratio assessment. The script counts the number of instrumentation blocks in the binary.
- [afl-sancov](https://github.com/bshastry/afl-sancov) - is like afl-cov but uses a clang sanitizer.
- [covnavi](https://github.com/Cisco-Talos/covnavi) - a script for covering code and analysis by Cisco Talos Group.
- [LAF LLVM Passes](https://gitlab.com/laf-intel/laf-llvm-pass) - something like a collection of patches for AFL that modify the code to make it easier for the fuzzer to find branches.

## A few scripts for the minimization of test cases

- [afl-pytmin](https://github.com/ilsani/afl-pytmin) - a wrapper for afl-tmin that tries to speed up the process of the minimization of the test case by using many CPU cores.
- [afl-ddmin-mod](https://github.com/MarkusTeufelberger/afl-ddmin-mod) - a variation of afl-tmin based on the ddmin algorithm.
- [halfempty](https://github.com/googleprojectzero/halfempty) - is a fast utility for minimizing test cases by Tavis Ormandy based on parallelization.

## Distributed execution

- [disfuzz-afl](https://github.com/MartijnB/disfuzz-afl) - distributed fuzzing for AFL.
- [AFLDFF](https://github.com/quantumvm/AFLDFF) - AFL distributed fuzzing framework.
- [afl-launch](https://github.com/bnagy/afl-launch) - a tool for the execution of many AFL instances.
- [afl-mothership](https://github.com/afl-mothership/afl-mothership) - management and execution of many synchronized AFL fuzzers on AWS cloud.
- [afl-in-the-cloud](https://github.com/abhisek/afl-in-the-cloud) - another script for running AFL in AWS.
- [VU_BSc_project](https://github.com/clvang000/VU_BSc_project) - fuzzing testing of the open source libraries with libFuzzer and AFL.

Recently, there has been published a very good article titled [Scaling AFL to a 256 thread machine](https://gamozolabs.github.io/fuzzing/2018/09/16/scaling_afl.html).

## Deployment, management, monitoring, reporting

- [afl-other-arch](https://github.com/shellphish/afl-other-arch) - is a set of patches and scripts for easily adding support for various non-x86 architectures for AFL.
- [afl-trivia](https://github.com/bnagy/afl-trivia) - a few small scripts to simplify the management of AFL.
- [afl-monitor](https://github.com/reflare/afl-monitor) - a script for monitoring AFL.
- [afl-manager](https://github.com/zx1340/afl-manager) - a web server on Python for managing multi-afl.
- [afl-tools](https://hub.docker.com/r/moflow/afl-tools) - an image of a docker with afl-latest, afl-dyninst, and Triforce-afl.
- [afl-remote](https://github.com/block8437/afl-remote) - a web server for the remote management of AFL instances.

# AFL Modifications

AFL had a very strong impact on the community of vulnerability researchers and fuzzing itself. It's not surprising at all that after some time people started making modifications inspired by the original AFL. Let's have a look at them. In different situations, each of these modifications has its own pros and cons compared to the original AFL.

Almost all mods can be found at [hub.docker.com](https://hub.docker.com).

What for?

- Increase the speed and/or code coverage
    - Algorithms
    - Environment
        - OS
        - Hardware
- Working without source code
    - Code emulation
    - Code instrumentation
        - Static
        - Dynamic
        
## Default modes of AFL operation

Before going on with examining different modifications and forks of AFL, we have to talk about two important modes, which also had been modifications in the past but were eventually incorporated. They are Syzygy and Qemu.

[Syzygy](https://doar-e.github.io/blog/2017/08/05/binary-rewriting-with-syzygy/) mode -- is the mode of working in instrument.exe

```instrument.exe --mode=afl --input-image=test.exe --output-image=test.instr.exe```

Syzygy allows to statically rewrite PE32 binaries with AFL but requires symbols and an additional dev to make WinAFL kernel aware.

Qemu mode -- the way it works under QEMU can be seen in [Internals of AFL fuzzer -- QEMU Instrumentation](https://tunnelshade.in/blog/2018/02/afl-internals-qemu-instrumentation/). The support of working with binaries with QEMU was added to upstream AFL in Version 1.31b. AFL QEMU mode works with the added functionality of binary instrumentation into qemu tcg (a tiny code generator) binary translation engine. For that, AFL has a build script qemu, which extracts the sources of a certain version of qemu (2.10.0), puts them onto several small patches and builds for a defined architecture. Then, a file called afl-qemu-trace is created, which is in fact a file of user mode emulation of (emulation of only executable ELF files) qemu-. Thus, it is possible to use fuzzing with feedback on elf binaries for many different architectures supported by qemu. Plus, you get all the cool AFL tools, from the monitor with information about the current session to advanced stuff like afl-analyze. But you also get the limitations of qemu. Also, if a file is built with toolchain using hardware SoC features, which launches the binary and is not supported by qemu, fuzzing will be interrupted as soon as there is a specific instruction or a specific MMIO is used.

Here's [another interesting fork](https://abiondo.me/2018/09/21/improving-afl-qemu-mode/) of the qemu mode, where the speed was increased 3-4 times with TCG code instrumentation and cashing.

## Forks

The appearance of forks of AFL is first of all related to the changes and improvements of the algorithms of the classic AFL.

- [pe-afl](https://github.com/wmliang/pe-afl) - a modification for fuzzing PE files that have no source code in the Windows OS. For its operation, the fuzzer analyzes a target program with IDA Pro and generates the information for the following static instrumentation. An instrumented version is then fuzzed with AFL.
- [afl-cygwin](https://github.com/arizvisa/afl-cygwin) - is an attempt to port the classic AFL to Windows with Cygwin. Unfortunately, it has many bugs, it's very slow, and the development of has been abandoned.
- [AFLFast](https://github.com/mboehme/aflfast) (extends AFL with Power Schedules) - one of the first AFL forks. It has added heuristics, which allow it to go through more paths in a short time period.
- [FairFuzz](https://github.com/carolemieux/afl-rb) - an extension for AFL, that targets rare branches.
- [AFLGo](https://github.com/aflgo/aflgo) - is an extension for AFL meant for getting to certain parts of code instead of full program coverage. It can be used for testing patches or newly added fragments of code.
- [PerfFuzz](https://github.com/carolemieux/perffuzz) - an extension for AFL, that looks for test cases which could significantly slow down the program.
- [Pythia](https://github.com/mboehme/pythia) - is an extension for AFL that is meant to forecast how hard it is to find new paths.
- [Angora](https://github.com/AngoraFuzzer/Angora) - is one of the latest fuzzers, written on rust. It uses new strategies for mutation and increasing the coverage.
- [Neuzz](https://github.com/Dongdongshe/neuzz) - fuzzing with neural netwoks.
- [UnTracer-AFL](https://github.com/FoRTE-Research/UnTracer-AFL) - integration of AFl with UnTracer for effective tracing.
- [Qsym](https://github.com/sslab-gatech/qsym) - practical Concolic Execution Engine Tailored for Hybrid Fuzzing. Essentially, it is a symbolic execution engine (basic components are realized as a plugin for intel pin) that together with AFL performs hybrid fuzzing. This is a stage in the evolution of feedback-based fuzzing and calls for a separate discussion. Its main advantage is that can do concolic execution relatively fast. This is due to the native execution of commands without intermediate representation of code, snapshots, and some heuristics. It uses the old Intel pin (due to support problems between libz3 and other DBTs) and currently can work with elf x86 and x86_64 architectures.
- [Superion](https://github.com/zhunki/Superion) - greybox fuzzer, an obvious advantage of which is that along with an instrumented program it also gets specification of input data using the ANTLR grammar and after that performs mutations with the help of this grammar.
- [AFLSmart](https://github.com/aflsmart/aflsmart) - another Graybox fuzzer. As input, it gets specification of input data in the format used by the Peach fuzzer.

There are many research papers dedicated to the implementation of the new approaches and fuzzing techniques where AFL is modified. Only white papers are available, so we didn't even bother mentioning those. You can google them if you want. For example, some of the latest are [CollAFL: Path Sensitive Fuzzing](https://chao.100871.net/papers/oakland18.pdf), [EnFuzz](https://arxiv.org/pdf/1807.00182.pdf), [Efficient approach to fuzzing interpreters](https://i.blackhat.com/asia-19/Fri-March-29/bh-asia-Dominiak-Efficient-Approach-to-Fuzzing-Interpreters-wp.pdf), [ML](https://arxiv.org/pdf/1811.08973.pdf) for AFL.

## Modifications based on Qemu

- [TriforceAFL](https://github.com/nccgroup/TriforceAFL) - AFL/QEMU fuzzing with full emulation of a system. A fork by nccgroup. Allows fuzzing the entire OS in qemu mode. It is realized with a special instruction (aflCall (0f 24)), which was added in QEMU x64 CPU. Unfortunately, it's no longer supported; the last version of AFL is 2.06b.
- [TriforceLinuxSyscallFuzzer](https://github.com/nccgroup/TriforceLinuxSyscallFuzzer) - the fuzzing of Linux system calls.
- [afl-qai](https://github.com/kanglictf/afl-qai) - a small demo project with QEMU Augmented Instrumentation (qai).

## A modification based on KLEE

- [kleefl](https://github.com/julieeen/kleefl) - for generating test cases by means of symbolic execution (very slow on big programs).

## A modification based on Unicorn

[afl-unicorn](https://github.com/Battelle/afl-unicorn) - allows for fuzzing of fragments of code by emulating it on [Unicorn Engine](https://www.unicorn-engine.org/). We successfully used this variation of AFL in our practice, on the areas of the code of a certain RTOS, which was executed on SOC, so we couldn't use QEMU mode. The use of this modification is justified in the case when we don't have sources (we can't build a stand-alone binary for the analysis of the parser) and the program doesn't take input data directly (for example, data is encrypted or is signal sample like in a CGC binary), then we can reverse and find the supposed places-functions, where the data is procced in a format convenient for the fuzzer. This is the most general/universal modification of AFL, i.e. it allows fuzzing anything. It's independent of architecture, sources, input data format, and binary format (the most striking example of bare-metal - just fragments of code from the controller's memory). The researcher first examines this binary and writes a fuzzer, which emulates the state at the input of the parser procedure. Obviously, unlike AFL, this requires a certain examination of binary. For bare-metal firmware, like Wi-FI or baseband, there are certain drawbacks that you need to keep in mind:

1. We have to localize the check of the control sum.
2. Keep in mind that the state of the fuzzer is a state of memory that was saved in the memory dump, which can prevent the fuzzer from getting to certain paths.
3. There's no sanitation of calls to dynamic memory, but it can be realized manually, and it will depend on RTOS (has to be researched).
4. Intertask RTOS interaction is not emulated, which can also prevent finding certain paths.

An example of working with this modification [afl-unicorn: Fuzzing Arbitrary Binary Code](https://hackernoon.com/afl-unicorn-fuzzing-arbitrary-binary-code-563ca28936bf) and [afl-unicorn: Part 2 — Fuzzing the ‘Unfuzzable’](https://hackernoon.com/afl-unicorn-part-2-fuzzing-the-unfuzzable-bea8de3540a5).

Before we go on to the modifications based on the frameworks of dynamic binary instrumentation (DBI), let's not forget that the highest speed of these frameworks is shown by DynamoRIO, Dynlnst and, finally, PIN.

## PIN-based modifications

- [aflpin](https://github.com/mothran/aflpin) - AFL with Intel PIN instrumentation.
- [afl_pin_mode](https://github.com/spinpx/afl_pin_mode) - another AFL instrumentation realized through Intel PIN.
- [afl-pin](https://github.com/vanhauser-thc/afl-pin) - AFL with PINtool.
- [NaFl](https://github.com/carlosgprado/NaFl) - a clone (of the basic core) of AFL fuzzer.
- [PinAFL](https://github.com/houcy/PinAFL) - the author of this tool tried to port AFL to Windows for the fuzzing of already compiled binaries. Seems like it was done overnight just for fun; the project has never gone any further. The repository doesn't have sources, only compiled binaries and launch instruction. We don't know which version of AFL it's based on, and it only supports 32-bit applications.

As you can see, there are many different modifications, but they are not very very useful in real life.

## Dyninst-based modifications

- [afl-dyninst](https://github.com/talos-vulndev/afl-dyninst) - American Fuzzy Lop + Dyninst == AFL balckbox fuzzing. The feature of this version is that first a researched program (without the source code) is instrumented statically (static binary instrumentation, static binary rewriting) with Duninst, and then is  fuzzed with the classic AFL that thinks that the program is build with afl-gcc/afl-g++/afl-as ;) As a result, it allows is to work with a very good productivity without the source code — It used to be at 0.25x speed compared to a native compile. It has a significant advantage compared to QEMU: it allows the instrumentation of dynamic linked libraries, while QEMU can only instrument the basic executable file statically linked with libraries.  Unfortunately, now it's only relevant for Linux. For Windows support, changes to Dyninst itself are needed, which is [being done](https://github.com/dyninst/dyninst/issues/120).

There's yet another [fork](https://github.com/vanhauser-thc/afl-dyninst) with improved speed and certain features (the support of AARCH64 and PPC architectures).

## Modifications based on DynamoRIO

- [drAFL](https://github.com/mxmssh/drAFL) - AFl + DynamoRIO – fuzzing without sources on Linux.
- [afl-dr](https://github.com/atrosinenko/afl-dr) - another realization based on DynamoRIO which very well described on [Habr](https://habrahabr.ru/post/332076/).
- [afl-dynamorio](https://github.com/vanhauser-thc/afl-dynamorio) - a modification by vanhauser-thc. Here's what he says about it: "run AFL with DynamoRIO when normal afl-dyninst is crashing the binary and qemu mode -Q is not an option". It supports ARM and AARCH64. Regarding the productivity: DynamoRIO is about 10 times slower than Qemu, 25 times slower than dyninst, but about 10 times faster than Pintool.
- [WinAFL](https://github.com/ivanfratric/winafl) - the most famous AFL fork Windows. (DynamoRIO, also syzygy mode). It was only a matter of time for this mod to appear because many wanted to try AFL on Windows and apply it to apps without sources. Currently, this tool is being actively improved, and regardless of a relatively outdated code base of AFL (2.43b when this article is written), it helped to find several vulnerabilities (CVE-2016-7212, CVE-2017-0073, CVE-2017-0190, CVE-2017-11816). The specialists from Google Zero Project team and MSRC Vulnerabilities and Mitigations Team are working in this project, so we can hope for the further development.  Instead of compilation time instrumentation, the developers used dynamic instrumentation(based on DynamoRIO), which significantly slowed down the execution of the analyzed software, but the resulting overhead (doubled) is comparable to that of the classic AFL in binary mode. They also solved the problem of fast process launch, having called it persistent fuzzing mode; they choose the function to fuzz (by the offset inside the file or by the name of function present in the export table) and instrument it so that it could be called in the cycle, thus launching several input data samples without restarting the process. An [article](https://research.checkpoint.com/50-adobe-cves-in-50-days/) came out recently, describing how the authors found around 50 vulnerabilities in about 50 days using WinAFL. And shorty before it was published, Intel PT mode had been added to WinAFL; detalis can be found [here](https://github.com/googleprojectzero/winafl/blob/master/readme_pt.md).

An advanced reader could notice that there are modifications with all the popular instrumentation frameworks except for [Frida](https://www.frida.re). The only mention of the use of Frida with AFL was found in [Chizpurfle: A Gray-Box Android Fuzzer for Vendor Service Customizations](https://wpage.unina.it/roberto.natella/papers/natella_androidfuzzing_issre2017.pdf). A version of AFL with Frida is really useful because Frida supports several RISC architectures.

Many researches are also looking forward to the release of DBI Scopio framework by the creator of Capstone, Unicorn, and Keystone. Based on this framework, the authors have already created a fuzzer (Darko) and, according to them, successfully use it to fuzz embedded devices. More on this can be found in [Digging Deep: Finding 0days in Embedded Systems with Code Coverage Guided Fuzzing](https://conference.hitb.org/hitbsecconf2018pek/materials/D2T1%20-%20Finding%200days%20in%20Embedded%20Systems%20with%20Code%20Coverage%20Guided%20Fuzzing%20-%20Dr%20Quynh%20and%20Kai%20Jern%20Lau.pdf).

## Modifications, based on processor hardware features

When it comes to AFL modifications with the support of processor hardware features, first of all, it allows fuzzing kernel code, and secondly - it allows for much faster fuzzing of apps without the source code.

And of course, speaking about processor hardware features, we are most of all interested in [Intel PT](https://software.intel.com/en-us/node/721535) (Processor Tracing). It is available from the 6th generation of processors onwards (approximately, since 2015). So, in order to be able to use the fuzzers listed below, you need a processor supporting Intel PT.

- [WinAFL-IntelPT](https://github.com/intelpt/winafl-intelpt) - a third-party WinAFL modification that uses Intel PT instead of DynamoRIO.
- [kAFL](https://github.com/RUB-SysSec/kAFL) - is an academic project aimed at solving the coverage-guided problem for the OS-independent fuzzing of the kernel. The problem is solved by using a hypervisor and Intel PT. More about it can be found in the white paper [kAFL: Hardware-Assisted Feedback Fuzzing for OS Kernels](https://www.usenix.org/system/files/conference/usenixsecurity17/sec17-schumilo.pdf).

