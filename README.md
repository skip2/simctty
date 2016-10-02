# simctty - Toy OpenRISC 1000 simulation.
This is a toy project, it does just enough to boot the bundled Linux binary.

More information & a web demo at https://skip.org/linux .

## Requirements:

    sudo apt-get install libgtest-dev libgflags-dev libgflags2

## How to build:

    mkdir build && cd build
    cmake ..
    make

# Running: (from the build dir):

    simctty/simctty ../linux/vmlinux.bin

To exit the simuation, run "poweroff".

## Tests:
These use gtest.

    ctest -V

## What it does:

```
tfh@snow:~/build$ simctty/simctty ~/simctty/linux/vmlinux.bin 

Loaded 2775436 bytes from /home/tfh/simctty/linux/vmlinux.bin
Linux version 3.13.0-rc7-or1ksim+ (tfh@pixel.skip.org) (gcc version 4.5.1-or32-1.0rc4 (OpenRISC 32-bit toolchain for or32-linux (built 20140512)) ) #14 Thu May 29 19:36:38 BST 2014
CPU: OpenRISC-10 (revision 0) @20 MHz
-- dcache disabled
-- icache disabled
-- dmmu:   64 entries, 1 way(s)
-- immu:   64 entries, 1 way(s)
-- additional features:
-- PIC
-- timer
setup_memory: Memory: 0x0-0x2000000
Reserved - 0x01fff820-0x000007e0
Setting up paging and PTEs.
map_ram: Memory: 0x0-0x2000000
itlb_miss_handler c0002160
dtlb_miss_handler c0002000
OpenRISC Linux -- http://openrisc.net
Built 1 zonelists in Zone order, mobility grouping off.  Total pages: 4080
Kernel command line: console=uart,mmio,0x90000000,115200
Early serial console at MMIO 0x90000000 (options '115200')
bootconsole [uart0] enabled
PID hash table entries: 128 (order: -4, 512 bytes)
Dentry cache hash table entries: 4096 (order: 1, 16384 bytes)
Inode-cache hash table entries: 2048 (order: 0, 8192 bytes)
Sorting __ex_table...
Memory: 29752K/32768K available (1298K kernel code, 91K rwdata, 128K rodata, 1184K init, 49K bss, 3016K reserved)
mem_init_done ...........................................
NR_IRQS:32 nr_irqs:32 0
40.00 BogoMIPS (lpj=200000)
pid_max: default: 32768 minimum: 301
Mount-cache hash table entries: 1024
devtmpfs: initialized
Switched to clocksource openrisc_timer
Serial: 8250/16550 driver, 4 ports, IRQ sharing disabled
90000000.serial: ttyS0 at MMIO 0x90000000 (irq = 2, base_baud = 1250000) is a 16550A
console [ttyS0] enabled
console [ttyS0] enabled
bootconsole [uart0] disabled
bootconsole [uart0] disabled
Freeing unused kernel memory: 1184K (c017e000 - c02a6000)
init started: BusyBox v1.22.1 (2014-05-21 22:58:56 BST)


BusyBox v1.22.1 (2014-05-21 22:58:56 BST) built-in shell (ash)
Enter 'help' for a list of built-in commands.

root@browser:/# ls
bin   dev   etc   home  init  mnt   proc  root  sbin  sys   usr   var
root@browser:/# cat /proc/cpuinfo
cpu		: OpenRISC-10
revision	: 0
frequency	: 20000000
dcache size	: 16 bytes
dcache block size	: 16 bytes
dcache ways	: 1
icache size	: 16 bytes
icache block size	: 16 bytes
icache ways	: 1
immu		: 64 entries, 1 ways
dmmu		: 64 entries, 1 ways
bogomips	: 40.00
root@browser:/# poweroff
The system is going down NOW!
Sent SIGTERM to all processes
Terminated
Sent SIGKILL to all processes
Requesting system poweroff
reboot: Power down

tfh@snow:~/build$ 
```
