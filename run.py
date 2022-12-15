import os
import datetime
import time
import subprocess

deg_th_list_small_dataset = [1, 2, 5, 10, 20, 30]
deg_th_list_large_dataset = [1, 5, 10, 20, 50, 100]
# epoch = 200
# task_num = len(deg_th_list)
arch_list = ['hygcn', 'awb-gcn', 'i-gcn', 'regnn', 'delta-gnn', 'delta-gnn-opt']
# arch_list = ['hygcn']
dataset_list = ['cora', 'citeseer', 'ogbn-arxiv', 'ogbn-mag']
# dataset_list = ['cora']

ISOTIMEFORMAT = '%m%d_%H%M'
theTime = datetime.datetime.now().strftime(ISOTIMEFORMAT)
theTime = str(theTime)

Task_time_start = time.perf_counter()

# Predefined
log_path = './log/' + theTime + '_log.txt'

# Remove previous log
os.system('rm ' + log_path)


def time_format(sec):
    if sec > 3600:
        hour, tmp = divmod(sec, 3600)
        min, s = divmod(tmp, 60)
        time = str(int(hour)) + 'h' + str(int(min)) + 'm' + str(int(s)) + 's'
    elif sec > 60:
        min, s = divmod(sec, 60)
        time = str(int(min)) + 'm' + str(int(s)) + 's'
    else:
        s = round(sec, 2)
        time = str(s) + 's'

    return time


# Main scipt body
for dataset in dataset_list:
    for arch in arch_list:
        if dataset == 'cora':
            if arch == 'delta-gnn' or arch == 'delta-gnn-opt':
                for deg_th in deg_th_list_small_dataset:
                    time_subtask_start = time.perf_counter()
                    subprocess.Popen('./dfsim_cora --dataset ' + dataset + ' --arch ' + arch +
                                     ' --deg_th ' + str(deg_th),
                                     shell=True)
                    print('\n>> SubTask {:s} @ {:s} deg_th={:s} exe time: {:s}'.format(
                        arch, dataset, str(deg_th),
                        time_format(time.perf_counter() - time_subtask_start)))
            else:
                time_subtask_start = time.perf_counter()
                subprocess.Popen('./dfsim_cora --dataset ' + dataset + ' --arch ' + arch,
                                 shell=True)
                print('\n>> SubTask {:s} @ {:s} exe time: {:s}'.format(
                    arch, dataset, time_format(time.perf_counter() - time_subtask_start)))
        elif dataset == 'citeseer':
            if arch == 'delta-gnn' or arch == 'delta-gnn-opt':
                for deg_th in deg_th_list_small_dataset:
                    time_subtask_start = time.perf_counter()
                    subprocess.Popen('./dfsim_citeseer --dataset ' + dataset + ' --arch ' + arch +
                                     ' --deg_th ' + str(deg_th),
                                     shell=True)
                    print('\n>> SubTask {:s} @ {:s} deg_th={:s} exe time: {:s}'.format(
                        arch, dataset, str(deg_th),
                        time_format(time.perf_counter() - time_subtask_start)))
            else:
                time_subtask_start = time.perf_counter()
                subprocess.Popen('./dfsim_citeseer --dataset ' + dataset + ' --arch ' + arch,
                                 shell=True)
                print('\n>> SubTask {:s} @ {:s} exe time: {:s}'.format(
                    arch, dataset, time_format(time.perf_counter() - time_subtask_start)))
        elif dataset == 'ogbn-arxiv' or dataset == 'ogbn-mag':
            if arch == 'delta-gnn' or arch == 'delta-gnn-opt':
                for deg_th in deg_th_list_small_dataset:
                    time_subtask_start = time.perf_counter()
                    subprocess.Popen('./dfsim_ogbn --dataset ' + dataset + ' --arch ' + arch +
                                     ' --deg_th ' + str(deg_th),
                                     shell=True)
                    print('\n>> SubTask {:s} @ {:s} deg_th={:s} exe time: {:s}'.format(
                        arch, dataset, str(deg_th),
                        time_format(time.perf_counter() - time_subtask_start)))
            else:
                time_subtask_start = time.perf_counter()
                subprocess.Popen('./dfsim_ogbn --dataset ' + dataset + ' --arch ' + arch,
                                 shell=True)
                print('\n>> SubTask {:s} @ {:s} exe time: {:s}'.format(
                    arch, dataset, time_format(time.perf_counter() - time_subtask_start)))

print('\n>> ** All tasks have completed, total execution time: {:s}'.format(
    time_format(time.perf_counter() - Task_time_start)))
