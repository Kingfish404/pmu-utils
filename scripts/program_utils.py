import os
from urllib.request import urlopen
import subprocess

NUM_TRIES = 3
pmu_events_filename = "this-cpu-pmu-events.json"

def get_cpustr():
    cpuinfo = os.getenv("CPUINFO")
    if cpuinfo is None:
        cpuinfo = '/proc/cpuinfo'
    f = open(cpuinfo, 'r')
    cpu = [None, None, None, None]
    for j in f:
        print(j, end='')
        n = j.split()
        if n[0] == 'vendor_id':
            cpu[0] = n[2]
        elif n[0] == 'model' and n[1] == ':':
            cpu[2] = int(n[2])
        elif n[0] == 'cpu' and n[1] == 'family':
            cpu[1] = int(n[3])
        elif n[0] == 'stepping' and n[1] == ':':
            cpu[3] = int(n[2])
        if all(v is not None for v in cpu):
            break
    # stepping for SKX only
    stepping = cpu[0] == "GenuineIntel" and cpu[1] == 6 and cpu[2] == 0x55
    if stepping:
        return "%s-%d-%X-%X" % tuple(cpu)
    return "%s-%d-%X" % tuple(cpu)[:3]

def getfile(url, dirfn, fn):
    tries = 0
    print("Downloading", url, "to", os.path.join(dirfn, fn))
    while True:
        try:
            f = urlopen(url)
            data = f.read()
        except IOError:
            tries += 1
            if tries >= NUM_TRIES:
                raise
            print("retrying download")
            continue
        break
    o = open(os.path.join(dirfn, fn), "wb")
    o.write(data)
    o.close()
    f.close()

def run_command(command:str):
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)

    output, error = process.communicate()

    output = output.decode('utf-8')
    error = error.decode('utf-8')
    return output, error
