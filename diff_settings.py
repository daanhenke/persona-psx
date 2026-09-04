"""asm-differ configuration.

tools/mfunc.py --diff writes the two object files it wants compared and points
these env vars at them, so asm-differ can be used per-function without a
whole-binary map file.
"""
import os


def apply(config, args):
    config["arch"] = "mipsel"
    config["objdump_executable"] = "mipsel-linux-gnu-objdump"
    config["baseimg"] = os.environ.get("DIFF_BASE", "")
    config["myimg"] = os.environ.get("DIFF_MY", "")
    config["mapfile"] = None
    config["source_directories"] = ["src", "include"]
    config["make_command"] = ["true"]
    config["makeflags"] = []
    config["build_dir"] = "build"
