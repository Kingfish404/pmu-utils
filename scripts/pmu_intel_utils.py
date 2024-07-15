import os
import json
from program_utils import pmu_events_filename, export_header, PMUEvent
from typing import List


def event_to_hexs(event):
    hexs = []
    event_codes = event["EventCode"].split(",")
    for event_code in event_codes:
        hex_num = 0
        counter_mask = int(event["CounterMask"])
        invert = int(event["Invert"])
        enable_counters = 1
        pc = 0
        edge_detect = int(event["EdgeDetect"])
        operating_system_mode = 0
        user_mode = 1
        umask = int(event["UMask"], 16)
        hex_num = (
            (counter_mask << 24)
            + (invert << 23)
            + (enable_counters << 22)
            + (pc << 19)
            + (edge_detect << 18)
            + (operating_system_mode << 17)
            + (user_mode << 16)
            + (umask << 8)
            + int(event_code, 16)
        )
        hex_str = "0x%x" % hex_num
        hexs.append(hex_str)
    return hexs


def get_doc_pmu_dict():
    if not os.path.exists(pmu_events_filename):
        from pmu_intel_event_download import main

        main()
    pmu_umask_event = {}
    with open(pmu_events_filename, "r+") as f:
        pmu_events = json.load(f)["Events"]
        for idx, event in enumerate(pmu_events):
            hexs = event_to_hexs(event)
            for hex_code in hexs:
                pmu_umask_event[hex_code] = event
    return pmu_umask_event


def export_c_header(pmu_dict):
    pmu_events: List[PMUEvent] = []
    for key in pmu_dict:
        pmu_event = pmu_dict[key]
        event_name = pmu_event["EventName"]
        pmu_events.append(PMUEvent(key, event_name, pmu_event["PublicDescription"]))

    export_header(pmu_events, "ia64_pmu_event.h", "IA64")


if __name__ == "__main__":
    pmu_dict = get_doc_pmu_dict()
    export_c_header(pmu_dict)
