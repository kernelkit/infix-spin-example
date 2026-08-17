#!/usr/bin/env python3
"""Hello World daemon

Verify the pieces this spin adds on top of Infix: the hello-world
package is installed, finit supervises it, its configuration file comes
from the rootfs overlay, and it greets us in the system log.

"""
import infamy

with infamy.Test() as test:
    with test.step("Set up topology and attach to target DUT"):
        env = infamy.Env()
        tgtssh = env.attach("target", "mgmt", "ssh")

    with test.step("Verify configuration file from the rootfs overlay"):
        tgtssh.runsh("test -f /etc/hello-world.conf", check=True)

    with test.step("Verify finit supervises the hello-world service"):
        status = tgtssh.runsh("initctl status hello-world", check=True).stdout
        print(status)
        if "running" not in status:
            test.fail()

    with test.step("Verify the greeting reached the system log"):
        tgtssh.runsh("sudo grep -q 'Hello World!' /var/log/syslog", check=True)

    test.succeed()
