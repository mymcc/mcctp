#pragma once

// Adapted from https://stackoverflow.com/a/64166

#include <windows.h>
#include <psapi.h>
#include <TCHAR.h>
#include <pdh.h>

namespace mcctp::utils {
	static std::optional<MEMORYSTATUSEX> get_global_memory_status_ex(void) {
		MEMORYSTATUSEX memInfo;
		memset(&memInfo, 0, sizeof(MEMORYSTATUSEX));
		memInfo.dwLength = sizeof(MEMORYSTATUSEX);

		if (GlobalMemoryStatusEx(&memInfo)) {
            return memInfo;
        } else {
            return std::nullopt;
		}
	}

	static DWORDLONG get_total_virtual_memory() {
        auto opt_mem_stat_ex = get_global_memory_status_ex();
		if (opt_mem_stat_ex.has_value()) {
			const auto& mem_stat = opt_mem_stat_ex.value();
			return mem_stat.ullTotalPageFile;
		}
		else {
			return 0;
		}
	}

	static DWORDLONG get_total_virtual_memory_used() {
		auto opt_mem_stat_ex = get_global_memory_status_ex();
		if (opt_mem_stat_ex.has_value()) {
			const auto& mem_stat = opt_mem_stat_ex.value();
			return mem_stat.ullTotalPageFile - mem_stat.ullAvailPageFile;
		}
		else {
			return 0;
		}
	}

	static DWORDLONG get_total_physical_memory(void) {
		auto opt_mem_stat_ex = get_global_memory_status_ex();
		if (opt_mem_stat_ex.has_value()) {
			const auto& mem_stat = opt_mem_stat_ex.value();
			return mem_stat.ullTotalPhys;
		}
		else {
			return 0;
		}
	}

	static DWORDLONG get_total_physical_memory_used(void) {
		auto opt_mem_stat_ex = get_global_memory_status_ex();
		if (opt_mem_stat_ex.has_value()) {
			const auto& mem_stat = opt_mem_stat_ex.value();
			return mem_stat.ullTotalPhys - mem_stat.ullAvailPhys;
		}
		else {
			return 0;
		}
	}

	static std::optional<PROCESS_MEMORY_COUNTERS_EX> get_process_memory_counters_ex(HANDLE handle) {
		PROCESS_MEMORY_COUNTERS_EX pmc;
		memset(&pmc, 0, sizeof(pmc));

		if (GetProcessMemoryInfo(handle, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
			return pmc;
		}
		else {
			return std::nullopt;
		}
	}

	static size_t get_current_process_virtual_memory(void) {
		HANDLE current_proc = GetCurrentProcess();
		auto opt_proc_mem = get_process_memory_counters_ex(current_proc);
		if (opt_proc_mem.has_value()) {
			const auto& proc_mem = opt_proc_mem.value();
			return proc_mem.PrivateUsage;
		}
		else {
			return 0;
		}
	}

	static size_t get_current_process_physical_memory(void) {
		HANDLE current_proc = GetCurrentProcess();
		auto opt_proc_mem = get_process_memory_counters_ex(current_proc);
		if (opt_proc_mem.has_value()) {
			const auto& proc_mem = opt_proc_mem.value();
			return proc_mem.WorkingSetSize;
		}
		else {
			return 0;
		}
	}

	static PDH_HQUERY cpuQuery;
	static PDH_HCOUNTER cpuTotal;
	static BOOL pdhInitialized = false;

	static void ensure_pdh(void) {
		if (!pdhInitialized) {
			memset(&cpuQuery, 0, sizeof(PDH_HQUERY));
			memset(&cpuTotal, 0, sizeof(PDH_HCOUNTER));

			if (PdhOpenQuery(NULL, NULL, &cpuQuery) != ERROR_SUCCESS) {
				return;
			}
			// You can also use L"\\Processor(*)\\% Processor Time" and get individual CPU values with PdhGetFormattedCounterArray()
			if (PdhAddEnglishCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal) != ERROR_SUCCESS) {
				return;
			}
			if (PdhCollectQueryData(cpuQuery) != ERROR_SUCCESS) {
				return;
			}

			pdhInitialized = true;
		}
	}

	static double get_total_cpu_used(void) {
		ensure_pdh();

		PDH_FMT_COUNTERVALUE counterVal;
		memset(&counterVal, 0, sizeof(PDH_FMT_COUNTERVALUE));

		if (PdhCollectQueryData(cpuQuery) != ERROR_SUCCESS) {
			return 0;
		}

		if (PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal) != ERROR_SUCCESS) {
			return 0;
		}

		return counterVal.doubleValue;
	}

	struct siInfo {
		ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
		int numProcessors;
		HANDLE toProcess;
	};

	static std::unordered_map<HANDLE, siInfo> s_siInfoMap;

	static void ensure_system_info(HANDLE handle) {
		auto it = s_siInfoMap.find(handle);
		if (it == s_siInfoMap.end()) {
			siInfo siInfo_;
			memset(&siInfo_, 0, sizeof(siInfo));
			siInfo_.toProcess = handle;

			SYSTEM_INFO sysInfo;
			memset(&sysInfo, 0, sizeof(SYSTEM_INFO));

			FILETIME ftime, fsys, fuser;
			memset(&ftime, 0, sizeof(FILETIME));
			memset(&fsys, 0, sizeof(FILETIME));
			memset(&fuser, 0, sizeof(FILETIME));

			GetSystemInfo(&sysInfo);
			siInfo_.numProcessors = sysInfo.dwNumberOfProcessors;

			GetSystemTimeAsFileTime(&ftime);
			memcpy(&siInfo_.lastCPU, &ftime, sizeof(FILETIME));

			GetProcessTimes(handle, &ftime, &ftime, &fsys, &fuser);
			memcpy(&siInfo_.lastSysCPU, &fsys, sizeof(FILETIME));
			memcpy(&siInfo_.lastUserCPU, &fuser, sizeof(FILETIME));

			s_siInfoMap.insert({ handle, siInfo_ });
		}
	}

	static double get_process_total_cpu_used(HANDLE handle) {
		ensure_system_info(handle);
		siInfo siInfo_ = s_siInfoMap.at(handle);

		FILETIME ftime, fsys, fuser;
		memset(&ftime, 0, sizeof(FILETIME));
		memset(&fsys, 0, sizeof(FILETIME));
		memset(&fuser, 0, sizeof(FILETIME));

		ULARGE_INTEGER now, sys, user;
		memset(&now, 0, sizeof(ULARGE_INTEGER));
		memset(&sys, 0, sizeof(ULARGE_INTEGER));
		memset(&user, 0, sizeof(ULARGE_INTEGER));

		double percent = 0.0;

		GetSystemTimeAsFileTime(&ftime);
		memcpy(&now, &ftime, sizeof(FILETIME));

		GetProcessTimes(siInfo_.toProcess, &ftime, &ftime, &fsys, &fuser);
		memcpy(&sys, &fsys, sizeof(FILETIME));
		memcpy(&user, &fuser, sizeof(FILETIME));

		percent = (sys.QuadPart - siInfo_.lastSysCPU.QuadPart) +
			(user.QuadPart - siInfo_.lastUserCPU.QuadPart);
		percent /= (now.QuadPart - siInfo_.lastCPU.QuadPart);
		percent /= siInfo_.numProcessors;
		siInfo_.lastCPU = now;
		siInfo_.lastUserCPU = user;
		siInfo_.lastSysCPU = sys;

		return percent * 100;
	}

	static double get_current_process_total_cpu_used(void) {
		HANDLE current_proc = GetCurrentProcess();
		return get_process_total_cpu_used(current_proc);
	}
}
