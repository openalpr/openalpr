import os
import shutil

BYTES_IN_A_MEGABYTE = 1048576

size_quota_mb=200000
size_quota_bytes = size_quota_mb * 1048576
dir='/var/www/html/plates/'


def get_size(start_path = '.'):
    total_size = 0
    for dirpath, dirnames, filenames in os.walk(start_path):
        for f in filenames:
            fp = os.path.join(dirpath, f)
            total_size += os.path.getsize(fp)
    return total_size





all_files = []

os.chdir(dir)

initial_dir_size = get_size()
print initial_dir_size

if (initial_dir_size <= size_quota_bytes):
    dir_size_mb = float(initial_dir_size) / float(BYTES_IN_A_MEGABYTE)
    print "Directory is within quota (" + str(dir_size_mb) + " / " + str(size_quota_mb) + " MB)"
    exit()

for files in os.listdir("."):
    #print files
    #print " -- " + str(os.stat(files))
    if os.path.isdir(files):
        filetuple = ( os.stat(files).st_mtime, get_size(files), files)
        all_files.append( filetuple )
    else:
        filetuple = ( os.stat(files).st_mtime, os.stat(files).st_size, files)
        all_files.append( filetuple )


print "UNSORTED"

for file in all_files:
    print file[0]

#print all_files

all_files.sort(key=lambda tup: tup[0])

print "SORTED"

for file in all_files:
    print file[2]

bytes_left_to_delete = initial_dir_size - size_quota_bytes

for fileinfo in all_files:
    if bytes_left_to_delete <= 0:
        break
    filename = fileinfo[2]
    filebytes = fileinfo[1]

    print "Deleting: " + filename + " (" + str(filebytes) + " bytes)"

    if (os.path.isdir(filename)):
        shutil.rmtree(filename, True)
    else:
        os.remove(filename)
    bytes_left_to_delete = bytes_left_to_delete - filebytes

