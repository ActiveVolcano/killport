Kill process listening on the specified TCP port,
and all the processes established on that port in Linux.

Suitable for killing applications using multiprocess like socat.

Author:
* CHEN Qingcan, 2024 spring, Foshan Nanhai China.

Build:
```bash
cc -std=gnu11 -Wall -s -o killport killport.c
```
Prebuild binaries are there in releases page.
