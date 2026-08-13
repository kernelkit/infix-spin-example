# Example Infix Spin

This is a *spin* of [Infix OS](https://github.com/kernelkit/infix) a
tiny skeleton repo showing how to make Infix your own without forking
it.  Infix is included as a submodule; this repo only adds a thin layer
on top:

  * **Branding** — OS name, vendor, hostname, and boot tagline, all in
    `config-snippets/`
  * **Packages** — an example `hello-world` daemon, built from source in
    this repo and supervised by finit (`package/`, `src/`)
  * **Files** — a rootfs overlay for configuration and service settings
    (`board/common/rootfs/`)

When a new Infix release comes out, you bump the submodule and rebuild.
No fork, no drift, no merge conflicts.  Clone it, rename a few things,
and you have your own OS.

## Get Started

Clone the repository along with its submodules, and install the tools
Buildroot needs:

```bash
git clone --recurse-submodules https://github.com/kernelkit/infix-spin-example.git
cd infix-spin-example

sudo apt install bc binutils build-essential bzip2 cpio \
                 diffutils file findutils git gzip      \
                 libncurses-dev libssl-dev perl patch   \
                 python3 rsync sed tar unzip wget       \
                 autopoint bison flex autoconf automake \
                 mtools
```

Configure and build.  The first build takes a while, since it compiles
the cross toolchain and the Linux kernel too:

```bash
make x86_64_defconfig
make
```

Then boot your new OS in QEMU:

```bash
sudo apt install qemu-system-x86
make run
```

Log in as `admin` with password `admin`, and have a look at the example
daemon, which greets you in the system log once a minute:

```
example-os:~$ initctl status hello-world
example-os:~$ sudo grep hello-world /var/log/syslog
```

That's it. Have a look around and make it your own.

## Learn More

  * [Infix documentation](https://kernelkit.org/)
  * [Buildroot br2-external manual](https://buildroot.org/downloads/manual/manual.html#outside-br-custom)
  * [finit](https://github.com/troglobit/finit)
