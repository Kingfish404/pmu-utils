from cpuinfo import CPUInfo, CPUVendor, get_linux_cpu_info

if __name__ == "__main__":
    cpu_info: CPUInfo = get_linux_cpu_info()

    if cpu_info.vendor_name == CPUVendor.INTEL.value:
        from pmu_intel_utils import get_doc_pmu_dict, export_c_header
    elif cpu_info.vendor_name == CPUVendor.AMD.value:
        from pmu_amd_utils import get_doc_pmu_dict, export_c_header
    elif cpu_info.vendor_name == CPUVendor.ARM.value:
        from pmu_arm_utils import get_doc_pmu_dict, export_c_header
    else:
        raise NotImplementedError(
            f"PMU initialization not implemented for CPU vendor: {cpu_info.vendor_name}"
        )
    pmu_dict = get_doc_pmu_dict()
    export_c_header(pmu_dict)

