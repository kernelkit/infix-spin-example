<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="doc/logo-dark.png">
    <img src="doc/logo.png" alt="ExampleOS -- an Infix spin" width="420">
  </picture>
</p>

This is a *spin* of [Infix OS](https://github.com/kernelkit/infix) a
tiny skeleton repo showing how to make Infix your own without forking
it.  Infix is included as a submodule; this repo only adds a thin layer
on top:

  * **Branding** — OS name, vendor, hostname, and boot tagline, all in
    `config-snippets/`
  * **Packages** — an example `hello-world` daemon, built from source in
    this repo and supervised by finit (`package/`, `src/`)
  * **Models** — a YANG model for that daemon, which is all it takes to
    get it into the CLI, NETCONF, and RESTCONF (`yang/`)
  * **Files** — a rootfs overlay for service settings, a CLI snippet, and
    a factory default (`board/common/rootfs/`)
  * **Tests** — a regression test for the daemon, run in QEMU by Infix's
    test harness (`test/`)

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

## A Model of Your Own

The daemon is configured the way everything in Infix is: with YANG.  The
model lives in `yang/example-hello@2026-08-26.yang` and is a container
with two leaves — `audience`, which you set, and `greetings`, which the
daemon reports.  It is loaded into sysrepo when the image is built, see
`package/hello-world/hello-world.mk`.

That is the entire integration.  There is no CLI code for it, because
configure mode is generated from whatever models sysrepo has loaded:

```
example-os:~$ configure
example-os:/config/> set hello audience there
example-os:/config/> leave
example-os:~$ show hello
audience:  there
greetings: 4
```

Tab completion, help texts, validation, `show`, `diff`, saving to
startup-config — all of it follows from the model.  The same data is
reachable over NETCONF and RESTCONF, again for free.

Only the last command needed writing: `show` in operational mode is an
ordinary CLI command, so the spin drops a klish snippet into
`board/common/rootfs/etc/klish/hello.xml` to add it.

A `default` in the model is not the same as a starting configuration.
The daemon reads `world` whether or not anything is set, but a leaf that
still holds its default is left out when the datastore is serialized, so
`show hello` prints nothing for it.  To make it visible, ship it as real
configuration: Infix collates the factory config from
`/usr/share/confd/factory.d/`, so the overlay drops a `50-hello.json`
there, and symlinks it into `test.d/` beside it, which is where the
image boots from under the test harness.  Anything a spin wants in a
factory fresh configuration goes the same way.

On the daemon side, two sysrepo subscriptions are all that is needed —
one to hear about `audience` changing, one to answer for `greetings`.
See `src/hello-world/hello-world.c`, it is about thirty lines of it.

## Test

Infix comes with a test harness that boots the image in QEMU and pokes
at it over SSH and NETCONF.  A spin can reuse it for its own tests; this
one has a single case that checks the `hello-world` daemon, see
`test/case/hello-world/`:

```bash
make test
```

Docker or Podman is required, the harness runs inside a container that
brings its own QEMU and Python tooling.  Tests describe the *logical*
topology they need (`topology.dot`) and the harness maps it onto the
virtual network it starts, so nothing here depends on real hardware.

The result of the last run can be rendered as a PDF, branded with
`doc/logo.png` (swap it for your own, and see `LOGO` in the Makefile if
you want it placed differently):

```bash
gem install asciidoctor-pdf
make test-report          # -> output/images/test-report.pdf
```

The same commands run in CI, on a stock GitHub runner, see
`.github/workflows/build.yml`.  Every run uploads the report; a bootable
image is a few hundred MB, more than a free account's artifact storage
will take on every push, so that one is built on demand with the "Run
workflow" button.  Push a `v*` tag and both are attached to a GitHub
release instead, where they are kept indefinitely.

## Learn More

  * [Infix documentation](https://kernelkit.org/)
  * [RFC 7950](https://www.rfc-editor.org/rfc/rfc7950), the YANG 1.1
    language
  * [Buildroot br2-external manual](https://buildroot.org/downloads/manual/manual.html#outside-br-custom)
  * [finit](https://github.com/troglobit/finit)
