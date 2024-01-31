import os
import json
from program_utils import getfile, pmu_events_filename

urlpath = "https://raw.githubusercontent.com/torvalds/linux/master/tools/perf/pmu-events/arch/arm64/"
pmu_data_path = "."
pmufile = "common-and-microarch.json"


def event_to_hexcode(event):
    hex_num = int(event["EventCode"], 16) if (event.get("EventCode", "") != "") else 0
    return hex_num


def get_doc_pmu_dict():
    """Get the PMU events from the JSON file"""
    if not os.path.exists(pmu_events_filename):
        os.makedirs(pmu_data_path, exist_ok=True)
        getfile(urlpath + pmufile, pmu_data_path, pmu_events_filename)
    pmu_umask_event = {}
    with open(pmu_events_filename, "r+") as f:
        pmu_events = json.load(f)
        for idx, event in enumerate(pmu_events):
            hexcode = event_to_hexcode(event)
            pmu_umask_event[hexcode] = event
    return pmu_umask_event


def export_c_header():
    pmu_dict = get_doc_pmu_dict()
    header_path = "../header/"
    header_file_name = "arm_pmu_event.h"
    header_file_path = os.path.join(header_path, header_file_name)
    with open(header_file_path, "w+") as f:
        f.write("#ifndef ARM_PMU_EVENT_H\n")
        f.write("#define ARM_PMU_EVENT_H\n\n")

        f.write("#include <stdint.h>\n\n")

        for key in pmu_dict:
            pmu_event = pmu_dict[key]
            print(hex(key), pmu_dict[key])
            f.write("// {}\n".format(pmu_event["PublicDescription"]))
            f.write(
                "#define ARM_PME_{} {}\n\n".format(
                    pmu_event["EventName"].upper(), hex(key)
                )
            )

        f.write(
            """
typedef struct
{
    char *event_name;
    uint64_t event_code;
} arm_pmu_event;

const arm_pmu_event pmu_events[] = {\n"""
        )
        for key in pmu_dict:
            pmu_event = pmu_dict[key]
            print(hex(key), pmu_dict[key])
            f.write(
                '    {{"{}", {}}},\n'.format(pmu_event["EventName"].upper(), hex(key))
            )
        f.write("};\n")

        f.write("\n#endif /* ARM_PMU_EVENT_H */\n")


if __name__ == "__main__":
    pmu_dict = get_doc_pmu_dict()
    export_c_header()
    for key in pmu_dict:
        print(hex(key), pmu_dict[key])
        break
