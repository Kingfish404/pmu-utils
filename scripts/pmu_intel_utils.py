import os
import json
from program_utils import pmu_events_filename


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


def export_c_header():
    pmu_dict = get_doc_pmu_dict()
    header_path = "../header/"
    header_file_name = "ia64_pmu_event.h"
    header_file_path = os.path.join(header_path, header_file_name)
    with open(header_file_path, "w+") as f:
        f.write("#ifndef IA64_PMU_EVENT_H\n")
        f.write("#define IA64_PMU_EVENT_H\n\n")

        f.write("#include <stdint.h>\n\n")

        for key in pmu_dict:
            pmu_event = pmu_dict[key]
            print(key, pmu_dict[key])
            f.write("// {}\n".format(pmu_event["PublicDescription"]))
            f.write(
                "#define IA64_PME_{} {}\n\n".format(pmu_event["EventName"].upper(), key)
            )

        f.write(
            """
typedef struct
{
    char *event_name;
    uint64_t event_code;
} IA64_pmu_event;

const IA64_pmu_event pmu_events[] = {\n"""
        )
        for key in pmu_dict:
            pmu_event = pmu_dict[key]
            print(key, pmu_dict[key])
            f.write('    {{"{}", {}}},\n'.format(pmu_event["EventName"].upper(), key))
        f.write("};\n")

        f.write("\n#endif /* IA64_PMU_EVENT_H */\n")


if __name__ == "__main__":
    pmu_dict = get_doc_pmu_dict()
    export_c_header()
    for key in pmu_dict:
        print(key, pmu_dict[key])
