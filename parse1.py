import os
import gzip 
result_list = []
file_list = os.listdir("my2/")
for filename in file_list:
    single_list = []
    one_dict = {"Policy": 'my2/'+filename}
    if filename=="run.sh":
        continue
    f = gzip.open('my2/'+filename, 'rb')
    lines = f.readlines()
    for line in lines:
        beginer = line.find("CPI:")
        if beginer != -1:
            beginer += 5
            ender = beginer
            while line[ender].isdigit() or line[ender] == '.':
                ender += 1
            cpi = line[beginer:ender]
            one_dict["CPI"] = cpi
            continue
        beginer = line.find("IFETCH    Miss Rate")
        if beginer != -1:
            beginer = line.find(":") + 3
            ender = beginer
            while line[ender].isdigit() or line[ender] == '.':
                ender += 1
            ifetch = line[beginer:ender]
            one_dict["IFETCH"] = ifetch
            continue
        beginer = line.find("LOAD      Miss Rate")
        if beginer != -1:
            beginer = line.find(":") + 3
            ender = beginer
            while line[ender].isdigit() or line[ender] == '.':
                ender += 1
            load = line[beginer:ender]
            one_dict["LOAD"] = load
            continue
        beginer = line.find("STORE     Miss Rate")
        if beginer != -1:
            beginer = line.find(":") + 3
            ender = beginer
            while line[ender].isdigit() or line[ender] == '.':
                ender += 1
            store = line[beginer:ender]
            one_dict["STORE"] = store
        beginer = line.find("WRITEBACK Miss Rate")
        if beginer != -1:
            beginer = line.find(":") + 3
            ender = beginer
            while line[ender].isdigit() or line[ender] == '.':
                ender += 1
            write = line[beginer:ender]
            one_dict["WRITEBACK"] = write
            continue
        beginer = line.find("Thread: 0 Lookups")
        if beginer != -1:
            beginer = line.rfind(":") + 2
            ender = beginer
            while line[ender].isdigit() or line[ender] == '.':
                ender += 1
            total = line[beginer:ender]
            one_dict["TOTAL"] = total
    single_list.append(one_dict)
    f.close()
    result_list.append(single_list)

wf = open("result_my8.txt", 'w')

wf.write("Filename                        CPI         IFETCH      LOAD        STORE       WRITE      TOTAL\n\n")
for case_list in result_list:
    wf.write(case_list[0]['Policy'].split('/')[1]   + '\n')
    for c in case_list:
        try:
            line = "%-30s  %-10s  %-10s  %-10s  %-10s  %-10s %-10s\n" % (
                c['Policy'].split('/')[0] ,
                c['CPI'],
                c['IFETCH'],
                c['LOAD'],
                c['STORE'],
                c['WRITEBACK'],
                c['TOTAL']
            )
            wf.write(line)
        except KeyError as e:
            print(c['Policy'])
    wf.write('\n')
wf.close()
print("Done!")