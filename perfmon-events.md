# Perfmon Events

## SkyLake

MEM_LOAD_RETIRED.FB_HIT	Counts retired load instructions with at least one uop was load missed in L1 but hit FB (Fill Buffers) due to preceding miss to the same cache line with data not ready.	EventSel=D1H UMask=40H
Counter=0,1,2,3 CounterHTOff=0,1,2,3
PEBS:[PreciseEventingIP]

MEM_LOAD_RETIRED.FB_HIT_PS	Counts retired load instructions with at least one uop was load missed in L1 but hit FB (Fill Buffers) due to preceding miss to the same cache line with data not ready.	EventSel=D1H UMask=40H
Counter=0,1,2,3 CounterHTOff=0,1,2,3
PEBS:[PreciseEventingIP, DataLinearAddress]
MEM_LOAD_RETIRED.L1_HIT	Counts retired load instructions with at least one uop that hit in the L1 data cache. This event includes all SW prefetches and lock instructions regardless of the data source.	EventSel=D1H UMask=01H
Counter=0,1,2,3 CounterHTOff=0,1,2,3

PEBS:[PreciseEventingIP]
MEM_LOAD_RETIRED.L1_HIT_PS	Counts retired load instructions with at least one uop that hit in the L1 data cache. This event includes all SW prefetches and lock instructions regardless of the data source.	EventSel=D1H UMask=01H
Counter=0,1,2,3 CounterHTOff=0,1,2,3
PEBS:[PreciseEventingIP, DataLinearAddress]
MEM_LOAD_RETIRED.L1_MISS	Counts retired load instructions with at least one uop that missed in the L1 cache.	EventSel=D1H UMask=08H
Counter=0,1,2,3 CounterHTOff=0,1,2,3

PEBS:[PreciseEventingIP]
MEM_LOAD_RETIRED.L1_MISS_PS	Counts retired load instructions with at least one uop that missed in the L1 cache.	EventSel=D1H UMask=08H
Counter=0,1,2,3 CounterHTOff=0,1,2,3
PEBS:[PreciseEventingIP, DataLinearAddress]
MEM_LOAD_RETIRED.L2_HIT	Retired load instructions with L2 cache hits as data sources.	EventSel=D1H UMask=02H
Counter=0,1,2,3 CounterHTOff=0,1,2,3

PEBS:[PreciseEventingIP]
MEM_LOAD_RETIRED.L2_HIT_PS	Retired load instructions with L2 cache hits as data sources.	EventSel=D1H UMask=02H
Counter=0,1,2,3 CounterHTOff=0,1,2,3
PEBS:[PreciseEventingIP, DataLinearAddress]
MEM_LOAD_RETIRED.L2_MISS	Retired load instructions missed L2 cache as data sources.	EventSel=D1H UMask=10H
Counter=0,1,2,3 CounterHTOff=0,1,2,3

PEBS:[PreciseEventingIP]
MEM_LOAD_RETIRED.L2_MISS_PS	Retired load instructions missed L2 cache as data sources.	EventSel=D1H UMask=10H
Counter=0,1,2,3 CounterHTOff=0,1,2,3
PEBS:[PreciseEventingIP, DataLinearAddress]

MEM_LOAD_RETIRED.L3_HIT	Counts retired load instructions with at least one uop that hit in the L3 cache.	EventSel=D1H UMask=04H
Counter=0,1,2,3 CounterHTOff=0,1,2,3
PEBS:[PreciseEventingIP]

MEM_LOAD_RETIRED.L3_HIT_PS	Retired load instructions with L3 cache hits as data sources.	EventSel=D1H UMask=04H
Counter=0,1,2,3 CounterHTOff=0,1,2,3
PEBS:[PreciseEventingIP, DataLinearAddress]

MEM_LOAD_RETIRED.L3_MISS	Counts retired load instructions with at least one uop that missed in the L3 cache.	EventSel=D1H UMask=20H
Counter=0,1,2,3 CounterHTOff=0,1,2,3
PEBS:[PreciseEventingIP]

MEM_LOAD_RETIRED.L3_MISS_PS	Retired load instructions missed L3 cache as data sources.	EventSel=D1H UMask=20H
Counter=0,1,2,3 CounterHTOff=0,1,2,3
PEBS:[PreciseEventingIP, DataLinearAddress]
