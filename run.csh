#!/bin/csh
if( $1 == "" ) then
	printf "Usage :  $0 <trace-file-directory>\n"
	exit 1
endif

if ( ! -e efectiu) then
	printf "program not built \n"
	exit 1
endif

set trace_list = `find $1 -name '*.trace.*' | sort`
set ipc = 0
set ipc_product = 1
foreach i ($trace_list)
	printf "%-40s\t" $i
	set ipc = `./efectiu $i | tail -1 | sed -e '/^.*0: /s///' | sed -e '/IPC.*$/s///'`
	printf "%0.3f \n" $ipc
	set ipc_product = `printf "$ipc\n$ipc_product\n*\np\n" | dc`
end
printf "Product of IPC: "
printf "%0.3f\n" $ipc_product
set lru_ipc = 36945250715.440
printf "net speedup "
echo $ipc_product $lru_ipc | awk '{print ($1/$2)^(1/27)}' 





