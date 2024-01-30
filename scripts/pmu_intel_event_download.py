import sys
import re
from urllib.request import urlopen
from urllib.error import URLError
import os
import string
from fnmatch import fnmatch
from program_utils import getfile, get_cpustr, pmu_events_filename

urlpath = os.environ.get(
    "PERFMONDIR", "https://raw.githubusercontent.com/intel/perfmon/main"
)
mapfile = "mapfile.csv"
modelpath = urlpath + "/" + mapfile


def sanitize(s, a):
    o = ""
    for j in s:
        if j in a:
            o += j
    return o


def getdir():
    try:
        d = os.getenv("XDG_CACHE_HOME")
        xd = d
        if not d:
            home = os.getenv("HOME")
            d = "%s/.cache" % (home)
        d += "/pmu-events"
        if not os.path.isdir(d):
            # try to handle the sudo case
            if not xd:
                user = os.getenv("SUDO_USER")
                if user:
                    nd = os.path.expanduser("~" + user) + "/.cache/pmu-events"
                    if os.path.isdir(nd):
                        return nd
            os.makedirs(d)
        return d
    except OSError:
        raise Exception("Cannot access " + d)


printed = set()


def warn_once(msg):
    if msg not in printed:
        print(msg, file=sys.stderr)
        printed.add(msg)


def cpu_without_step(match):
    if match.count("-") < 3:
        return match
    n = match.split("-")
    return "%s-%s-%s" % tuple(n[:3])


allowed_chars = string.ascii_letters + "_-." + string.digits


def parse_map_file(
    match, key=None, link=True, onlyprint=False, acceptfile=False, hybridkey=None
):
    match2 = cpu_without_step(match)
    files = []
    dirfn = getdir()
    try:
        mfn = os.getenv("MAPFILE")
        if mfn:
            mapfn = mfn
            acceptfile = True
        else:
            mapfn = os.path.join(dirfn, mapfile)
        if onlyprint and not os.path.exists(mapfn) and not mfn:
            print("Download", mapfn, "first for --print")
            return []
        if acceptfile and os.path.exists(mapfn):
            pass
        elif not onlyprint and not mfn:
            getfile(modelpath, dirfn, mapfile)
        models = open(mapfn)
        for j in models:
            if j.startswith("Family-model"):
                continue
            n = j.rstrip().split(",")
            if len(n) < 4:
                if len(n) > 0:
                    print("Cannot parse", n)
                continue
            cpu, version, name, typ = n[:4]
            if not (
                fnmatch(cpu, match)
                or fnmatch(cpu, match2)
                or fnmatch(match2, cpu)
                or fnmatch(match, cpu)
            ):
                continue
            if key is not None and typ not in key:
                continue
            if hybridkey and len(n) >= 7 and n[6] != hybridkey:
                continue
            cpu = sanitize(cpu, allowed_chars)
            url = urlpath + name
            matchfn = match
            if matchfn == "*":
                matchfn = cpu
            if ".json" not in matchfn:
                if hybridkey:
                    fn = "%s-%s-%s.json" % (
                        matchfn,
                        sanitize(typ, allowed_chars),
                        hybridkey,
                    )
                else:
                    fn = "%s-%s.json" % (matchfn, sanitize(typ, allowed_chars))
            path = os.path.join(dirfn, fn)
            if acceptfile and os.path.exists(path):
                if onlyprint:
                    print(path)
                    continue
            else:
                try:
                    os.remove(path)
                except OSError:
                    pass
                if onlyprint:
                    print(path)
                    continue
                if mfn:
                    print("error accessing", path)
                    continue
                try:
                    fn = fn.replace("01234", "4")
                    fn = fn.replace("56789ABCDEF", "5")  # XXX
                    getfile(url, dirfn, fn)
                except URLError as e:
                    print("error accessing %s: %s" % (url, e), file=sys.stderr)
                    if match == "*":
                        continue
                    raise
            if link:
                lname = re.sub(r".*/", "", name)
                lname = sanitize(lname, allowed_chars)
                try:
                    os.remove(os.path.join(dirfn, lname))
                except OSError:
                    pass
                try:
                    os.symlink(fn, os.path.join(dirfn, lname))
                except OSError as e:
                    print("Cannot link %s to %s:" % (name, lname), e, file=sys.stderr)
            files.append(fn)
        models.close()
        for file_name in ["README.md", "LICENSE"]:
            if (
                not onlyprint
                and not os.path.exists(os.path.join(dirfn, file_name))
                and not mfn
            ):
                getfile(urlpath + "/" + file_name, dirfn, file_name)
    except URLError as e:
        print("Cannot access event server:", e, file=sys.stderr)
        warn_once(
            """
If you need a proxy to access the internet please set it with:
\texport https_proxy=http://proxyname...
If you are not connected to the internet please run this on a connected system:
\tevent_download.py '%s'
and then copy ~/.cache/pmu-events to the system under test
To get events for all possible CPUs use:
\tevent_download.py -a"""
            % match
        )
    except OSError as e:
        print("Cannot write events file:", e, file=sys.stderr)
    return files


def download(match, key=None, link=True, onlyprint=False, acceptfile=False):
    return len(parse_map_file(match, key, link, onlyprint, acceptfile))


def download_current(link=False, onlyprint=False):
    """Download JSON event list for current cpu.
    Returns >0 when a event list is found"""
    return download(
        get_cpustr(),
        link=link,
        onlyprint=onlyprint,
    )


def eventlist_name(name=None, key="core", hybridkey=None):
    if not name:
        name = get_cpustr()
    cache = getdir()
    fn = name
    if os.path.exists(fn):
        return fn
    if ".json" not in name:
        if hybridkey:
            fn = "%s-%s-%s.json" % (name, key, hybridkey)
        else:
            fn = "%s-%s.json" % (name, key)
    if "/" not in fn:
        fn = "%s/%s" % (cache, fn)
    if not os.path.exists(fn):
        files = parse_map_file(name, key, acceptfile=True, hybridkey=hybridkey)
        if files:
            return files[0]
        name = cpu_without_step(name)
        if "*" in fn:
            fn = "%s/%s" % (cache, name)
        elif hybridkey:
            fn = "%s/%s-%s-%s.json" % (cache, name, key, hybridkey)
        else:
            fn = "%s/%s-%s.json" % (cache, name, key)
        files = parse_map_file(name, key, acceptfile=True, hybridkey=hybridkey)
        if files:
            fn = files[0]
    return fn


def main():
    # only import argparse when actually called from command line
    # this makes ocperf work on older python versions without it.
    import argparse

    p = argparse.ArgumentParser(usage="download Intel event files")
    p.add_argument(
        "--all", "-a", help="Download all available event files", action="store_true"
    )
    p.add_argument("--verbose", "-v", help="Be verbose", action="store_true")
    p.add_argument("--mine", help="Print name of current CPU", action="store_true")
    p.add_argument(
        "--link",
        help="Create links with the original event file name",
        action="store_true",
        default=True,
    )
    p.add_argument(
        "--print",
        help="Print file names of all event files instead of downloading. Requires existing mapfile.csv.",
        dest="print_",
        action="store_true",
    )
    p.add_argument("cpus", help="CPU identifiers to download", nargs="*")
    args = p.parse_args()

    if args.verbose or args.mine:
        print(get_cpustr())
    if args.mine:
        sys.exit(0)
    d = getdir()
    if args.all:
        found = download("*", link=args.link, onlyprint=args.print_)
    elif len(args.cpus) == 0:
        found = download_current(link=args.link, onlyprint=args.print_)
    else:
        found = 0
        for j in args.cpus:
            found += download(j, link=args.link, onlyprint=args.print_)

    if found == 0 and not args.print_:
        print("Nothing found", file=sys.stderr)

    el = eventlist_name()
    if os.path.exists(el) and not args.print_:
        print("my event list", el)

    import shutil

    shutil.copy(el, ("./%s" % (pmu_events_filename,)), follow_symlinks=True)
    print("copied %s to ./%s" % (el, pmu_events_filename))
    shutil.copytree(d, "./pmu-events", symlinks=True)
    print("copied %s to ./pmu-events" % (d,))


if __name__ == "__main__":
    main()
