#!/usr/bin/env python3
"""Hello World daemon

Verify what this spin adds on top of Infix: the hello-world daemon runs
under finit, takes its configuration from the example-hello YANG model
this repo ships, and reports state back through the same model.

Nothing here is specific to NETCONF -- the CLI and RESTCONF reach the
very same data, because they are all generated from the model.

"""
import infamy

NEW = "there"

with infamy.Test() as test:
    with test.step("Set up topology and attach to target DUT"):
        env = infamy.Env()
        target = env.attach("target", "mgmt")
        tgtssh = env.attach("target", "mgmt", "ssh")

    with test.step("Verify finit supervises the hello-world service"):
        status = tgtssh.runsh("initctl status hello-world", check=True).stdout
        print(status)
        if "running" not in status:
            test.fail()

    with test.step("Verify the daemon greets the default audience"):
        # Nothing has been configured, so the daemon is running on the
        # default from the YANG model.
        infamy.until(lambda: tgtssh.runsh("sudo grep -q 'Hello world!' /var/log/syslog",
                                          check=False).returncode == 0)

    with test.step("Verify the greeting counter is reported as operational data"):
        oper = target.get_data("/example-hello:hello")
        print(f"greetings: {oper['hello']['greetings']}")
        if int(oper["hello"]["greetings"]) < 1:
            test.fail()

    with test.step(f"Set audience to '{NEW}'"):
        target.put_config_dicts({
            "example-hello": {
                "hello": {
                    "audience": NEW,
                }
            }
        })

    with test.step(f"Verify '{NEW}' is in the running configuration"):
        running = target.get_config_dict("/example-hello:hello")
        if running["hello"]["audience"] != NEW:
            test.fail()

    with test.step(f"Verify the daemon starts greeting '{NEW}'"):
        # The daemon greets once a minute, so give it a full cycle to
        # pick the new audience up.
        infamy.until(lambda: tgtssh.runsh(f"sudo grep -q 'Hello {NEW}!' /var/log/syslog",
                                          check=False).returncode == 0,
                     attempts=90, interval=1)

    test.succeed()
