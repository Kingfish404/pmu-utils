import os
import json
from program_utils import pmu_events_filename, export_header, PMUEvent
from typing import List


def event_to_hexcode(event):
    event_select = (
        int(event["EventCode"], 16) if (event.get("EventCode", "") != "") else 0
    )
    unit_mask = int(event["UMask"], 16) if (event.get("UMask", "") != "") else 0
    user_mode = 1
    os_mode = 1
    edge_detect = 0
    interrupt_enable = 1
    counter_enable = 1
    invert = 0
    counter_mask = 0
    hg_only = 0

    hex_num = (unit_mask << 8) + (event_select & 0xFF)
    hex_num |= user_mode << 16
    hex_num |= os_mode << 17
    hex_num |= edge_detect << 18
    hex_num |= interrupt_enable << 20
    hex_num |= counter_enable << 22
    hex_num |= invert << 23
    hex_num |= (counter_mask & 0xFF) << 24
    hex_num |= (event_select & 0xF00) << 32
    hex_num |= (hg_only & 0x3) << 40
    return "0x%x" % hex_num


def get_doc_pmu_dict():
    """Get the PMU events from the JSON file"""
    if not os.path.exists(pmu_events_filename):
        from pmu_amd_event_download import main

        main()
    pmu_umask_event = {}
    with open(pmu_events_filename, "r+") as f:
        pmu_events = json.load(f)["Events"]
        for idx, event in enumerate(pmu_events):
            hexcode = event_to_hexcode(event)
            pmu_umask_event[hexcode] = event
    return pmu_umask_event


def export_c_header(pmu_dict):
    pmu_events: List[PMUEvent] = []
    for key in pmu_dict:
        pmu_event = pmu_dict[key]
        event_name = (
            pmu_event["EventName"]
            if pmu_event.get("EventName", "") != ""
            else pmu_event["MetricName"]
        )
        pmu_events.append(
            PMUEvent(key, event_name, pmu_event.get("BriefDescription", ""))
        )
    export_header(pmu_events, "amd64_pmu_event.h", "AMD64")


if __name__ == "__main__":
    pmu_dict = get_doc_pmu_dict()
    export_c_header(pmu_dict)
    for key in pmu_dict:
        print(key, pmu_dict[key])
