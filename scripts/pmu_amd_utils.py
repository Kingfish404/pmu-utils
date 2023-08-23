import os
import json
from program_utils import pmu_events_filename

def event_to_hexcode(event):
    event_select = int(event["EventCode"], 16) if (event.get("EventCode", "") != "") else 0
    unit_mask = int(event["UMask"], 16) if (event.get('UMask', "") != "") else 0
    hex_num = (unit_mask << 8) + (event_select & 0xFF)
    hex_num += (1 << 20) # Interrupt enable
    hex_num += (1 << 22) # Counter Enable
    hex_num += (event_select & 0xF00) << 32
    return "0x%x" % hex_num

def get_doc_pmu_dict():
    """Get the PMU events from the JSON file"""
    if not os.path.exists(pmu_events_filename):
        from pmu_amd_event_download import main
        main()
    pmu_umask_event = {}
    with open(pmu_events_filename,'r+') as f:
        pmu_events = json.load(f)["Events"]
        for idx, event in enumerate(pmu_events):
            hexcode = event_to_hexcode(event)
            pmu_umask_event[hexcode] = event
    return pmu_umask_event

if __name__ == "__main__":
    pmu_dict = get_doc_pmu_dict()
    for key in pmu_dict:
        print(key, pmu_dict[key])
