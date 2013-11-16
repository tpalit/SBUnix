#/bin/sh
make clean && make && qemu-system-x86_64 -curses -cdrom tpalit.iso -gdb tcp::88888 -d cpu_reset -D ./qemu.log
