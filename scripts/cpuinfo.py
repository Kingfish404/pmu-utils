import sys
import platform
from pathlib import Path
from typing import NamedTuple, List
from enum import Enum

# Keys that identify the CPU manufacturer (Vendor ID) across different architectures
VENDOR_KEYS: List[str] = ["vendor_id", "CPU implementer", "mvendorid"]

# Keys that provide a general model identifier (Model ID/Number)
MODEL_KEYS: List[str] = ["Processor", "cpu", "model", "CPU architecture", "marchid"]

# Keys that provide the human-readable model name
MODEL_NAME_KEYS: List[str] = ["model name", "CPU part", "Hardware", "mimpid"]


class CPUVendor(Enum):
    INTEL = "Intel"
    AMD = "AMD"

    ARM = "ARM"
    APPLE = "Apple"

    SIFIVE = "SiFive"
    SPACEMIT = "Spacemit"
    UNKNOWN = "Unknown"


class CPUInfo(NamedTuple):
    """A structured container for parsed CPU details."""

    architecture: str
    vendor_id: str
    vendor_name: str
    model_raw: str
    model_name_human: str


def map_cpu_vendor_id(vendor_id: str, arch: str) -> str:
    """
    Maps raw vendor IDs (e.g., AuthenticAMD, 0x41) to a human-readable vendor name.

    Args:
        vendor_id (str): The raw vendor ID string.
        arch (str): The system architecture (e.g., x86_64, aarch64).

    Returns:
        str: The interpreted vendor name.
    """
    # 1. x86 (Intel/AMD)
    if arch.startswith("x86") or arch.startswith("i386"):
        return {
            "GenuineIntel": CPUVendor.INTEL.value,
            "AuthenticAMD": CPUVendor.AMD.value,
        }.get(vendor_id, vendor_id)

    # 2. ARM / AArch64 (Implementer IDs are typically hex)
    elif arch.startswith("arm") or arch.startswith("aarch"):
        # Common ARM Implementer IDs
        return {
            "0x41": CPUVendor.ARM.value,
            "0x61": CPUVendor.APPLE.value,
        }.get(vendor_id, f"Implementer ID: {vendor_id}")

    # 3. RISC-V (mvendorid is typically hex)
    elif arch.startswith("riscv"):
        # Common RISC-V mvendorid values
        return {
            "0x489": CPUVendor.SIFIVE.value,
            "0x710": CPUVendor.SPACEMIT.value,
        }.get(vendor_id, f"mvendorid: {vendor_id}")

    # Default return for unrecognized or N/A IDs
    return vendor_id


# --- Main Detection Function ---


def get_linux_cpu_info(cpuinfo_path: str = "/proc/cpuinfo") -> CPUInfo:
    """
    Detects detailed CPU information on Linux systems by parsing the /proc/cpuinfo file.

    Args:
        cpuinfo_path (str): Path to the cpuinfo file. Default is '/proc/cpuinfo'.

    Returns:
        CpuInfo: A NamedTuple containing structured CPU information.
    """

    # Initialize raw data holders
    raw_info = {
        "vendor_id": "N/A",
        "model": "N/A",
        "model_name": "N/A",
    }

    # Get architecture early
    arch = platform.machine().lower()

    # --- 1. Environment and File Check ---
    if sys.platform != "linux":
        print(
            "Error: only Linux platform is supported for CPU detection.",
            file=sys.stderr,
        )
        return CPUInfo(arch, "N/A", "N/A", "N/A", "N/A")

    cpuinfo_file = Path(cpuinfo_path)
    if not cpuinfo_file.exists():
        print(f"Warning: CPU info file {cpuinfo_path} not found.", file=sys.stderr)
        return CPUInfo(arch, "N/A", "N/A", "N/A", "N/A")

    # --- 2. Parse /proc/cpuinfo ---
    try:
        with open(cpuinfo_file, "r") as f:
            for line in f:
                line = line.strip()
                if not line:
                    continue

                # Stop parsing after the first processor block (processor 0)
                if line.startswith("processor\t: 1"):
                    break

                if ":" in line:
                    key, value = line.split(":", 1)
                    key = key.strip()
                    value = value.strip()

                    # Find and assign the first match for each key type
                    if key in VENDOR_KEYS and raw_info["vendor_id"] == "N/A":
                        raw_info["vendor_id"] = value
                    elif key in MODEL_KEYS and raw_info["model"] == "N/A":
                        raw_info["model"] = value
                    elif key in MODEL_NAME_KEYS and raw_info["model_name"] == "N/A":
                        raw_info["model_name"] = value

    except IOError as e:
        print(f"Error reading or parsing {cpuinfo_path}: {e}", file=sys.stderr)
        # Mark error for clarity
        raw_info["vendor_id"] = "Error"
        raw_info["model_name"] = "Error"

    # --- 3. Interpretation and Final Output ---

    interpreted_vendor = map_cpu_vendor_id(raw_info["vendor_id"], arch)

    return CPUInfo(
        architecture=arch,
        vendor_id=raw_info["vendor_id"],
        vendor_name=interpreted_vendor,
        model_raw=raw_info["model"],
        model_name_human=raw_info["model_name"],
    )


# --- Example Usage ---

if __name__ == "__main__":
    cpu_info: CPUInfo = get_linux_cpu_info()

    print("--- CPU Information (Linux Detection) ---")
    print(f"System Architecture: {cpu_info.architecture}")
    print(f"        Vendor Name: {cpu_info.vendor_name}")
    print(f" Model Name (Human): {cpu_info.model_name_human}")
    print(f"      Raw Vendor ID: {cpu_info.vendor_id}")
    print(f"       Raw Model ID: {cpu_info.model_raw}")
