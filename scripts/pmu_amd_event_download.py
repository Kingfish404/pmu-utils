import re
import os
import csv
import datetime
import json
import platform
from program_utils import getfile, get_cpustr, pmu_events_filename, run_command

urlpath = 'https://raw.githubusercontent.com/torvalds/linux/master/tools/perf/pmu-events/arch/x86/'
pmu_data_path = './pmu-events'
mapfile = 'mapfile.csv'

pmu_files ={
    "amdzen1":["branch.json", "cache.json", "core.json", "data-fabric.json", "floating-point.json", "memory.json", "other.json", "recommended.json"],
    "amdzen2":["branch.json", "cache.json", "core.json", "data-fabric.json", "floating-point.json", "memory.json", "other.json", "recommended.json"],
    "amdzen3":["branch.json", "cache.json", "core.json", "data-fabric.json", "floating-point.json", "memory.json", "other.json", "recommended.json"],
    "amdzen4":["branch.json", "cache.json", "core.json", "data-fabric.json", "floating-point.json", "memory.json", "other.json", "pipeline.json", "recommended.json"],
}

def main():
    if not os.path.exists(pmu_data_path):
        os.makedirs(pmu_data_path)
    # get mapfile.csv
    model_file = os.path.join(pmu_data_path, mapfile)
    if not os.path.exists(model_file):
        getfile(urlpath+mapfile, pmu_data_path, mapfile)
    
    # get cpu info
    cpustr = get_cpustr()
    print("Current cpu: "+cpustr)

    # get cpu microarchitecture
    model = None
    with open(model_file, 'r', newline='') as csvfile:
        spamreader = csv.reader(csvfile)
        for row in spamreader:
            output, error = run_command('echo "%s" | grep -Po \'^%s\'' % (cpustr, row[0],))
            error = error.strip()
            output = output.strip()
            if error:
                print("Error: " + error)
            elif output == cpustr:
                model = row
                break
    assert model is not None
    print("Current microarchitecture: "+" ".join(model[1:]))
    
    # get pmu files
    pmu_file = pmu_files[model[2]]
    print(pmu_file)
    for pfile in pmu_file:
        if not os.path.exists(os.path.join(pmu_data_path, pfile)):
            url = urlpath+model[2]+"/"+pfile
            getfile(url, pmu_data_path, pfile)
    
    # parse pmu files
    all_events_code = []
    all_events = []
    for pfile in pmu_file:
        data = None
        with open(os.path.join(pmu_data_path, pfile), 'r') as f:
            data = json.load(f)
        print("File: "+pfile)
        for event in data:
            ecode = event.get('EventCode', "")
            umask = event.get('UMask', "")
            tcode = umask+ecode
            if tcode in all_events_code:
                continue
            all_events_code.append(tcode)
            all_events.append(event)
    print("Total events: "+str(len(all_events)))
    with open(pmu_events_filename, 'w') as f:
        json.dump({
            "Header":{
                "CPU": cpustr,
                "DateCreated": datetime.datetime.now().strftime("%Y-%m-%d"),
                "Microarchitecture": " ".join(model[1:]),
            },
            "Events": all_events,
        }, f, indent=4)
    
if __name__ == "__main__":
    main()
